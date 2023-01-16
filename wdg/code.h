void cdinit(CodeWidget cd)
{
	cd->cur = cd->first = malloc(sizeof*cd->first);
	memset(cd->first, 0, sizeof*cd->first);
}

void cduninit(CodeWidget cd)
{
	cdclear(cd);
}

void cdclear(CodeWidget cd)
{
	CodeBlock block, next;

	for(block = cd->first; block; block = next)
	{
		next = block->next;
		free(block);
	}
	cd->cur = cd->first = malloc(sizeof*cd->first);
	memset(cd->first, 0, sizeof*cd->first);
}

void cdopen(CodeWidget cd, const char *fileName)
{
	FILE *file;
	int c;
	CodeBlock block, next;

	cdclear(cd);
	block = cd->first;
	cd->fileName = strdup(fileName);
	if(!(file = fopen(fileName, "r")))
    	return;
	while((block->len = fread(block->buf, 1, sizeof block->buf, file)) == sizeof block->buf)
	{
		for(int i = 0; i < block->len; i++)
			if(block->buf[i] == '\n')
				block->height++;
		if(feof(file))
			break;
		next = malloc(sizeof*next);
		next->height = 0;
		next->prev = block;
		next->next = NULL;
		block->next = next;
		block = next;
	}
	for(int i = 0; i < block->len; i++)
		if(block->buf[i] == '\n')
			block->height++;
    fclose(file);
}

void cdsave(CodeWidget cd)
{
	FILE *file;
	CodeBlock block;

    file = fopen(cd->fileName, "w");
    assert(file);
	for(block = cd->first; block; block = block->next)
		fwrite(block->buf, 1, block->len, file);
    fclose(file);
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
	CodeBlock block;
	char *buf;
	int index;

	index = cd->cursor;
	for(block = cd->cur;; )
	{
		for(buf = block->buf + index; index; index--, buf--)
			if(*(buf - 1) == '\n')
				goto end;
		if(!block->prev)
			break;
		block = block->prev;
		index = block->len;
	}
end:
	cd->cur = block;
	cd->cursor = index;
}

void cdend(CodeWidget cd)
{
	CodeBlock block;
	char *buf;
	int index;

	index = cd->cursor;
	for(block = cd->cur;; )
	{
		for(buf = block->buf + index; index != block->len; index++, buf++)
			if(*buf == '\n')
				goto end;
		if(!block->next)
			break;
		block = block->next;
		index = 0;
	}
end:
	cd->cur = block;
	cd->cursor = index;
}

void cddelete(CodeWidget cd)
{
	CodeBlock block, next;

	block = cd->cur;
	next = block->next;
	if(cd->cursor == block->len)
	{
		if(next)
		{
			if(next->len == 1)
			{
				if(next->next)
					next->next->prev = block;
				block->next = next->next;
				free(next);
			}
			else
			{
				if(*next->buf == '\n')
					next->height--;
				next->len--;
				memmove(next->buf, next->buf + 1, next->len);
			}
		}
	}
	else
	{
		if(block->len == 1)
		{
			if(block->prev)
			{
				block->prev->next = next;
				cd->cur = block->prev;
				cd->cursor = cd->cur->len;
				free(block);
			}
			else if(next)
			{
				cd->cur = next;
				next->prev = NULL;
				free(block);
			}
			else
			{
				memset(block, 0, sizeof*block);
			}
	}
		else
		{
			if(block->buf[cd->cursor] == '\n')
				block->height--;
			block->len--;
			memmove(block->buf + cd->cursor, block->buf + cd->cursor + 1, block->len - cd->cursor);
		}
	}
}

void cdbackdelete(CodeWidget cd)
{
	cdleft(cd);
	cddelete(cd);
}

void cdputs(CodeWidget cd, const char *str, int n)
{
	CodeBlock block, next;
	int delta;
	char *buf;
	int len;
	int needed;
	int space;
	int overflow = 0;
	int index;
	char overflowBuf[2048];

	block = cd->cur;
	index = cd->cursor;
	cd->cursor += n;
append:
	buf = block->buf + index;
	len = block->len - index;
	space = sizeof(block->buf) - index;
	space = min(n, space);
	if(len)
	{
		overflow = len;
		memcpy(overflowBuf, buf, overflow);
	}
	memcpy(buf, str, space);
	index += space;
	block->len = index;
	n -= space;
	str += space;
	if(n)
	{
		next = malloc(sizeof*next);
		memset(next, 0, sizeof*next);
		next->prev = block;
		next->next = block->next;
		if(block->next)
			block->next->prev = next;
		block->next = next;
		block = next;
		index = 0;
		goto append;
	}
	if(overflow)
	{
		str = overflowBuf;
		n = overflow;
		overflow = 0;
		goto append;
	}
	while(cd->cursor > sizeof block->buf)
	{
		cd->cursor -= sizeof block->buf;
		cd->cur = cd->cur->next;
	}
}

