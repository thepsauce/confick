void cdinit(CodeWidget cd)
{
	cd->cur = cd->first = malloc(sizeof*cd->first);
	memset(cd->first, 0, sizeof*cd->first);
}

void cduninit(CodeWidget cd)
{
	struct codetoken *tok, *next;

	for(tok = cd->first, next = tok->next; tok; tok = next, next = tok->next)
	{
		free(tok->str);
		free(tok);
	}
}

CodeToken _cdsplittoken(CodeWidget cd, int type)
{
	CodeToken tok;

	if(!cd->cur->len)
	{
		cd->cur->type = type;
		return cd->cur;
	}
	tok = malloc(sizeof*tok);
	tok->type = type;
	tok->str = NULL;
	tok->len = 0;
	if(!cd->cursor)
	{
		// insert token before current
		// changes prev > cd->cur > next
		// to prev > tok > cd->cur > next
		if(cd->cur->prev)
		{
			cd->cur->prev->next = tok;
			tok->prev = cd->cur->prev;
		}
		else
		{
			tok->prev = NULL;
			cd->first = tok;
		}
		cd->cur->prev = tok;
		tok->next = cd->cur;
	}
	else if(cd->cursor == cd->cur->len)
	{
		// insert token after current
		// changes prev > cd->cur > next
		// to prev > cd->cur > tok > next
		if(cd->cur->next)
		{
			cd->cur->next->prev = tok;
			tok->next = cd->cur->next;
		}
		else
			tok->next = NULL;
		cd->cur->next = tok;
		tok->prev = cd->cur;
	}
	else 
	{
		// split the token in two
		// changes prev > cd->cur > next
		// to prev > t1 > tok > t2 > next
		CodeToken t1, t2;
		t1 = cd->cur;
		t2 = malloc(sizeof*tok);
		t2->type = t1->type;
		t2->len = t1->len - cd->cursor;
		if(t1->str)
		{
			t2->str = malloc(t2->len);
			memcpy(t2->str, t1->str + cd->cursor, t2->len);
		}
		t1->len = cd->cursor;
		if(t1->next)
		{
			t1->next->prev = t2;
			t2->next = t1->next;	
		}
		else
			t2->next = NULL;
		t1->next = tok;
		tok->prev = t1;
		tok->next = t2;
		t2->prev = tok;
	}
	return tok;
}

void cdputc(CodeWidget cd, int c)
{
	CodeToken tok;
	
	if(c < 0 || c > 255)
		return;
	tok = cd->cur;
	switch(c)
	{
	case '\n':
		if(tok->type == TTNEWLINE)
		{
			tok->len++;
			cd->cursor++;
		}
		else
		{
			tok = _cdsplittoken(cd, TTNEWLINE);
			tok->len = 1;
			cd->cursor = 1;
		}
		break;
	case ' ':
		if(tok->type == TTSPACE)
		{
			tok->len++;
			cd->cursor++;
		}
		else
		{
			tok = _cdsplittoken(cd, TTSPACE);
			tok->len = 1;
			cd->cursor = 1;
		}
		break;
	case '\t':
		if(tok->type == TTINDENT)
		{
			tok->len++;
			cd->cursor++;
		}
		else
		{
			tok = _cdsplittoken(cd, TTINDENT);
			tok->len = 1;
			cd->cursor = 1;
		}
		break;
	default:
		tok = _cdsplittoken(cd, TTNONE);
		tok->str = malloc(1);
		tok->len = 1;
		tok->str[0] = c;
		cd->cursor = 1;
	}
	cd->cur = tok;
}

void cddraw(CodeWidget cd, int c)
{
	struct codetoken *tok;
	int x, y;

	x = cd->x;
	y = cd->y;
	for(tok = cd->first; tok; tok = tok->next)
	{
		switch(tok->type)
		{
		case TTNEWLINE:
			x = 0;
			y += tok->len;
			break;
		case TTSPACE:
			x += tok->len;
			break;
		case TTINDENT:
			x += tok->len * 4; // TODO: store the tab constant somewhere
			break;
		default:
			for(int i = 0; i < tok->len; i++)
			{
				mvaddch(y, x, tok->str[i]);
				x++;
			}
		}
	}
}

