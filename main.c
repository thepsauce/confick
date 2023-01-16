#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <ncurses.h>
#include <stdio.h>

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
#include "wdg/console.h"
#include "wdg/text.h"
#include "wdg/code.h"

#define CURSEDRGB(color) ((color>>16)&0xFF)*1000/256, ((color>>8)&0xFF)*1000/256, (color&0xFF)*1000/256

int main(int argc, char **argv)
{
	Widget wdg, parent;
	int szX, szY;
	int c;

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

	Widgetclass wc;
	void tmp_close(Widget wdg)
	{
		// wdgmgrremove(wdg);
		wdgfree(wdg);
	}
	struct motion txMotions[] = {
		{ WDGINIT, (motionproc) txinit },
		{ WDGUNINIT, (motionproc) txuninit },
		{ WDGDRAW, (motionproc) txdraw },
		{ WDGDRAWCURSOR, (motionproc) txdrawcursor },
		{ KEY_LEFT, (motionproc) txleft },
		{ KEY_UP, (motionproc) txup },
		{ KEY_RIGHT, (motionproc) txright },
		{ KEY_DOWN, (motionproc) txdown },
		{ KEY_DC, (motionproc) txdelete },
		{ KEY_BACKSPACE, (motionproc) txbackdelete },
		{ KEY_F(2), (motionproc) txsave },
		{ 'w' - ('a' - 1), (motionproc) tmp_close },
		{ KEY_END, (motionproc) txend },
		{ KEY_HOME, (motionproc) txhome },
	};
	wc = malloc((sizeof*wc) + (sizeof txMotions));
	wc->name = strdup("Text");
	wc->size = sizeof(struct text);
	wc->proc = (eventproc) txevent;
	wc->insets = (struct insets) { 5, 0, 0, 1 };
	wc->motionCnt = ARRLEN(txMotions);
	memcpy(wc->motions, txMotions, sizeof txMotions);
	wdgaddclass(wc);

	struct motion consoleMotions[] = {
		{ WDGINIT, (motionproc) txinit },
		{ WDGUNINIT, (motionproc) txuninit },
		{ WDGDRAW, (motionproc) csdraw },
		{ WDGDRAWCURSOR, (motionproc) txdrawcursor },
		{ KEY_LEFT, (motionproc) csleft },
		{ KEY_UP, (motionproc) csup },
		{ KEY_RIGHT, (motionproc) csright },
		{ KEY_DOWN, (motionproc) csdown },
		{ KEY_HOME, (motionproc) cshome },
		{ KEY_END, (motionproc) csend },
		{ KEY_DC, (motionproc) csdelete },
		{ KEY_BACKSPACE, (motionproc) csbackdelete },
		{ '\n', (motionproc) csenter },
	};
	wc = malloc((sizeof*wc) + (sizeof consoleMotions));
	wc->name = strdup("Console");
	wc->size = sizeof(struct console);
	wc->proc = (eventproc) csevent;
	wc->insets = (struct insets) { 0, 0, 0, 0 };
	wc->motionCnt = ARRLEN(consoleMotions);
	memcpy(wc->motions, consoleMotions, sizeof consoleMotions);
	wdgaddclass(wc);

	struct motion codeMotions[] = {
		{ WDGINIT, (motionproc) cdinit },
		{ WDGUNINIT, (motionproc) cduninit },
		{ WDGDRAW, (motionproc) cddraw },
		{ WDGDRAWCURSOR, (motionproc) cddrawcursor },
		{ KEY_LEFT, (motionproc) cdleft },
		{ KEY_RIGHT, (motionproc) cdright },
		{ KEY_UP, (motionproc) cdup },
		{ KEY_DOWN, (motionproc) cddown },
		{ KEY_DC, (motionproc) cddelete },
		{ KEY_BACKSPACE, (motionproc) cdbackdelete },
		{ KEY_HOME, (motionproc) cdhome },
		{ KEY_END, (motionproc) cdend },
	};
	wc = malloc((sizeof*wc) + (sizeof codeMotions));
	wc->name = strdup("Code");
	wc->size = sizeof(struct code);
	wc->proc = (eventproc) cdputc;
	wc->insets = (struct insets) { 0, 0, 0, 0 };
	wc->motionCnt = ARRLEN(codeMotions);
	memcpy(wc->motions, codeMotions, sizeof codeMotions);
	wdgaddclass(wc);

	parent = wdgcreate(NULL);
	for(int t = 1; t < argc; t++)
	{
		wdg = wdgcreate("Text");
		txopen((TextWidget) wdg, argv[t]);
		wdgattach(wdg, parent);
	}
	if(argc == 1)
	{
		wdg = wdgcreate("Text");
		wdgattach(wdg, parent);
	}
	wdgattach(parent, NULL);

	wdg = wdgcreate("Code");
	if(argc > 1)
		cdopen((CodeWidget) wdg, argv[1]);
	wdgattach(wdg, NULL);

	do
	{
		getmaxyx(stdscr, szY, szX);
		wdgmgrupdate(szX, szY);
		erase();
		wdgmgrdraw();
		refresh();
		c = getch();
		switch(c)
		{
		case 'f' - ('a' - 1):
			if(Focus->nextFocus)
				Focus = Focus->nextFocus;
			else
			{
				while(Focus->prevFocus)
					Focus = Focus->prevFocus;
			}
			continue;
		case 'r' - ('a' - 1):
			//wdgmgrrotate();		
			break;
		case 'q' - ('a' - 1):
			wdgmgrdiscard();		
			continue;
		}
		if((wdg = wdgmgrgetfocus()))
			wdgevent(wdg, c);
	} while(1);
	return 0;
}