void cdputc(CodeWidget cd, int c)
{
	CodeBlock block, next;

	if(c < 0 || c > 255)
		return;
	block = cd->cur;
	if(block->len == sizeof block->buf)
	{
		next = block->next;
		if(next->len == sizeof block->buf)
		{
			next = malloc(sizeof*next);
			next->len = 1;
			next->height = 0;
			next->prev = block;
			next->next = block->next;
			if(block->next)
				block->next->prev = next;
			block->next = next;
		}
		else
			next->len++;
		if(cd->cursor == sizeof block->buf)
		{
			next->buf[0] = c;
			cd->cur = next;
			cd->cursor = 1;
		}
		else
		{
			memmove(next->buf + 1, next->buf, next->len - 1);
			next->buf[0] = block->buf[sizeof(block->buf) - 1];
			memmove(block->buf + cd->cursor + 1, block->buf + cd->cursor, sizeof(block->buf) - cd->cursor - 1);
			block->buf[cd->cursor] = c;
			cd->cursor++;
		}
	}
	else
	{
		memmove(block->buf + cd->cursor + 1, block->buf + cd->cursor, block->len - cd->cursor);
		block->buf[cd->cursor] = c;
		block->len++;
		cd->cursor++;
	}
	if(c == '\n')
		cd->cur->height++;
}

