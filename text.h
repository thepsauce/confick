/* Line */
int
lnaddnstr(line_t *line, 
		int index, 
		const wchar_t *str, 
		int nStr) 
{
	wchar_t *buf;
	int nBuf, nNewBuf;
	int szBuf;
	int nTail;

	nBuf = line->nBuf;
	if(!line || index < 0 || index > nBuf)
		return ERROR(!line ? "line is null" : "index out of bounds");

	buf = line->buf;
	nNewBuf = nBuf + nStr;
	szBuf = line->szBuf;
	if(nNewBuf > szBuf)
	{
		szBuf *= 2;
		szBuf += nStr;
		buf = realloc(buf, szBuf * sizeof*buf);
		if(!buf)
			return ERROR("out of memory");

		line->buf = buf;
		line->szBuf = szBuf;
	}
	buf += index;
	nTail = nBuf - index;
	memmove(buf + nStr, buf, nTail * sizeof*buf);
	if(str)
		memcpy(buf, str, nStr * sizeof*buf);

	line->nBuf = nNewBuf;
	return OK;
}

int lnadd(line_t *line,
		int index,
		wchar_t ch)
{
	return lnaddnstr(line, index, &ch, 1);
}

int
lnaddstr(line_t *line,
		int index,
		const wchar_t *str)
{
	if(!str)
		return ERROR("string is null");
	return lnaddnstr(line, index, str, wcslen(str));
}

/* Text */
int 
txinit(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	text->lines = malloc(sizeof*text->lines);
	if(!text->lines)
		return ERROR("out of memory");
	memset(text->lines, 0, sizeof*text->lines);
	text->nLines = 1;
	text->szLines = 1;
	return OK;
}

int 
txdiscard(text_t *text)
{
	if(txclear(text))
		return -1;

	free(text->lines);
	return OK;
}

int
txclear(text_t *text)
{
	const int clearKeepThreshold = 64;
	struct line tmp;

	if(!text)
		return ERROR("text is null");

	text->lines[0].nBuf = 0;
	for(int i = 1; i < text->nLines; i++)
		free(text->lines[i].buf);
	text->nLines = 1;
	if(text->szLines > clearKeepThreshold)
	{
		tmp = *text->lines;
		free(text->lines);
		text->lines = malloc(sizeof*text->lines);
		if(!text->lines)
			return ERROR("out of memory");
		text->szLines = 1;
		*text->lines = tmp;
	}
	text->cursor.x = 0;
	text->cursor.y = 0;
	return OK;
}

int
txopen(text_t *text,
		const char *fileName)
{
	FILE *file;
	int c;
	line_t *lines;
	int nLines;
	int szLines;

	if(txclear(text))
		return ERROR("text is null");

	text->fileName = strdup(fileName);
	if(!(file = fopen(fileName, "r")))
    	return WARN("file could not be opened");
	lines = text->lines;
	nLines = text->nLines;
	szLines = text->szLines;
	while((c = fgetwc(file)) != EOF)
	{
        if(c == '\n')
        {
			if(nLines + 1 > szLines)
			{
				szLines *= 2;
				szLines++;
				lines = realloc(lines, szLines * sizeof*lines);
				if(!lines)
					return ERROR("out of memory");
			}
            memset(&lines[nLines], 0, sizeof*lines);
			nLines++;
        }
		else
		{
			lnadd(&lines[nLines - 1], lines[nLines - 1].nBuf, c);
		}
	}
    fclose(file);
	text->lines = lines;
	text->nLines = nLines;
	text->szLines = szLines;
	return OK;
}

int
txsave(text_t *text)
{
	FILE *file;
	line_t *line;
	int nLines;

	if(!text)
		return ERROR("text is null");

    file = fopen(text->fileName, "w");
    if(!file)
		return WARN("file could not be opened");
	line = text->lines;
	nLines = text->nLines;
	fwrite(line->buf, 1, line->nBuf, file);
    for(line++, nLines--; nLines; line++, nLines--)
    {
        fputc('\n', file);
        fwrite(line->buf, 1, line->nBuf, file);
    }
    fclose(file);
	return OK;
}

