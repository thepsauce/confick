#ifndef INCLUDED_CFK_WDG_H
#define INCLUDED_CFK_WDG_H

#define _XOPEN_SOURCE_EXTENDED
#define _GNU_SOURCE

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))

#define min(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__a:__b; \
})

#define max(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__b:__a; \
})

#define CURSEDRGB(color) ((color>>16)&0xFF)*1000/256, ((color>>8)&0xFF)*1000/256, (color&0xFF)*1000/256

struct widget;

// #define OK 0 // curses.h already defines an OK with value 0
#define WARN(cond, msg, ...) if(cond) return ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); 1; __VA_ARGS__; })
#define ERROR(cond, msg, ...) if(cond) return ({ fprintf(stderr, "%s:%s:%d > %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); exit(-1); -1; __VA_ARGS__; })

int safe_init(void);
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
int wdgattach(widget_t wdg, widget_t parent);

// widget manager
void wdgmgrdiscard(void);
void wdgmgrupdate(int szX, int szY);
void wdgmgrdraw(void);
void wdgmgradd(widget_t wdg);
void wdgmgrremove(widget_t wdg);
widget_t wdgmgrgetfocus(void);
void wdgmgrfocusnext(void);
void wdgmgrrotate(void);

#define TXFLINECROSSING (1<<1)
#define TXBUFSIZE 2048
struct txbuffer {
	wchar_t data[TXBUFSIZE];
	int nData;
	struct txbuffer *prev, *next;
	void *meta;
};
typedef struct text {
	int flags;
	char *fileName;
	size_t cursor;
	struct txbuffer *first, *cur;
	int iCur;
	int nBuffers, szBuffers;
} text_t;
int txinit(text_t *text);
int txdiscard(text_t *text);
int txclear(text_t *text);
int txopen(text_t *text, const char *fileName);
int txsave(text_t *text);
int txaddnstr(text_t *text, const wchar_t *str, int n);
int txadd(text_t *text, int c);
int txdelete(text_t *text);

struct state_info {
#define _STATE_INFO_HEADER WINDOW *window; /* cursor information */ \
	int tx, ty; /* translation of the cursor (this include scrolling and other) */ \
	int x, y; /* position of the cursor relative to the text origin */ \
	int visX, visY; /* visual cursor position */ \
	int maxVX; /* maximum visual x position of current line */ \
	int adjX, adjY; /* adjusted visual cursor position */ \
	int curMemX, curMemY, memX, memY; /* position inside memory buffer (line system) */ \
	int curMemIndex, memIndex; /* position inside memory buffer (applicable for any system) */ \
	bool lrMotion; /* cursor moved from left to right */ \
	wchar_t w; \
	int state; \
	bool (**stateFuncs)(struct state_info *si); \
	int stateStack[64]; \
	int iState
	_STATE_INFO_HEADER;
};
int sisetstate(struct state_info *si, int state);
int sipushstate(struct state_info *si);
int sipopstate(struct state_info *si);
int sipushandsetstate(struct state_info *si, int state);
#define siaddextra siadd
int siadd(struct state_info *si, int c, attr_t a, short color_pair);
int siaddword(struct state_info *si, const wchar_t *word, attr_t a, short color_pair);
int sifeed(struct state_info *si, int c);

// c flavor
enum {
	C_PAIR_TEXT = 5,
	C_PAIR_CHAR,
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
int cdproc(code_t code, int event, int key);

typedef struct console {
	_WIDGET_HEADER;
	text_t text;
} *console_t;

#endif
