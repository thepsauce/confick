typedef struct line {
	int flags;
	char *buf;
	int len, cap;
} *Line;

struct text;

typedef struct motion {
	int id;
	void (*motion)(struct text*, int *x, int *y);
} *Motion;

#define TXREADONLY 1
typedef struct text {
	int flags;
	int state;
	int scrollX, scrollY;
	int x, y;
	int width, height;
	struct {
		Motion elems;
		int cnt, cap;
	} motions;
	int curX, curY;
	Line lines;
	int lineCnt, lineCap;
} *Text;

Text txcreate(int initLineCap, int x, int y, int width, int height);
void txfree(Text text);
void txmove(Text tx, int x, int y);
void txdelc(Text tx);
void txputc(Text text, int c);
void txputs(Text text, const char *s);
#define txline(t) ((t)->lines+(t)->curY)
void txstate(Text text, int state);
#define TXMOTION_UP 1
#define TXMOTION_LEFT 2
#define TXMOTION_RIGHT 3
#define TXMOTION_DOWN 4
#define TXMOTION_SOL 5
#define TXMOTION_EOL 6
bool txmotion(Text text, int motion);
