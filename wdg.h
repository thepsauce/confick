struct {
	base_t *p;
	int n, sz;
} Bases;
	
base_t
bscreate(void)
{
	base_t base;

	base = malloc(sizeof*base);
	if(!base)
		return ERROR("out of memory", NULL);
	memset(base, 0, sizeof*base);
	base->flags |= BASECREATED;
	return base;
}

int
bsname(base_t base, 
		const char *name)
{
	char *bsName;

	if(!base || !(base->flags & BASECREATED) || !name)
		return ERROR(!base ? "base is null" : 
				!name ? "name is null" : 
				"base was not created yet");
	if(bsfind(name))
		return WARN("base with that name exists already");
	bsName = strdup(name);
	if(!bsName)
		return ERROR("out of memory");

	base->name = bsName;
	base->flags |= BASENAME;
	if(Bases.n + 1 > Bases.sz)
	{
		Bases.sz *= 2;
		Bases.sz++;
		Bases.p = realloc(Bases.p, Bases.sz * sizeof*Bases.p);
	}
	Bases.p[Bases.n++] = base;
	return OK;
}

int 
bssize(base_t base, 
		size_t size)
{
	if(!base || !(base->flags & BASECREATED))
		return ERROR(!base ? "base is null" : "base was not created yet");

	base->flags |= BASESIZE;
	base->size = size;
	return OK;
}

int 
bsproc(base_t base, 
		eventproc_t proc)
{
	if(!base || !(base->flags & BASECREATED))
		return ERROR(!base ? "base is null" : "base was not created yet");

	base->flags |= BASEPROC;
	base->proc = proc;
	return OK;
}

base_t
bsfind(const char *name)
{
	if(!name)
		return WARN("name is null", NULL);
	
	for(int i = 0; i < Bases.n; i++)
		if(!strcmp(Bases.p[i]->name, name))
			return Bases.p[i];
	return NULL;
}

widget_t 
wdgcreate(const char *bsName, 
		int flags)
{
	base_t base;
	WINDOW *win;
	widget_t wdg;

	if(!bsName)
		return ERROR("base name is null", NULL);
	if(!(base = bsfind(bsName)))
		return WARN("base with that name doesn't exist", NULL);
	if((base->flags & BASECOMPLETE) != BASECOMPLETE)
		return WARN("base is not complete", NULL);
	win = newwin(1, 1, 0, 0);
	if(!win)
		return ERROR("could not make new window", NULL);
	keypad(win, TRUE);
	idlok(win, TRUE);
	idcok(win, TRUE);
	immedok(win, FALSE);
	leaveok(win, FALSE);
	scrollok(win, FALSE);
	wdg = malloc(base->size);
	if(!wdg)
		return ERROR("out of memory", NULL);
	memset(wdg, 0, base->size);
	wdg->window = win;
	wdg->base = base;
	wdg->flags = flags;
	if(wdg->base->proc(wdg, WDGINIT))
	{
		free(wdg);
		return WARN("widget refused initializing", NULL);
	}
	return wdg;
}

int
wdgfree(widget_t wdg)
{
	if(!wdg)
		return ERROR("widget is null");

	wdg->base->proc(wdg, WDGUNINIT);
	free(wdg);
	return OK;
}

int 
wdgevent(widget_t wdg, 
		int c)
{
	base_t base;

	if(!wdg)
		return ERROR("widget is null");

	base = wdg->base;
	return base->proc(wdg, c);
}
