#include <cfk/wdg.h>
#include "../C/C.h"

void cddraw(code_t code);

void
cdupdatecursor(code_t code)
{
	int x, y, w, h;
	
	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	w -= x;
	h -= y;
	w -= code->lOff;
	h -= code->bOff;
	w--;
	h--;
	if(code->vx < 0)
	{
		code->scrollX += code->vx;
		code->scrollX = max(code->scrollX, 0);
		code->vx = 0;
	}
	else if(code->vx > w)
	{
		code->scrollX += code->vx - w;
		code->vx = w;
	}
	if(code->vy < 0)
	{
		code->scrollY += code->vy;
		code->scrollY = max(code->scrollY, 0);
		code->vy = 0;
	}				
	else if(code->vy > h)
	{
		code->scrollY += code->vy - h;
		code->vy = h;
	}
	cddraw(code);
}

void
cddraw(code_t code)
{
	struct txbuffer *buf;
	wchar_t *data;
	int n;
	int x, y;
	int w, h;
	int iLine;
	int lOff;
	bool showLines;
	bool showStatus;
	struct state_info *si;

	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	w -= x;
	h -= y;

	showLines = !!(code->flags & CDFSHOWLINES);
	showStatus = !!(code->flags & CDFSHOWSTATUS);
	
	code->lOff = lOff = 0;
	code->bOff = showStatus;
	h -= showStatus;

	werase(code->window);

	si = c_state_create();
	si->window = code->window;
	si->x = 0;
	si->y = 0;
	si->tx = code->lOff - code->scrollX;
	si->ty = -code->scrollY;
	si->visX = code->vx;
	si->visY = code->vy;
	si->lrMotion = 1;
	for(buf = code->text.first; buf; buf = buf->next)
	for(n = buf->nData, data = buf->data; n; n--, data++)
		sifeed(si, *data);
	sifeed(si, EOF);
	if(showStatus)
		mvwprintw(code->window, h, 0, "%d:%d; %d:%d", si->adjY, si->adjX, code->vy, code->vx);
	wmove(code->window, si->adjY, si->adjX + lOff);
	c_state_free(si);
}

void 
cdleft(code_t code)
{
	code->vx--;
}

void
cdright(code_t code)
{
	code->vx++;
}

void cdup(code_t code)
{
	code->vy--;
}

void cddown(code_t code)
{
	code->vy++;
}

void cdhome(code_t code)
{
	code->vx = INT_MIN;
}

void cdend(code_t code)
{
	code->vx = INT_MAX;
}

void
cdremove(code_t code)
{
	txdelete(&code->text);
}

void
cdleftremove(code_t code)
{
	code->vx--;
	//txleftremove(&code->text);
}

int 
cdproc(code_t code, 
		int event,
		int key)
{
	struct {
		int key;
		void (*cdfunc)(code_t code);
	} table[] = {
		{ KEY_LEFT, cdleft },
		{ KEY_RIGHT, cdright },
		{ KEY_UP, cdup },
		{ KEY_DOWN, cddown },
		{ KEY_HOME, cdhome },
		{ KEY_END, cdend },
		{ KEY_DC, cdremove },
		{ KEY_BACKSPACE, cdleftremove },
	};
	switch(event)
	{
	case WDGINIT:
		txinit(&code->text);
		break;
	case WDGUNINIT:
		txdiscard(&code->text);
		break;
	default:
		for(int i = 0; i < ARRLEN(table); i++)
		{
			if(table[i].key == key)
			{
				table[i].cdfunc(code);
				cdupdatecursor(code);
				return OK;
			}
		}

		code->recentInput[code->iRci++] = key;
		code->iRci %= ARRLEN(code->recentInput);
		txadd(&code->text, key);
		if(key == '\t')
			code->vx += 4 - code->vx % TABSIZE;
		else if(key == '\n')
		{
			code->vx = INT_MIN;
			code->vy++;
		}
		else
			code->vx++;
		cdupdatecursor(code);
	}
	return OK;
}

