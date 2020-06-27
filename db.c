#define DS_NO_CELLO
#include <stdint.h>
#include <mysql.h>
#include <assert.h>
#include "utstring.h"
#include "dscan.h"

static char* tables[6] = { "adaptor",
                           "adaptoralbum",
                           "album",
                           "author",
                           "authoralbum",
                           "track"
                         };

MYSQL* connection[DS_NUMBER_OF_THREADS];
static MYSQL mysql[DS_NUMBER_OF_THREADS];

static char* insert_author_sql = "INSERT INTO author SET name = \"%s\"";
static char* get_author_sql = "SELECT aid, name FROM author WHERE name = \"%s\"";
static char* insert_adaptor_sql = "INSERT INTO adaptor SET name = \"%s\"";
static char* get_adaptor_sql = "SELECT adid, name FROM adaptor WHERE name = \"%s\"";
static char* get_album_sql = "SELECT alid, name FROM album WHERE name = \"%s\"";
static char* insert_album_sql = "INSERT INTO album SET name = \"%s\"";
static char* insert_author_album_sql = "INSERT INTO authoralbum VALUES (%d, %d)";
static char* insert_adaptor_album_sql = "INSERT INTO adaptoralbum VALUES (%d, %d)";

void dbconnect(int thid)
{
    mysql_init(&mysql[thid]);
    connection[thid] = mysql_real_connect(&mysql[thid], "localhost", "bstephen", "rice37", "drama", 0,
                                          0, 0);

    if (connection == NULL) {
        printf(mysql_error(&mysql[thid]));
        exit(1);
    }

    return;
}

void truncate()
{
    int i;
    int state;
    UT_string command;
    utstring_init(&command);

    dbconnect(0);

    for (i = 0; i < 6; i++) {
        utstring_printf(&command, "TRUNCATE TABLE %s", tables[i]);
        state = mysql_query(connection[0], utstring_body(&command));
        assert(state == 0);
        utstring_clear(&command);
    }

    utstring_done(&command);

    disconnect(0);
}

int64_t get_author_id(int tid, char* name)
{
    MYSQL_RES* result;
    MYSQL_ROW row;
    UT_string command;
    int state;
    int64_t rc;

    utstring_init(&command);
    utstring_printf(&command, get_author_sql, name);
    state = mysql_query(connection[tid], utstring_body(&command));
    //assert(state == 0);

    if (state != 0) {
        printf(mysql_error(connection[tid]));
        exit(1);
    }

    result = mysql_store_result(connection[tid]);
    row = mysql_fetch_row(result);

    if (row == NULL) {
        mysql_free_result(result);
        utstring_clear(&command);
        utstring_printf(&command, insert_author_sql, name);
        state = mysql_query(connection[tid], utstring_body(&command));
        //assert(state == 0);

        if (state != 0) {
            printf(mysql_error(connection[tid]));
            exit(1);
        }

        rc = mysql_insert_id(connection[tid]);
    } else {
        rc = atoll(row[0]);
        mysql_free_result(result);
    }

    utstring_done(&command);

    return rc;
}

int64_t get_adaptor_id(int tid, char* name)
{
    MYSQL_RES* result;
    MYSQL_ROW row;
    UT_string command;
    int state;
    int64_t rc;

    utstring_init(&command);
    utstring_printf(&command, get_adaptor_sql, name);
    state = mysql_query(connection[tid], utstring_body(&command));

    if (state != 0) {
        printf(mysql_error(connection[tid]));
        exit(1);
    }

    result = mysql_store_result(connection[tid]);
    row = mysql_fetch_row(result);

    if (row == NULL) {
        mysql_free_result(result);
        utstring_clear(&command);
        utstring_printf(&command, insert_adaptor_sql, name);
        state = mysql_query(connection[tid], utstring_body(&command));

        if (state != 0) {
            printf(mysql_error(connection[tid]));
            exit(1);
        }

        rc = mysql_insert_id(connection[tid]);
    } else {
        rc = atoll(row[0]);
        mysql_free_result(result);
    }

    utstring_done(&command);

    return rc;
}

int64_t get_album_id(int tid, char* bname, char* pname)
{
    MYSQL_RES* result;
    MYSQL_ROW row;
    UT_string command;
    int state;
    int64_t rc;

    utstring_init(&command);
    utstring_printf(&command, get_album_sql, bname);
    state = mysql_query(connection[tid], utstring_body(&command));

    if (state != 0) {
        printf(mysql_error(connection[tid]));
        exit(1);
    }

    result = mysql_store_result(connection[tid]);
    row = mysql_fetch_row(result);

    if (row == NULL) {
        utstring_clear(&command);
        utstring_printf(&command, insert_album_sql, bname);
        state = mysql_query(connection[tid], utstring_body(&command));

        if (state != 0) {
            printf(mysql_error(connection[tid]));
            exit(1);
        }

        rc = mysql_insert_id(connection[tid]);
    } else {
        printf("DUPLICATE ALBUM NAME => %s\n", pname);
        rc = 0;
    }

    mysql_free_result(result);
    utstring_done(&command);

    return rc;
}

void insert_author_album(int tid, int64_t aid, int64_t alid)
{
    UT_string command;
    int state;

    utstring_init(&command);
    utstring_printf(&command, insert_author_album_sql, (int) aid, (int) alid);
    state = mysql_query(connection[tid], utstring_body(&command));

    if (state != 0) {
        printf(mysql_error(connection[tid]));
        exit(1);
    }

    utstring_done(&command);

    return;
}

void insert_adaptor_album(int tid, int64_t adid, int64_t alid)
{
    UT_string command;
    int state;

    utstring_init(&command);
    utstring_printf(&command, insert_adaptor_album_sql, (int) adid, (int) alid);
    state = mysql_query(connection[tid], utstring_body(&command));

    if (state != 0) {
        printf(mysql_error(connection[tid]));
        exit(1);
    }

    utstring_done(&command);

    return;
}

void disconnect(int thid)
{
    mysql_close(connection[thid]);

    return;
}
