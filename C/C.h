static const wchar_t *C_keywords[] = {
	L"auto",
	L"break",
	L"case",
	L"char",
	L"const",
	L"continue",
	L"default",
	L"do",
	L"double",
	L"else",
	L"enum",
	L"extern",
	L"float",
	L"for",
	L"goto",
	L"if",
	L"int",
	L"long",
	L"register",
	L"return",
	L"short",
	L"signed",
	L"sizeof",
	L"static",
	L"struct",
	L"switch",
	L"typedef",
	L"typeof",
	L"union",
	L"unsigned",
	L"void",
	L"volatile",
	L"while",

	L"FILE",
};

static const short C_bracketColors[] = {
	C_PAIR_CHAR,
	C_PAIR_STRING2,
	C_PAIR_KEYWORD1,
};

static const struct {
	wchar_t w, w2, to[3];
} C_chars[] = {
	{ L',',    0, L"" },
	{ L'.',    0, L"" },
	{ L';',    0, L"" },
	{ L':',    0, L"" },
	{ L'+',    0, L"" },
	{ L'-', L'>', L"\u2794 " },
	{ L'*',    0, L"" },
	{ L'/',    0, L"" },
	{ L'%',    0, L"" },
	{ L'<', L'=', L"\u2264" },
	{ L'>', L'=', L"\u2265" },
	{ L'=',    0, L"" },
	{ L'?',    0, L"" },
	{ L'!', L'=', L"\u2260" },
	{ L'~',    0, L"" },
	{ L'&',    0, L"" },
	{ L'|',    0, L"" },
};

enum {
	C_STATE_GLOBAL,
	C_STATE_WORD,
	C_STATE_ZERO,
	C_STATE_NUMBER,
	C_STATE_HEX,
	C_STATE_DECIMAL,
	C_STATE_OCTAL,
	C_STATE_BINARY,
	C_STATE_USUFFIX,
	C_STATE_LSUFFIX,
	C_STATE_ERRSUFFIX,
	C_STATE_MAYBEFLOAT,
	C_STATE_EXP,
	C_STATE_FLOAT,
	C_STATE_STRING,
	C_STATE_CHAR,
	C_STATE_MAYBECOMMENT,
	C_STATE_LINECOMMENT,
	C_STATE_COMMENT,
	C_STATE_FUSION,
};

struct c_state_info {
	WINDOW *window;
	int tx, ty;
	int x, y;
	wchar_t w;
	int state;
	bool (**stateFuncs)(struct state_info *si);
	int stateStack[64];
	int iState;

	int iFusion;

	wchar_t word[512];
	int nWord;
	int nOpenCorn;
	int nOpenRoun;
	int nOpenCurl;
	bool escaped;
	int needHexChars;
};

#include "number.h"
#include "string.h"
#include "comment.h"

bool
c_state_global(struct c_state_info *si)
{
	switch(si->w)
	{
	case EOF:
		break;
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '$': case '_':
		sipushandsetstate((struct state_info*) si, C_STATE_WORD);
		return 1;
	case '0':
		sipushandsetstate((struct state_info*) si, C_STATE_ZERO);
		return 1;
	case '1' ... '9':
		sipushandsetstate((struct state_info*) si, C_STATE_DECIMAL);
		return 1;
	case '\"':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING1);
		sipushandsetstate((struct state_info*) si, C_STATE_STRING);
		break;
	case '/':
		sipushandsetstate((struct state_info*) si, C_STATE_MAYBECOMMENT);
		break;
	case '(':
		siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenRoun % ARRLEN(C_bracketColors)]);
		si->nOpenRoun++;
		break;
	case '{':
		siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenCurl % ARRLEN(C_bracketColors)]);
		si->nOpenCurl++;
		break;
	case '[':
		siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenCorn % ARRLEN(C_bracketColors)]);
		si->nOpenCorn++;
		break;
	case ')':
		if(!si->nOpenRoun)
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		}
		else
		{
			si->nOpenRoun--;
			siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenRoun % ARRLEN(C_bracketColors)]);
		}
		break;
	case '}':
		if(!si->nOpenCurl)
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		}
		else
		{
			si->nOpenCurl--;
			siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenCurl % ARRLEN(C_bracketColors)]);
		}
		break;
	case ']':
		if(!si->nOpenCorn)
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		}
		else
		{
			si->nOpenCorn--;
			siadd((struct state_info*) si, si->w, 0, C_bracketColors[si->nOpenCorn % ARRLEN(C_bracketColors)]);
		}
		break;
	default:
		for(int i = 0; i < ARRLEN(C_chars); i++)
		{
			if(C_chars[i].w == si->w)
			{
				if(C_chars[i].w2)
				{
					si->iFusion = i;
					sipushandsetstate((struct state_info*) si, C_STATE_FUSION);
				}
				else
					siadd((struct state_info*) si, si->w, 0, C_PAIR_CHAR);
				return 0;
			}
		}
		siadd((struct state_info*) si, si->w, 0, C_PAIR_TEXT);
	}
	return 0;
}

