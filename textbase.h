typedef struct line {
	int flags;
	char *buf;
	int len, cap;
} *Line;

struct text;

typedef struct motion {
	int id;
	void (*motion)(struct text *text);
} *Motion;

#define TXREADONLY 1

#define TXNORMAL 0
#define TXSPECIAL 1
#define TXTYPEWRITER 2

#define TXCLEARKEEPTHRESHOLD 64
#define TXTABWIDTH 4

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
void txclear(Text tx);
void txdraw(Text tx);
void txmove(Text tx, int x, int y);
void txdelc(Text tx);
void txputc(Text text, int c);
void txputs(Text text, const char *s);
#define txline(t) ((t)->lines+(t)->curY)
void txmode(Text text, int mode);
void txputmotion(Text tx, int mode, int id, void (*motion)(Text tx));
void txopen(Text tx, const char *fileName);
void txsave(Text tx);

// these functions are incomplete, don't call them with out extra supporting structures
// returns the index of the char under the given visible cursor X 
int _txshiftvisx(Text tx, int visX, int y);
// returns the x coordinate of the visible cursor x
int _txviscurx(Text tx, int x, int y);
// inserts a new line at the end of the text that is UNitialized
void _txgrow(Text tx);
// breaks the line at the cursor
void _txbreak(Text tx);
// sets the line capacity to cap
void _txgrowline(Line line, int to);
// inserts a char at given index
void _txinsertchar(Line line, int index, int c);
// inserts a string at given index with given length
void _txinsertnstr(Line line, int index, const char *s, int n);
// deletes character at given index coordinates
void _txdelete(Text tx, int x, int y);
// returns the end of the line of this raw string
const char *_txlinesep(const char *s);
// returns the length of the line separator (\r, \n, \r\n)
int _txlineseplen(const char *s);
// gets the number of spaces in front of a line
int _txunitspacing(Text tx, int y);

// text manager
void txmgrdiscard(void);
void txmgrupdate(int szX, int szY);
void txmgrdraw(void);
void txmgradd(Text tx);
void txmgrremove(Text tx);
Text txmgrgetfocus(void);
void txmgrfocusnext(void);
void txmgrrotate(void);
