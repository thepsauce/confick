#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>

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

#define MOVE_LEFT 1
#define MOVE_RIGHT 2
#define MOVE_UP 3
#define MOVE_DOWN 4
void movement(int movement)
{
	int x, y;
	x = getcurx(stdscr);
	y = getcury(stdscr);

	switch(movement)
	{
	case MOVE_LEFT:
		x--;
		break;
	case MOVE_RIGHT:
		x++;
		break;
	case MOVE_UP: 
		y--;
		break;
	case MOVE_DOWN:
		y++;
		break;	
	}
	move(y, x);
}

void handlekey(Text tx, int key)
{
	switch(key)
	{
	case KEY_LEFT: movement(MOVE_LEFT); break;
	case KEY_UP: movement(MOVE_UP); break;
	case KEY_RIGHT: movement(MOVE_RIGHT); break;
	case KEY_DOWN: movement(MOVE_DOWN); break;
	case 'q':
		discard();
		break;
	default:
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

	Text tx = txcreate(10);

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
		refresh();
	}
	txfree(tx);
	discard();
	return 0;
}