bool
c_state_word(struct c_state_info *si)
{
	attr_t a;
	short color_pair;

	switch(si->w)
	{
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '0' ... '9':
	case '$': case '_':
		si->word[si->nWord++] = si->w;
		break;
	default:
		a = 0;
		color_pair = C_PAIR_TEXT;
		si->word[si->nWord] = 0;
		if(si->nWord > 2 && si->word[si->nWord - 2] == '_' && si->word[si->nWord - 1] == 't')
		{
			a = A_BOLD;
			color_pair = C_PAIR_KEYWORD1;
		}
		else for(int i = 0; i < ARRLEN(C_keywords); i++)
		{
			if(!wcscmp(si->word, C_keywords[i]))
			{
				a = A_BOLD;
				color_pair = C_PAIR_KEYWORD1;
				break;
			}
		}
		siaddword((struct state_info*) si, si->word, a, color_pair);
		si->nWord = 0;
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_fusion(struct c_state_info *si)
{
	if(si->w == C_chars[si->iFusion].w2)
	{
		siaddword((struct state_info*) si, C_chars[si->iFusion].to, 0, C_PAIR_CHAR);
		sipopstate((struct state_info*) si);
		return 0;
	}
	siadd((struct state_info*) si, C_chars[si->iFusion].w, 0, C_PAIR_CHAR);
	sipopstate((struct state_info*) si);
	return 1;
}

struct state_info *
c_state_create(void)
{
	static bool (*stateFuncs[])(struct c_state_info *si) = {
		[C_STATE_GLOBAL] = c_state_global,
		[C_STATE_WORD] = c_state_word,
		[C_STATE_ZERO] = c_state_zero,
		[C_STATE_HEX] = c_state_hex,
		[C_STATE_DECIMAL] = c_state_decimal,
		[C_STATE_OCTAL] = c_state_octal,
		[C_STATE_BINARY] = c_state_binary,
		[C_STATE_USUFFIX] = c_state_usuffix,
		[C_STATE_LSUFFIX] = c_state_lsuffix,
		[C_STATE_ERRSUFFIX] = c_state_errsuffix,
		[C_STATE_MAYBEFLOAT] = c_state_maybefloat,
		[C_STATE_FLOAT] = c_state_float,
		[C_STATE_EXP] = c_state_exp,
		[C_STATE_STRING] = c_state_string,
		[C_STATE_MAYBECOMMENT] = c_state_maybecomment,
		[C_STATE_LINECOMMENT] = c_state_linecomment,
		[C_STATE_COMMENT] = c_state_comment,
		[C_STATE_FUSION] = c_state_fusion,
	};

	struct c_state_info *si;

	si = safe_malloc(sizeof*si);
	memset(si, 0, sizeof*si);
	si->stateFuncs = (bool (**)(struct state_info *si)) stateFuncs;
	return (struct state_info*) si;
}

int
c_state_free(struct state_info *si)
{
	if(!si)
		return ERROR("state info is null");
	
	free(si);
	return OK;
}
