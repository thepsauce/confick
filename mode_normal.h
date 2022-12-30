int _txunitspacing(Text tx, int y)
{
	int sp;
	Line line;
	int len;
	char *buf;
	
	// get spacing of current line	
	sp = 0;
	line = tx->lines + y;
	for(len = line->len, buf = line->buf; len; len--, buf++)
	{
		if(*buf == '\t')
			sp += TXTABWIDTH - sp % TXTABWIDTH;
		else if(*buf == ' ')
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
	// give the next line the same spacing
	_txbreak(tx);
	// break modifies these
	x = tx->curX;
	y = tx->curY;
	tabs = sp / TXTABWIDTH;
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
	int sp;

	x = tx->curX;
	y = tx->curY;

	if(y)
	{
		sp = _txunitspacing(tx, y);
	}
	
	

	txmove(tx, x, y);
}
