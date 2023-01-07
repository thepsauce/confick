Widget *Widgets;
int WidgetCnt, WidgetCap;
int SizeX, SizeY;
int Focus;
int Rotation;

void _wdgmgrupdate(void)
{
	int x, y, w, h;
	int wdgc;
	Widget *wdgs, wdg;
	int dir;

	if(!SizeX || !SizeY)
		return;
	
	dir = 1;
	wdgc = WidgetCnt;
	wdgs = Widgets;
	switch(Rotation)
	{
	case 2:
		dir = -1;
		wdgs += WidgetCnt - 1;
	case 0:
		x = 0;
		w = (SizeX - 1) / WidgetCnt;
		if(w > 5)
		{
			for(; --wdgc; wdgs += dir)
			{
				wdg = *wdgs;
				wdg->x = x + 5;
				wdg->y = 0;
				wdg->width = w - 5 - 1;
				wdg->height = SizeY - 1 - 1;
				x += w + 1;
			}
		}
		wdg = *wdgs;
		wdg->x = x + 5;
		wdg->y = 0;
		wdg->width = SizeX - 1 - (x + 5);
		wdg->height = SizeY - 1 - 1;
		break;
	case 3:
		dir = -1;
		wdgs += WidgetCnt - 1;
	case 1:
		y = 0;
		h = (SizeY - 1) / WidgetCnt;
		if(h > 1)
		{
			for(; --wdgc; wdgs += dir)
			{
				wdg = *wdgs;
				wdg->x = 5;
				wdg->y = y;
				wdg->width = SizeX - 5 - 1;
				wdg->height = h - 1;
				y += h + 1;
			}
		}
		wdg = *wdgs;
		wdg->x = 5;
		wdg->y = y;
		wdg->width = SizeX - 1 - 5;
		wdg->height = SizeY - 1 - y - 1;
		break;
	}
}

void wdgmgrupdate(int szX, int szY)
{
	if(SizeX == szX || SizeY == szY)
		return;
	SizeX = szX;
	SizeY = szY;
	_wdgmgrupdate();
}

void wdgmgradd(Widget wdg)
{
	if(WidgetCnt + 1 > WidgetCap)
	{
		WidgetCap *= 2;
		WidgetCap++;
		Widgets = realloc(Widgets, WidgetCap * sizeof*Widgets);
	}
	Focus = WidgetCnt;
	Widgets[WidgetCnt++] = wdg;
	_wdgmgrupdate();
}

void wdgmgrremove(Widget wdg)
{
	Widget *wdgs;
	int wdgc;
	
	for(wdgs = Widgets, wdgc = WidgetCnt; wdgc; wdgs++)
	{
		wdgc--;
		if(*wdgs == wdg)
		{
			memmove(wdgs, wdgs + 1, wdgc * sizeof*wdgs);
			WidgetCnt--;
			if(Focus == WidgetCnt)
				Focus--;
			if(!WidgetCnt)
				wdgmgrdiscard();
			else
				_wdgmgrupdate();
			break;
		}
	}
}

void wdgmgrdiscard(void)
{
	for(int t = 0; t < WidgetCnt; t++)
		wdgfree(Widgets[t]);
	free(Widgets);
	Widgets = NULL;
	WidgetCnt = WidgetCap = 0;
	endwin();
	exit(0);
}

void wdgmgrdraw(void)
{
	int w;
	int visX;

	for(int t = 0; t < WidgetCnt; t++)
		wdgevent(Widgets[t], WDGDRAW);
	// draw barrier
	attron(A_DIM | COLOR_PAIR(3));
	if(!(Rotation % 2))
	{
		w = (SizeX - 1) / WidgetCnt;
		for(int t = 1; t < WidgetCnt; t++)
			for(int y = 0; y < SizeY; y++)
				mvaddch(y, w * t + t - 1, ' ');
	}
	attroff(A_DIM | COLOR_PAIR(3));
	// draw cursor
	//visX = _wdgviscurx(Widgets[Focus], Widgets[Focus]->curX, Widgets[Focus]->curY);
	//move(Widgets[Focus]->y + Widgets[Focus]->curY - Widgets[Focus]->scrollY, Widgets[Focus]->x + visX);
}

Widget wdgmgrgetfocus(void)
{
	return Widgets[Focus];
}

void wdgmgrfocusnext(void)
{
	Focus++;
	Focus %= WidgetCnt;
}

void wdgmgrrotate(void)
{
	Rotation++;
	Rotation %= 4;
	_wdgmgrupdate();
}
