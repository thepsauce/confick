static const struct {
	const wchar_t *name;
	int state;
} C_preprocTransitions[] = {
	{ L"define"      , C_STATE_MACRO },
	{ L"elif"        , C_STATE_PREPROCCONTENT },
	{ L"else"        , 0 },
	{ L"endif"       , 0 },
	{ L"error"       , C_STATE_PREPROCCONTENT },
	{ L"ident"       , C_STATE_PREPROCIDENT },
	{ L"if"          , C_STATE_PREPROCCONTENT },
	{ L"ifdef"       , C_STATE_PREPROCCONTENT },
	{ L"ifndef"      , C_STATE_PREPROCCONTENT },
	{ L"import"      , C_STATE_PREPROCINCLUDE },
	{ L"include"     , C_STATE_PREPROCINCLUDE },
	{ L"include_next", C_STATE_PREPROCINCLUDE },
	{ L"line"        , C_STATE_PREPROCCONTENT },
	{ L"pragma"      , C_STATE_PREPROCCONTENT },
	{ L"sccs"        , C_STATE_PREPROCIDENT },
	{ L"undef"       , C_STATE_PREPROCUNDEF },
	{ L"warning"     , C_STATE_PREPROCCONTENT },
};

bool
c_check_preprocescape(struct c_state_info *si)
{
	bool esc = si->preProcEscaped;
	if(si->w == '\\')
		si->preProcEscaped = !si->preProcEscaped;
	else
		si->preProcEscaped = 0;
	return esc;
}

bool
c_state_preprocbegin(struct c_state_info *si)
{
	si->preProcEscaped = 0;
	siadd((struct state_info*) si, L'#', 0, C_PAIR_PREPROC1);
	sisetstate((struct state_info*) si, C_STATE_PREPROC);
	return 0;
}

bool
c_state_preproc(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	switch(si->w)
	{
	C_CASE_WHITESPACE:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
		break;
	case '\\':
		siadd((struct state_info*) si, L'\\', 0, C_PAIR_PREPROC1);
		break;
	C_CASE_IDENTFSTART:
		sisetstate((struct state_info*) si, C_STATE_PREPROCNAME);
		sipushandsetstate((struct state_info*) si, C_STATE_RAWWORDBEGIN);
		return 1;
	case '\n':
		siadd((struct state_info*) si, L'\n', 0, C_PAIR_PREPROC1);
		if(!esc)
			sipopstate((struct state_info*) si);
		break;
	default:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
	}
	return 0;
}

bool
c_state_preprocname(struct c_state_info *si)
{
	for(int i = 0; i < ARRLEN(C_preprocTransitions); i++)
		if(!wcscmp(si->word, C_preprocTransitions[i].name))
		{
			siaddword((struct state_info*) si, si->word, 0, C_PAIR_PREPROC1);
			if(!C_preprocTransitions[i].state)
				sipopstate((struct state_info*) si);
			else
				sisetstate((struct state_info*) si, C_preprocTransitions[i].state);
			return 1;
		}
	siaddword((struct state_info*) si, si->word, 0, C_PAIR_ERROR);
	sipopstate((struct state_info*) si);
	return 1;
}

bool
c_state_preproccontent(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	si->textPair = C_PAIR_PREPROC1;
	si->textAttr = 0;
	switch(si->w)
	{
	case EOF:
		sipopstate((struct state_info*) si);
		return 1;
	C_CASE_IDENTFSTART:
		sipushandsetstate((struct state_info*) si, C_STATE_WORDBEGIN);
		return 1;
	case L'0' ... L'9':
		sipushandsetstate((struct state_info*) si, C_STATE_ANYNUMBERBEGIN);
		return 1;
	case L'\n':
		siadd((struct state_info*) si, L'\n', 0, C_PAIR_PREPROC1);
		if(!esc)
			sipopstate((struct state_info*) si);
		break;
	case L'\'':
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
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
	}
	return 0;
}

