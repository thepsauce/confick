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
	Text *txs;
	int txCnt;
	int focus;
	int r;
	
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
		focus++;
		focus %= txCnt;
	}
	void tmp_resize()
	{
		int x, y, w, h;
		Text *ptx, tx;
		int cnt;
		int dir;

		getmaxyx(stdscr, szY, szX);
		dir = 1;
		ptx = txs;
		cnt = txCnt;
		switch(r)
		{
		case 2:
			dir = -1;
			ptx += txCnt - 1;
		case 0:
			x = 0;
			w = (szX - 1) / txCnt;
			if(w > 5)
			{
				for(; --cnt; ptx += dir)
				{
					tx = *ptx;
					tx->x = x + 5;
					tx->y = 0;
					tx->width = w - 5 - 1;
					tx->height = szY - 1 - 1;
					x += w + 1;
				}
			}
			tx = *ptx;
			tx->x = x + 5;
			tx->y = 0;
			tx->width = szX - 1 - (x + 5);
			tx->height = szY - 1 - 1;
			break;
		case 3:
			dir = -1;
			ptx += txCnt - 1;
		case 1:
			y = 0;
			h = (szY - 1) / txCnt;
			if(h > 1)
			{
				for(; --cnt; ptx += dir)
				{
					tx = *ptx;
					tx->x = 5;
					tx->y = y;
					tx->width = szX - 5 - 1;
					tx->height = h - 1;
					y += h + 1;
				}
			}
			tx = *ptx;
			tx->x = 5;
			tx->y = y;
			tx->width = szX - 1 - 5;
			tx->height = szY - 1 - y - 1;
			break;
		}
	}
	void tmp_rotate(Text tx)
	{
		r++;
		r %= 4;
		tmp_resize();
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
		{ 'r' - ('a' - 1), tmp_rotate },
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

	txCnt = 4;
	txs = malloc(txCnt * sizeof*txs);
	r = 0;
	focus = 0;
	for(int t = 0; t < txCnt; t++)
	{
		txs[t] = txcreate(1, 0, 0, 0, 0);
		txs[t]->mode = TXTYPEWRITER; 
		for(int i = 0; i < (int) ARRLEN(typewriterMotions); i++)
			txputmotion(txs[t], TXTYPEWRITER, typewriterMotions[i].id, typewriterMotions[i].motion);
		for(int i = 0; i < (int) ARRLEN(normalMotions); i++)
			txputmotion(txs[t], TXNORMAL, normalMotions[i].id, normalMotions[i].motion);
	}
	for(int i = 0; i < min(argc - 1, txCnt); i++)
		txopen(txs[i], argv[i + 1]);
	tmp_resize();
	do
	{
		erase();
		for(int t = 0; t < txCnt; t++)
			txdraw(txs[t]);
		// draw barrier
		attron(A_DIM | COLOR_PAIR(3));
		if(!(r % 2))
		{
			int w = (szX - 1) / txCnt;
			for(int t = 1; t < txCnt; t++)
				for(int y = 0; y < szY; y++)
					mvaddch(y, w * t + t - 1, ' ');
		}
		attroff(A_DIM | COLOR_PAIR(3));
		// draw cursor
		int visX = _txviscurx(txs[focus], txs[focus]->curX, txs[focus]->curY);
		move(txs[focus]->y + txs[focus]->curY - txs[focus]->scrollY, txs[focus]->x + visX);
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
			txkey(txs[focus], c);
		}
	} while(1);
	return 0;
}
