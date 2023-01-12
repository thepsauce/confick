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
	init_pair(4, COLOR_GREEN, COLOR_BLACK);

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
		{ WDGCURSORDRAW, (motionproc) txdrawcursor },
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
		{ WDGCURSORDRAW, (motionproc) txdrawcursor },
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
		{ KEY_LEFT, (motionproc) cdleft },
		{ KEY_RIGHT, (motionproc) cdright },
		{ KEY_DC, (motionproc) cddelete },
		{ KEY_BACKSPACE, (motionproc) cdbackdelete },
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
			break;
		case 'r' - ('a' - 1):
			//wdgmgrrotate();		
			break;
		case 'q' - ('a' - 1):
			wdgmgrdiscard();		
			break;
		}
		if((wdg = wdgmgrgetfocus()))
			wdgevent(wdg, c);
	} while(1);
	return 0;
}
