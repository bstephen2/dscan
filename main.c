#include "Cello.h"
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include "dscan.h"

var screen_mutex;
var tls_mutex;
var db_author_mutex;
var db_adaptor_mutex;
var db_album_mutex;
var log_mutex;
FILE* ds_log;

static var t1;
static var t2;
static var t3;
static var batch0;
static var batch1;
static var batch2;

static void exitfunc();
static clock_t prog_start, prog_end;
static double run_time;
static bool screen = false;

void Shared_Del(var self)
{
    struct SHARED_DATA* ptr = (struct SHARED_DATA*) self;
    del_raw(ptr->author_array);
    del_raw(ptr->a_id_array);
    del_raw(ptr->adaptor_array);
    del_raw(ptr->ad_id_array);
    del_raw(ptr->album_name);
    del_raw(ptr->base_album_name);

    return;
}

var do_base_author(var);
var url;
var SHARED_DATA = Cello(SHARED_DATA, Instance(New, NULL, Shared_Del));
var shared_table;

int main(int argc, char* argv[])
{
    struct stat buf;
    int rc = 0;
    DIR* basedir;
    struct dirent* pdir;
    var ref0;
    var ref1;
    var ref2;

    prog_start = clock();
    rc = atexit(exitfunc);
    assert(rc == 0);

    assert(argc == 2);
    assert(argv[1] != NULL);
    rc = stat(argv[1], &buf);
    assert(rc == 0);
    rc = chdir(argv[1]);
    assert(rc == 0);

    url = new_raw(String, $S(argv[1]));
    append(url, $S("\\"));

    ds_log = popen("perl -X c:\\bin\\dscan.pl", "w");
    assert(ds_log != NULL);

    shared_table = new_raw(Table, Int, SHARED_DATA);

    screen_mutex = new_raw(Mutex);
    tls_mutex = new_raw(Mutex);
    db_author_mutex = new_raw(Mutex);
    db_adaptor_mutex = new_raw(Mutex);
    db_album_mutex = new_raw(Mutex);
    log_mutex = new_raw(Mutex);
    batch0 = new_raw(List, String);
    ref0 = $(Ref, batch0);
    batch1 = new_raw(List, String);
    ref1 = $(Ref, batch1);
    batch2 = new_raw(List, String);
    ref2 = $(Ref, batch2);

    basedir = opendir(argv[1]);
    assert(basedir !=  NULL);

COLLECT_AUTHORS: {
        int i = 0;
        int r;

        while ((pdir = readdir(basedir)) !=  NULL) {
            if ((strcmp(pdir->d_name, ".") != 0) && (strcmp(pdir->d_name, "..") != 0)) {
                r = i % 3;

                switch (r) {
                case 0:
                    push(batch0, $S(pdir->d_name));
                    break;

                case 1:
                    push(batch1, $S(pdir->d_name));
                    break;

                case 2:
                    push(batch2, $S(pdir->d_name));
                    break;

                default:
                    break;
                }

                i++;
            }
        }
    }

    closedir(basedir);
    truncate();

    open_screen();
    screen = true;

    t1 = new_raw(Thread, $(Function, do_base_author));
    t2 = new_raw(Thread, $(Function, do_base_author));
    t3 = new_raw(Thread, $(Function, do_base_author));

    call(t1, $I(0), ref0);
    call(t2, $I(1), ref1);
    call(t3, $I(2), ref2);

    join(t1);
    join(t2);
    join(t3);

    return rc;
}

void exitfunc()
{
    if (screen == true) {
        close_screen();
    }

    del_raw(url);
    del_raw(t1);
    del_raw(t2);
    del_raw(t3);
    del_raw(batch0);
    del_raw(batch1);
    del_raw(batch2);
    del_raw(screen_mutex);
    del_raw(tls_mutex);
    del_raw(log_mutex);
    del_raw(db_author_mutex);
    del_raw(db_adaptor_mutex);
    del_raw(db_album_mutex);
    del_raw(shared_table);

    fprintf(ds_log, "\n%s ended\n", DS_PROG_NAME);
    prog_end = clock();
    run_time = (double)(prog_end - prog_start) / CLOCKS_PER_SEC;
    run_time = run_time / 60.0;
    fprintf(ds_log, "'C' Running Time = %2.8f minutes\n", run_time);
    pclose(ds_log);

    return;
}
