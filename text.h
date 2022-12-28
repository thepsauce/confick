Text txcreate(int initLineCap)
{
	Text tx;

	tx = malloc(sizeof*tx);
	tx->flags = 0;
	tx->state = 0;
	tx->x = 0;
	tx->y = 0;
	tx->curX = 0;
	tx->curY = 0;
	initLineCap = max(initLineCap, 1);
	tx->lines = initLineCap ? malloc(initLineCap * sizeof*tx->lines) : NULL;
	tx->lineCap = initLineCap;
	tx->lineCnt = 0;
	memset(tx->lines, 0, initLinecap * sizeof*tx->lines);
	
	return tx;
}

void txfree(Text tx)
{
	for(int i = 0; i < tx->lineCnt; i++)
		free(tx->lines[i].buf);
	free(tx->lines);
	free(tx);
}