void cddraw(CodeWidget cd)
{
	CodeBlock block, f;
	int x, y;
	const char *keywords[] = {
		"auto",
		"break",
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
	bool open = 0;
	char key; // either " ' / * # 
	int c = 0, prevC, nextC;
	int startX;
	char *buf;
	int rem;
	char keep[9];
	int index;
	bool wasNumber = 0, wasFloat = 0;
#define STRATTRRENDER(str, len, attr, pair) \
	mvaddnstr(y, x, str, len); \
	mvchgat(y, x, len, attr, pair, NULL); \
	x += len;
	if(!cd->first->len)
		return;
	attron(COLOR_PAIR(C_PAIR_TEXT));
	// find the first blocken that is within a visible region
#define NEXTCHAR \
	prevC = c; \
	c = buf[0]; \
	buf++; \
	rem--; \
	if(!rem) \
	{ \
		block = block->next; \
		if(block) \
		{ \
			buf = block->buf; \
			rem = block->len; \
		} \
	} \
	nextC = block ? buf[0] : 0
	y = cd->y - cd->scrollY;
	for(block = cd->first, buf = block->buf, rem = block->len; block && y < cd->y;)
	{
		NEXTCHAR;
		switch(c)
		{
		case '\n':
			if(open && key != '*' && prevC != '\\')
			{
				attron(COLOR_PAIR(C_PAIR_TEXT));
				open = 0;
			}
			y++;
			break;
		default:
			if(!open)
			{
				switch(c)
				{
				case '/':
					// check for // or /*
					if(nextC != '*' && nextC != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_COMMENT1));
					open = 1;
					key = c;
					NEXTCHAR; // skip / or *
					break;
				case '#':
					attron(COLOR_PAIR(C_PAIR_PREPROC1));
					open = 1;
					key = '#';
					break;
				case '\"': case '\'':
					if(prevC != '\\')
					{
						attron(COLOR_PAIR(C_PAIR_STRING1));
						open = 1;
						key = c;
					}
					break;
				}
			}
			else if(key == c)
			{
				switch(c)
				{
				// case '/': // a line comment can't be ended with another /
				case '*':
					if(nextC != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
					NEXTCHAR; // skip /
					break;
				case '\"': case '\'':
					if(prevC != '\\')
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
	// draw all blocks that are visible
	x = cd->x;
	for(; block; )
	{
		NEXTCHAR;
		switch(c)
		{
		case 'A' ... 'Z':
		case 'a' ... 'z':
		case '$':
		case '_':
			if(open && key != '#')
				goto char_render;
			wasNumber = 0;
			startX = x;
			index = 0;
			while(1)
			{
				keep[index++] = c;
				if(index == sizeof keep)
					goto no_keyword;
				if(!block || (!isalnum(nextC) && nextC != '$' && nextC != '_'))
					break;
				NEXTCHAR;
			}
			keep[index] = 0;
			for(int i = 0; i < ARRLEN(keywords); i++)
				if(!strcmp(keywords[i], keep))
				{
					STRATTRRENDER(keep, index, A_BOLD, C_PAIR_KEYWORD1);
					goto render_done;
				}
		no_keyword:
			mvaddnstr(y, x, keep, index);
			x += index;
			while(block && (isalnum(nextC) || nextC == '$' || nextC == '_'))
			{
				NEXTCHAR;
				addch(c);
				x++;
				index = 2;
				keep[0] = keep[1];
				keep[1] = c;
			}
			if(nextC == '(')
				mvchgat(y, startX, x - startX, 0, C_PAIR_FUNCTION, NULL);
			else if(x - startX > 2 && keep[index - 2] == '_' && keep[index - 1] == 't')
				mvchgat(y, startX, x - startX, A_BOLD, C_PAIR_KEYWORD1, NULL);
			break;
		case '0':
			if(open && key != '#')
				goto char_render;
			wasNumber = wasFloat = 1;
			mvaddch(y, x, c);
			mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
			x++;
			if(nextC == 'x' || nextC == 'X')
			{
				// hexadecimal number
				do 
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				} while(isxdigit(nextC));
			}
			else if(nextC == 'b' || nextC == 'B')
			{
				// binary number
				NEXTCHAR;
				mvaddch(y, x, c);
				mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
				x++;
				while(isxdigit(nextC))
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					if(c != '0' && c != '1')
						mvchgat(y, x, 1, 0, C_PAIR_ERROR, NULL);
					else
						mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				}
			}
			else if(isdigit(nextC))
			{
				// octal number
				do 
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					if(c < '0' || c > '7')
						mvchgat(y, x, 1, 0, C_PAIR_ERROR, NULL);
					else
						mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				} while(isxdigit(nextC));
			}
			else
				goto int_zero;
			goto int_suffix;
		case '1' ... '9':
			if(open && key != '#')
				goto char_render;
			wasNumber = wasFloat = 1;
			mvaddch(y, x, c);
			mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
			x++;
			// decimal number
			while(isxdigit(nextC) && nextC != 'e' && nextC != 'E')
			{
				NEXTCHAR;
				mvaddch(y, x, c);
				if(c < '0' || c > '9')
					mvchgat(y, x, 1, 0, C_PAIR_ERROR, NULL);
				else
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
				x++;
			}
		int_zero:
			if(nextC == 'e' || nextC == 'E')
				goto exponential_render;
			if(nextC != '.')
			{
		int_suffix:
				wasFloat = 0;
				if(nextC == 'u' || nextC == 'U')
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				}
				if(nextC == 'l' || nextC == 'L')
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				}
				if(nextC == 'l' || nextC == 'L')
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				}
			}
			break;
		case '.':
			if(open || ((!wasNumber || !isdigit(prevC)) && !isdigit(nextC)))
				goto char_render;
			mvaddch(y, x, '.');
			if(wasNumber && !wasFloat)
				mvchgat(y, x, 1, 0, C_PAIR_ERROR, NULL);
			else
				mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
			x++;
			while(isdigit(nextC))
			{
				NEXTCHAR;
				mvaddch(y, x, c);
				mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
				x++;
			}
			if(nextC == 'e' || nextC == 'E')
			{
		exponential_render:
				NEXTCHAR;
				mvaddch(y, x, c);
				mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
				x++;
				startX = x;
				if(nextC == '-')
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
					startX = x;
				}
				while(isdigit(nextC))
				{
					NEXTCHAR;
					mvaddch(y, x, c);
					mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
					x++;
				}
				if(startX == x)
					mvchgat(y, x - 1, 1, 0, C_PAIR_ERROR, NULL);
			}
			if(nextC == 'l' || nextC == 'L' || nextC == 'f' || nextC == 'F' || nextC == 'd' || nextC == 'D')
			{
				NEXTCHAR;
				mvaddch(y, x, c);
				mvchgat(y, x, 1, 0, C_PAIR_NUMBER, NULL);
				x++;
			}
			break;
		case '\n':
			if(y >= cd->y + cd->height)
				return; // the next char/block would be out of view, so don't bother drawing any further
			x = 0;
			y++;
			// stops at eol if there is no \ at the end
			if(open && key != '*' && prevC != '\\')
			{
				attron(COLOR_PAIR(C_PAIR_TEXT));
				open = 0;
			}
			break;
		case '\t':
			x += 4; // TODO: store the tab constant somewhere
			break;
		case '\\':
			if(open && (key == '\'' || key == '\"'))
			{
				const char tChars[] = { 'a', 'b', 'e', 'f', 'n', 'r', 't', 'v', 'u', 'x', '\\', '\"', '\'' };
				for(int i = 0; i < ARRLEN(tChars); i++)
					if(nextC == tChars[i])
					{
						move(y, x);
						addch('\\');
						NEXTCHAR;
						addch(c);
						if(tChars[i] == 'x')
						{
							for(int i = 2; i <= 3; i++)
							{
								if(!isxdigit(nextC))
								{
									mvchgat(y, x, i, 0, C_PAIR_ERROR, NULL);
									x += i;
									goto render_done;
								}
								NEXTCHAR;
								addch(c);
							}	
							mvchgat(y, x, 4, 0, C_PAIR_STRING2, NULL);
							x += 4;
						}
						else if(tChars[i] == 'u')
						{
							for(int i = 2; i <= 5; i++)
							{
								if(!isxdigit(nextC))
								{
									mvchgat(y, x, i, 0, C_PAIR_ERROR, NULL);
									x += i;
									goto render_done;
								}
								NEXTCHAR;
								addch(c);
							}	
							mvchgat(y, x, 6, 0, C_PAIR_STRING2, NULL);
							x += 6;
						}
						else
						{
							mvchgat(y, x, 2, 0, C_PAIR_STRING2, NULL);
							x += 2;
						}
						goto render_done;
					}
				if(nextC == '\n')
				{
					mvaddch(y, x, '\\');
					mvchgat(y, x, 1, 0, C_PAIR_STRING2, NULL);
					x++;
					goto render_done;
				}
				mvaddch(y, x, '\\');
				mvchgat(y, x, 1, 0, C_PAIR_ERROR, NULL);
				x++;
				goto render_done;
			}
		default:
			if(!open)
			{
				switch(c)
				{
				case '/':
					// check for // or /*
					if(nextC != '*' && nextC != '/')
						break;
					attron(COLOR_PAIR(C_PAIR_COMMENT1));
					open = 1;
					key = nextC;
					mvaddch(y, x, '/');
					x++;
					mvaddch(y, x, key);
					NEXTCHAR;
					x++;
					goto render_done;
				case '#':
					attron(COLOR_PAIR(C_PAIR_PREPROC1));
					open = 1;
					key = '#';
					break;
				case '\"': case '\'':
					if(prevC != '\\')
					{
						attron(COLOR_PAIR(C_PAIR_STRING1));
						open = 1;
						key = c;
					}
					break;
				}
			}
			else if(key == c)
			{
				switch(c)
				{
				// case '/': // a line comment can't be ended with another /
				case '*':
					if(nextC != '/')
						break;
					mvaddstr(y, x, "*/");
					x++;
					NEXTCHAR;
					x++;
					attron(COLOR_PAIR(C_PAIR_TEXT));
					open = 0;
					goto render_done;
				case '\"': case '\'':
					if(prevC != '\\')
					{
						mvaddch(y, x, c);
						x++;
						attron(COLOR_PAIR(C_PAIR_TEXT));
						open = 0;
						goto render_done;
					}
					break;
				}
			}
		char_render:
			mvaddch(y, x, c);
			x++;
		}
	render_done:;
	}
#undef NORMALRENDER
}

void cddrawcursor(CodeWidget cd)
{
	CodeBlock block;
	int x, y;
	int cursor;
	char *buf;
	int len;

	y = 0;
	for(block = cd->first; block != cd->cur; block = block->next)
		y += block->height;
	x = 0;
	y += cd->y - cd->scrollY;
	cursor = cd->cursor;
	for(buf = block->buf, len = block->len; len && cursor; len--, buf++, cursor--)
	{
		switch(*buf)
		{
		case '\n':
			x = 0;
			y++;
			break;
		case '\t':
			x += 4;
			break;
		default:
			x++;
		}
	}
	move(y, x);
}
