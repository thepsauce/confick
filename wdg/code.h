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
		if(feof(file))
			break;
		next = malloc(sizeof*next);
		next->prev = block;
		next->next = NULL;
	}
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

	// TODO:
}

void cdend(CodeWidget cd)
{
	CodeBlock block;

	// TODO: MAP TO POS SYSTEM
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

void cdputc(CodeWidget cd, int c)
{
	CodeBlock block, next;

	if(c < 0 || c > 255)
		return;
	block = cd->first;
	if(block->len == sizeof block->buf)
	{
		next = block->next;
		if(next->len == sizeof block->buf)
		{
			next = malloc(sizeof*next);
			next->prev = block;
			next->next = block->next;
			if(block->next)
				block->next->prev = next;
			block->next = next;
		}
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
		next->len++;
	}
	else
	{
		memmove(block->buf + cd->cursor + 1, block->buf + cd->cursor, block->len - cd->cursor);
		block->buf[cd->cursor] = c;
		block->len++;
		cd->cursor++;
	}
}

void cddraw(CodeWidget cd)
{
	CodeBlock block, f;
	int x, y;
	int curX, curY;
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
	int c = 0, prevC, nextC;
	char *buf;
	int rem;
#define STRATTRRENDER(str, len, attr, pair) \
	mvaddnstr(y, x, str, len); \
	mvchgat(y, x, len, attr, pair, NULL); \
	x += len;
	if(!cd->first->len)
	{
		cd->relCurX = 0;
		cd->relCurY = 0;
		return;
	}
	attron(COLOR_PAIR(C_PAIR_TEXT));
	// find the first blocken that is within a visible region
	x = cd->x;
	y = cd->y - cd->scrollY;
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
			x = 0;
			y++;
			break;
		case '\t':
			x += 4;
			break;
		default:
			x++;
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
					x++;
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
					x++;
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
	// draw all blockens that are visible
	for(; block; )
	{
		NEXTCHAR;
		switch(c)
		{
		case '\n':
			x = 0;
			y++;
			// stops at eol if there is no \ at the end
			if(open && key != '*' && prevC != '\\')
			{
				attron(COLOR_PAIR(C_PAIR_TEXT));
				open = 0;
			}
			if(y > cd->y + cd->height)
				goto all_done; // the next blocken would be out of view, so don't bother drawing any further
			break;
		case '\t':
			x += 4; // TODO: store the tab constant somewhere
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
					key = nextC;
					mvaddch(y, x, '/');
					addch(key);
					x++;
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
			mvaddch(y, x, c);
			x++;
		}
	render_done:;
	}
all_done:
	cd->relCurX = curX - cd->x;
	cd->relCurY = curY - cd->y;
#undef NORMALRENDER
}

void cddrawcursor(CodeWidget cd)
{
	move(cd->relCurY + cd->y, cd->relCurX + cd->x);
}
