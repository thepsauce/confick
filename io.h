// #include "text.h"

// #include <stdio.h>


void txopen(Text tx, const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    assert(file);
    int c;
    int i = 0;
    while((c = fgetc(file)) != EOF)
    {
        _txinsertchar(&tx->lines[i], tx->lines[i].len, c);
        if(c == '\n' || c == '\r') {
            _txgrow(tx);
            ++i;
            memset(&tx->lines[i], 0, sizeof(*tx->lines));
            tx->lineCnt++;
        }
    }
    tx->fileName = strdup(fileName);
}

void txsave(Text tx)
{
    
}
