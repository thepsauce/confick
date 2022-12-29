#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <ncurses.h>

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

#include "text.h"

void discard(void)
{
	endwin();
	exit(0);
}

void handlemouse(MEVENT *me)
{
	
}

void handlekey(Text tx, int key)
{
	switch(key)
	{
	case KEY_LEFT: txmotion(tx, TXMOTION_LEFT); break;
	case KEY_UP: txmotion(tx, TXMOTION_UP); break;
	case KEY_RIGHT: txmotion(tx, TXMOTION_RIGHT); break;
	case KEY_DOWN: txmotion(tx, TXMOTION_DOWN); break;
	case 330: txdelc(tx); break;
	case 'q':
		discard();
		break;
	default:
		//_txgrow(tx);
		//tx->lineCnt++;
		if(key <= 255)
			txputc(tx, key);
	}
}

int main(int argc, char **argv)
{
	initscr();

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

	if(!has_colors())
	{
		printw("terminal doesn't support color, exiting...");
		getch();
		discard();
	}

	start_color();
	init_pair(0, COLOR_RED, COLOR_GREEN);

	Text tx = txcreate(3, 0, 0, 30, 10);

	int running = 1;
	MEVENT me;
	int c;
	while(running)
	{
		c = getch();
		switch(c)
		{
		case KEY_MOUSE:
			if(getmouse(&me) != OK)
				break;
			handlemouse(&me);
			break;
		default:
			handlekey(tx, c);
		}
		erase();
		for(int i = 0; i <= tx->height; i++)
		{
			int y = i + tx->scrollY;
			if(y >= tx->lineCnt)
				mvaddch(i, 0, '~');
			else
			{
				// number of visible chars
				int visCh = tx->lines[y].len - tx->scrollX;
				if(visCh > 0)
					mvaddnstr(i + tx->y, tx->x, tx->lines[y].buf + tx->scrollX, min(visCh, tx->width + 1));
			}
			mvaddch(i + tx->y, tx->x + tx->width + 1, '#');
		}
		for(int i = 0; i <= tx->width; i++)
			mvaddch(tx->y + tx->height + 1, tx->x + i, '#');
		wmove(stdscr, tx->curY + tx->y - tx->scrollY, tx->curX + tx->x - tx->scrollX);	
		refresh();
	}
	txfree(tx);
	discard();
	return 0;
}
