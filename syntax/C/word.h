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
	"white",

	"FILE",
};

// 0 - char received
// 1 - end of word
int C_word(C_receiver_t r, int c)
{
	if(isalnum(c) || c == '_' || c == '$')
	{
		if(r->nWord + 1 > r->szWord)
		{
			r->szWord *= 2;
			r->szWord++;
			r->word = realloc(r->word, r->szWord);
		}
		r->word[r->nWord++] = c;
		return 0;
	}
	return 1;
}

int C_state_globalword(C_receiver_t r, cursor_t cursor, int c)
{
	int i;
	
	if(!C_word(r, c))
		return 0;

	c = COLOR_PAIR(C_PAIR_TEXT);
	for(int i = 0; i < ARRLEN(keywords); i++)
		if(strlen(keywords[i]) == r->nWord && !memcmp(keywords[i], r->word, r->nWord))
		{
			c = A_BOLD | COLOR_PAIR(C_PAIR_KEYWORD1);
			break;
		}
	r->state = C_STATE_GLOBAL;
	curputns(cursor, c, r->word, r->nWord);
	return 1;
}
