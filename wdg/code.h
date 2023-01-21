void
cdupdatecursor(code_t code)
{

}

int
cdsetsyntax(code_t code, syntax_t syntax)
{
	if(!code || !syntax)
		return ERROR(!code ? "code is null" : "syntax is null");
	code->syntax = syntax;
	return OK;
}

void
cddraw(code_t code)
{
	syntax_t syntax;
	chtype cht;
	line_t *lines, line;
	int nLines;
	int iLine;
	int x, y;
	int h;
	int lOff;
	bool showLines;
	bool showStatus;
	int num, nDigits;
	char lineNumberBuf[2 + (int) log10((double) INT_MAX)];

	syntax = code->syntax;
	synreset(syntax);

	lines = code->text.lines;
	nLines = code->text.nLines;
	iLine = 0;

	for(num = nLines, nDigits = 0; num; num /= 10, nDigits++);

	x = code->window->_begx;
	y = code->window->_begy + code->window->_yoffset;
	x = y = 0;

	showLines = !!(code->flags & CDFSHOWLINES);
	showStatus = !!(code->flags & CDFSHOWSTATUS);
	
	lOff = showLines * (nDigits + 1);

	h = code->window->_maxy - y - showStatus;
	h = max(h, 30);
	for(; h; h--, iLine++)
	{
		if(iLine < nLines)
		{
			line = lines[iLine];
			sprintf(lineNumberBuf, "%d", iLine + 1);
			for(int i = 0, n = strlen(lineNumberBuf); i < n; i++)
				mvaddch(y + iLine, x + nDigits - n + i, lineNumberBuf[i]);
			for(int i = 0, o = x + lOff; i < line.nBuf; i++)
			{
				cht = synfeed(syntax, line.buf[i]);
				mvaddch(y + iLine, o, cht);
				o++;
			}
			if(iLine + 1 < nLines)
				synfeed(syntax, '\n');
		}
		else if(showLines)
		{
			mvaddch(y + iLine, x + nDigits - 1, '~' | COLOR_PAIR(CD_PAIR_EMPTY_LINE_PREFIX));
		}
	}
}

void
cddrawcursor(code_t code)
{
	int nLines;
	bool showLines;
	int num, nDigits;
	
	nLines = code->text.nLines;
	showLines = !!(code->flags & CDFSHOWLINES);
	for(num = nLines, nDigits = 0; num; num /= 10, nDigits++);

	move(code->text.cursor.y, code->text.cursor.x + showLines * (nDigits + 1));
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
	case WDGDRAWCURSOR:
		cddrawcursor(code);
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
		if(c >= 0x00 && c < 0xFF)
		{
			txadd(&code->text, c);
			cdupdatecursor(code);
		}
	}
	return OK;
}
