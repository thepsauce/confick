#include <cfk/wdg.h>

int
sisetstate(struct state_info *si, int state)
{	
	ERROR(!si, "state info is null");

	si->state = state;
	return OK;
}

int
sipushstate(struct state_info *si)
{
	ERROR(!si, "state info is null");
	ERROR(sizeof(si->stateStack) == si->iState, "stack was overpushed: ran out of state stack space");

	si->stateStack[si->iState++] = si->state;
	return OK;
}

int
sipushandsetstate(struct state_info *si, int state)
{
	sipushstate(si);
	sisetstate(si, state);
	return OK;
}

int
sipopstate(struct state_info *si)
{
	ERROR(!si, "state info is null");
	ERROR(!si->iState, "stack was overpopped: each pop needs a precedent push");

	si->state = si->stateStack[--si->iState];
	return OK;
}

void
_sicaretnl(struct state_info *si,
		int x,
		int y)
{
	if(y == si->visY)
	{
		si->maxVX = x;
		si->adjX = x;
		si->adjY = y;
	}
	else if(y + 1 == si->visY && si->visX <= 0)
	{
		si->adjX = 0;
		si->adjY = y;
	}
}

void
_sicareteof(struct state_info *si)
{

}

void
_sicaretinc(struct state_info *si,
		int x,
		int y,
		int inc)
{
	if(y == si->visY)
	{
		if(x == si->visX)
		{
			si->adjX = x;
			si->adjY = y;
		}
		else if(x + inc <= si->visX)
		{
			if(si->lrMotion)
			{
				si->adjX = x + inc;
				si->adjY = y;
			}
			else
			{
				si->adjX = x;
				si->adjY = y;
			}
		}
	}
}

#define siaddextra siadd

int
siadd(struct state_info *si, wchar_t w, attr_t a, short color_pair)
{
	wchar_t ws[2];
	cchar_t cc;
	int x, y;

	ERROR(!si, "state info is null");

	ws[0] = w;
	ws[1] = 0;
	setcchar(&cc, ws, a, color_pair, NULL);
	x = si->x + si->tx;
	y = si->y + si->ty;
	mvwadd_wch(si->window, y, x, &cc);
	if(w == L'\t')
	{
		int i = 4 - si->x % 4;
		si->x += i;
		_sicaretinc(si, x, y, i);
	}
	else if(w == L'\n')
	{
		si->x = 0;
		si->y++;
		_sicaretnl(si, x, y);
	}
	else
	{
		si->x++;
		_sicaretinc(si, x, y, 1);
	}
	return OK;
}

// a word shall not contain a new line or tab character (anything that's not one cell wide)
int
siaddword(struct state_info *si, const wchar_t *word, attr_t a, short color_pair)
{
	wchar_t ws[2];
	cchar_t cc;
	int nWord;
	int x, y;

	ERROR(!si, "state info is null");

	nWord = wcslen(word);
	ws[1] = 0;
	x = si->x + si->tx;
	y = si->y + si->ty;
	for(int i = 0; i < nWord; i++)
	{
		ws[0] = word[i];
		setcchar(&cc, ws, a, color_pair, NULL);
		mvwadd_wch(si->window, y, x, &cc);
		_sicaretinc(si, x, y, 1);
		x++;
	}
	si->x += nWord;
	return OK;
}

int
sifeed(struct state_info *si, wchar_t w)
{
	ERROR(!si, "state is null");

	si->w = w;
	while(si->stateFuncs[si->state](si));
	return OK;
}
