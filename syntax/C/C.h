typedef struct c_receiver {
	void *v;
} *C_receiver_t;

void *
C_create(void)
{
	C_receiver_t r;
	
	if(!(r = malloc(sizeof*r)))
		return ERROR("out of memory", NULL);
	r->v = NULL;
	return r;
}

int 
C_destroy(C_receiver_t r)
{
	free(r);
	return OK;
}

int
C_receive(C_receiver_t r, cursor_t cursor, int c)
{
	if(c == 'a')
	{
		curputc(cursor, c | COLOR_PAIR(C_PAIR_STRING1));
	}
	else
	{
		curputc(cursor, c | COLOR_PAIR(C_PAIR_TEXT));
	}
	return OK;
}

