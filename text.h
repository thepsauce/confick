Text txcreate(int initLineCap)
{
	Text tx;

	tx = malloc(sizeof*tx);
	tx->flags = 0;
	tx->state = 0;
	tx->scrollX = 0;
	tx->scrollY = 0;
	tx->x = 0;
	tx->y = 0;
	tx->curX = 0;
	tx->curY = 0;
	initLineCap = initLineCap < 1 ? 1 : initLineCap;
	tx->lines = initLineCap ? malloc(initLineCap * sizeof*tx->lines) : NULL;
	tx->lineCap = initLineCap;
	tx->lineCnt = 0;
	memset(tx->lines, 0, initLineCap * sizeof*tx->lines);
	
	return tx;
}

void txfree(Text tx)
{
	for(int i = 0; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	free(tx->lines);
	free(tx);
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
	
	_txgrow(tx);
	line = txline(tx);
	nextLine = line + 1;
	nextLine->buf = malloc(line->len - tx->curX);
	memcpy(nextLine->buf, line->buf + tx->curX, line->len - tx->curX);
	nextLine->cap = nextLine->len = line->len - tx->curX;
	memmove(nextLine, line, (tx->lineCnt - tx->curY) * sizeof*line);
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
}
