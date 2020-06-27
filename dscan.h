#ifndef DSCANH
#define DSCANH

#define DS_PROG_NAME "dscan"
#define DS_NUMBER_OF_THREADS 3

#define DS_INSERT_TRACK 0
#define DS_INVALID_AUTHOR 1
#define DS_MISPLACED_DIRECTORY 2
#define DS_MISPLACED_MP3 3
#define DS_INVALID_ADAPTOR 4
#define DS_INCONSISTENT_ALBUM_NAMES 5

#ifndef DS_NO_CELLO

struct SHARED_DATA {
    int user_thread_id;
    var author_array;
    var a_id_array;
    var adaptor_array;
    var ad_id_array;
    var album_name;
    var base_album_name;
    int64_t album_id;
};

#endif /* ifndef DS_SCREEN */

void open_screen();
void scr_add_author(const char*, int);
void scr_add_adaptor(const char*, int);
void scr_add_album(const char*, int);
void scr_add_track(const char*, int);
void close_screen();
void dbconnect(int);
void truncate();
void disconnect(int);
int64_t get_author_id(int, char*);
int64_t get_adaptor_id(int, char*);
int64_t get_album_id(int, char*, char*);
void insert_author_album(int, int64_t, int64_t);
void insert_adaptor_album(int, int64_t, int64_t);

#endif /* DSCANH */
