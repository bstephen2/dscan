#include "Cello.h"
#include <sys/stat.h>
#include <ftw.h>
#include <assert.h>
#include "dscan.h"

extern var url;
extern var SHARED_DATA;
extern var shared_table;
extern var screen_mutex;
extern var tls_mutex;
extern var db_author_mutex;
extern var db_adaptor_mutex;
extern var db_album_mutex;
extern var log_mutex;
extern FILE* ds_log;

static int process_entry(const char*, const struct stat*, int, struct FTW*);
static void do_dir(int level, const char*, const char*);
static void do_adaptor(char*, char*);
static void do_album(char*, char*);
static void do_base_album(char*, char*);
static void do_author(var, var);
static void do_file(int level, const struct stat*, char*, char*);

var do_base_author(var args)
{
    var thid = get(args, $I(0));
    var refa = get(args, $I(1));

    var authors = deref(refa);
    int tid = (int) c_int(thid);
    var thno = $I(c_int(current(Thread)));

SET_UP_THREAD_DATA: {
        var sd = $(SHARED_DATA);
        struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
        ptr->user_thread_id = tid;
        ptr->author_array = new_raw(Array, String);
        ptr->a_id_array = new_raw(Array, Int);
        ptr->adaptor_array = new_raw(Array, String);
        ptr->ad_id_array = new_raw(Array, Int);
        ptr->album_id = 0;
        ptr->album_name = new_raw(String);
        ptr->base_album_name = new_raw(String);
        lock(tls_mutex);
        set(shared_table, thno, sd);
        unlock(tls_mutex);
    }

    dbconnect(tid);

    while (len(authors) > 0) {
        var author = get(authors, $I(0));
        var base_path = new_raw(String, url);
        concat(base_path, author);

        do_author(base_path, author);

        lock(screen_mutex);
        scr_add_author(c_str(author), tid);
        unlock(screen_mutex);

        int rc = nftw(c_str(base_path), process_entry, 6, 0);
        assert(rc == 0);

        pop_at(authors, $I(0));
        del_raw(base_path);
    }

    disconnect(tid);

    return NULL;
}

int process_entry(const char* fname, const struct stat* ftw_buf, int flag, struct FTW* ftwb)
{
    if (flag == FTW_F) {
        int len = strlen(fname);

        char* ptr = (char*) fname + len - 4;

        if (strcmp(ptr, ".mp3") == 0) {
            char* bptr = (char*) fname + ftwb->base;
            do_file(ftwb->level, ftw_buf, (char*) fname, bptr);
        }
    } else if (flag == FTW_D) {
        const char* bptr = fname + ftwb->base;
        do_dir(ftwb->level, fname, bptr);
    }

    return 0;
}

void do_author(var pname, var author)
{
    var thno = $I(c_int(current(Thread)));
    var sd = get(shared_table, thno);
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
    int tid = ptr->user_thread_id;
    char* bname = c_str(author);

    if ((strchr(bname, '&') != NULL) || (strstr(bname, " and ") != NULL)) {
		char* lptr = c_str(pname);
		lptr += len(url);
        lock(log_mutex);
        fprintf(ds_log, "%d:%s\n", DS_INVALID_AUTHOR, lptr);
        unlock(log_mutex);
    }

PARSE_AUTHORS: {
        resize(ptr->author_array, 0);
        resize(ptr->a_id_array, 0);
        var copy_bname = new_raw(String, $S(bname));

        char* bptr = strtok(c_str(copy_bname), ";");
        push(ptr->author_array, $S(bptr));

        while ((bptr = strtok(NULL, ";")) != NULL) {
            size_t a = strspn(bptr, " ");
            bptr += a;
            push(ptr->author_array, $S(bptr));
        }

        del_raw(copy_bname);
    }

RECORD_AUTHORS: {
        foreach (au in ptr->author_array) {
            lock(db_author_mutex);
            push(ptr->a_id_array, $I(get_author_id(tid, c_str(au))));
            unlock(db_author_mutex);
        }
    }
    return;
}

