struct widget;

typedef void (*motionproc)(struct widget *wdg);

typedef struct motion {
	int id;
	motionproc motion;
} *Motion;

typedef void (*eventproc)(struct widget *wdg, int eId);

struct insets {
	int left, top, right, bottom;
};

typedef struct widgetclass {
	char *name;
	size_t size;
	eventproc proc;
	struct insets insets;
	int motionCnt;
	struct motion motions[0];
} *Widgetclass;

void wdgaddclass(Widgetclass wdgcls);
Widgetclass wdgfindclass(const char *name);

#define _WIDGET_HEADER Widgetclass wc; \
	int flags; \
	int x, y, width, height; \
	struct widget *prevFocus, *nextFocus; \
	struct widget *parent, *prev, *next, *child

typedef struct widget {
	_WIDGET_HEADER;
} *Widget;

Widget wdgcreate(const char *clsName);
void wdgfree(Widget wdg);
#define WDGINIT (-1)
#define WDGUNINIT (-2)
#define WDGDRAW (-3)
#define WDGDRAWCURSOR (-4)
#define WDGDEFAULT (-5)
void wdgevent(Widget wdg, int eId);

// widget manager
void wdgmgrdiscard(void);
void wdgmgrupdate(int szX, int szY);
void wdgmgrdraw(void);
void wdgmgradd(Widget wdg);
void wdgmgrremove(Widget wdg);
Widget wdgmgrgetfocus(void);
void wdgmgrfocusnext(void);
void wdgmgrrotate(void);

typedef struct codeblock {
	char buf[2048];
	int len;
	int height; // number of new line characters inside this block
	struct codeblock *prev, *next;
} *CodeBlock;

// c flavor
enum {
	C_PAIR_TEXT = 5,
	C_PAIR_NUMBER,
	C_PAIR_STRING1,
	C_PAIR_STRING2,
	C_PAIR_COMMENT1,
	C_PAIR_COMMENT2,
	C_PAIR_KEYWORD1,
	C_PAIR_KEYWORD2,
	C_PAIR_FUNCTION,
	C_PAIR_PREPROC1,
	C_PAIR_PREPROC2,
	C_PAIR_ERROR,
};
typedef struct code {
	_WIDGET_HEADER;
	char *fileName;
	int scrollX, scrollY;
	CodeBlock first;
	CodeBlock cur;	
	size_t cursor;
} *CodeWidget;

void cdclear(CodeWidget cd);
void cdmove(CodeWidget cd, int x, int y);
void cdputc(CodeWidget cd, int c);

typedef struct line {
	int flags;
	char *buf;
	int len, cap;
} *Line;

#define TXTABWIDTH 4
#define TXCLEARKEEPTHRESHOLD 64
typedef struct text {
	_WIDGET_HEADER;
	char *fileName;
	int scrollX, scrollY;
	int curX, curY;
	Line lines;
	int lineCnt, lineCap;
} *TextWidget;

// clears the text area
void txclear(TextWidget tx);
// moves the cursor
void txmove(TextWidget tx, int x, int y);
// deletes char at cursor
void txdelc(TextWidget tx);
// puts char at cursor
void txputc(TextWidget tx, int c);
// puts string at cursor (untested and unused function)
void txputs(TextWidget tx, const char *s);
// clears all text and replaces it with given file
void txopen(TextWidget tx, const char *fileName);
// saves all text to file
void txsave(TextWidget tx);
// breaks the line at the cursor
void txbreak(TextWidget tx);

// these functions are incomplete, don't call them with out extra supporting structures
// returns the index of the char under the given visible cursor x 
int _txshiftvisx(TextWidget tx, int visX, int y);
// returns the x coordinate of the visible cursor x
int _txviscurx(TextWidget tx, int x, int y);
// inserts a new line at the end of the text that is UNitialized
void _txgrow(TextWidget tx);
// sets the line capacity to cap
void _txgrowline(Line line, int to);
// inserts a char at given index
void _txinsertchar(Line line, int index, int c);
// inserts a string at given index with given length
void _txinsertnstr(Line line, int index, const char *s, int n);
// deletes character at given index coordinates
void _txdelete(TextWidget tx, int x, int y);
// returns the end of the line of this raw string
const char *_txlinesep(const char *s);
// returns the length of the line separator (\r, \n, \r\n)
int _txlineseplen(const char *s);
// gets the number of spaces in front of a line
int _txunitspacing(TextWidget tx, int y);

typedef struct console {
	_WIDGET_HEADER;
	char *_padding1;
	int scrollX, scrollY;
	int curX, curY;
	Line lines;
	int lineCnt, lineCap;
} *ConsoleWidget;
