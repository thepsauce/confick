bool
c_state_charbegin(struct c_state_info *si)
{
	siadd((struct state_info*) si, L'\'', 0, C_PAIR_STRING1);
	sisetstate((struct state_info*) si, C_STATE_CHAR);
	return 0;
}
	
bool
c_state_char(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'\'':
		siadd((struct state_info*) si, L'\'', 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
		break;
	case L'\\':
		siadd((struct state_info*) si, L'\\', 0, C_PAIR_STRING2);
		sisetstate((struct state_info*) si, C_STATE_CHARESCAPED);
		break;
	default:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING1);
		sisetstate((struct state_info*) si, C_STATE_CHAREND);
	}
	return 0;
}

bool
c_state_charescaped(struct c_state_info *si)
{
	if(si->needHexChars)
	{
		if(!isxdigit(si->w))
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
			si->needHexChars = 0;
			sipopstate((struct state_info*) si);
			return si->w != L'\'';
		}
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
		si->needHexChars--;
		if(!si->needHexChars)
			sisetstate((struct state_info*) si, C_STATE_CHAREND);
		return 0;
	}
	switch(si->w)
	{
	case L'a':
	case L'b':
	case L'e':
	case L'f':
	case L'n':
	case L'm':
	case L'r':
	case L'v':
	case L'\\':
	case L'\"':
	case L'\'':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
		sisetstate((struct state_info*) si, C_STATE_CHAREND);
		break;
	case L'\n':
		siadd((struct state_info*) si, L'\n', 0, C_PAIR_STRING2);
		sisetstate((struct state_info*) si, C_STATE_CHAR);
		break;
	case L'x':
		si->needHexChars = 2;
		siadd((struct state_info*) si, L'x', 0, C_PAIR_STRING2);
		break;
	case L'u':
		si->needHexChars = 4;
		siadd((struct state_info*) si, L'u', 0, C_PAIR_STRING2);
		break;
	case L'U':
		si->needHexChars = 8;
		siadd((struct state_info*) si, L'U', 0, C_PAIR_STRING2);
		break;
	default:
		siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		sisetstate((struct state_info*) si, C_STATE_CHAREND);
	}
	return 0;
}

bool
c_state_charend(struct c_state_info *si)
{
	if(si->w != L'\'')
	{
		siaddextra((struct state_info*) si, L'\'', 0, C_PAIR_ERROR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	siadd((struct state_info*) si, L'\'', 0, C_PAIR_STRING1);
	sipopstate((struct state_info*) si);
	return 0;
}
