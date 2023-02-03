enum {
	C_STATE_GLOBAL,
	
	C_STATE_WORD,
	
	C_STATE_STRING,

	C_STATE_ZERO,
	C_STATE_DECIMAL,
	C_STATE_HEX,
	C_STATE_OCTAL,
	C_STATE_BINARY,
	C_STATE_USUFFIX,
	C_STATE_LSUFFIX,
	C_STATE_FLOAT,
	C_STATE_MAYBEFLOAT,
	C_STATE_EXP,
	C_STATE_ERRSUFFIX,

	C_STATE_LINECOMMENT,
	C_STATE_COMMENT,
	C_STATE_COMMENTSTART,
	C_STATE_MAYBECOMMENTEND,

	C_STATE_PP,
	C_STATE_PPWORD,
	C_STATE_PPSTRING,

	C_STATE_MAX,
};
#define C_WORDTYPE_MACRO 1
#define C_WORDTYPE_FUNCTION 2
#define C_WORDTYPE_ENUM 3
#define C_WORDTYPE_OTHER 4
typedef struct c_tunit {
	_TUNIT_HEADER;
	cchar_t data[1024];
	int iWrite, iRead;
	int states[32];
	int iState;
	char *word;
	int nWord, szWord;
	bool escaped;
	int needHexChars;
	wchar_t delimiter;
	struct {
		int type;
		char *name;
		char **args;
		int nArgs;
	} *words;
	int nWords, szWords;
} *C_tunit_t;

void
C_pushstate(C_tunit_t r,
		int newState)
{
	r->states[++r->iState] = newState;
}

void
C_popstate(C_tunit_t r)
{
	r->iState--;
}

void
C_setstate(C_tunit_t r,
		int newState)
{
	r->states[r->iState] = newState;
}

void
C_buf_out(C_tunit_t r, int c, attr_t a, short color_pair)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	setcchar(r->data + r->iWrite, w, a, color_pair, NULL);
	r->iWrite++;
	r->iWrite %= ARRLEN(r->data);
}

void
C_buf_write(C_tunit_t r, wchar_t *w, attr_t a, short color_pair)
{
	setcchar(r->data + r->iWrite, w, a, color_pair, NULL);
	r->iWrite++;
	r->iWrite %= ARRLEN(r->data);
}

void
C_word_push(C_tunit_t r, int c)
{
	if(r->nWord + 1 > r->szWord)
	{
		r->szWord *= 2;
		r->szWord++;
		r->word = safe_realloc(r->word, r->szWord);
	}
	r->word[r->nWord++] = c;
}

void
C_word_pop(C_tunit_t r, attr_t a, short color_pair)
{
	for(int i = 0; i < r->nWord; i++)
		C_buf_out(r, r->word[i], a, color_pair);
	r->nWord = 0;
}

#define C_isidentf(c) (isalpha(c)||(c)=='_'||c=='$')
#define C_isidentfnum(c) (C_isidentf(c)||isdigit(c))

#include "word.h"
#include "string.h"
#include "comment.h"
#include "number.h"
#include "preproc.h"

int
C_init(C_tunit_t r)
{
	memset((void*) r + sizeof(struct tunit), 0, sizeof(*r) - sizeof(struct tunit));
	r->word = malloc(8 * sizeof*r->word);
	r->szWord = 8;
	r->words = malloc(8 * sizeof*r->words);
	r->szWords = 8;
	return OK;
}

int
C_destroy(C_tunit_t r)
{
	free(r->word);
	free(r->words);
	free(r);
	return OK;
}

int
C_state_global(C_tunit_t r, int c)
{
	wchar_t w[2];

	switch(c)
	{
	case L'a' ... L'z':
	case L'A' ... L'Z':
	case L'_': case L'$':
		C_pushstate(r, C_STATE_WORD);
		r->word[0] = c;
		r->nWord = 1;
		break;
	case L'\"':
		C_pushstate(r, C_STATE_STRING);
		C_buf_write(r, L"\"", A_NORMAL, C_PAIR_STRING1);
		break;
	case L'/':
		C_pushstate(r, C_STATE_COMMENTSTART);
		break;
	case L'0':
		C_pushstate(r, C_STATE_ZERO);
		C_number_out(r, c);
		break;
	case L'1' ... L'9':
		C_pushstate(r, C_STATE_DECIMAL);
		C_number_out(r, c);
		break;
	case L'.':
		C_pushstate(r, C_STATE_MAYBEFLOAT);
		break;
	case L'#':
		C_pushstate(r, C_STATE_PP);
		C_buf_out(r, L'#', 0, C_PAIR_PREPROC1);
		break;
	case EOF - 1:
		break;
	case EOF:
		r->iState = 0;
		r->states[0] = C_STATE_GLOBAL;
		r->iWrite = r->iRead = 0;
		r->nWord = 0;
		r->escaped = 0;
		r->needHexChars = 0;
		for(int i = 0; i < r->nWords; i++)
		{
			free(r->words[i].name);
			free(r->words[i].args);
		}
		r->nWords = 0;
		break;
	default:
		w[0] = c;
		w[1] = L'\0';
		C_buf_write(r, w, A_NORMAL, C_PAIR_TEXT);
	}	
	return 0;
}

int
C_write(C_tunit_t r, int c)
{
	int (*states[C_STATE_MAX])(C_tunit_t r, int c) = {
		[C_STATE_GLOBAL] = C_state_global,
		[C_STATE_WORD] = C_state_word,
		[C_STATE_STRING] = C_state_string,
		[C_STATE_COMMENTSTART] = C_state_commentstart,
		[C_STATE_LINECOMMENT] = C_state_linecomment,
		[C_STATE_COMMENT] = C_state_comment,
		[C_STATE_MAYBECOMMENTEND] = C_state_maybecommentend,
		[C_STATE_ZERO] = C_state_zero,
		[C_STATE_HEX] = C_state_hex,
		[C_STATE_OCTAL] = C_state_octal,
		[C_STATE_BINARY] = C_state_binary,
		[C_STATE_DECIMAL] = C_state_decimal,
		[C_STATE_USUFFIX] = C_state_usuffix,
		[C_STATE_LSUFFIX] = C_state_lsuffix,
		[C_STATE_ERRSUFFIX] = C_state_errsuffix,
		[C_STATE_FLOAT] = C_state_float,
		[C_STATE_MAYBEFLOAT] = C_state_maybefloat,
		[C_STATE_EXP] = C_state_exp,
		[C_STATE_PP] = C_state_pp,
		[C_STATE_PPWORD] = C_state_ppword,
	};
	while(states[r->states[r->iState]](r, c));
	return OK;
}

int
C_read(C_tunit_t r, cchar_t *cc)
{
	if(r->iWrite == r->iRead)
		return EOF;
	memcpy(cc, r->data + r->iRead, sizeof*cc);
	r->iRead++;
	r->iRead %= ARRLEN(r->data);
	return OK;
}
