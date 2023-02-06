#define _XOPEN_SOURCE_EXTENDED
#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))

#define min(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__a:__b; \
})

#define max(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__b:__a; \
})

#include "wdgbase.h"
#include "wdg.h"
#include "wdgmgr.h"
#include "text.h"
#include "state.h"
#include "C/C.h"
#include "wdg/code.h"

#define CURSEDRGB(color) ((color>>16)&0xFF)*1000/256, ((color>>8)&0xFF)*1000/256, (color&0xFF)*1000/256

void *keeperPiece;

void
heavy_breathing(void)
{
	endwin();
	free(keeperPiece);
	keeperPiece = NULL;
	printf("Ran out of memory, using keeper's memory for last breath\nWhat's your call?\n");
options:
	printf("1. Exit quietly\n");
	printf("2. Retry\n");
	int c = getchar();
	while(getchar() != '\n');
	switch(c)
	{
	case '1':
		fcloseall();
		wdgmgrdiscard();
		break;
	case '2':
		if(!(keeperPiece = malloc(4096)))
		{
			free(keeperPiece);
			printf("Retry failed!\n");
			goto options;
		}
		printf("Retry successful!\n");
		sleep(1);
		refresh();
		break;
	default:
		goto options;
	}
}

void *
safe_malloc(size_t size)
{
	void *ptr;
	while(!(ptr = malloc(size)))
		heavy_breathing();
	return ptr;
}

void *
safe_realloc(void *ptr, size_t size)
{
	while(!(ptr = realloc(ptr, size)))
		heavy_breathing();
	return ptr;
}

void *
safe_strdup(const char *str)
{
	void *ptr;
	while(!(ptr = strdup(str)))
		heavy_breathing();
	return ptr;
}

void *
safe_strndup(const char *str, size_t nStr)
{
	void *ptr;
	while(!(ptr = strndup(str, nStr)))
		heavy_breathing();
	return ptr;
}

void
finish(int sig)
{
	wdgmgrdiscard();
}

void __attribute__((noreturn))
main(int argc,
		char **argv)
{
	signal(SIGINT, finish);

	setlocale(LC_ALL, "");

	keeperPiece = malloc(4096);
	if(!keeperPiece)
	{
		printf("startup error: out of memory\n");
		exit(-1);
	}

	initscr();
	if(!has_colors())
	{
		endwin();
		printf("terminal doesn't support color\n");
		exit(0);
	}
	noecho();
	// timeout(-1);
	// cbreak();
	raw();
	mouseinterval(0);
	//mousemask(ALL_MOUSE_EVENTS, NULL);
	nl();
	set_tabsize(4);
	/* Vaxeral was here. */

	start_color();
	init_pair(1, COLOR_RED, 0);
	init_pair(2, COLOR_MAGENTA, 0);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);

/*	init_color(COLOR_BLACK, CURSEDRGB(0x0e062b));
	init_color(COLOR_WHITE, CURSEDRGB(0xefefef));
	init_color(COLOR_YELLOW, CURSEDRGB(0xd2ff20));
	init_color(COLOR_BLUE, CURSEDRGB(0x4d20ff));
	init_color(COLOR_MAGENTA, CURSEDRGB(0xff20d2));
	init_color(COLOR_GREEN, CURSEDRGB(0x20ff4d));
	init_color(COLOR_CYAN, CURSEDRGB(0x20ffbc));
	init_color(COLOR_RED, CURSEDRGB(0xff2063));*/
	
	init_pair(C_PAIR_TEXT, COLOR_WHITE, COLOR_BLACK);
	init_pair(C_PAIR_CHAR, COLOR_RED, COLOR_BLACK);
	init_pair(C_PAIR_NUMBER, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(C_PAIR_STRING1, COLOR_RED, COLOR_BLACK);
	init_pair(C_PAIR_STRING2, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_PAIR_KEYWORD1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(C_PAIR_KEYWORD2, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_PAIR_COMMENT1, COLOR_WHITE, COLOR_BLACK);
	init_pair(C_PAIR_FUNCTION, COLOR_GREEN, COLOR_BLACK);
	init_pair(C_PAIR_PREPROC1, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_PAIR_PREPROC2, COLOR_RED, COLOR_BLACK);
	init_pair(C_PAIR_ERROR, COLOR_YELLOW, COLOR_RED);
	init_pair(C_PAIR_LINENUMBER, COLOR_RED, COLOR_BLACK);

	init_pair(CD_PAIR_EMPTY_LINE_PREFIX, COLOR_CYAN, COLOR_BLACK);

	base_t base;
	base = bscreate();
	bsname(base, "Code");
	bssize(base, sizeof(struct code));
	bsproc(base, (eventproc_t) cdproc);

	widget_t wdg;
	wdg = wdgcreate("Code", CDFSHOWLINES | CDFSHOWSTATUS);
	((code_t) wdg)->text.flags |= TXFLINECROSSING;
	if(argc > 1)
		txopen(&((code_t) wdg)->text, argv[1]);
	wdgattach(wdg, NULL);

	while(1)
	{
		int szX, szY;
		wint_t c;

		getmaxyx(stdscr, szY, szX);
		wdgmgrupdate(szX, szY);
		wdgmgrdraw();
		wdg = wdgmgrgetfocus();
		wget_wch(wdg->window, &c);
		switch(c)
		{
		case L'q' - (L'a' - 1):
			finish(0);
			break;
		default:
			wdgevent(wdg, WDGKEY, c);
		}
	}
}
