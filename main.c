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
	MEVENT me;
	int c;
	int szX, szY;
	Text txs[2];
	Text txFocus;
	
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
	init_pair(3, 0, COLOR_WHITE);

	void tmp_discard(Text tx)
	{
		txfree(tx);
		endwin();
		exit(0);
	}
	void tmp_switch(Text tx)
	{
		txFocus = tx == txs[0] ? txs[1] : txs[0];
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
		{ KEY_HOME, txmotion_home },
		{ 'f' - ('a' - 1), tmp_switch },
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

	void tmp_resize()
	{
		getmaxyx(stdscr, szY, szX);
		txs[0]->x = 5;
		txs[0]->y = 0;
		txs[0]->width = szX / 2 - txs[0]->x;
		txs[0]->height = szY;
		txs[1]->x = txs[0]->x + txs[0]->width + 5 + 1 + 1;
		txs[1]->y = txs[0]->y;
		txs[1]->width = szX - txs[1]->x;
		txs[1]->height = txs[0]->height;
	}

	txs[0] = txcreate(1, 0, 0, 0, 0);
	txs[1] = txcreate(1, 0, 0, 0, 0);
	for(int t = 0; t < (int) ARRLEN(txs); t++)
	{
		txs[t]->mode = TXTYPEWRITER; 
		for(int i = 0; i < (int) ARRLEN(typewriterMotions); i++)
			txputmotion(txs[t], TXTYPEWRITER, typewriterMotions[i].id, typewriterMotions[i].motion);
		for(int i = 0; i < (int) ARRLEN(normalMotions); i++)
			txputmotion(txs[t], TXNORMAL, normalMotions[i].id, normalMotions[i].motion);
	}
	if(argc > 1)
		txopen(txs[0], argv[1]);
	txFocus = txs[0];
	tmp_resize();
	do
	{
		erase();
		for(int t = 0; t < (int) ARRLEN(txs); t++)
			txdraw(txs[t]);
		// draw barrier
		attron(COLOR_PAIR(3));
		for(int y = 0; y < szY; y++)
			mvaddch(y, txs[0]->x + txs[0]->width + 1, ' ');
		attroff(COLOR_PAIR(3));
		// draw cursor
		int visX = _txviscurx(txFocus, txFocus->curX, txFocus->curY);
		move(txFocus->y + txFocus->curY - txFocus->scrollY, txFocus->x + visX);
		refresh();
		c = getch();
		switch(c)
		{
		case KEY_MOUSE:
			if(getmouse(&me) != OK)
				continue;
			handlemouse(&me);
			break;
		case KEY_RESIZE:
			tmp_resize();
			break;
		default:
			txkey(txFocus, c);
		}
	} while(1);
	return 0;
}
