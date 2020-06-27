/*
 *	dscan (screen.c)
 *	(c) 2019, Brian Stephenson
 *	brian@bstephen.me.uk
 */
#define DS_NO_CELLO
#include <ncursesw/ncurses.h>
#include "utstring.h"
#include "dscan.h"

#define DS_THREAD_1_LINE 2
#define DS_THREAD_2_LINE 8
#define DS_THREAD_3_LINE 14
#define DS_AUTHOR_INDENT 3
#define DS_ADAPTOR_INDENT 5
#define DS_ALBUM_INDENT 7
#define DS_TRACK_INDENT 9

static int bline[DS_NUMBER_OF_THREADS] = {DS_THREAD_1_LINE, DS_THREAD_2_LINE, DS_THREAD_3_LINE};
static UT_string* blank;
static UT_string* long_blank;

void open_screen()
{
    utstring_new(blank);
    utstring_printf(blank, "%100s", " ");
    utstring_new(long_blank);
    utstring_printf(long_blank, "%250s", " ");
    initscr();
    move(0, 1);
    addstr(DS_PROG_NAME);
    refresh();

    return;
}

void scr_add_author(const char* author, int thid)
{
    move(bline[thid], DS_AUTHOR_INDENT);
    addstr(utstring_body(blank));
    move(bline[thid], DS_AUTHOR_INDENT);
    addstr(author);
    refresh();

    return;
}

void scr_add_command(const char* command, int thid)
{
    return;
}

void scr_add_adaptor(const char* adaptor, int thid)
{
    move(bline[thid] + 1, DS_ADAPTOR_INDENT);
    addstr(utstring_body(blank));
    move(bline[thid] + 1, DS_ADAPTOR_INDENT);
    addstr(adaptor);
    refresh();

    return;
}

void scr_add_album(const char* album, int thid)
{
    move(bline[thid] + 2, DS_ALBUM_INDENT);
    addstr(utstring_body(blank));
    move(bline[thid] + 2, DS_ALBUM_INDENT);
    addstr(album);
    refresh();

    return;
}

void scr_add_track(const char* track, int thid)
{
    move(bline[thid] + 3, DS_TRACK_INDENT);
    addstr(utstring_body(blank));
    move(bline[thid] + 3, DS_TRACK_INDENT);
    addstr(track);
    refresh();

    return;
}

void close_screen()
{
    utstring_free(blank);
    utstring_free(long_blank);
    endwin();

    return;
}

