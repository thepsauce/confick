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
#define TXNORMAL 0
#define TXSPECIAL 1
#define TXTYPEWRITER 2
typedef struct text {
	int flags;
	char *fileName;
	int mode;
	int scrollX, scrollY;
	int x, y;
	int width, height;
	struct {
		Motion elems;
		int cnt, cap;
	} motions[3];
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
void txmode(Text text, int mode);
void txputmotion(Text tx, int mode, int id, void (*motion)(Text tx, int *px, int *py));
void txopen(Text tx, const char *fileName);
void txsave(Text tx);
