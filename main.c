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

#include "textbase.h"

#include "txmgr.h"
#include "mode_normal.h"
#include "typewriter.h"
#include "text.h"
#include "io.h"

int main(int argc, char **argv)
{
	MEVENT me;
	int c;
	int szX, szY;
	Text tx;	

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
		(void) tx;
		txmgrdiscard();
	}
	void tmp_rotate(Text tx)
	{
		(void) tx;
		txmgrrotate();
	}
	void tmp_switch(Text tx)
	{
		(void) tx;
		txmgrfocusnext();
	}
	void tmp_close(Text tx)
	{
		txremove(tx);
		txfree(tx);
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
		{ 'w' - ('a' - 1), tmp_close },
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

	for(int t = 0; t < argc - 1; t++)
	{
		tx = txcreate(1, 0, 0, 0, 0);
		tx->mode = TXTYPEWRITER; 
		for(int i = 0; i < (int) ARRLEN(typewriterMotions); i++)
			txputmotion(tx, TXTYPEWRITER, typewriterMotions[i].id, typewriterMotions[i].motion);
		for(int i = 0; i < (int) ARRLEN(normalMotions); i++)
			txputmotion(tx, TXNORMAL, normalMotions[i].id, normalMotions[i].motion);
		txopen(tx, argv[t + 1]);
		txmgradd(tx);
	}
	if(argc == 1)
	{
		tx = txcreate(1, 0, 0, 0, 0);
		tx->mode = TXTYPEWRITER; 
		for(int i = 0; i < (int) ARRLEN(typewriterMotions); i++)
			txputmotion(tx, TXTYPEWRITER, typewriterMotions[i].id, typewriterMotions[i].motion);
		for(int i = 0; i < (int) ARRLEN(normalMotions); i++)
			txputmotion(tx, TXNORMAL, normalMotions[i].id, normalMotions[i].motion);
		txmgradd(tx);
	}
	do
	{
		getmaxyx(stdscr, szY, szX);
		txmgrupdate(szX, szY);
		erase();
		txmgrdraw();
		refresh();
		c = getch();
		if((tx = txmgrgetfocus()))
			txkey(tx, c);
	} while(1);
	return 0;
}
