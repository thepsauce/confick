void
cdupdatecursor(code_t code)
{
	bool showStatus, showLines;
	int num, nDigits;
	int x, y, w, h;
	int vx, vy;
	
	showLines = !!(code->flags & CDFSHOWLINES);
	showStatus = !!(code->flags & CDFSHOWSTATUS);
	
	nDigits = 0;
	for(int num = code->text.nLines; num; num /= 10, nDigits++);

	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	w -= x;
	h -= y;
	w -= showLines * (nDigits + 1);
	h -= showStatus;
	w--;
	h--;
	vx = 0;
	for(int i = 0; i < code->text.cursor.x; i++)
		if(code->text.lines[code->text.cursor.y].buf[i] == L'\t')
			vx += 4 - vx % 4;
		else
			vx++;
	vy = code->text.cursor.y;
	if(vx < code->scrollX)
		code->scrollX = max(vx - code->scrollX - 4, 0);
	else if(vx - code->scrollX > w)
		code->scrollX = vx - w + 4;
	if(vy < code->scrollY)
		code->scrollY = vy;
	else if(vy - code->scrollY > h)
		code->scrollY = vy - h;
	code->cursor.rvx = vx;
	code->cursor.rvy = vy;
	code->cursor.vx = vx - code->scrollX + showLines * (nDigits + 1);
	code->cursor.vy = vy - code->scrollY;
}

int
cdsetsyntax(code_t code, tunit_t syntax)
{
	if(!code || !syntax)
		return ERROR(!code ? "code is null" : "syntax is null");
	code->syntax = syntax;
	return OK;
}

void
cddraw(code_t code)
{
	tunit_t tunit;
	cchar_t cc;
	line_t *lines, line;
	int nLines;
	int iLine;
	int i;
	int x, y;
	int w, h;
	int lOff;
	bool eofWritten = 0;
	bool showLines;
	bool showStatus;
	int num, nDigits;
	char lineNumberBuf[3 + (int) log10((double) INT_MAX)];

	void read_chars(void)
	{
		wchar_t w[CCHARW_MAX];
		attr_t a;
		short cp;
		while(!tunit->read(tunit, &cc))
		{
			getcchar(&cc, w, &a, &cp, NULL);
			if(!wcscmp(w, L"\t"))
			{
				x += 4 - x % 4;
			}
			else
			{
				if(x >= 0)
					mvwadd_wch(code->window, y, x + lOff, &cc);
				x++;
			}
		}
	}

	lines = code->text.lines;
	nLines = code->text.nLines;
	iLine = code->scrollY;

	for(num = nLines, nDigits = 0; num; num /= 10, nDigits++);

	getbegyx(code->window, y, x);
	getmaxyx(code->window, h, w);
	h -= y;

	showLines = !!(code->flags & CDFSHOWLINES);
	showStatus = !!(code->flags & CDFSHOWSTATUS);
	
	lOff = showLines * (nDigits + 1);
	h -= showStatus;

	tunit = code->syntax;

	werase(code->window);
	if(showStatus)
		mvwprintw(code->window, h, 0, "%d:%d", code->cursor.vx, code->cursor.vy);
	for(x = -code->scrollX, y = 0; h; h--, iLine++, y++, x = -code->scrollX)
	{
		if(iLine < nLines)
		{
			line = lines[iLine];
			int n = sprintf(lineNumberBuf, "%d", iLine + 1);
			wmove(code->window, y, nDigits - n);
			for(int i = 0; i < n; i++)
				waddch(code->window, lineNumberBuf[i]);
			for(i = 0; i < line.nBuf; i++)
			{
				tunit->write(tunit, line.buf[i]);
				read_chars();
			}
			if(iLine + 1 == nLines)
				tunit->write(tunit, EOF - 1);
			else
				tunit->write(tunit, L'\n');
			read_chars();
		}
		else if(showLines)
		{
			mvwaddch(code->window, y, nDigits - 1, '~' | COLOR_PAIR(CD_PAIR_EMPTY_LINE_PREFIX));
		}
	}
	tunit->write(tunit, EOF);
	wmove(code->window, code->cursor.vy, code->cursor.vx);
}

int 
cdproc(code_t code, 
		int c)
{
	struct {
		int c;
		int (*txfunc)(text_t*);
	} table[] = {
		{ KEY_LEFT, txleft },
		{ KEY_RIGHT, txright },
		{ KEY_UP, txup },
		{ KEY_DOWN, txdown },
		{ KEY_HOME, txhome },
		{ KEY_END, txend },
		{ KEY_DC, txremove },
		{ KEY_BACKSPACE, txleftremove },
	};
	switch(c)
	{
	case WDGINIT:
		txinit(&code->text);
		break;
	case WDGUNINIT:
		txdiscard(&code->text);
		break;
	case WDGDRAW:
		cddraw(code);
		break;
	default:
		for(int i = 0; i < ARRLEN(table); i++)
		{
			if(table[i].c == c)
			{
				table[i].txfunc(&code->text);
				cdupdatecursor(code);
				return OK;
			}
		}
		txadd(&code->text, c);
		cdupdatecursor(code);
	}
	return OK;
}
