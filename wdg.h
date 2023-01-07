struct {
	Widgetclass *data;
	int cnt, cap;
} WdgClasses;

void wdgaddclass(Widgetclass wc)
{
	if(WdgClasses.cnt + 1 > WdgClasses.cap)
	{
		WdgClasses.cap *= 2;
		WdgClasses.cap++;
		WdgClasses.data = realloc(WdgClasses.data, WdgClasses.cap * sizeof*WdgClasses.data);
	}
	WdgClasses.data[WdgClasses.cnt++] = wc;
}

Widgetclass wdgfindclass(const char *name)
{
	int wcc;
	Widgetclass *wcs, wc;
	for(wcc = WdgClasses.cnt, wcs = WdgClasses.data; wcc; wcc--, wcs++)
	{
		wc = *wcs;
		if(!strcmp(wc->name, name))
			return wc;
	}
	return NULL;
}

Widget wdgcreate(const char *clsName)
{
	Widgetclass wc;
	Widget wdg;

	if(!clsName)
	{
		if(!(wc = wdgfindclass("#NULLCLASS")))
		{
			wc = malloc(sizeof*wc);
			wc->name = strdup("#NULLCLASS");
			wc->size = sizeof(struct widget);
			wc->proc = NULL;
			wc->motionCnt = 0;
			wdgaddclass(wc);
		}
	}
	else
	{
		wc = wdgfindclass(clsName);
		if(!wc)
			return NULL;
	}
	wdg = malloc(wc->size);
	memset(wdg, 0, wc->size);
	wdg->wc = wc;
	wdgevent(wdg, WDGINIT);
	return wdg;
}

void wdgfree(Widget wdg)
{
	wdgevent(wdg, WDGUNINIT);
	free(wdg);
}

void wdgevent(Widget wdg, int eId)
{
	Widgetclass wc;

	wc = wdg->wc;
	for(int i = 0; i < wc->motionCnt; i++)
	{
		if(wc->motions[i].id == eId)
		{
			wc->motions[i].motion(wdg);
			return;
		}
	}
	if(wc->proc)
		wc->proc(wdg, eId);
}
