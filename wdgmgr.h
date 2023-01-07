Widget FirstWidget;
Widget *Widgets;
int WidgetCnt, WidgetCap;
Widget Focus;
int SizeX, SizeY;

void wdgattach(Widget wdg, Widget parent)
{
	Widget last;
	
	wdg->parent = parent;
	if(!parent)
	{
		if(!FirstWidget)
			FirstWidget = wdg;
		else
		{
			for(last = FirstWidget; last->next; last = last->next);
			wdg->prev = last;
			last->next = wdg;
		}
	}
	else
	{
		if(!parent->child)
			parent->child = wdg;
		else
		{
			for(last = FirstWidget; last->next; last = last->next);
			wdg->prev = last;
			last->next = wdg;
		}
	}
	if(WidgetCnt + 1 > WidgetCap)
	{
		WidgetCap *= 2;
		WidgetCap++;
		Widgets = realloc(Widgets, WidgetCap * sizeof*Widgets);
	}
	Widgets[WidgetCnt++] = wdg;
	Focus = wdg;
}

void wdgdetach(Widget wdg)
{
	Widget parent;

	parent = wdg->parent;
	wdg->parent = NULL;
	if(parent && wdg == parent->child)
		parent->child = wdg->next;
	if(wdg->prev)
		wdg->prev->next = wdg->next;
	if(wdg->next)
		wdg->next->prev = wdg->prev;
}

void _wdgmgrrelayout(Widget cont, int minX, int minY, int maxWidth, int maxHeight)
{
	int x, y, w, h;
	int cnt;
	int dir;
	Widget first, next;
	int layout;

	if(!cont)
	{
		if(!FirstWidget)
			return;
		first = FirstWidget;
		layout = 1;
	}
	else
	{
		cont->x = minX;
		cont->y = minY;
		cont->width = maxWidth;
		cont->height = maxHeight;
		if(!cont->child)
			return;
		first = cont->child;
		layout = 0;
	}
	// get number of children
	for(cnt = 1, cont = first; cont->next; cnt++, cont = cont->next);
	dir = -1;
	switch(layout)
	{
	case 0:
		dir = 1;
		cont = first;
	case 2:
		x = minX;
		w = maxWidth / cnt;
		if(w > 5)
		{
			for(; --cnt; cont = dir < 0 ? cont->prev : cont->next)
			{
				_wdgmgrrelayout(cont, x + 5, minY, w - 5, maxHeight - 1);
				x += w + 1;
			}
		}
		_wdgmgrrelayout(cont, x + 5, 0, maxWidth - (x + 5), maxHeight - 1);
		break;
	case 1:
		dir = 1;
		cont = first;
	case 3:
		y = minY;
		h = maxHeight / cnt;
		if(h > 1)
		{
			for(; --cnt; cont = dir < 0 ? cont->prev : cont->next)
			{
				_wdgmgrrelayout(cont, minX + 5, y, maxWidth - 5, h);
				y += h + 1;
			}
		}
		_wdgmgrrelayout(cont, minX + 5, y, maxWidth - 5, maxHeight - y);
		break;
	}
}

void wdgmgrupdate(int szX, int szY)
{
	if(SizeX == szX || SizeY == szY)
		return;
	SizeX = szX;
	SizeY = szY;
	_wdgmgrrelayout(NULL, 0, 0, SizeX - 1, SizeY - 1);
}

void wdgmgrdiscard(void)
{
	endwin();
	exit(0);
}

void wdgmgrdraw(void)
{
	int w;
	int visX;

	for(int i = 0; i < WidgetCnt; i++)
		wdgevent(Widgets[i], WDGDRAW);
	// draw barrier
	/*attron(A_DIM | COLOR_PAIR(3));
	if(!(Rotation % 2))
	{
		w =  / WidgetCnt;
		for(int i = 1; i < WidgetCnt; i++)
			for(int y = 0; y < SizeY; y++)
				mvaddch(y, w * t + t - 1, ' ');
	}
	attroff(A_DIM | COLOR_PAIR(3));*/
	// draw cursor
	wdgevent(Focus, WDGCURSORDRAW);
}

Widget wdgmgrgetfocus(void)
{
	return Focus;
}
