struct state_info {
	WINDOW *window;
	int tx, ty;
	int x, y;
	wchar_t w;
	int state;
	bool (**stateFuncs)(struct state_info *si);
	int stateStack[64];
	int iState;
};

void
sisetstate(struct state_info *si, int state)
{
	si->state = state;
}

void
sipushstate(struct state_info *si)
{
	assert(sizeof(si->stateStack) != si->iState && "stack was overpushed: ran out of state stack space");
	si->stateStack[si->iState++] = si->state;
}

void
sipushandsetstate(struct state_info *si, int state)
{
	sipushstate(si);
	sisetstate(si, state);
}

void
sipopstate(struct state_info *si)
{
	assert(si->iState && "stack was overpopped: each pop needs a precedent push");
	si->state = si->stateStack[--si->iState];
}

void
siadd(struct state_info *si, wchar_t w, attr_t a, short color_pair)
{
	wchar_t ws[2];
	cchar_t cc;

	ws[0] = w;
	ws[1] = 0;
	setcchar(&cc, ws, a, color_pair, NULL);
	mvwadd_wch(si->window, si->y + si->ty, si->x + si->tx, &cc);
	if(w == L'\t')
		si->x += 4 - si->x % 4;
	else if(w == L'\n')
	{
		si->x = 0;
		si->y++;
	}
	else
		si->x++;
}

void
siaddword(struct state_info *si, const wchar_t *word, attr_t a, short color_pair)
{
	wchar_t ws[2];
	cchar_t cc;
	int nWord;

	nWord = wcslen(word);
	ws[1] = 0;
	for(int i = 0; i < nWord; i++)
	{
		ws[0] = word[i];
		setcchar(&cc, ws, a, color_pair, NULL);
		mvwadd_wch(si->window, si->y + si->ty, si->x + si->tx, &cc);
		si->x++;
	}
}

void
sifeed(struct state_info *si, wchar_t w)
{
	si->w = w;
	while(si->stateFuncs[si->state](si));
}
