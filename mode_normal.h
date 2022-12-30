int _txunitspacing(Text tx, int y)
{
	int tsp;
	Line line;
	int len;
	char *buf;
	
	tsp = 0;
	line = tx->lines + y;
	for(len = line->len, buf = line->buf; len; len--, buf++)
	{
		if(*buf == '\t')
			tsp += TXTABWIDTH - tsp % TXTABWIDTH;
		else if(*buf == ' ')
			tsp++;
		else
			break;
	}
	return tsp;
}

int _txspacing(Text tx, int y)
{
	int sp;
	Line line;
	int len;
	char *buf;
	
	sp = 0;
	line = tx->lines + y;
	for(len = line->len, buf = line->buf; len; len--, buf++)
	{
		if(*buf == '\t' || *buf == ' ')
			sp++;
		else
			break;
	}
	return sp;
}

void txmotion_c_nl_indent(Text tx)
{
	int x, y;
	int sp;
	int tabs;

	x = tx->curX;
	y = tx->curY;
	sp = _txunitspacing(tx, y);
	tabs = sp / TXTABWIDTH;
	if(x && tx->lines[y].buf[x - 1] == '{')
		tabs++;
	_txbreak(tx);
	// break modifies these
	x = tx->curX;
	y = tx->curY;
	if(tabs)
	{
		char tabBuf[tabs];
		memset(tabBuf, '\t', tabs);
		_txinsertnstr(tx->lines + y, 0, tabBuf, tabs);
		x = tabs;
	}
	txmove(tx, x, y);
}

void txmotion_c_curlyclose(Text tx)
{
	int x, y;
	int psp;
	int sp;

	x = tx->curX;
	y = tx->curY;
	if(y)
	{
		psp = _txspacing(tx, y - 1);
		if(psp)
		{
			sp = _txspacing(tx, y);
			if(sp == tx->lines[y].len)
			{
				x = psp - 1;
				char tabBuf[x];
				memset(tabBuf, '\t', x);
				tx->lines[y].len = 0;
				_txinsertnstr(tx->lines + y, 0, tabBuf, x);
			}			
		}
	}
	_txinsertchar(tx->lines + y, x, '}');
	txmove(tx, x, y);
}

void txmotion_c_if(Text tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	txputs(tx, "if()\n{\n}\n");

	x += 3;

	txmove(tx, x, y);
}


