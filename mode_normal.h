void txmotion_c_nl_indent(Text tx)
{
	int x, y;
	int sp;
	Line line;
	char *buf;
	int len;
	int tabs;

	x = tx->curX;
	y = tx->curY;

	// get spacing of current line	
	sp = 0;
	line = tx->lines + y;
	for(len = line->len, buf = line->buf; len; len--, buf++)
	{
		if(*buf == '\t')
			sp += 4 - sp % 4;
		else if(*buf == ' ')
			sp++;
		else
			break;
	}
	// give the next line the same spacing
	_txbreak(tx);
	// break modifies these
	x = tx->curX;
	y = tx->curY;
	tabs = sp / 4;
	if(len && *buf == '{')
		tabs++;
	if(tabs)
	{
		char tabBuf[tabs];
		memset(tabBuf, '\t', tabs);
		line = tx->lines + y;
		_txinsertnstr(line, 0, tabBuf, tabs);
		x = tabs;
	}

	txmove(tx, x, y);
}
