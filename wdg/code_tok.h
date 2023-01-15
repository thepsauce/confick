void cdinit(CodeWidget cd)
{
	cd->cur = cd->first = malloc(sizeof*cd->first);
	memset(cd->first, 0, sizeof*cd->first);
}

void cdtokfree(CodeToken tok)
{
	if(tok->type & TTUSESTR)
		free(tok->str);
	free(tok);
}

void cduninit(CodeWidget cd)
{
	cdclear(cd);
}

void cdclear(CodeWidget cd)
{
	struct codetoken *tok, *next;

	for(tok = cd->first; tok; tok = next)
	{
		next = tok->next;
		cdtokfree(tok);
	}
	cd->cur = cd->first = malloc(sizeof*cd->first);
	memset(cd->first, 0, sizeof*cd->first);
}

void cdopen(CodeWidget cd, const char *fileName)
{
	FILE *file;
	int c;

	cdclear(cd);
	cd->fileName = strdup(fileName);
	if(!(file = fopen(fileName, "r")))
    	return;
	while((c = fgetc(file)) != EOF)
		cdputc(cd, c);
    fclose(file);
}

void cdsave(CodeWidget cd)
{
	FILE *file;
	CodeToken tok;

    file = fopen(cd->fileName, "w");
    assert(file);
	for(tok = cd->cur; tok; tok = tok->next)
	{
		switch(tok->type)
		{
		case TTNEWLINE:
			for(int i = 0; i < tok->len; i++)
				fputc('\n', file);
			break;
		case TTSPACE:
			for(int i = 0; i < tok->len; i++)
				fputc(' ', file);
			break;
		case TTINDENT:
			for(int i = 0; i < tok->len; i++)
				fputc('\t', file);
			break;
		case TTWORD:
			fwrite(tok->str, 1, tok->len, file);
			break;
		default:
			fputc(tok->c, file);
		}
	}
    fclose(file);
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
	memmove(tok->str + index + 1, tok->str + index, tok->len - index);
	tok->str[index] = c;
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

void cdup(CodeWidget cd)
{
	if(cd->scrollY)
		cd->scrollY--;
}

void cddown(CodeWidget cd)
{
	cd->scrollY++;
}

void cdhome(CodeWidget cd)
{
	CodeToken tok;

	tok = cd->cur;
	if(tok->type != TTNEWLINE || !cd->cursor)
	{
		for(; tok->prev && tok->prev->type != TTNEWLINE; tok = tok->prev);
		cd->cursor = 0;
		cd->cur = tok;
	}
}

void cdend(CodeWidget cd)
{
	CodeToken tok;

	tok = cd->cur;
	if(tok->type != TTNEWLINE || cd->cursor == tok->len)
	{
		for(; tok->next && tok->next->type != TTNEWLINE; tok = tok->next);
		cd->cursor = tok->len;
		cd->cur = tok;
	}
}

void cddelete(CodeWidget cd)
{
	CodeToken tok;
	CodeToken next;

	tok = cd->cur;
	next = tok->next;

	if(cd->cursor == tok->len)
	{
		if(next)
		{
			if(next->len == 1)
			{
				// example case: 32>a>;>po
				// the ; is deleted (> represent links)
				if(next->next)
					next->next->prev = tok;
				tok->next = next->next;
				cdtokfree(next);
				next = tok->next;
				if(next && (tok->type & TTFUSE) && tok->type == next->type)
				{
					if(next->next)
					{
						next->next->prev = tok;
						tok->next = next->next;
					}
					else
						tok->next = NULL;
					if(tok->type & TTUSESTR)
					{
						tok->str = realloc(tok->str, tok->len + next->len);
						memcpy(tok->str + tok->len, next->str, next->len);
					}
					tok->len += next->len;
					cdtokfree(next);
				}
			}
			else
			{
				next->len--;
				if(next->type & TTUSESTR)
					memmove(next->str, next->str + 1, next->len);
			}
		}
	}
	else
	{
		if(tok->len == 1)
		{
			if(tok->prev)
			{
				if(next && (next->type & TTFUSE) && next->type == tok->prev->type)
				{
					if(next->next)
						next->next->prev = tok->prev;
					tok->prev->next = next->next;
					if(tok->prev->type & TTUSESTR)
					{
						tok->prev->str = realloc(tok->prev->str, tok->prev->len + next->len);
						memcpy(tok->prev->str + tok->prev->len, next->str, next->len);
					}
					cd->cur = tok->prev;
					cd->cursor = tok->prev->len;
					tok->prev->len += next->len;
					cdtokfree(next);
				}
				else
				{
					tok->prev->next = next;
					cd->cur = tok->prev;
					cd->cursor = cd->cur->len;
				}
				cdtokfree(tok);
			}
			else if(next)
			{
				cd->cur = next;
				next->prev = NULL;
				cdtokfree(tok);
			}
			else
			{
				memset(tok, 0, sizeof*tok);
			}
		}
		else
		{
			tok->len--;
			if(tok->type & TTUSESTR)
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
	
	if(c < 0 || c > 255)
		return;
	tok = cd->cur;
	switch(c)
	{
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '_': case '$':
		if(tok->type == TTWORD)
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
		tok->c = c;
		tok->len = 1;
		cd->cursor = 1;
	}
	cd->cur = tok;
}

#define FINDTOKLEFT 0
#define FINDTOKRIGHT 1
CodeToken cdfindtok(CodeToken tok, int ignMaskCnt, const int *ignMask, int find, int dir)
{
#define INNERLOOP { \
	__label__ next; \
	if(tok->type == find) \
		return tok; \
	for(int i = 0; i < ignMaskCnt; i++) \
		if(ignMask[i] == tok->type) \
			goto next; \
	return NULL; \
	next:; }
	if(dir == FINDTOKLEFT)
	{
		for(tok = tok->prev; tok; tok = tok->prev)
			INNERLOOP
	}
	else
	{
		for(tok = tok->next; tok; tok = tok->next)
			INNERLOOP
	}
#undef INNERLOOP
}

void cddraw(CodeWidget cd, int c)
{
	CodeToken tok, f;
	int x, y;
	int curX, curY;
	const int spaceMask[] = { TTSPACE, TTINDENT, TTNEWLINE };
	const char *keywords[] = {
		"auto",
		"break"
		"case",
		"char",
		"const",
		"continue",
		"default",
		"do",
		"double",
		"else",
		"enum",
		"extern",
		"float",
		"for",
		"goto",
		"if",
		"int",
		"long",
		"register",
		"return",
		"short",
		"signed",
		"sizeof",
		"static",
		"struct",
		"switch",
		"typedef",
		"typeof",
		"union",
		"unsigned",
		"void",
		"volatile",
		"while",

		"FILE",
	};
	const char chars[] = {
		'[', ']', '(', ')', '{', '}', '<', '>', 
		'=', '+', '-', '*', '/', '%', '!', '~', 
		'?', ';', ':', ',', '.', '\\'
	};
	bool open = 0;
	char key; // either " ' / * # 
	char *str;
	int len, cap;
#define STRATTRRENDER(str, len, attr, pair) \
	mvaddnstr(y, x, str, len); \
	mvchgat(y, x, len, attr, pair, NULL); \
	x += len;
#define MANAGECURSOR(tok) \
	if(tok == cd->cur) \
	{ \
		switch(tok->type) \
		{ \
		case TTNEWLINE: \
			if(cd->cursor) \
			{ \
				curX = 0; \
				curY = y + cd->cursor; \
			} \
			else \
			{ \
				curX = x; \
				curY = y; \
			} \
			break; \
		case TTSPACE: \
			curX = x + cd->cursor; \
			curY = y; \
			break; \
		case TTINDENT: \
			curX = x + cd->cursor * 4; \
			curY = y; \
			break; \
		default: \
			curX = x + cd->cursor; \
			curY = y; \
		} \
	}
	if(!cd->first)
	{
		cd->relCurX = 0;
		cd->relCurY = 0;
		return;
	}
	attron(COLOR_PAIR(C_PAIR_TEXT));
	// find the first token that is within a visible region
	x = cd->x;
	y = cd->y - cd->scrollY;
	for(tok = cd->first; tok && y < cd->y; tok = tok->next)
	{
		MANAGECURSOR(tok);
		switch(tok->type)
		{
		case TTNEWLINE:
			if(open && key != '*')
				if(!tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
				{
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
				}
			x = 0;
			y += tok->len;
			break;
		case TTINDENT:
			x += tok->len * 4;
			break;
		case TTSPACE:
		case TTWORD:
			x += tok->len;
			break;
		case TTNONE:
			x += tok->len;
			if(!open)
			{
				switch(tok->c)
				{
				case '/':
					// check for // or /*
					if(!tok->next || tok->next->type != TTNONE)
						break;
					if(tok->next->c != '*' && tok->next->c != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_COMMENT1));
					open = 1;
					key = tok->next->c;
					tok = tok->next; // skip / or *
					x++;
					break;
				case '#':
					attron(COLOR_PAIR(C_PAIR_PREPROC1));
					open = 1;
					key = '#';
					break;
				case '\"': case '\'':
					if(!tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
					{
						attron(COLOR_PAIR(C_PAIR_STRING1));
						open = 1;
						key = tok->c;
					}
					break;
				}
			}
			else if(key == tok->c)
			{
				switch(tok->c)
				{
				// case '/': // a line comment can't be ended with another /
				case '*':
					if(!tok->next || tok->next->type != TTNONE || tok->next->c != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
					tok = tok->next; // skip /
					x++;
					break;
				case '\"': case '\'':
					if(!tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
					{
						attron(COLOR_PAIR(C_PAIR_TEXT));
						open = 0;
					}
					break;
				}
			}
			break;
		}
	}
	// draw all tokens that are visible
	str = malloc(cap = 1024);
	len = 0;
	for(; tok; tok = tok->next)
	{
		MANAGECURSOR(tok);
		switch(tok->type)
		{
		case TTNEWLINE:
			x = 0;
			y += tok->len;
			// stops at eol if there is no \ at the end
			if(open && key != '*')
				if(tok->len > 1 || !tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
				{
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
				}
			if(y > cd->y + cd->height)
				goto all_done; // the next token would be out of view, so don't bother drawing any further
			break;
		case TTSPACE:
			x += tok->len;
			break;
		case TTINDENT:
			x += tok->len * 4; // TODO: store the tab constant somewhere
			break;
		case TTWORD:
			// combine word and number tokens to one gigantic token
			len = tok->len;
			if(len > cap)
			{
				cap = len;
				str = realloc(str, cap);
			}
			memcpy(str, tok->str, tok->len);
			while(tok->next && (tok->next->type == TTNUMBER || tok->next->type == TTWORD))
			{
				tok = tok->next;
				x += len;
				MANAGECURSOR(tok);
				x -= len;
				if(len + tok->len > cap)
				{
					cap *= 2;
					cap += tok->len;
					str = realloc(str, cap);
				}
				memcpy(str + len, tok->str, tok->len);
				len += tok->len;
			}
			// check if word is keyword
			for(int i = 0; i < ARRLEN(keywords); i++)
				if(len == strlen(keywords[i]) && !memcmp(str, keywords[i], len))
				{
					STRATTRRENDER(str, len, A_BOLD, C_PAIR_KEYWORD1);
					goto render_done;
				}
			// check if word is token is word after "struct"
			f = cdfindtok(tok, ARRLEN(spaceMask), spaceMask, TTWORD, FINDTOKLEFT);
			if(f && f->len == sizeof("struct") - 1 && !strcmp(f->str, "struct"))
			{
				STRATTRRENDER(str, len, 0, C_PAIR_KEYWORD2);
				goto render_done;
			}
			// check if "(" is after word
			f = cdfindtok(tok, ARRLEN(spaceMask), spaceMask, TTNONE, FINDTOKRIGHT);
			if(f && f->c == '(')
			{
				STRATTRRENDER(str, len, 0, C_PAIR_FUNCTION);
				goto render_done;
			}
			// check if word ends with "_t"
			if(len >= 3 && str[len - 1] == 't' && str[len - 2] == '_')
			{
				STRATTRRENDER(str, len, A_BOLD, C_PAIR_KEYWORD1);
				goto render_done;
			}
			// draw default
			mvaddnstr(y, x, str, len);
			x += len;
			break;
		case TTNUMBER:
			str = tok->str;
			len = tok->len;
			STRATTRRENDER(str, len, 0, C_PAIR_NUMBER);
			break;
		default: // TTNONE
			if(!open)
			{
				switch(tok->c)
				{
				case '/':
					// check for // or /*
					if(!tok->next || tok->next->type != TTNONE)
						break;
					if(tok->next->c != '*' && tok->next->c != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_COMMENT1));
					open = 1;
					key = tok->next->c;
					mvaddch(y, x, '/');
					addch(key);
					x++;
					tok = tok->next;
					MANAGECURSOR(tok);
					x++;
					goto render_done;
				case '#':
					attron(COLOR_PAIR(C_PAIR_PREPROC1));
					open = 1;
					key = '#';
					break;
				case '\"': case '\'':
					if(!tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
					{
						attron(COLOR_PAIR(C_PAIR_STRING1));
						open = 1;
						key = tok->c;
					}
					break;
				}
			}
			else if(key == tok->c)
			{
				switch(tok->c)
				{
				// case '/': // a line comment can't be ended with another /
				case '*':
					if(!tok->next || tok->next->type != TTNONE || tok->next->c != '/')
						break;
					mvaddstr(y, x, "*/");
					x++;
					tok = tok->next;
					MANAGECURSOR(tok);
					x++;
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
					goto render_done;
				case '\"': case '\'':
					if(!tok->prev || tok->prev->type != TTNONE || tok->prev->c != '\\')
					{
						mvaddch(y, x, tok->c);
						x++;
						attron(COLOR_PAIR(C_PAIR_TEXT));
						open = 0;
						goto render_done;
					}
					break;
				}
			}
			mvaddch(y, x, tok->c);
			x++;
		}
	render_done:;
	}
all_done:
	free(str);
	cd->relCurX = curX - cd->x;
	cd->relCurY = curY - cd->y;
#undef NORMALRENDER
}

void cddrawcursor(CodeWidget cd)
{
	move(cd->relCurY + cd->y, cd->relCurX + cd->x);
}
