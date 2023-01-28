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
int C_word(C_tunit_t r, int c)
{
	if(isalnum(c) || c == L'_' || c == L'$')
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

int C_state_word(C_tunit_t r, int c)
{
	attr_t a;
	short color_pair;
	
	if(!C_word(r, c))
		return 0;

	a = 0;
	color_pair = C_PAIR_TEXT;
	for(int i = 0; i < ARRLEN(keywords); i++)
		if(strlen(keywords[i]) == r->nWord && !memcmp(keywords[i], r->word, r->nWord))
		{
			a = A_BOLD;
			color_pair = C_PAIR_KEYWORD1;
			break;
		}
	for(int i = 0; i < r->nWord; i++)
	{
		wchar_t w[2];
		w[0] = r->word[i];
		w[1] = L'\0';
		setcchar(r->data + r->iWrite, w, a, color_pair, NULL);
		r->iWrite++;
		r->iWrite %= ARRLEN(r->data);
	}
	r->state = r->prevState;
	return 1;
}
