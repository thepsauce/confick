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
	C_STATE_MAX,
};
typedef struct c_tunit {
	_TUNIT_HEADER;
	cchar_t data[1024];
	int iWrite, iRead;
	int state, prevState;
	char *word;
	int nWord, szWord;
	bool escaped;
	int needHexChars;
} *C_tunit_t;

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

#include "word.h"
#include "string.h"
#include "comment.h"
#include "number.h"

int
C_init(C_tunit_t r)
{
	memset((void*) r + sizeof(struct tunit), 0, sizeof(*r) - sizeof(struct tunit));
	r->word = malloc(8);
	r->szWord = 8;
	return OK;
}

int
C_destroy(C_tunit_t r)
{
	free(r->word);
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
		r->state = C_STATE_WORD;
		r->word[0] = c;
		r->nWord = 1;
		break;
	case L'\"':
		C_buf_write(r, L"\"", A_NORMAL, C_PAIR_STRING1);
		r->state = C_STATE_STRING;
		break;
	case L'/':
		r->state = C_STATE_COMMENTSTART;
		break;
	case L'0':
		C_number_out(r, c);
		r->state = C_STATE_ZERO;
		break;
	case L'1' ... L'9':
		C_number_out(r, c);
		r->state = C_STATE_DECIMAL;
		break;
	case L'.':
		r->state = C_STATE_MAYBEFLOAT;
		break;
	case EOF - 1:
		break;
	case EOF:
		r->iWrite = r->iRead = 0;
		r->nWord = 0;
		r->escaped = 0;
		r->needHexChars = 0;
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
	};
	while(states[r->state](r, c));
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
