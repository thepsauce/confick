Widget FirstWidget;
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
			for(last = parent->child; last->next; last = last->next);
			wdg->prev = last;
			last->next = wdg;
		}
	}
	if(wdg->wc->name[0] == '#')
		return;
	if(Focus)
	{
		wdg->nextFocus = Focus;
		Focus->prevFocus = wdg;
	}
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

void _wdgmgrrelayout(Widget wdg, int minX, int minY, int maxWidth, int maxHeight)
{
	int x, y, w, h;
	int cnt;
	int dir;
	Widget first, next;
	struct insets insets;
	int layout;

	if(!wdg)
	{
		if(!FirstWidget)
			return;
		first = FirstWidget;
		layout = 1;
	}
	else
	{
		insets = wdg->wc->insets;
		wdg->x = minX + insets.left;
		wdg->y = minY + insets.top;
		wdg->width = maxWidth - insets.left - insets.right;
		wdg->height = maxHeight - insets.top - insets.bottom;
		if(!wdg->child)
			return;
		first = wdg->child;
		layout = 0;
	}
	// get number of children
	for(cnt = 1, wdg = first; wdg->next; cnt++, wdg = wdg->next);
	dir = -1;
	switch(layout)
	{
	case 0:
		dir = 1;
		wdg = first;
	case 2:
		x = minX;
		w = maxWidth / cnt;
		if(w > 5)
		{
			for(; --cnt; wdg = dir < 0 ? wdg->prev : wdg->next)
			{
				_wdgmgrrelayout(wdg, x, minY, w - 1, maxHeight);
				x += w + 1;
			}
		}
		_wdgmgrrelayout(wdg, x, minY, maxWidth - x, maxHeight);
		break;
	case 1:
		dir = 1;
		wdg = first;
	case 3:
		y = minY;
		h = maxHeight / cnt;
		if(h > 1)
		{
			for(; --cnt; wdg = dir < 0 ? wdg->prev : wdg->next)
			{
				_wdgmgrrelayout(wdg, minX, y, maxWidth, h);
				y += h + 1;
			}
		}
		_wdgmgrrelayout(wdg, minX, y, maxWidth, maxHeight - y);
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

void wdgdraw(Widget wdg)
{
	Widget child;
	int cnt;

again:
	wdgevent(wdg, WDGDRAW);
	for(child = wdg->child; child; child = child->next)
		wdgevent(child,  WDGDRAW);
	attron(A_DIM | COLOR_PAIR(3));
	for(child = wdg->child; child; child = child->next)
	for(int y = 0; y <= wdg->height; y++)
		mvaddch(wdg->y + y, child->x + child->width + 1, ' ');
	attroff(A_DIM | COLOR_PAIR(3));
	if(wdg->next)
	{
		wdg = wdg->next;
		goto again;
	}
}

void wdgmgrdraw(void)
{
	wdgdraw(FirstWidget);
	wdgevent(Focus, WDGCURSORDRAW);
}

Widget wdgmgrgetfocus(void)
{
	return Focus;
}
