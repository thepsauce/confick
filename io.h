void txopen(Text tx, const char *fileName)
{
	FILE *file;
	int c;
	int i;

	txclear(tx);
	tx->fileName = strdup(fileName);
    if(!(file = fopen(fileName, "r")))
        return;
    i = 0;
	while((c = fgetc(file)) != EOF)
    {
        if(c == '\n')
        {
            _txgrow(tx);
            i++;
            memset(&tx->lines[i], 0, sizeof*tx->lines);
            tx->lineCnt++;
        }
		else
		{
            _txinsertchar(&tx->lines[i], tx->lines[i].len, c);
    	}
	}
    fclose(file);
}

void txsave(Text tx)
{
	FILE *file;
	int i;

    file = fopen(tx->fileName, "w");
    assert(file);
	fwrite(tx->lines->buf, 1, tx->lines->len, file);
    for(i = 1; i < tx->lineCnt; i++)
    {
        fputc('\n', file);
        fwrite(tx->lines[i].buf, 1, tx->lines[i].len, file);
    }
    fclose(file);
}
