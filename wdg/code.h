void cddraw(code_t code);

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
	line_t *lines, line;
	int nLines;
	int i, n;
	int x, y;
	int w, h;
	int iLine;
	int lOff;
	bool showLines;
	bool showStatus;
	int num, nDigits;
	char lineNumberBuf[3 + (int) log10((double) INT_MAX)];
	struct state_info *si;

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

	si = c_state_create();
	si->window = code->window;
	si->x = -code->scrollX;
	si->y = -code->scrollY;
	si->tx = code->lOff;
	for(x = -code->scrollX, y = 0; y < h; y++, x = -code->scrollX)
	{
		iLine = y + code->scrollY;
		if(iLine >= 0 && iLine < nLines)
		{
			line = lines[iLine];
			n = sprintf(lineNumberBuf, "%d", iLine + 1);
			wmove(code->window, y, nDigits - n);
			for(i = 0; i < n; i++)
				waddch(code->window, lineNumberBuf[i] | A_BOLD | COLOR_PAIR(C_PAIR_LINENUMBER));
			wmove(code->window, y, lOff);
			for(i = 0; i < line.nBuf; i++)
				sifeed(si, line.buf[i]);
			if(iLine + 1 != nLines)
				sifeed(si, L'\n');
		}
		else if(showLines)
		{
			mvwaddch(code->window, y, nDigits - 1, '~' | COLOR_PAIR(CD_PAIR_EMPTY_LINE_PREFIX));
		}
	}
	if(showStatus)
		mvwprintw(code->window, h, 0, "%d:%d;%d:%d", code->text.cursor.y + 1, code->text.cursor.x + 1,
				code->vy, code->vx);
	sifeed(si, EOF);
	//wmove(code->window, vy, vx + lOff);
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

