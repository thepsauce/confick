struct widget;

// #define OK 0 // curses.h already defines an OK with value 0
#define WARN(msg, ...) ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); 1; __VA_ARGS__; })
#define ERROR(msg, ...) ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); exit(-1); -1; __VA_ARGS__; })

typedef int (*eventproc_t)(struct widget *wdg, int eId);

#define BASECREATED (1<<1)
#define BASENAME (1<<2)
#define BASESIZE (1<<3)
#define BASEPROC (1<<4)
#define BASECOMPLETE (BASECREATED|BASENAME|BASESIZE|BASEPROC)
typedef struct base {
	int flags;
	char *name;
	size_t size;
	eventproc_t proc;
} *base_t;

base_t bscreate(void);
int bsname(base_t base, const char *name);
int bssize(base_t base, size_t size);
int bsproc(base_t base, eventproc_t proc);
base_t bsfind(const char *name);

#define _WIDGET_HEADER base_t base; \
	int flags; \
	WINDOW *window; \
	struct widget *prevFocus, *nextFocus; \
	struct widget *parent, *prev, *next, *child

typedef struct widget {
	_WIDGET_HEADER;
} *widget_t;

widget_t wdgcreate(const char *bsName, int flags);
int wdgfree(widget_t wdg);
#define WDGINIT (-1)
#define WDGUNINIT (-2)
#define WDGDRAW (-3)
#define WDGDRAWCURSOR (-4)
#define WDGDEFAULT (-5)
int wdgevent(widget_t wdg, int c);

// widget manager
void wdgmgrdiscard(void);
void wdgmgrupdate(int szX, int szY);
void wdgmgrdraw(void);
void wdgmgradd(widget_t wdg);
void wdgmgrremove(widget_t wdg);
widget_t wdgmgrgetfocus(void);
void wdgmgrfocusnext(void);
void wdgmgrrotate(void);

// line text system
typedef struct line {
	int flags;
	char *buf;
	int nBuf, szBuf;
} line_t;

#define TXFLINECROSSING (1<<1)
typedef struct text {
	int flags;
	char *fileName;
	struct {
		int x, y;
	} cursor;
	line_t *lines;
	int nLines, szLines;
} text_t;
int txclear(text_t *text);

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

	CD_PAIR_EMPTY_LINE_PREFIX,
};

typedef struct condition {
	unsigned short set[16];
} condition_t;
condition_t conset(const char *str);
condition_t condefault(void);
condition_t connegate(condition_t con);
condition_t conor(condition_t c1, condition_t c2);
condition_t conand(condition_t c1, condition_t c2);
condition_t conxor(condition_t c1, condition_t c2);

typedef struct state {
	struct {
		condition_t condition;
		struct state *nextState;
	} *subStates;
	int nSubStates, szSubStates;
	chtype cht;
} *state_t;
state_t stacreate(chtype cht);
int staaddstate(state_t parent, state_t child, condition_t condition);

typedef struct syntax {
	struct state initialState;
	state_t currentState;
} *syntax_t;
syntax_t syncreate(chtype cht);
state_t syninitialstate(syntax_t syntax);
int synaddstate(syntax_t syntax, state_t state, condition_t condition);

#define CDFSHOWLINES (1<<1)
#define CDFSHOWSTATUS (1<<2)
typedef struct code {
	_WIDGET_HEADER;
	text_t text;
	syntax_t syntax;
} *code_t;

typedef struct console {
	_WIDGET_HEADER;
	text_t text;
} *console_t;
