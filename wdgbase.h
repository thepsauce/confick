struct widget;

// #define OK 0 // curses.h already defines an OK with value 0
#define WARN(msg, ...) ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); 1; __VA_ARGS__; })
#define ERROR(msg, ...) ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); exit(-1); -1; __VA_ARGS__; })

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t newSize);
void *safe_strdup(const char *str);
void *safe_strndup(const char *str, size_t nStr);

typedef int (*eventproc_t)(struct widget *wdg, int event, int key);

#define BASECREATED (1<<1)
#define BASENAME (1<<2)
#define BASESIZE (1<<3)
#define BASEPROC (1<<4)
#define BASECOMPLETE (BASECREATED|BASENAME|BASESIZE|BASEPROC)
#define BASENAME_MAX 64
typedef struct base {
	int flags;
	char name[BASENAME_MAX + 1];
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
#define WDGINIT 1
#define WDGUNINIT 2
#define WDGKEY 3
int wdgevent(widget_t wdg, int event, int key);

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
	wchar_t *buf;
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
	C_PAIR_LINENUMBER,

	CD_PAIR_EMPTY_LINE_PREFIX,
};

#define CDFSHOWLINES (1<<1)
#define CDFSHOWSTATUS (1<<2)
typedef struct code {
	_WIDGET_HEADER;
	text_t text;
	int vx, vy;
	int lOff, bOff;
	int scrollX, scrollY;
	wchar_t recentInput[512];
	int iRci;
} *code_t;

typedef struct console {
	_WIDGET_HEADER;
	text_t text;
} *console_t;
