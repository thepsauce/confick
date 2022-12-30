void txopen(Text tx, const char *fileName)
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

void txsave(Text tx)
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
