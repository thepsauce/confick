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
void wdgfree(Widget text);
void wdgclear(Widget wdg);
#define WDGINIT (-1)
#define WDGUNINIT (-2)
#define WDGDRAW (-3)
#define WDGCURSORDRAW (-4)
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

#define TTNONE 0
#define TTWORD 1
#define TTNUMBER 2

typedef struct token {
	int type;
	const char *str;
	size_t len;
	struct token *prev, *next;
} *Token;

typedef struct codewidget {
	_WIDGET_HEADER;
	struct token first;
	struct token *cur;
	size_t cursor;
} *CodeWidget;

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

void txclear(TextWidget tx);
void txmove(TextWidget tx, int x, int y);
void txdelc(TextWidget tx);
void txputc(TextWidget text, int c);
void txputs(TextWidget text, const char *s);
void txopen(TextWidget tx, const char *fileName);
void txsave(TextWidget tx);
// breaks the line at the cursor
void txbreak(TextWidget tx);

// these functions are incomplete, don't call them with out extra supporting structures
// returns the index of the char under the given visible cursor X 
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
