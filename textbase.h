struct line {
	int flags;
	char *buf;
	int len, cap;
};

#define TXREADONLY 1
struct text {
	int flags;
	int state;
	int x, y;
	int curX, curY;
	struct line *lines;
	int lineCnt, lineCap;
} *Text;

Text txcreate(int initLineCap);
void txputc(Text text, int c);
void txputs(Text text, const char *s);
void txdelete(Text text);
#define TXMOVE_UP 1
#define TXMOVE_LEFT 2
#define TXMOVE_RIGHT 3
#define TXMOVE_BOTTOM 4
void txmove(Text text, int move);
void txstate(Text text, int state);
void txdestroy(Text text);

