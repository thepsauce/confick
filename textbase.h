typedef struct line {
	int flags;
	char *buf;
	int len, cap;
} *Line;

#define TXREADONLY 1
typedef struct text {
	int flags;
	int state;
	int scrollX, scrollY;
	int x, y;
	int curX, curY;
	Line lines;
	int lineCnt, lineCap;
} *Text;

Text txcreate(int initLineCap);
void txputc(Text text, int c);
void txputs(Text text, const char *s);
void txfree(Text text);
#define txline(t) ((t)->lines+(t)->curY)
#define TXMOVE_UP 1
#define TXMOVE_LEFT 2
#define TXMOVE_RIGHT 3
#define TXMOVE_BOTTOM 4
void txmove(Text text, int move);
void txstate(Text text, int state);
void txdestroy(Text text);

