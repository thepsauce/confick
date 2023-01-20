void
cdupdatecursor(code_t code)
{

}

void
cddraw(code_t code)
{
	line_t *lines, line;
	int nLines;
	int iLine;
	int x, y;
	int h;
	int lOff;
	bool showLines;
	bool showStatus;
	int num, nDigits;
	char lineNumberBuf[2 + (int) log10((double) ((1ULL << (8 * sizeof(int))) - 1))];

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
	move(code->text.cursor.y, code->text.cursor.x);
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
