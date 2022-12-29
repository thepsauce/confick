Text txcreate(int initLineCap, int x, int y, int width, int height)
{
	Text tx;

	tx = malloc(sizeof(*tx));
	tx->flags = 0;
	tx->state = 0;
	tx->scrollX = 0;
	tx->scrollY = 0;
	tx->x = x;
	tx->y = y;
	tx->width = width;
	tx->height = height;
	tx->curX = 0;
	tx->curY = 0;
	initLineCap = initLineCap < 1 ? 1 : initLineCap;
	tx->lines = malloc(initLineCap * sizeof*tx->lines);
	tx->lineCap = initLineCap;
	tx->lineCnt = 1;
	memset(tx->lines, 0, sizeof*tx->lines);
	return tx;
}

void txfree(Text tx)
{
	for(int i = 0; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	free(tx->lines);
	free(tx);
}

void txmove(Text tx, int x, int y)
{
	assert(x >= 0 && y >= 0 && y < tx->lineCnt && x <= tx->lines[y].len && "txmove > out of bounds");
	tx->curX = x;
	tx->curY = y;
	// make caret visible if it is outside
	int sx = x - tx->scrollX;
	int sy = y - tx->scrollY;
	if(sx < 0)
		tx->scrollX = x;
	else if(sx > tx->width)
		tx->scrollX += tx->width - sx + 4;
	// same for y
	if(sy < 0)
		tx->scrollY = y;
	else if(sy > tx->height)
		tx->scrollY += tx->height - sy + 1;
}

void txmotion(Text tx, int motion)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;
	switch(motion)
	{
	case TXMOTION_LEFT:
		if(!x)
		{
			if(!y)
				break;
			y--;
			x = tx->lines[y].len;
			break;
		}
		x--;
		break;
	case TXMOTION_UP:
		if(!y)
			break;
		y--;
		x = min(x, tx->lines[y].len);
		break;
	case TXMOTION_RIGHT:
		if(x == tx->lines[y].len)
		{
			if(y + 1 == tx->lineCnt)
				break;
			y++;
			x = 0;
			break;
		}
		x++;
		break;
	case TXMOTION_DOWN: 
		if(y + 1 == tx->lineCnt)
			break;
		y++;
		x = min(x, tx->lines[y].len);
		break;
	}
	txmove(tx, x, y);
}

void _txgrow(Text tx)
{
	if(tx->lineCnt + 1 > tx->lineCap)
	{
		tx->lineCap *= 2;
		tx->lineCap++;
		tx->lines = realloc(tx->lines, tx->lineCap * sizeof*tx->lines);
	}
}

void _txbreak(Text tx)
{
	Line line, nextLine;
	int breakLen;
	
	_txgrow(tx);
	line = txline(tx);
	nextLine = line + 1;	
	memmove(nextLine, line, (tx->lineCnt - 1 - tx->curY) * sizeof*line);
	if((breakLen = line->len - tx->curX))
	{
		nextLine->buf = malloc(breakLen);
		nextLine->cap = nextLine->len = breakLen;
		line->len -= breakLen;
		memmove(nextLine->buf, line->buf + line->len, breakLen);
	}
	else
	{
		nextLine->buf = NULL;
		nextLine->cap = nextLine->len = 0;
	}
	tx->lineCnt++;
	tx->curX = 0;
	tx->curY++;
}

void _txgrowline(Line line, int to)
{
	int delta = to - line->cap;
	if(delta > 0)
	{
		line->cap *= 2;
		line->cap += delta;
		line->buf = realloc(line->buf, line->cap);
	}
}

void _txinsertchar(Line line, int index, int c)
{
	char *dest;	

	_txgrowline(line, line->len + 1);
	dest = line->buf + index;
	memmove(dest + 1, dest, line->len - index);
	line->len++;
	*dest = c;
}

void txputc(Text tx, int c)
{
	Line line;

	if(c == '\r' || c == '\n')
	{
		_txbreak(tx);
	}
	else
	{
		line = txline(tx);
		_txinsertchar(line, tx->curX, c);
		mvaddch(tx->curY + tx->y - tx->scrollY, tx->curX + tx->x - tx->scrollX, c);
		tx->curX++;
	}
	// update cursor
	txmove(tx, tx->curX, tx->curY);
}

void txdelc(Text tx)
{


}

void _txinsertnstr(Line line, int index, const char *s, int n)
{
	char *dest;

	_txgrowline(line, line->len + n);
	dest = line->buf + index;
	memmove(dest + n, dest, line->len - index);
	line->len += n;
	memcpy(dest, s, n);
}

const char *_txlinesep(const char *s)
{
	for(; *s; s++)
	{
		if(*s == '\r' || *s == '\n')
			return s;
	}
	return NULL;
}

int _txlineseplen(const char *s)
{
	return 1 + (s[0] == '\r' && s[1] == '\n');
}

void txputs(Text tx, const char *s)
{
	const char *e;
	int l;

	while(1)
	{
		e = _txlinesep(s);
		l = e ? e - s : strlen(s);
		_txinsertnstr(txline(tx), tx->curX, s, l);
		mvaddnstr(tx->curY + tx->x - tx->scrollY, tx->curX + tx->y - tx->scrollX, s, l);
		tx->curX += l;
		if(!e)
			break;
		_txbreak(tx);
		s = e + _txlineseplen(e);
	}
	// update cursor
	txmove(tx, tx->curX, tx->curY);
}
