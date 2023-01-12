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
	memset(tok, 0, sizeof*tok);
	tok->type = type;
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
			cd->first = tok;
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
		memset(t2, 0, sizeof*tok);
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
		t1->next = tok;
		tok->prev = t1;
		tok->next = t2;
		t2->prev = tok;
	}
	return tok;
}

void _cdtokinsert(CodeToken tok, int index, int c)
{
	tok->str = realloc(tok->str, tok->len + 1);
	tok->str[tok->len] = c;
	tok->len++;
}

void cdleft(CodeWidget cd)
{
	if(cd->cursor)
		cd->cursor--;
	else if(cd->cur->prev)
	{
		cd->cur = cd->cur->prev;
		cd->cursor = cd->cur->len - 1;
	}
}

void cdright(CodeWidget cd)
{
	if(cd->cursor != cd->cur->len)
		cd->cursor++;
	else if(cd->cur->next)
	{
		cd->cur = cd->cur->next;
		cd->cursor = 1;
	}
}

void cddelete(CodeWidget cd)
{
	CodeToken tok;

	if(cd->cursor == cd->cur->len)
	{
		tok = cd->cur->next;
		if(tok)
		{
			if(tok->len == 1)
			{
				if(tok->next)
					tok->next->prev = cd->cur;
				cd->cur->next = tok->next;
				free(tok);
			}
			else
			{
				memmove(tok->str, tok->str + 1, tok->len - 1);
				tok->len--;
			}
		}
	}
	else
	{
		tok = cd->cur;
		if(tok->len == 1)
		{
			if(tok->next)
				tok->next->prev = tok->prev;
			if(tok->prev)
				tok->prev->next = tok->next;
			free(tok);
		}
		else
		{
			tok->len--;
			memmove(tok->str + cd->cursor, tok->str + cd->cursor + 1, tok->len - cd->cursor);
		}
	}
}

void cdbackdelete(CodeWidget cd)
{
	cdleft(cd);
	cddelete(cd);
}

void cdputc(CodeWidget cd, int c)
{
	CodeToken tok;
	const struct {
		const char *word;
		int type;
	} keywords[] = {
		{ "auto", TTKEYWORD1 },
		{ "break", TTKEYWORD1 },
		{ "case", TTKEYWORD1 },
		{ "char", TTKEYWORD1 },
		{ "const", TTKEYWORD1 },
		{ "continue", TTKEYWORD1 },
		{ "default", TTKEYWORD1 },
		{ "do", TTKEYWORD1 },
		{ "double", TTKEYWORD1 },
		{ "else", TTKEYWORD1 },
		{ "enum", TTKEYWORD1 },
		{ "extern", TTKEYWORD1 },
		{ "float", TTKEYWORD1 },
		{ "for", TTKEYWORD1 },
		{ "goto", TTKEYWORD1 },
		{ "if", TTKEYWORD1 },
		{ "int", TTKEYWORD1 },
		{ "long", TTKEYWORD1 },
		{ "register", TTKEYWORD1 },
		{ "return", TTKEYWORD1 },
		{ "short", TTKEYWORD1 },
		{ "signed", TTKEYWORD1 },
		{ "sizeof", TTKEYWORD1 },
		{ "static", TTKEYWORD1 },
		{ "struct", TTKEYWORD1 },
		{ "switch", TTKEYWORD1 },
		{ "typedef", TTKEYWORD1 },
		{ "typeof", TTKEYWORD1 },
		{ "union", TTKEYWORD1 },
		{ "unsigned", TTKEYWORD1 },
		{ "void", TTKEYWORD1 },
		{ "volatile", TTKEYWORD1 },
		{ "white", TTKEYWORD1 },
	};
	
	if(c < 0 || c > 255)
		return;
	tok = cd->cur;
	switch(c)
	{
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '_': case '$':
		if(tok->type == TTWORD || tok->type == TTKEYWORD1)
		{
			_cdtokinsert(tok, cd->cursor, c);
			cd->cursor++;
		}
		else
		{
			tok = _cdsplittoken(cd, TTWORD);
			tok->str = malloc(1);
			tok->len = 1;
			tok->str[0] = c;
			cd->cursor = 1;
		}
		tok->type = TTWORD;
		for(int i = 0; i < ARRLEN(keywords); i++)
			if(tok->len == strlen(keywords[i].word) && !memcmp(tok->str, keywords[i].word, tok->len))
			{
				tok->type = TTKEYWORD1;
				break;
			}
		break;
	case '0' ... '9':
		if(tok->type == TTNUMBER)
		{
			_cdtokinsert(tok, cd->cursor, c);
			cd->cursor++;
		}
		else
		{
			tok = _cdsplittoken(cd, TTNUMBER);
			tok->str = malloc(1);
			tok->len = 1;
			tok->str[0] = c;
			cd->cursor = 1;
		}
		break;
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
	static int colors[255];
	struct codetoken *tok;
	int x, y;
	int curX, curY;

	colors[TTNUMBER] = COLOR_PAIR(2);
	colors[TTWORD] = COLOR_PAIR(3);
	colors[TTKEYWORD1] = COLOR_PAIR(4);

	x = cd->x;
	y = cd->y;
	for(tok = cd->first; tok; tok = tok->next)
	{
		switch(tok->type)
		{
		case TTNEWLINE:
			if(tok == cd->cur)
			{
				curX = 0;
				curY = y + cd->cursor;
			}
			x = 0;
			y += tok->len;
			break;
		case TTSPACE:
			if(tok == cd->cur)
			{
				curX = x + cd->cursor;
				curY = y;
			}
			x += tok->len;
			break;
		case TTINDENT:
			if(tok == cd->cur)
			{
				curX = x + cd->cursor * 4;
				curY = y;
			}
			x += tok->len * 4; // TODO: store the tab constant somewhere
			break;
		default:
			if(tok == cd->cur)
			{
				curX = x + cd->cursor;
				curY = y;
			}
			attron(colors[tok->type]);
			for(int i = 0; i < tok->len; i++)
			{
				mvaddch(y, x, tok->str[i]);
				x++;
			}
			attroff(colors[tok->type]);
		}
	}
	move(curY, curX);
}

