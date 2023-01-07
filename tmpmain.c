#include <ncurses.h>


int main(void)
{
	initscr();
	start_color();

	int colors[] = {
		COLOR_BLACK,
		COLOR_BLUE,
		COLOR_GREEN,
		COLOR_CYAN,
		COLOR_RED,
		COLOR_MAGENTA,
		COLOR_YELLOW,
		COLOR_WHITE
	};
	for(int i = 0; i < 16; i++)
	for(int j = 0; j < 16; j++)
		init_pair(1 + 8 * i + j, colors[i & 0x7], colors[j & 0x7]);
	
	for(int i = 0; i < 16; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			attron((((i & 0x8) >> 3) * A_BOLD) | COLOR_PAIR(1 + 8 * i + j));
			mvaddstr(i, 10 * j, "...text...");
			attroff((((i & 0x8) >> 3) * A_BOLD) | COLOR_PAIR(1 + 8 * i + j));
		}
		for(int j = 8; j < 16; j++)
		{
			attron((((i & 0x8) >> 3) * A_BOLD) | A_BLINK | A_UNDERLINE | COLOR_PAIR(1 + 8 * i + j));
			mvaddstr(i + 17, 10 * (j - 8), "...text...");
			attroff((((i & 0x8) >> 3) * A_BOLD) | A_BLINK | A_UNDERLINE | COLOR_PAIR(1 + 8 * i + j));
		}
	}

	getch();
	endwin();
	return 0;
}
