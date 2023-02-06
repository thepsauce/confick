bool
c_state_rawwordbegin(struct c_state_info *si)
{
	si->word[0] = si->w;
	si->nWord = 1;
	sisetstate((struct state_info*) si, C_STATE_RAWWORD);
	return 0;
}
	
bool
c_state_rawword(struct c_state_info *si)
{
	switch(si->w)
	{
	C_CASE_IDENTF:
		si->word[si->nWord++] = si->w;
		break;
	default:
		si->word[si->nWord] = 0;
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_wordbegin(struct c_state_info *si)
{
	si->word[0] = si->w;
	si->nWord = 1;
	sisetstate((struct state_info*) si, C_STATE_WORD);
	return 0;
}

bool
c_state_word(struct c_state_info *si)
{
	attr_t a;
	short color_pair;

	switch(si->w)
	{
	C_CASE_IDENTF:
		si->word[si->nWord++] = si->w;
		break;
	case L'\'':
	case L'\"':
		if(si->word[0] == L'L' && si->nWord == 1)
		{
			siadd((struct state_info*) si, L'L', 0, C_PAIR_STRING1);
			sisetstate((struct state_info*) si, si->w == L'\"' ? C_STATE_STRINGBEGIN : C_STATE_CHARBEGIN);
			return 1;
		}
	default:
		a = si->textAttr;
		color_pair = si->textPair;
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
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