bool
c_state_preprocident(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	switch(si->w)
	{
	C_CASE_WHITESPACE:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
		break;
	case '\"':
		sisetstate((struct state_info*) si, C_STATE_PREPROCIDENTSTRING);
		sipushandsetstate((struct state_info*) si, C_STATE_STRINGBEGIN);
		return 1;
	case L'\n':
		if(esc)
		{
			siadd((struct state_info*) si, L'\n', 0, C_PAIR_PREPROC2);
			break;
		}
	default:
		siadd((struct state_info*) si, L'\"', 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_preprocidentstring(struct c_state_info *si)
{
	sipopstate((struct state_info*) si);
	return 0;
}

bool
c_state_preprocinclude(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	switch(si->w)
	{
	C_CASE_WHITESPACE:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
		break;
	case L'<':
		siadd((struct state_info*) si, L'<', 0, C_PAIR_PREPROC2);
		sisetstate((struct state_info*) si, C_STATE_PREPROCINCLUDEBRACKETS);
		break;
	case L'\"':
		siadd((struct state_info*) si, L'\"', 0, C_PAIR_PREPROC2);
		sisetstate((struct state_info*) si, C_STATE_PREPROCINCLUDESTRING);
		break;
	case L'\n':
		if(esc)
		{
			siadd((struct state_info*) si, L'\n', 0, C_PAIR_PREPROC2);
			break;
		}
	default:
		siadd((struct state_info*) si, L'<', 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_preprocincludebrackets(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	switch(si->w)
	{
	case L'\n':
		if(!esc)
		{
			siadd((struct state_info*) si, L'>', 0, C_PAIR_ERROR);
			sipopstate((struct state_info*) si);
		}
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC2);
		break;
	case L'>':
		siadd((struct state_info*) si, L'>', 0, C_PAIR_PREPROC2);
		sipopstate((struct state_info*) si);
		break;
	default:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC2);
	}
	return 0;
}

bool
c_state_preprocincludestring(struct c_state_info *si)
{
	bool esc = c_check_preprocescape(si);
	switch(si->w)
	{
	case L'\n':
		if(!esc)
		{
			siadd((struct state_info*) si, L'\"', 0, C_PAIR_ERROR);
			sipopstate((struct state_info*) si);
			break;
		}
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC2);
		break;
	case L'\"':
		siadd((struct state_info*) si, L'\"', 0, C_PAIR_PREPROC2);
		sipopstate((struct state_info*) si);
		break;
	default:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC2);
	}
	return 0;
}

bool
c_state_preprocundef(struct c_state_info *si)
{
	switch(si->w)
	{
	C_CASE_WHITESPACE:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
		break;
	C_CASE_IDENTFSTART:
		sisetstate((struct state_info*) si, C_STATE_PREPROCUNDEFWORD);
		sipushandsetstate((struct state_info*) si, C_STATE_RAWWORDBEGIN);
		break;
	default:
		siaddword((struct state_info*) si, L"???", 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_preprocundefword(struct c_state_info *si)
{
	siaddword((struct state_info*) si, si->word, 0, C_PAIR_PREPROC1);
	sipopstate((struct state_info*) si);
	return 1;
}
bool
c_state_macro(struct c_state_info *si)
{
	switch(si->w)
	{
	C_CASE_WHITESPACE:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_PREPROC1);
		return 0;
	C_CASE_IDENTFSTART:
		sisetstate((struct state_info*) si, C_STATE_MACRONAME);
		sipushandsetstate((struct state_info*) si, C_STATE_RAWWORDBEGIN);
		break;
	default:
		siaddword((struct state_info*) si, L" ???", 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
	}
	return 1;
}

bool
c_state_macroname(struct c_state_info *si)
{
	siaddword((struct state_info*) si, si->word, 0, C_PAIR_PREPROC2);
	sisetstate((struct state_info*) si, C_STATE_PREPROCCONTENT);
	return 1;
}
