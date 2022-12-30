void txmotion_up(Text tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(y)
	{
		x = _txviscurx(tx);
		y--;
		x = _txshiftvisx(tx, x, y);
	}

	txmove(tx, x, y);
}

void txmotion_left(Text tx)
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

void txmotion_right(Text tx)
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

void txmotion_down(Text tx)
{
	int x, y;

	x = tx->curX;
	y = tx->curY;

	if(y + 1 != tx->lineCnt)
	{
		x = _txviscurx(tx);
		y++;
		x = _txshiftvisx(tx, x, y);
	}

	txmove(tx, x, y);
}

void txmotion_delete(Text tx)
{
	int x, y;

	x = tx->curX;
    y = tx->curY;

	// only delete if the cursor is not at the end of the text
	if(y + 1 != tx->lineCnt || x != tx->lines[y].len)
		_txdelete(tx, x, y);	

	txmove(tx, x, y);
}

void txmotion_backdelete(Text tx)
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

