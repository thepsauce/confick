widget_t FirstWidget;
widget_t Focus;
int SizeX, SizeY;

int
wdgattach(widget_t wdg, 
		widget_t parent)
{
	widget_t last;
	
	if(!wdg)
		return ERROR("widget is null");

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
	if(Focus)
	{
		wdg->nextFocus = Focus;
		Focus->prevFocus = wdg;
	}
	Focus = wdg;
	return OK;
}

int
wdgdetach(widget_t wdg)
{
	widget_t parent;

	if(!wdg)
		return ERROR("widget is null");

	parent = wdg->parent;
	wdg->parent = NULL;
	if(parent && wdg == parent->child)
		parent->child = wdg->next;
	if(wdg->prev)
		wdg->prev->next = wdg->next;
	if(wdg->next)
		wdg->next->prev = wdg->prev;
	return OK;
}

void
_wdgmgrrelayout(widget_t wdg,
		int minX, 
		int minY, 
		int maxWidth, 
		int maxHeight)
{
	int x, y, w, h;
	int cnt;
	int dir;
	widget_t first, next;
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
		wresize(wdg->window, maxHeight + 1, maxWidth + 1);
		mvwin(wdg->window, minY, minX);
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

void 
wdgmgrupdate(int szX, 
		int szY)
{
	if(SizeX == szX || SizeY == szY)
		return;
	SizeX = szX;
	SizeY = szY;
	_wdgmgrrelayout(NULL, 0, 0, SizeX - 1, SizeY - 1);
}

void 
wdgmgrdiscard(void)
{
	widget_t wdg, prev, parent;

	wdg = FirstWidget;
	while(1)
	{
		// move to deepest child
		while(wdg->child)
		{
			wdg = wdg->child;
			while(wdg->next)
				wdg = wdg->next;
		}
		prev = wdg->prev;
		parent = wdg->parent;
		wdgevent(wdg, WDGUNINIT);
		free(wdg);
		if(wdg == FirstWidget)
			break;
		if(!prev)
		{
			// move back up to parent and isolate children from parent
			wdg = parent;
			wdg->child = NULL;
		}
		else
		{
			// move to sibling and isolate destroyed sibling
			wdg = prev;
			wdg->next = NULL;
		}
	}

	endwin();
	exit(0);
}

void
wdgdraw(widget_t wdg)
{
	widget_t child;
	int cnt;

again:
	wdgevent(wdg, WDGDRAW);
	wnoutrefresh(wdg->window);
	for(child = wdg->child; child; child = child->next)
	{
		wdgevent(child, WDGDRAW);
		wnoutrefresh(child->window);
	}
	for(child = wdg->child; child; child = child->next)
	for(int y = 0, h = 0 /* TODO */; y <= h; y++)
		;//mvaddch(wdg->y + y, child->x + child->width + 1, ' ' | A_DIM | COLOR_PAIR(3));
	if(wdg->next)
	{
		wdg = wdg->next;
		goto again;
	}
	doupdate();
}

void
wdgmgrdraw(void)
{
	wdgdraw(FirstWidget);
}

widget_t
wdgmgrgetfocus(void)
{
	return Focus;
}
