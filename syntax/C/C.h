enum {
	C_STATE_GLOBAL,
	C_STATE_WORD,
};
typedef struct c_receiver {
	int state;
	char *word;
	int nWord, szWord;
} *C_receiver_t;

#include "word.h"

void *
C_create(void)
{
	C_receiver_t r;
	
	if(!(r = malloc(sizeof*r)))
		return ERROR("out of memory", NULL);
	memset(r, 0, sizeof*r);
	r->word = malloc(8);
	r->szWord = 8;
	return r;
}

int 
C_destroy(C_receiver_t r)
{
	free(r->word);
	free(r);
	return OK;
}

int
C_receive(C_receiver_t r, cursor_t cursor, int c)
{
	switch(r->state)
	{
	case C_STATE_GLOBAL:
		if(c)
		if(isalpha(c) || c == '_' || c == '$')
		{
			r->state = C_STATE_WORD;
			r->word[0] = c;
			r->nWord = 1;
		}
		else
			curputc(cursor, c | COLOR_PAIR(C_PAIR_TEXT));
		break;
	case C_STATE_WORD:
		return C_state_globalword(r, cursor, c);
	}
	return 0;
}

