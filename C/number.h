bool
c_check_intsuffix(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'u':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_USUFFIX);
		break;
	case L'i':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		break;
	case L'l':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_LSUFFIX);
		break;
	default:
		if(isxdigit(si->w))
		{
			siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
			return 0;
		}
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_errsuffix(struct c_state_info *si)
{
	if(isalnum(si->w))
	{
		siadd((struct state_info*) si, si->w, 0, C_PAIR_ERROR);
		return 0;
	}
	sipopstate((struct state_info*) si);
	return 1;
}

bool
c_state_usuffix(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'l':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_LSUFFIX);
		break;
	default:
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		return 1;	
	}
	return 0;
}

bool
c_state_lsuffix(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'l':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		break;
	default:
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		return 1;	
	}
	return 0;
}

bool
c_state_anynumberbegin(struct c_state_info *si)
{
	siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
	switch(si->w)
	{
	case '0':
		sisetstate((struct state_info*) si, C_STATE_ZERO);
		break;
	default:
		sisetstate((struct state_info*) si, C_STATE_DECIMAL);
	}
	return 0;
}

bool
c_state_zero(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'x': case L'X':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_HEX);
		break;
	case L'0' ... L'7':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_OCTAL);
		break;
	case L'B': case L'b':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_BINARY);
		break;
	case L'.':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_FLOAT);
		break;
	case L'e':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_NEGEXP);
		break;
	default:
		return c_check_intsuffix(si);
	}
	return 0;
}

bool
c_state_decimal(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0' ... L'9':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	case L'.':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_FLOAT);
		break;
	case L'e':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_NEGEXP);
		break;
	default:
		return c_check_intsuffix(si);
	}
	return 0;
}

bool
c_state_hex(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'a' ... L'f':
	case L'A' ... L'F':
	case L'0' ... L'9':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	default:
		return c_check_intsuffix(si);
	}
	return 0;
}

bool
c_state_octal(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0' ... L'7':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	default:
		return c_check_intsuffix(si);
	}
	return 0;
}

bool
c_state_binary(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0':
	case L'1':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	default:
		return c_check_intsuffix(si);
	}
	return 0;
}

bool
c_state_float(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0' ... L'9':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	case L'e':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_NEGEXP);
		break;
	case L'f': case L'F':
	case L'l': case L'L':
	case L'd': case L'D':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		break;
	default:
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		return 1;
	}
	return 0;
}

bool
c_state_maybefloat(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0' ... L'9':
		siadd((struct state_info*) si, L'.', 0, C_PAIR_NUMBER);
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_FLOAT);
		break;
	default:
		siadd((struct state_info*) si, L'.', 0, C_PAIR_CHAR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_exp(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'0' ... L'9':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		break;
	case L'f': case L'F':
	case L'l': case L'L':
	case L'd': case L'D':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		break;
	default:
		sisetstate((struct state_info*) si, C_STATE_ERRSUFFIX);
		return 1;
	}
	return 0;
}

bool
c_state_negexp(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'-':
		siadd((struct state_info*) si, si->w, 0, C_PAIR_NUMBER);
		sisetstate((struct state_info*) si, C_STATE_EXP);
		break;
	default:
		return c_state_exp(si);
	}
	return 0;
}
