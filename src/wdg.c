#include <cfk/wdg.h>

struct {
	base_t *p;
	int n, sz;
} Bases;
	
base_t
bscreate(void)
{
	base_t base;

	base = safe_malloc(sizeof*base);
	memset(base, 0, sizeof*base);
	base->flags |= BASECREATED;
	return base;
}

int
bsname(base_t base, 
		const char *name)
{
	ERROR(!base, "base is null");
	ERROR(!name, "name is null");
	ERROR(!(base->flags & BASECREATED), "base was not created yet");
	ERROR(strlen(name) > BASENAME_MAX, "string is too long");
	
	WARN(bsfind(name), "base with that name exists already");

	if(Bases.n + 1 > Bases.sz)
	{
		Bases.sz *= 2;
		Bases.sz++;
		Bases.p = safe_realloc(Bases.p, Bases.sz * sizeof*Bases.p);
	}
	Bases.p[Bases.n++] = base;
	strcpy(base->name, name);
	base->flags |= BASENAME;
	return OK;
}

int 
bssize(base_t base, 
		size_t size)
{
	ERROR(!base, "base is null");
	ERROR(!(base->flags & BASECREATED), "base was not created yet");

	base->flags |= BASESIZE;
	base->size = size;
	return OK;
}

int 
bsproc(base_t base, 
		eventproc_t proc)
{
	ERROR(!base, "base is null");
	ERROR(!(base->flags & BASECREATED), "base was not created yet");

	base->flags |= BASEPROC;
	base->proc = proc;
	return OK;
}

base_t
bsfind(const char *name)
{
	ERROR(!name, "name is null", NULL);
	
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

	ERROR(!bsName, "base name is null", NULL);
	ERROR(!(base = bsfind(bsName)), "base with that name doesn't exist", NULL);
	ERROR((base->flags & BASECOMPLETE) != BASECOMPLETE, "base is incomplete", NULL);
	ERROR(!(win = newwin(1, 1, 0, 0)), "could not make new window", NULL);

	keypad(win, TRUE);
	idlok(win, TRUE);
	idcok(win, TRUE);
	immedok(win, FALSE);
	leaveok(win, FALSE);
	scrollok(win, FALSE);
	wdg = safe_malloc(base->size);
	memset(wdg, 0, base->size);
	wdg->window = win;
	wdg->base = base;
	wdg->flags = flags;
	if(wdg->base->proc(wdg, WDGINIT, 0))
	{
		free(wdg);
		WARN(1, "widget refused initializing", NULL);
	}
	return wdg;
}

int
wdgfree(widget_t wdg)
{
	ERROR(!wdg, "widget is null");

	wdg->base->proc(wdg, WDGUNINIT, 0);
	free(wdg);
	return OK;
}

int 
wdgevent(widget_t wdg,
		int event,
		int key)
{
	base_t base;

	ERROR(!wdg, "widget is null");

	base = wdg->base;
	return base->proc(wdg, event, key);
}
