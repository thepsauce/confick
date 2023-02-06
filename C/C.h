static const wchar_t *C_keywords[] = {
	L"auto",
	L"__auto_type",
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
	C_PAIR_STRING1,
	C_PAIR_STRING2,
	C_PAIR_KEYWORD1,
	C_PAIR_KEYWORD2,
	C_PAIR_FUNCTION,
	C_PAIR_NUMBER,
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
	C_STATE_RAWWORDBEGIN,
	C_STATE_RAWWORD,
	C_STATE_WORDBEGIN,
	C_STATE_WORD,
	C_STATE_ANYNUMBERBEGIN,
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
	C_STATE_NEGEXP,
	C_STATE_FLOAT,
	C_STATE_STRINGBEGIN,
	C_STATE_STRING,
	C_STATE_CHARBEGIN,
	C_STATE_CHAR,
	C_STATE_CHARESCAPED,
	C_STATE_CHAREND,
	C_STATE_PREPROCBEGIN,
	C_STATE_PREPROC,
	C_STATE_PREPROCNAME,
	C_STATE_PREPROCCONTENT,
	C_STATE_PREPROCIDENT,
	C_STATE_PREPROCIDENTSTRING,
	C_STATE_PREPROCINCLUDE,
	C_STATE_PREPROCINCLUDESTRING,
	C_STATE_PREPROCINCLUDEBRACKETS,
	C_STATE_PREPROCUNDEF,
	C_STATE_PREPROCUNDEFWORD,
	C_STATE_MACRO,
	C_STATE_MACRONAME,
	C_STATE_MAYBECOMMENT,
	C_STATE_LINECOMMENT,
	C_STATE_COMMENT,
	C_STATE_FUSION,
};

#define C_CASE_WHITESPACE case ' ': case '\t': case '\f': case '\r': case '\v'
#define C_CASE_WHITESPACENL C_CASE_WHITESPACE: case '\n'
#define C_CASE_IDENTFSTART case 'A' ... 'Z': case 'a' ... 'z': case '$': case '_'
#define C_CASE_IDENTF C_CASE_IDENTFSTART: case '0' ... '9'

struct c_state_info {
	_STATE_INFO_HEADER;
	int iFusion;
	wchar_t word[512];
	int nWord;
	int nOpenCorn;
	int nOpenRoun;
	int nOpenCurl;
	bool escaped;
	int needHexChars;
	bool preProcEscaped;
	short textPair;
	attr_t textAttr;
};

#include "word.h"
#include "number.h"
#include "string.h"
#include "char.h"
#include "macro.h"
#include "comment.h"

bool
c_state_global(struct c_state_info *si)
{
	si->textPair = C_PAIR_TEXT;
	si->textAttr = 0;
	switch(si->w)
	{
	case EOF:
		break;
	C_CASE_IDENTFSTART:
		sipushandsetstate((struct state_info*) si, C_STATE_WORDBEGIN);
		return 1;
	case '0' ... '9':
		sipushandsetstate((struct state_info*) si, C_STATE_ANYNUMBERBEGIN);
		return 1;
	case '#':
		sipushandsetstate((struct state_info*) si, C_STATE_PREPROCBEGIN);
		return 1;
	case '\'':
		sipushandsetstate((struct state_info*) si, C_STATE_CHARBEGIN);
		return 1;
	case '\"':
		sipushandsetstate((struct state_info*) si, C_STATE_STRINGBEGIN);
		return 1;
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
	static const bool (*stateFuncs[])(struct c_state_info *si) = {
		[C_STATE_GLOBAL] = c_state_global,

		[C_STATE_WORDBEGIN] = c_state_wordbegin,
		[C_STATE_WORD] = c_state_word,
		[C_STATE_RAWWORDBEGIN] = c_state_rawwordbegin,
		[C_STATE_RAWWORD] = c_state_rawword,

		[C_STATE_ANYNUMBERBEGIN] = c_state_anynumberbegin,
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
		[C_STATE_NEGEXP] = c_state_negexp,

		[C_STATE_STRINGBEGIN] = c_state_stringbegin,
		[C_STATE_STRING] = c_state_string,

		[C_STATE_CHARBEGIN] = c_state_charbegin,
		[C_STATE_CHAR] = c_state_char,
		[C_STATE_CHARESCAPED] = c_state_charescaped,
		[C_STATE_CHAREND] = c_state_charend,

		[C_STATE_PREPROCBEGIN] = c_state_preprocbegin,
		[C_STATE_PREPROC] = c_state_preproc,
		[C_STATE_PREPROCNAME] = c_state_preprocname,
		[C_STATE_PREPROCCONTENT] = c_state_preproccontent,
		[C_STATE_PREPROCINCLUDE] = c_state_preprocinclude,
		[C_STATE_PREPROCINCLUDEBRACKETS] = c_state_preprocincludebrackets,
		[C_STATE_PREPROCINCLUDESTRING] = c_state_preprocincludestring,

		[C_STATE_PREPROCIDENT] = c_state_preprocident,
		[C_STATE_PREPROCIDENTSTRING] = c_state_preprocidentstring,
		[C_STATE_PREPROCUNDEF] = c_state_preprocundef,
		[C_STATE_PREPROCUNDEFWORD] = c_state_preprocundefword,
		[C_STATE_MACRO] = c_state_macro,
		[C_STATE_MACRONAME] = c_state_macroname,

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
