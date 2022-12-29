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
	int width, height;
	int curX, curY;
	Line lines;
	int lineCnt, lineCap;
} *Text;

Text txcreate(int initLineCap, int x, int y, int width, int height);
void txputc(Text text, int c);
void txputs(Text text, const char *s);
void txfree(Text text);
#define txline(t) ((t)->lines+(t)->curY)
void txstate(Text text, int state);
#define TXMOTION_UP 1
#define TXMOTION_LEFT 2
#define TXMOTION_RIGHT 3
#define TXMOTION_DOWN 4
#define TXMOTION_SOL 5
#define TXMOTION_EOL 6
void txmotion(Text text, int motion);
