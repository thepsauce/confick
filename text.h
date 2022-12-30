Text txcreate(int initLineCap, int x, int y, int width, int height)
{
	Text tx;

	tx = malloc(sizeof*tx);
	memset(tx, 0, sizeof*tx);
	tx->x = x;
	tx->y = y;
	tx->width = width;
	tx->height = height;
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
	for(int i = 0; i < sizeof(tx->motions) / sizeof(*tx->motions); i++)
		free(tx->motions[i].elems);
	free(tx->fileName);
	free(tx);
}

void txclear(Text tx)
{
	tx->lines[0].len = 0;
	for(int i = 1; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	tx->curX = 0;
	tx->curY = 0;
	tx->scrollX = 0;
	tx->scrollY = 0;
	tx->lineCnt = 1;
	if(tx->lineCap > TXCLEARKEEPTHRESHOLD)
	{
		struct line tmp = *tx->lines;
		free(tx->lines);
		tx->lines = malloc(sizeof*tx->lines);
		*tx->lines = tmp;
	}
	free(tx->fileName);
	tx->fileName = NULL;
}

void txkey(Text tx, int key)
{
	int x, y;
	for(int i = 0; i < tx->motions[tx->mode].cnt; i++)
	{
		if(tx->motions[tx->mode].elems[i].id == key)
		{
			tx->motions[tx->mode].elems[i].motion(tx);
			return;
		}
	}
	if(key <= 255)
		txputc(tx, key);
}

void txputmotion(Text tx, int mode, int id, void (*motion)(Text tx))
{
	for(int i = 0; i < tx->motions[mode].cnt; i++)
		if(tx->motions[mode].elems[i].id == id)
		{
			tx->motions[mode].elems[i].motion = motion;
			return;
		}
	if(tx->motions[mode].cnt + 1 > tx->motions[mode].cap)
	{
		tx->motions[mode].cap *= 2;
		tx->motions[mode].cap++;
		tx->motions[mode].elems = realloc(tx->motions[mode].elems, tx->motions[mode].cap * sizeof *tx->motions[mode].elems);
	}
	tx->motions[mode].elems[tx->motions[mode].cnt].id = id;
	tx->motions[mode].elems[tx->motions[mode].cnt].motion = motion;
	tx->motions[mode].cnt++;
}

int _txshiftvisx(Text tx, int visX, int indY)
{
	Line line;
	int len;
	char *buf;
	int index;

	visX += tx->scrollX;
	index = 0;
	line = tx->lines + indY;
	for(len = line->len, buf = line->buf; index < visX && len; len--, buf++)
	{
		if(*buf == '\t')
			index += TXTABWIDTH - index % TXTABWIDTH;
		else
			index++;
	}
	return line->len - len;
}

int _txviscurx(Text tx)
{
	Line line;
	char *buf;
	int len;
	int x = 0;
	line = tx->lines + tx->curY;
	for(buf = line->buf, len = tx->curX; len; len--, buf++)
	{
		if(*buf == '\t')
			x += TXTABWIDTH - x % TXTABWIDTH;
		else
			x++;
	}
	return x - tx->scrollX;
}

void txdraw(Text tx)
{	
	// this buffer holds the line number
	char lnNBuf[(8 * sizeof(int) - 1) / 3 + 2];
	Line line;
	char *buf;
	int len;
	int y;
	int visCh;
	int index;
	int visX;

	for(int i = 0; i <= tx->height; i++)
	{
		y = i + tx->scrollY;
		if(y >= tx->lineCnt)
		{
			attron(COLOR_PAIR(2));
			mvaddch(i, tx->x - 2, '~');
			attroff(COLOR_PAIR(2));
		}
		else
		{
			snprintf(lnNBuf, sizeof lnNBuf, "%d", y + 1);
			attron(COLOR_PAIR(1));
			mvaddstr(i + tx->y, tx->x - 1 - strlen(lnNBuf), lnNBuf);
			attroff(COLOR_PAIR(1));
			
			index = -tx->scrollX;
			line = tx->lines + y;
			for(len = line->len, buf = line->buf; len; len--, buf++)
			{
				if(index >= 0 && !isspace(*buf))
					mvaddch(tx->y + i, tx->x + index, *buf);
				if(*buf == '\t')
					index += TXTABWIDTH - index % TXTABWIDTH;	
				else
					index++;
				if(index > tx->width)
					break;
			}
		}
		mvaddch(i + tx->y, tx->x + tx->width + 1, '#');
	}
	for(int i = 0; i <= tx->width; i++)
		mvaddch(tx->y + tx->height + 1, tx->x + i, '#');
	visX = _txviscurx(tx);
	move(tx->y + tx->curY - tx->scrollY, tx->x + visX);

}

void txmove(Text tx, int x, int y)
{
	int sx, sy; // scrolled x and y

	assert(x >= 0 && y >= 0 && y < tx->lineCnt && x <= tx->lines[y].len && "txmove > out of bounds");
	tx->curX = x;
	tx->curY = y;
	// make caret visible if it is outside
	sx = _txviscurx(tx);
	sy = y - tx->scrollY;
	if(sx < 0)
		tx->scrollX = max(sx + tx->scrollX - TXTABWIDTH, 0);
	else if(sx > tx->width)
		tx->scrollX = sx + tx->scrollX - tx->width + TXTABWIDTH;
	// same for y
	if(sy < 0)
		tx->scrollY = y;
	else if(sy > tx->height)
		tx->scrollY = y - tx->height;
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
	memmove(nextLine, line, (tx->lineCnt - tx->curY) * sizeof*line);
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

void _txinsertnstr(Line line, int index, const char *s, int n)
{
	char *dest;

	_txgrowline(line, line->len + n);
	dest = line->buf + index;
	memmove(dest + n, dest, line->len - index);
	line->len += n;
	memcpy(dest, s, n);
}

void _txdelete(Text tx, int x, int y)
{
	Line line;

	line = tx->lines + y;
	if(x == line->len)
	{
		// bring the next line up to this one
		if(y + 1 != tx->lineCnt)
		{
			Line nextLine = line + 1;
			_txinsertnstr(line, line->len, nextLine->buf, nextLine->len);
			free(nextLine->buf);
			tx->lineCnt--;
			memmove(nextLine, nextLine + 1, (tx->lineCnt - 1 - y) * sizeof*line);
		}
	}
	else
	{
		// remove char 
		char *dest = line->buf + x;
		line->len--;
		memmove(dest, dest + 1, line->len - x);
	}
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
