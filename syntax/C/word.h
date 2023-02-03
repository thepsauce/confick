const char *keywords[] = {
	"auto",
	"break"
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"int",
	"long",
	"register",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"typeof",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while",

	"FILE",
};

int C_state_word(C_tunit_t r, int c)
{
	attr_t a;
	short color_pair;
	
	if(C_isidentfnum(c))
	{
		C_word_push(r, c);
		return 0;
	}

	if(r->nWord > 2 && r->word[r->nWord - 2] == '_' && r->word[r->nWord - 1] == 't')
	{
		a = A_BOLD;
		color_pair = C_PAIR_KEYWORD1;
	}
	else
	{
		for(int i = 0; i < ARRLEN(keywords); i++)
			if(strlen(keywords[i]) == r->nWord && !memcmp(keywords[i], r->word, r->nWord))
			{
				a = A_BOLD;
				color_pair = C_PAIR_KEYWORD1;
				goto is_keyword;
			}
		a = 0;
		color_pair = C_PAIR_TEXT;
		if(r->nWords + 1 > r->szWords)
		{
			r->szWords *= 2;
			r->szWords++;
			r->words = safe_realloc(r->words, r->szWords * sizeof*r->words);
		}
		r->words[r->nWords].name = safe_strndup(r->word, r->nWord);
		r->words[r->nWords].args = NULL;
		r->words[r->nWords].nArgs = 0;
		r->nWords++;
	}
is_keyword:
	C_word_pop(r, a, color_pair);
	C_popstate(r);
	return 1;
}