void do_dir(int level, const char* pname, const char* bname)
{
    switch (level) {
    case 0:
        break;

    case 1:
        do_adaptor((char*) pname, (char*) bname);
        break;

    case 2:
        do_base_album((char*) pname, (char*) bname);
        break;

    case 3:
        do_album((char*) pname, (char*) bname);
        break;

    default:
       lock(log_mutex);
        fprintf(ds_log, "%d:%s\n", DS_MISPLACED_DIRECTORY, pname + len(url));
        unlock(log_mutex);
        break;
    }

    return;
}

void do_file(int level, const struct stat* buf, char* pname, char* bname)
{
    var thno = $I(c_int(current(Thread)));
    var sd = get(shared_table, thno);
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
    int tid = ptr->user_thread_id;

    if (level == 4) {
        lock(log_mutex);
        fprintf(ds_log, "%d:%d:%d:%s\n", DS_INSERT_TRACK, (int) len(url), (int) ptr->album_id, pname);
        unlock(log_mutex);
        //lock(screen_mutex);
        //scr_add_track(bname, tid);
        //unlock(screen_mutex);
    } else {
        lock(log_mutex);
        fprintf(ds_log, "%d:%s\n", DS_MISPLACED_MP3, pname + len(url));
        unlock(log_mutex);
    }

    return;
}

void do_adaptor(char* pname, char* bname)
{
    var thno = $I(c_int(current(Thread)));
    var sd = get(shared_table, thno);
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
    int tid = ptr->user_thread_id;

    if ((strchr(bname, '&') != NULL) || (strstr(bname, " and ") != NULL)) {
        lock(log_mutex);
        fprintf(ds_log, "%d:%s\n", DS_INVALID_ADAPTOR, pname + len(url));
        unlock(log_mutex);
    }

PARSE_ADAPTORS: {
        resize(ptr->adaptor_array, 0);
        resize(ptr->ad_id_array, 0);
        var copy_bname = new_raw(String, $S(bname));

        char* bptr = strtok(c_str(copy_bname), ";");
        push(ptr->adaptor_array, $S(bptr));

        while ((bptr = strtok(NULL, ";")) != NULL) {
            size_t a = strspn(bptr, " ");
            bptr += a;
            push(ptr->adaptor_array, $S(bptr));
        }

        del_raw(copy_bname);
    }

RECORD_ADAPTORS: {
        foreach (ad in ptr->adaptor_array) {
            lock(db_adaptor_mutex);
            push(ptr->ad_id_array, $I(get_adaptor_id(tid, c_str(ad))));
            unlock(db_adaptor_mutex);
        }
    }

    lock(screen_mutex);
    scr_add_adaptor(bname, tid);
    unlock(screen_mutex);

    return;
}

void do_base_album(char* pname, char* bname)
{
    var thno = $I(c_int(current(Thread)));
    var sd = get(shared_table, thno);
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
    resize(ptr->base_album_name, 0);
    append(ptr->base_album_name, $S(bname));

    return;
}

void do_album(char* pname, char* bname)
{
    int64_t rc;
    var thno = $I(c_int(current(Thread)));
    var sd = get(shared_table, thno);
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) sd;
    int tid = ptr->user_thread_id;
    resize(ptr->album_name, 0);
    append(ptr->album_name, $S(bname));

    size_t lenba = len(ptr->base_album_name);

    if (strncmp(c_str(ptr->base_album_name), bname, lenba) != 0) {
        lock(log_mutex);
        fprintf(ds_log, "%d:%s\n", DS_INCONSISTENT_ALBUM_NAMES, pname + len(url));
        unlock(log_mutex);
    }

    lock(db_adaptor_mutex);
    rc = get_album_id(tid, bname, pname);
    unlock(db_adaptor_mutex);
    ptr->album_id = rc;

    if (rc != 0) {
UPDATE_AUTHOR_ALBUM: {
            foreach (aid in ptr->a_id_array) {
                insert_author_album(tid, c_int(aid), rc);
            }
        }

UPDATE_ADAPTOR_ALBUM: {
            foreach (adid in ptr->ad_id_array) {
                insert_adaptor_album(tid, c_int(adid), rc);
            }
        }
    }

    lock(screen_mutex);
    scr_add_album(bname, tid);
    unlock(screen_mutex);

    return;
}


