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

void discard(void)
{
	endwin();
	exit(0);
}

#include "textbase.h"

#include "motion.h"
#include "text.h"

void handlemouse(MEVENT *me)
{
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
	init_pair(1, COLOR_RED, 0);
	init_pair(2, COLOR_MAGENTA, 0);

	Text tx = txcreate(3, 5, 0, 30, 10);
	txputmotion(tx, KEY_UP, txmotion_up);
	txputmotion(tx, KEY_LEFT, txmotion_left);
	txputmotion(tx, KEY_RIGHT, txmotion_right);
	txputmotion(tx, KEY_DOWN, txmotion_down);
	txputmotion(tx, 330, txmotion_delete);
	txputmotion(tx, KEY_BACKSPACE, txmotion_backdelete);
	void tmp_discard(Text tx, int *px, int *py)
	{
		txfree(tx);
		discard();
	}
	txputmotion(tx, 'q', tmp_discard);

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
