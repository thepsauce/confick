#include <cfk/wdg.h>
	
int
txinit(text_t *text) 
{
	ERROR(!text, "text is null");

	memset(text, 0, sizeof*text);
	text->first = text->cur = safe_malloc(sizeof*text->cur);
	memset(text->cur, 0, sizeof*text->cur);
	return OK;
}

int
txdiscard(text_t *text)
{
	ERROR(!text, "text is null");

	txclear(text);
	free(text->first);
	return OK;
}
	
int
txclear(text_t *text)
{
	struct txbuffer *buf, *prev;

	ERROR(!text, "text is null");

	for(buf = text->cur; buf->next; buf = buf->next)
		;
	for(; prev = buf->prev; buf = prev)
		free(buf);

	text->cur = text->cur = buf;
	buf->nData = 0;
	text->cursor = 0;
	text->iCur = 0;
	return OK;
}

int
txopen(text_t *text,
		const char *fileName)
{
	FILE *file;
	int c;
	int iCur;
	struct txbuffer *buf, *next;

	if(txclear(text))
		return ERR;

	free(text->fileName);
	text->fileName = safe_strdup(fileName);
	WARN(!(file = fopen(fileName, "r")), "file could not be opened");
	
	buf = text->first;
	iCur = 0;
	while((c = fgetwc(file)) != EOF)
	{
		buf->data[buf->nData++] = c;
		if(buf->nData == TXBUFSIZE)
		{
			next = safe_malloc(sizeof*next);
			memset(next, 0, sizeof*next);
			buf->next = next;
			next->prev = buf;
			buf = next;
		}
	}
    fclose(file);
	if(buf->nData)
	{
		next = safe_malloc(sizeof*next);
		memset(next, 0, sizeof*next);
		buf->next = next;
		next->prev = buf;
		buf = next;
	}
	text->cur = buf;
	text->iCur = 0;
	return OK;
}

int
txsave(text_t *text)
{
	FILE *file;
	struct txbuffer *buf;
	wchar_t *data;
	int n;

	ERROR(!text, "text is null");
	WARN(!(file = fopen(text->fileName, "w")), "file could not be opened");
	
	for(buf = text->first; buf; buf = buf->next)
	for(n = buf->nData, data = buf->data; n; n--, data++)
		fputwc(*data, file);
    fclose(file);
	return OK;
}

int
txdelete(text_t *tx)
{
	struct txbuffer *buf, *next;

	ERROR(!tx, "text is null");

	buf = tx->cur;
	if(!buf->nData)
	{
		next = buf->next;
		if(next)
		{
			if(next->nData == 1)
			{
				if(next->next)
					next->next->prev = buf;
				buf->next = next->next;
				free(next);
			}
			else
			{
				next->nData--;
				memmove(next->data, next->data + 1, next->nData * sizeof*next->data);
			}
		}
	}
	else
	{
		buf->nData--;
		if(buf->nData == tx->iCur)
		{
			tx->cur = buf->next;
			tx->iCur = 0;
		}
		else
			memmove(buf->data + tx->iCur, buf->data + tx->iCur + 1, (buf->nData - tx->iCur) * sizeof*next->data);
	}
	return OK;
}

int
txaddnstr(text_t *tx,
		const wchar_t *str,
		int n)
{
	struct txbuffer *buf, *next;
	wchar_t *data;
	int nData;
	int delta;
	int needed;
	int space;
	int nOverflow = 0;
	int index;
	wchar_t overflow[TXBUFSIZE];
	
	ERROR(!tx, "text is null");
	ERROR(!str, "string is null");
	WARN(!n, "length is 0");

	buf = tx->cur;
	index = tx->iCur;
	tx->iCur += n;
append:
	data = buf->data + index;
	nData = buf->nData - index;
	space = TXBUFSIZE - index;
	space = min(n, space);
	if(nData)
	{
		nOverflow = nData;
		memcpy(overflow, data, nOverflow * sizeof*str);
	}
	memcpy(data, str, space * sizeof*str);
	index += space;
	buf->nData = index;
	n -= space;
	str += space;
	if(n)
	{
		next = safe_malloc(sizeof*next);
		memset(next, 0, sizeof*next);
		next->prev = buf;
		next->next = buf->next;
		if(buf->next)
			buf->next->prev = next;
		buf->next = next;
		buf = next;
		index = 0;
		goto append;
	}
	if(nOverflow)
	{
		str = overflow;
		n = nOverflow;
		nOverflow = 0;
		goto append;
	}
	while(tx->iCur > TXBUFSIZE)
	{
		tx->iCur -= TXBUFSIZE;
		tx->cur = tx->cur->next;
	}
	return OK;
}

int
txadd(text_t *tx,
		int c)
{
	struct txbuffer *buf, *next, *nextnext;

	ERROR(!tx, "text is null");

	buf = tx->cur;
	memmove(buf->data + tx->iCur + 1, buf->data + tx->iCur, buf->nData - tx->iCur);
	buf->data[tx->iCur] = c;
	buf->nData++;
	tx->iCur++;
	if(tx->iCur == TXBUFSIZE)
	{
		next = buf->next;
		if(!next)
		{
			next = safe_malloc(sizeof*next);
			memset(next, 0, sizeof*next);
		}
		else if(next->nData == TXBUFSIZE)
		{
			nextnext = next;
			next = safe_malloc(sizeof*next);
			memset(next, 0, sizeof*next);
			next->next = nextnext;
			nextnext->prev = next;
		}
		next->prev = buf;
		buf->next = next;
		tx->iCur = 0;
		tx->cur = next;
	}
	return OK;
}
