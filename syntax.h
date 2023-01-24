/* Cursor */
int
curmove(cursor_t cursor,
		int x,
		int y)
{
	if(!cursor)
		return ERROR("cursor is null");

	cursor->x = x;
	cursor->y = y;
	return OK;
}

int
curputc(cursor_t cursor,
		int c)
{
	if(!cursor)
		return ERROR("cursor is null");
	//iif(cursor->x >= cursor->minX && cursor->x <= cursor->maxY &&
	//	cursor->y >= cursor->minY && cursor->y <= cursor->maxY)
		mvaddch(cursor->y, cursor->x, c);
	if(c == '\n')
	{
		cursor->y++;
		cursor->x = cursor->minX;
	}
	else
		cursor->x++;
	return OK;
}

int
curputs(cursor_t cursor,
		const char *n)
{
	if(!cursor)
		return ERROR("cursor is null");
	// TODO
	return OK;
}

/* Receiver */
struct {
	receiverbase_t *bases;
	int n, sz;	
} ReceiverBases;

int
recvaddbase(const char *name,
		void *(*create)(void),
		int (*destroy)(void *receiver),
		int (*receive)(void *receive, cursor_t cursor, int c))
{
	receiverbase_t base;

	if(!name)
		return ERROR("name is null");
	if(!create || !destroy || !receive)
		return ERROR("a function is null");

	base = malloc(sizeof*base);
	if(!base)
		return ERROR("out of memory");
	if(ReceiverBases.n + 1 > ReceiverBases.sz)
	{
		ReceiverBases.sz *= 2;
		ReceiverBases.sz++;
		ReceiverBases.bases = realloc(ReceiverBases.bases, ReceiverBases.sz * sizeof*ReceiverBases.bases);
		if(!ReceiverBases.bases)
		{
			free(base);
			return ERROR("out of memory");
		}
	}
	ReceiverBases.bases[ReceiverBases.n++] = base;
	base->name = strdup(name);
	base->create = create;
	base->destroy = destroy;
	base->receive = receive;
	return OK;
}

receiverbase_t recvgetbase(const char *name)
{
	for(int i = 0; i < ReceiverBases.n; i++)
		if(!strcmp(ReceiverBases.bases[i]->name, name))
			return ReceiverBases.bases[i];
	return WARN("base was not found", NULL);
}

/* Syntax */
syntax_t
syncreate(const char *nameReceiverDrawBase, 
		const char *inputReceiverDrawBase)
{
	syntax_t syntax;

	syntax = malloc(sizeof*syntax);
	if(!syntax)
		return ERROR("out of memory", NULL);
	memset(syntax, 0, sizeof*syntax);
	if(!(syntax->draw = recvgetbase(nameReceiverDrawBase)))
	{
		free(syntax);
		return ERROR("invalid name for draw receiver", NULL);
	}
	if(!(syntax->input = recvgetbase(inputReceiverDrawBase)))
	{
		free(syntax);
		return ERROR("invalid name for input receiver", NULL);
	}
	return syntax;
}

int
synfree(syntax_t syntax)
{
	if(!syntax)
		return ERROR("syntax is null");
	if(syntax->drawReceiver)
		syntax->draw->destroy(syntax->drawReceiver);
	if(syntax->inputReceiver)
		syntax->input->destroy(syntax->inputReceiver);
	free(syntax);
	return OK;
}

int
synfeed(syntax_t syntax,
		void *data,
		int r,
		int c)
{
	if(!syntax)
		return ERROR("syntax is null");
	
	if(r == SYNFEEDDRAW)
	{
		if(c == EOF)
		{
			syntax->draw->destroy(syntax->drawReceiver);
			syntax->drawReceiver = NULL;
		}
		else
		{
			if(!syntax->drawReceiver && !(syntax->drawReceiver = syntax->draw->create()))
					return -1;
			syntax->draw->receive(syntax->drawReceiver, (cursor_t) data, c);
		}
	}
	else
	{
		if(!syntax->inputReceiver && !(syntax->inputReceiver = syntax->input->create()))
			return -1;
		syntax->input->receive(syntax->inputReceiver, (text_t*) data, c);
	}		
	return OK;
}
