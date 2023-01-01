Text *Texts;
int TxCnt, TxCap;
int SizeX, SizeY;
int Focus;
int Rotation;

void _txmgrupdate(void)
{
	int x, y, w, h;
	Text *ptx, tx;
	int cnt;
	int dir;

	if(!SizeX || !SizeY)
		return;
	
	dir = 1;
	ptx = Texts;
	cnt = TxCnt;
	switch(Rotation)
	{
	case 2:
		dir = -1;
		ptx += TxCnt - 1;
	case 0:
		x = 0;
		w = (SizeX - 1) / TxCnt;
		if(w > 5)
		{
			for(; --cnt; ptx += dir)
			{
				tx = *ptx;
				tx->x = x + 5;
				tx->y = 0;
				tx->width = w - 5 - 1;
				tx->height = SizeY - 1 - 1;
				x += w + 1;
			}
		}
		tx = *ptx;
		tx->x = x + 5;
		tx->y = 0;
		tx->width = SizeX - 1 - (x + 5);
		tx->height = SizeY - 1 - 1;
		break;
	case 3:
		dir = -1;
		ptx += TxCnt - 1;
	case 1:
		y = 0;
		h = (SizeY - 1) / TxCnt;
		if(h > 1)
		{
			for(; --cnt; ptx += dir)
			{
				tx = *ptx;
				tx->x = 5;
				tx->y = y;
				tx->width = SizeX - 5 - 1;
				tx->height = h - 1;
				y += h + 1;
			}
		}
		tx = *ptx;
		tx->x = 5;
		tx->y = y;
		tx->width = SizeX - 1 - 5;
		tx->height = SizeY - 1 - y - 1;
		break;
	}
}

void txmgrupdate(int szX, int szY)
{
	if(SizeX == szX || SizeY == szY)
		return;
	SizeX = szX;
	SizeY = szY;
	_txmgrupdate();
}

void txmgradd(Text tx)
{
	if(TxCnt + 1 > TxCap)
	{
		TxCap *= 2;
		TxCap++;
		Texts = realloc(Texts, TxCap * sizeof*Texts);
	}
	Focus = TxCnt;
	Texts[TxCnt++] = tx;
	_txmgrupdate();
}

void txremove(Text tx)
{
	Text *ptx;
	int cnt;
	
	for(ptx = Texts, cnt = TxCnt; cnt; ptx++)
	{
		cnt--;
		if(*ptx == tx)
		{
			memmove(ptx, ptx + 1, cnt * sizeof*ptx);
			TxCnt--;
			if(Focus == TxCnt)
				Focus--;
			if(!TxCnt)
				txmgrdiscard();
			else
				_txmgrupdate();
			break;
		}
	}
}

void txmgrdiscard(void)
{
	for(int t = 0; t < TxCnt; t++)
		txfree(Texts[t]);
	free(Texts);
	Texts = NULL;
	TxCnt = TxCap = 0;
	endwin();
	exit(0);
}

void txmgrdraw(void)
{
	int w;
	int visX;

	for(int t = 0; t < TxCnt; t++)
		txdraw(Texts[t]);
	// draw barrier
	attron(A_DIM | COLOR_PAIR(3));
	if(!(Rotation % 2))
	{
		w = (SizeX - 1) / TxCnt;
		for(int t = 1; t < TxCnt; t++)
			for(int y = 0; y < SizeY; y++)
				mvaddch(y, w * t + t - 1, ' ');
	}
	attroff(A_DIM | COLOR_PAIR(3));
	// draw cursor
	visX = _txviscurx(Texts[Focus], Texts[Focus]->curX, Texts[Focus]->curY);
	move(Texts[Focus]->y + Texts[Focus]->curY - Texts[Focus]->scrollY, Texts[Focus]->x + visX);
}

Text txmgrgetfocus(void)
{
	return Texts[Focus];
}

void txmgrfocusnext(void)
{
	Focus++;
	Focus %= TxCnt;
}

void txmgrrotate(void)
{
	Rotation++;
	Rotation %= 4;
	_txmgrupdate();
}
