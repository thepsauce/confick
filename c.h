void cputc(CWidget wdg, int c)
{
	struct token *toks[2];
	int tokc;

	tokc = 1;
	toks[0] = wdg.cur;
	if(!wdg.cursor)
	{
		if(wdg.cur->prev)
		{
			tokc++;
			toks[1] = wdg.cur->prev;
		}
	}
	else if(wdg.cursor == wdg->cur.len)
	{
		if(wdg.cur->next)
		{
			tokc++;
			toks[1] = wdg.cur->next;
		}
	}
	for(int i = 0; i < tokc; i++)
	{
		switch(c)
		{
		case 0 ... 9:
			if(toks[i].type == TTNUMBER)
			{
				
			}
		}
	}
}