int
txbreak(text_t *text)
{
	line_t *lines;
	int nLines, nNewLines;
	int szLines;
	int iLine;
	int index;
	wchar_t *tail;
	int nTail;

	if(!text)
		return ERROR("text is null");

	lines = text->lines;
	nLines = text->nLines;
	nNewLines = nLines + 1;
	szLines = text->szLines;
	iLine = text->cursor.y;
	index = text->cursor.x;
	if(nNewLines > szLines)
	{
		szLines *= 2;
		szLines++;
		lines = realloc(lines, szLines * sizeof*lines);
		if(!lines)
			return ERROR("out of memory");

		text->lines = lines;
		text->szLines = szLines;
	}
	memmove(&lines[iLine + 1], &lines[iLine], (nLines - iLine) * sizeof*lines);
	nLines = nNewLines;

	tail = lines[iLine].buf + index;
	nTail = lines[iLine].nBuf - index;
	lines[iLine].nBuf = index;

	iLine++;
	lines[iLine].buf = malloc(nTail * sizeof*tail);
	lines[iLine].nBuf = nTail;
	lines[iLine].szBuf = nTail;
	memcpy(lines[iLine].buf, tail, nTail * sizeof*tail);

	text->nLines = nLines;

	text->cursor.y = iLine;
	text->cursor.x = 0;
	return OK;
}

int
txadd(text_t *text,
		int c)
{
	line_t *lines;
	int iLine;
	int index;

	if(!text)
		return ERROR("text is null");

	lines = text->lines;
	iLine = text->cursor.y;
	index = text->cursor.x;
	if(c == L'\n')
		return txbreak(text);
	text->cursor.x++;
	return lnadd(&lines[iLine], index, c);
}

int 
txaddnstr(text_t *text, 
		const wchar_t *str,
		int nStr)
{
	int iLine;
	line_t *lines;
	int n;
	int index;
	const wchar_t *lineStart;

	if(!text)
		return ERROR("text is null");

	iLine = text->cursor.y;
	lines = text->lines;
	index = text->cursor.x;
	for(;; nStr--, str++)
	{
		lineStart = str;
		for(; nStr && *str != '\n'; nStr--, str++);
		n = str - lineStart;
		lnaddnstr(&lines[iLine], index, lineStart, n);
		if(!nStr)
			break;
		text->cursor.x = index + n;
		txbreak(text); // TODO: OPTIMIZE
		lines = text->lines;

		iLine++;
		index = 0;
	}
	return OK;
}

int
txaddstr(text_t *text,
		const wchar_t *str)
{
	if(!str)
		return ERROR("string is null");
	return txaddnstr(text, str, wcslen(str));
}

int
txremove(text_t *text)
{
	int nLines;
	int iLine;
	int index;
	line_t *line, *nextLine;

	if(!text)
		return ERROR("text is null");

	nLines = text->nLines;
	iLine = text->cursor.y;
	index = text->cursor.x;
	line = text->lines + iLine;
	if(index == line->nBuf)
	{
		if(iLine + 1 != nLines && (text->flags & TXFLINECROSSING))
		{
			nextLine = line + 1;
			lnaddnstr(line, line->nBuf, nextLine->buf, nextLine->nBuf);
			free(nextLine->buf);
			nLines--;
			memmove(nextLine, nextLine + 1, (nLines - 1 - iLine) * sizeof*line);

			text->nLines = nLines;
		}
	}
	else
	{
		line->nBuf--;
		memmove(line->buf + index, line->buf + index + 1, (line->nBuf - index) * sizeof*line->buf);
	}
	return OK;
}

int
txleft(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	if(!text->cursor.x)
	{
		if(text->cursor.y && (text->flags & TXFLINECROSSING))
		{
			text->cursor.y--;
			text->cursor.x = text->lines[text->cursor.y].nBuf;
		}
	}
	else
		text->cursor.x--;
	return OK;
}

int
txleftremove(text_t *text)
{
	return !txleft(text) && !txremove(text);
}


int 
txright(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	if(text->cursor.x == text->lines[text->cursor.y].nBuf)
	{
		if(text->cursor.y + 1 < text->nLines && (text->flags & TXFLINECROSSING))
		{
			text->cursor.y++;
			text->cursor.x = 0;
		}
	}
	else
		text->cursor.x++;
	return OK;
}

int 
txup(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	if(text->cursor.y)
	{
		text->cursor.y--;
		text->cursor.x = min(text->cursor.x, text->lines[text->cursor.y].nBuf);
	}
	return OK;
}

int 
txdown(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	if(text->cursor.y + 1 < text->nLines)
	{
		text->cursor.y++;
		text->cursor.x = min(text->cursor.x, text->lines[text->cursor.y].nBuf);
	}
	return OK;
}

int 
txhome(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	text->cursor.x = 0;
	return OK;
}

int
txend(text_t *text)
{
	if(!text)
		return ERROR("text is null");

	text->cursor.x = text->lines[text->cursor.y].nBuf;
	return OK;
}


