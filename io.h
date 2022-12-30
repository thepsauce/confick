void txopen(Text tx, const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    assert(file);
    int c, pc;
    int i = 0;
    pc = 0;
    while(pc != EOF)
    {
        c = fgetc(file);
        if(c != EOF)
            _txinsertchar(&tx->lines[i], tx->lines[i].len, c);
        if(c == '\n' || c == '\r')
        {
            _txgrow(tx);
            ++i;
            memset(&tx->lines[i], 0, sizeof(*tx->lines));
            tx->lineCnt++;
        }
        pc = c;
    }
    tx->fileName = strdup(fileName);
    fclose(file);
}

void txsave(Text tx)
{
    FILE *file = fopen(tx->fileName, "w");
    assert(file);
    for(int i = 0; i < tx->lineCnt; ++i)
    {
        fwrite(tx->lines[i].buf, sizeof(char), tx->lines[i].len, file);
    }
    fclose(file);
}
