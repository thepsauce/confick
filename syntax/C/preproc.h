const char *preProcessorWords[] = {
	"assert",
	"define",
	"elif",
	"else",
	"endif",
	"error",
	"ident",
	"if",
	"ifdef",
	"ifndef",
	"import",
	"include",
	"include_next",
	"line",
	"pragma",
	"sccs",
	"unassert",
	"undef",
	"warning",
};

int
C_state_pp(C_tunit_t r,
		int c)
{
	if(isspace(c))
		C_word_push(r, c);
	else if(C_isidentf(c))
	{
		C_word_pop(r, 0, C_PAIR_TEXT);
		r->word[0] = c;
		r->nWord = 1;
		C_setstate(r, C_STATE_PPWORD);
	}
	else
	{
		C_word_pop(r, 0, C_PAIR_TEXT);
		C_popstate(r);
	}
	return 0;
}

int
C_state_ppword(C_tunit_t r,
		int c)
{
	if(C_isidentfnum(c))
	{
		C_word_push(r, c);
		return 0;
	}
	for(int i = 0; i < ARRLEN(preProcessorWords); i++)
	{
		if(strlen(preProcessorWords[i]) == r->nWord && !memcmp(preProcessorWords[i], r->word, r->nWord))
		{
			C_word_pop(r, A_BOLD, C_PAIR_PREPROC2);
			C_popstate(r);
			return 1;
		}
	}
	C_word_pop(r, 0, C_PAIR_PREPROC1);
	C_popstate(r);
	return 1;
}

int
C_state_ppstring(C_tunit_t r,
		int c)
{
	return 0;
}
