bool
c_state_string(struct c_state_info *si)
{
	if(si->needHexChars)
	{
		if(!isxdigit(si->w))
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
			si->needHexChars = 0;
			if(si->w == '\"')
				sipopstate((struct state_info*) si);
		}
		else
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
			si->needHexChars--;
		}
		return 0;
	}
	if(si->w == L'\"' && !si->escaped)
	{
		sipopstate((struct state_info*) si);
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING1);
		return 0;
	}
	if(si->escaped)
	{
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
		case L'\n':
			siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
			break;
		case L'x':
			si->needHexChars = 2;
			siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
			break;
		case L'u':
			si->needHexChars = 4;
			siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
			break;
		case L'U':
			si->needHexChars = 8;
			siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
			break;
		default:
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		}
		si->escaped = 0;
	}
	else if(si->w == L'\\')
	{
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
		si->escaped = 1;
	}
	else if(si->w == L'\n')
	{
		sipopstate((struct state_info*) si);
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING2);
	}
	else
	{
		siadd((struct state_info*) si, si->w, 0, C_PAIR_STRING1);
	}
	return 0;
}
