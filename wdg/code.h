void
cdupdatecursor(code_t code)
{
	int x, y, w, h;
	
	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	w -= x;
	h -= y;
	w -= code->lOff;
	h -= code->bOff;
	w--;
	h--;
	if(code->vx < 0)
	{
		code->scrollX += code->vx;
		code->scrollX = max(code->scrollX, 0);
		code->vx = 0;
	}
	else if(code->vx > w)
	{
		code->scrollX += code->vx - w;
		code->vx = w;
	}
	if(code->vy < 0)
	{
		code->scrollY += code->vy;
		code->scrollY = max(code->scrollY, 0);
		code->vy = 0;
	}				
	else if(code->vy > h)
	{
		code->scrollY += code->vy - h;
		code->vy = h;
	}
	cddraw(code);
}

void
cddraw(code_t code)
{
	cchar_t cc;
	line_t *lines, line;
	int nLines;
	int i, n;
	int x, y;
	int w, h;
	int iLine;
	int lOff;
	int vx, vy = -1;
	bool eofWritten = 0;
	bool showLines;
	bool showStatus;
	int num, nDigits;
	char lineNumberBuf[3 + (int) log10((double) INT_MAX)];

	lines = code->text.lines;
	nLines = code->text.nLines;

	for(num = nLines, nDigits = 0; num; num /= 10, nDigits++);

	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	w -= x;
	h -= y;

	showLines = !!(code->flags & CDFSHOWLINES);
	showStatus = !!(code->flags & CDFSHOWSTATUS);
	
	code->lOff = lOff = showLines * (nDigits + 1);
	code->bOff = showStatus;
	h -= showStatus;

	werase(code->window);
	for(x = -code->scrollX, y = 0; y < h; y++, x = -code->scrollX)
	{
		iLine = y + code->scrollY;
		if(iLine >= 0 && iLine < nLines)
		{
			line = lines[iLine];
			n = sprintf(lineNumberBuf, "%d", iLine + 1);
			wmove(code->window, y, nDigits - n);
			for(i = 0; i < n; i++)
				waddch(code->window, lineNumberBuf[i] | COLOR_PAIR(C_PAIR_LINENUMBER));
			wmove(code->window, y, lOff);
			for(i = 0, x = 0; i < line.nBuf; i++)
			{
				wchar_t ws[2] = { line.buf[i], 0 };
				if(x < w)
				{
					setcchar(&cc, ws, 0, C_PAIR_TEXT, NULL);
					mvwadd_wch(code->window, y, x + lOff, &cc);
				}
				if(vy < 0 && y == code->vy && x >= code->vx)
				{
					vx = x;
					vy = y;
					code->text.cursor.x = i;
					code->text.cursor.y = iLine;
				}
				if(ws[0] == L'\t')
					x += 4 - x % TABSIZE;
				else
					x++;
			}
			if(vy < 0 && y == code->vy)
			{
				vx = x;
				vy = y;
				code->text.cursor.x = line.nBuf;
				code->text.cursor.y = iLine;
			}
		}
		else if(showLines)
		{
			mvwaddch(code->window, y, nDigits - 1, '~' | COLOR_PAIR(CD_PAIR_EMPTY_LINE_PREFIX));
		}
	}
	if(vy < 0)
	{
		vx = x;
		vy = code->text.nLines - 1 - code->scrollY;
		code->text.cursor.x = code->text.lines[code->text.nLines - 1].nBuf;
		code->text.cursor.y = code->text.nLines - 1;
	}
	if(showStatus)
		mvwprintw(code->window, h, 0, "%d:%d;%d:%d", code->text.cursor.y + 1, code->text.cursor.x + 1,
				code->vy, code->vx);
	
	wmove(code->window, vy, vx + lOff);
}

void 
cdleft(code_t code)
{
	code->vx--;
}

void
cdright(code_t code)
{
	code->vx++;
}

void cdup(code_t code)
{
	code->vy--;
}

void cddown(code_t code)
{
	code->vy++;
}

void cdhome(code_t code)
{
	code->vx = INT_MIN;
}

void cdend(code_t code)
{
	code->vx = INT_MAX;
}

void
cdremove(code_t code)
{
	txremove(&code->text);
}

void
cdleftremove(code_t code)
{
	code->vx--;
	txleftremove(&code->text);
}

int 
cdproc(code_t code, 
		int event,
		int key)
{
	struct {
		int key;
		void (*cdfunc)(code_t code);
	} table[] = {
		{ KEY_LEFT, cdleft },
		{ KEY_RIGHT, cdright },
		{ KEY_UP, cdup },
		{ KEY_DOWN, cddown },
		{ KEY_HOME, cdhome },
		{ KEY_END, cdend },
		{ KEY_DC, cdremove },
		{ KEY_BACKSPACE, cdleftremove },
	};
	switch(event)
	{
	case WDGINIT:
		txinit(&code->text);
		break;
	case WDGUNINIT:
		txdiscard(&code->text);
		break;
	default:
		for(int i = 0; i < ARRLEN(table); i++)
		{
			if(table[i].key == key)
			{
				table[i].cdfunc(code);
				cdupdatecursor(code);
				return OK;
			}
		}

		code->recentInput[code->iRci++] = key;
		code->iRci %= ARRLEN(code->recentInput);
		txadd(&code->text, key);
		if(key == '\t')
			code->vx += 4 - code->vx % TABSIZE;
		else if(key == '\n')
		{
			code->vx = INT_MIN;
			code->vy++;
		}
		else
			code->vx++;
		cdupdatecursor(code);
	}
	return OK;
}

