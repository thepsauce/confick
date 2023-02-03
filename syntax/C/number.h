void
C_number_out(C_tunit_t r,
		int c)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	C_buf_write(r, w, A_NORMAL, C_PAIR_NUMBER);
}

void
C_error_out(C_tunit_t r,
		int c)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	C_buf_write(r, w, A_NORMAL, C_PAIR_ERROR);
}

int
C_check_intsuffix(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'u':
		C_setstate(r, C_STATE_USUFFIX);
		C_number_out(r, c);
		break;
	case L'i':
		C_setstate(r, C_STATE_ERRSUFFIX);
		C_number_out(r, c);
		break;
	case L'l':
		C_setstate(r, C_STATE_LSUFFIX);
		C_number_out(r, c);
		break;
	default:
		if(isxdigit(c))
		{
			C_error_out(r, c);
			return 0;
		}
		C_popstate(r);
		return 1;
	}
	return 0;
}

int
C_state_errsuffix(C_tunit_t r,
		int c)
{
	if(isalnum(c))
	{
		C_error_out(r, c);
		return 0;
	}
	C_popstate(r);
	return 1;
}

int
C_state_usuffix(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'l':
		C_setstate(r, C_STATE_LSUFFIX);
		C_number_out(r, c);
		break;
	default:
		C_setstate(r, C_STATE_ERRSUFFIX);
		return 1;	
	}
	return 0;
}

int
C_state_lsuffix(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'l':
		C_setstate(r, C_STATE_ERRSUFFIX);
		C_number_out(r, c);
		break;
	default:
		C_setstate(r, C_STATE_ERRSUFFIX);
		return 1;	
	}
	return 0;
}

int
C_state_zero(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'x': case L'X':
		C_setstate(r, C_STATE_HEX);
		C_number_out(r, c);
		break;
	case L'0' ... L'7':
		C_setstate(r, C_STATE_OCTAL);
		C_number_out(r, c);
		break;
	case L'B': case L'b':
		C_setstate(r, C_STATE_BINARY);
		C_number_out(r, c);
		break;
	case L'.':
		C_setstate(r, C_STATE_FLOAT);
		C_number_out(r, c);
		break;
	case L'e':
		C_setstate(r, C_STATE_EXP);
		C_number_out(r, c);
		break;
	default:
		return C_check_intsuffix(r, c);
	}
	return 0;
}

int
C_state_decimal(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0' ... L'9':
		C_number_out(r, c);
		break;
	case L'.':
		C_setstate(r, C_STATE_FLOAT);
		C_number_out(r, c);
		break;
	case L'e':
		C_setstate(r, C_STATE_EXP);
		C_number_out(r, c);
		break;
	default:
		return C_check_intsuffix(r, c);
	}
	return 0;
}

int
C_state_hex(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'a' ... L'f':
	case L'A' ... L'F':
	case L'0' ... L'9':
		C_number_out(r, c);
		break;
	default:
		return C_check_intsuffix(r, c);
	}
	return 0;
}

int
C_state_octal(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0' ... L'7':
		C_number_out(r, c);
		break;
	default:
		return C_check_intsuffix(r, c);
	}
	return 0;
}

int
C_state_binary(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0':
	case L'1':
		C_number_out(r, c);
		break;
	default:
		return C_check_intsuffix(r, c);
	}
	return 0;
}

int
C_state_float(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0' ... L'9':
		C_number_out(r, c);
		break;
	case L'e':
		C_setstate(r, C_STATE_EXP);
		C_number_out(r, c);
		break;
	case L'f': case L'F':
	case L'l': case L'L':
		C_setstate(r, C_STATE_ERRSUFFIX);
		C_number_out(r, c);
		break;
	default:
		C_setstate(r, C_STATE_ERRSUFFIX);
		return 1;
	}
	return 0;
}

int
C_state_maybefloat(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0' ... L'9':
		C_setstate(r, C_STATE_FLOAT);
		C_number_out(r, L'.');
		C_number_out(r, c);
		break;
	default:
		C_popstate(r);
		C_buf_out(r, L'.', 0, C_PAIR_TEXT);
		return 1;
	}
	return 0;
}

int
C_state_exp(C_tunit_t r,
		int c)
{
	switch(c)
	{
	case L'0' ... L'9':
		C_number_out(r, c);
		break;
	case L'f': case L'F':
	case L'l': case L'L':
		C_setstate(r, C_STATE_ERRSUFFIX);
		C_number_out(r, c);
		break;
	default:
		C_setstate(r, C_STATE_ERRSUFFIX);
		return 1;
	}
	return 0;
}
