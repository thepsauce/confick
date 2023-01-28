#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    WINDOW* my_win;
    int height = 10;
    int width = 40;
    int srtheight = 1;
    int srtwidth = 1;
    initscr();
    refresh();  //  need to draw the root window
                //  without this, apparently the children never draw
    my_win = newwin(height, width, 15, 5);
    mvwprintw(my_win, 0, 0, "First line");
    wnoutrefresh(my_win);
	doupdate();
    wgetch(my_win);
    delwin(my_win);
    endwin();
    return 0;
}   
