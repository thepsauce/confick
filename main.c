#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <ncurses.h>
#include <stdio.h>

#define ARRLEN(a) (sizeof(a)/sizeof(*a))

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

#include "textbase.h"

#include "mode_normal.h"
#include "typewriter.h"
#include "text.h"
#include "io.h"

void handlemouse(MEVENT *me)
{
}

int main(int argc, char **argv)
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
	mousemask(ALL_MOUSE_EVENTS, NULL);
	idlok(stdscr, 0);
	idcok(stdscr, 0);
	/* Vaxeral was here. */

	start_color();
	init_pair(1, COLOR_RED, 0);
	init_pair(2, COLOR_MAGENTA, 0);

	void tmp_discard(Text tx)
	{
		txfree(tx);
		endwin();
		exit(0);
	}
	struct {
		int id;
		void (*motion)(Text tx);
	} typewriterMotions[] = {
		{ KEY_UP, txmotion_up },
		{ KEY_LEFT, txmotion_left },
		{ KEY_RIGHT, txmotion_right },
		{ KEY_DOWN, txmotion_down },
		{ KEY_DC, txmotion_delete },
		{ KEY_BACKSPACE, txmotion_backdelete },
		{ KEY_F(2), txsave },
		{ 'q' - ('a' - 1), tmp_discard },
		{ KEY_END, txmotion_end },
		{ KEY_HOME, txmotion_home }

	};
	struct {
		int id;
		void (*motion)(Text tx);
	} normalMotions[] = {
		{ '\n', txmotion_c_nl_indent },
		{ '}', txmotion_c_curlyclose },
		{ 'i', txmotion_c_if }
	};/*
	struct {
		int id;
		void (*motion)(Text tx);
	} specialMotions[] = {
		{ KEY_UP, txspecialup },
		{ KEY_LEFT, txspecialleft },
		{ KEY_RIGHT, txspecialright },
		{ KEY_DOWN, txspecialdown },
		{ KEY_HOME, txspecialhome },
		{ KEY_END, txspecialend },
	};*/

	Text tx = txcreate(3, 5, 0, 30, 10);
	tx->mode = TXTYPEWRITER; 
	for(int i = 0; i < ARRLEN(typewriterMotions); i++)
		txputmotion(tx, TXTYPEWRITER, typewriterMotions[i].id, typewriterMotions[i].motion);
	for(int i = 0; i < ARRLEN(normalMotions); i++)
		txputmotion(tx, TXNORMAL, normalMotions[i].id, normalMotions[i].motion);

	if(argc > 1)
		txopen(tx, argv[1]);

	erase();
	txdraw(tx);
	refresh();

	MEVENT me;
	int c;
	while(1)
	{
		c = getch();
		switch(c)
		{
		case KEY_MOUSE:
			if(getmouse(&me) != OK)
				continue;
			handlemouse(&me);
			break;
		default:
			txkey(tx, c);
		}
		erase();
		txdraw(tx);
		refresh();
	}
	return 0;
}
