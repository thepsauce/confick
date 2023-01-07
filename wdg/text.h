TextWidget txinit(TextWidget tx)
{
	tx->lines = malloc(sizeof*tx->lines);
	tx->lineCap = 1;
	tx->lineCnt = 1;
	memset(tx->lines, 0, sizeof*tx->lines);
	return tx;
}

void txuninit(TextWidget tx)
{
	for(int i = 0; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	free(tx->lines);
	free(tx->fileName);
}

void txclear(TextWidget tx)
{
	tx->lines[0].len = 0;
	for(int i = 1; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	tx->lineCnt = 1;
	if(tx->lineCap > TXCLEARKEEPTHRESHOLD)
	{
		struct line tmp = *tx->lines;
		free(tx->lines);
		tx->lines = malloc(sizeof*tx->lines);
		tx->lineCap = 1;
		*tx->lines = tmp;
	}
	tx->curX = 0;
	tx->curY = 0;
	tx->scrollX = 0;
	tx->scrollY = 0;
	free(tx->fileName);
	tx->fileName = NULL;
}

int _txshiftvisx(TextWidget tx, int visX, int y)
{
	Line line;
	int len;
	char *buf;
	int index;

	visX += tx->scrollX;
	index = 0;
	line = tx->lines + y;
	for(len = line->len, buf = line->buf; index < visX && len; len--, buf++)
	{
		if(*buf == '\t')
			index += TXTABWIDTH - index % TXTABWIDTH;
		else
			index++;
	}
	return line->len - len;
}

int _txviscurx(TextWidget tx, int x, int y)
{
	Line line;
	char *buf;
	int len;
	int visX = 0;
	line = tx->lines + y;
	for(buf = line->buf, len = x; len; len--, buf++)
	{
		if(*buf == '\t')
			visX += TXTABWIDTH - visX % TXTABWIDTH;
		else
			visX++;
	}
	return visX - tx->scrollX;
}

void txmove(TextWidget tx, int x, int y)
{
	int sx, sy; // scrolled x and y

	assert(x >= 0 && y >= 0 && y < tx->lineCnt && x <= tx->lines[y].len && "txmove > out of bounds");
	tx->curX = x;
	tx->curY = y;
	// make caret visible if it is outside
	sx = _txviscurx(tx, x, y);
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

void txevent(TextWidget tx, int eId)
{
	if(eId <= 255)
		txputc(tx, eId);
}

void txdraw(TextWidget tx)
{	
	char strBuf[512];
	Line line;
	char *buf;
	int len;
	int y;
	int visX;
	int a;

	for(int i = 0; i <= tx->height; i++)
	{
		y = i + tx->scrollY;
		if(y >= tx->lineCnt)
		{
			attron(COLOR_PAIR(2));
			mvaddch(tx->y + i, tx->x - 2, '~');
			attroff(COLOR_PAIR(2));
		}
		else
		{
			snprintf(strBuf, sizeof strBuf, "%d", y + 1);
			attron(COLOR_PAIR(1));
			mvaddstr(tx->y + i, tx->x - 1 - strlen(strBuf), strBuf);
			attroff(COLOR_PAIR(1));
			
			visX = 0;
			line = tx->lines + y;
			for(len = line->len, buf = line->buf; len; len--, buf++)
			{
				if(visX >= tx->scrollX && !isspace(*buf))
					mvaddch(tx->y + i, tx->x - tx->scrollX + visX, *buf);
				if(*buf == '\t')
					visX += TXTABWIDTH - visX % TXTABWIDTH;	
				else
					visX++;
				if(visX - tx->scrollX > tx->width)
					break;
			}
		}
	}
	a = COLOR_PAIR(3);
	if(wdgmgrgetfocus() == (Widget) tx)
		a |= A_BOLD;
	attron(a);
	snprintf(strBuf, sizeof strBuf, "%s:%d:%d", tx->fileName ? : "", tx->curY + 1, tx->curX + 1);
	len = min((int) strlen(strBuf), tx->width + 5);
	mvaddnstr(tx->y + tx->height + 1, tx->x - 5, strBuf, len);
	for(int i = len; i <= tx->width + 5; i++)
		addch(' ');
	attroff(a);
}

void _txgrow(TextWidget tx)
{
	if(tx->lineCnt + 1 > tx->lineCap)
	{
		tx->lineCap *= 2;
		tx->lineCap++;
		tx->lines = realloc(tx->lines, tx->lineCap * sizeof*tx->lines);
	}
}

void txbreak(TextWidget tx)
{
	Line line, nextLine;
	int breakLen;
	
	_txgrow(tx);
	line = tx->lines + tx->curY;
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

void txputc(TextWidget tx, int c)
{
	Line line;

	if(c == '\r' || c == '\n')
	{
		txbreak(tx);
	}
	else
	{
		line = txline(tx);
		_txinsertchar(line, tx->curX, c);
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

void _txdelete(TextWidget tx, int x, int y)
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

void txputs(TextWidget tx, const char *s)
{
	const char *e;
	int l;

	while(1)
	{
		e = _txlinesep(s);
		l = e ? (int) (e - s) : (int) strlen(s);
		_txinsertnstr(txline(tx), tx->curX, s, l);
		tx->curX += l;
		if(!e)
			break;
		txbreak(tx);
		s = e + _txlineseplen(e);
	}
	// update cursor
	txmove(tx, tx->curX, tx->curY);
}


void txup(TextWidget tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(y)
	{
		x = _txviscurx(tx, x, y);
		y--;
		x = _txshiftvisx(tx, x, y);
	}

	txmove(tx, x, y);
}

void txleft(TextWidget tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(!x)
	{
		if(y)
		{
			y--;
			x = tx->lines[y].len;
		}
	}
	else 
	{
		x--;
	}

	txmove(tx, x, y);
}

void txright(TextWidget tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(x == tx->lines[y].len)
	{
		if(y + 1 != tx->lineCnt)
		{
			y++;
			x = 0;
		}
	}
	else
	{
		x++;
	}

	txmove(tx, x, y);
}

void txdown(TextWidget tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(y + 1 != tx->lineCnt)
	{
		x = _txviscurx(tx, x, y);
		y++;
		x = _txshiftvisx(tx, x, y);
	}

	txmove(tx, x, y);
}

void txhome(TextWidget tx)
{
	int x, y;

	y = 0;
	x = 0;
	txmove(tx, x, y);
}

void txend(TextWidget tx)
{
	int x, y;
	
	y = tx->lineCnt - 1;
	x = tx->lines[y].len;
	txmove(tx, x, y);
}

void txdelete(TextWidget tx)
{
	int x, y;

	x = tx->curX;
    y = tx->curY;

	// only delete if the cursor is not at the end of the text
	if(y + 1 != tx->lineCnt || x != tx->lines[y].len)
		_txdelete(tx, x, y);	

	txmove(tx, x, y);
}

void txbackdelete(TextWidget tx)
{
	int x, y;

	x = tx->curX;
    y = tx->curY;

	if(!x)
	{
		if(y)
		{
			y--;
			x = tx->lines[y].len;
		}
		else
		{
			// nothing to delete
			goto no_delete;
		}
	}
	else
	{
		x--;
	}
	_txdelete(tx, x, y);
no_delete:
	txmove(tx, x, y);
}

void txopen(TextWidget tx, const char *fileName)
{
	FILE *file;
	int c;

	txclear(tx);
	tx->fileName = strdup(fileName);
	if(!(file = fopen(fileName, "r")))
    	return;
	while((c = fgetc(file)) != EOF)
	{
        if(c == '\n')
        {
            _txgrow(tx);
            memset(&tx->lines[tx->lineCnt++], 0, sizeof*tx->lines);
        }
		else
		{
            _txinsertchar(&tx->lines[tx->lineCnt - 1], tx->lines[tx->lineCnt - 1].len, c);
    	}
	}
    fclose(file);
}

void txsave(TextWidget tx)
{
	FILE *file;

    file = fopen(tx->fileName, "w");
    assert(file);
	fwrite(tx->lines->buf, 1, tx->lines->len, file);
    for(int i = 1; i < tx->lineCnt; i++)
    {
        fputc('\n', file);
        fwrite(tx->lines[i].buf, 1, tx->lines[i].len, file);
    }
    fclose(file);
}
