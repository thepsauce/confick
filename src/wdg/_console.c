void csevent(ConsoleWidget cs, int eId)
{
	if(eId <= 0xFF)
		txputc((TextWidget) cs, eId);
}

void csdraw(ConsoleWidget cs)
{
	Line line;
	char *buf;
	int len;
	int y;
	int visX;

	for(int i = 0; i <= cs->height; i++)
	{
		y = i + cs->scrollY;
		if(y >= cs->lineCnt)
			break;
		visX = 0;
		line = cs->lines + y;
		for(len = line->len, buf = line->buf; len; len--, buf++)
		{
			if(visX >= cs->scrollX && !isspace(*buf))
				mvaddch(cs->y + i, cs->x - cs->scrollX + visX, *buf);
			if(*buf == '\t')
				visX += TXTABWIDTH - visX % TXTABWIDTH;	
			else
				visX++;
			if(visX - cs->scrollX > cs->width)
				break;
		}
	}
}

void csenter(ConsoleWidget cs)
{
	txputc((TextWidget) cs, '\n');
}

void csleft(ConsoleWidget cs)
{
	int x, y;

	x = cs->curX;
	y = cs->curY;

	if(x)
		x--;

	txmove((TextWidget) cs, x, y);
}

void csright(ConsoleWidget cs)
{
	int x, y;

	x = cs->curX;
	y = cs->curY;

	if(x != cs->lines[y].len)
		x++;

	txmove((TextWidget) cs, x, y);	
}

void csup(ConsoleWidget cs)
{

}

void csdown(ConsoleWidget cs)
{

}

void cshome(ConsoleWidget cs)
{
	txmove((TextWidget) cs, 0, cs->curY);
}

void csend(ConsoleWidget cs)
{
	txmove((TextWidget) cs, cs->lines[cs->curY].len, cs->curY);
}

void csdelete(ConsoleWidget cs)
{
	int x, y;

	x = cs->curX;
	y = cs->curY;

	if(x != cs->lines[y].len)
		_txdelete((TextWidget) cs, x, y);

	txmove((TextWidget) cs, x, y);
}

void csbackdelete(ConsoleWidget cs)
{
	int x, y;

	x = cs->curX;
	y = cs->curY;

	if(x)
	{
		x--;
		_txdelete((TextWidget) cs, x, y);
	}

	txmove((TextWidget) cs, x, y);
}
