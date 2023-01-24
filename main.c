#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <curses.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

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
#include "syntax.h"
#include "text.h"
#include "wdg/code.h"
#include "syntax/C/C.h"

#define CURSEDRGB(color) ((color>>16)&0xFF)*1000/256, ((color>>8)&0xFF)*1000/256, (color&0xFF)*1000/256

int 
main(int argc, 
		char **argv)
{
	initscr();

	if(!has_colors())
	{
		printw("terminal doesn't support color");
		endwin();
		exit(0);
	}
	noecho();
	// timeout(-1);
	// cbreak();
	raw();
	mouseinterval(0);
	keypad(stdscr, 1);
	//mousemask(ALL_MOUSE_EVENTS, NULL);
	idlok(stdscr, 0);
	idcok(stdscr, 0);
	/* Vaxeral was here. */

	start_color();
	init_pair(1, COLOR_RED, 0);
	init_pair(2, COLOR_MAGENTA, 0);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);

	init_color(COLOR_BLACK, CURSEDRGB(0x1c0d57));
	init_color(COLOR_WHITE, CURSEDRGB(0xd2ff20));
	init_color(COLOR_YELLOW, CURSEDRGB(0x48570d));
	init_color(COLOR_BLUE, CURSEDRGB(0x4d20ff));
	init_color(COLOR_MAGENTA, CURSEDRGB(0x20ff4d));
	init_color(COLOR_GREEN, CURSEDRGB(0x20ff4d));
	init_color(COLOR_CYAN, CURSEDRGB(0x20ffbc));
	init_color(COLOR_RED, CURSEDRGB(0xff2063));
	
	init_pair(C_PAIR_TEXT, COLOR_WHITE, COLOR_BLACK);
	init_pair(C_PAIR_NUMBER, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(C_PAIR_STRING1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(C_PAIR_STRING2, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_PAIR_KEYWORD1, COLOR_GREEN, COLOR_BLACK);
	init_pair(C_PAIR_KEYWORD2, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_PAIR_COMMENT1, COLOR_RED, COLOR_BLACK);
	init_pair(C_PAIR_FUNCTION, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_PAIR_PREPROC1, COLOR_RED, COLOR_BLACK);
	init_pair(C_PAIR_ERROR, COLOR_WHITE, COLOR_RED);

	init_pair(CD_PAIR_EMPTY_LINE_PREFIX, COLOR_CYAN, COLOR_BLACK);

	base_t base;
	base = bscreate();
	bsname(base, "Code");
	bssize(base, sizeof(struct code));
	bsproc(base, (eventproc_t) cdproc);

	recvaddbase("C", C_create, C_destroy, C_receive);

	widget_t wdg;
	wdg = wdgcreate("Code", CDFSHOWLINES);
	syntax_t syntax;
	syntax = syncreate("C", "C");
	cdsetsyntax((code_t) wdg, syntax);
	wdgattach(wdg, NULL);
	
	while(1)
	{
		int szX, szY;
		int c;

		getmaxyx(stdscr, szY, szX);
		wdgmgrupdate(szX, szY);
		erase();
		wdgmgrdraw();
		//wrefresh(wdg->window);
		refresh();
		c = getch();
		switch(c)
		{
		case 'q' - ('a' - 1):
			wdgmgrdiscard();
			break;
		default:
			if((wdg = wdgmgrgetfocus()))
				wdgevent(wdg, c);
		}
	}

	endwin();
	return 0;
}
