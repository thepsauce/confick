// 0 - char received
// 1 - end of string
int
C_string(C_tunit_t r,
		int c)
{
	wchar_t w[2];

	if(c == EOF - 1)
	{
		C_buf_out(r, L'?', A_NORMAL, C_PAIR_ERROR);
		return 0;
	}
	if(c == EOF)
		return 1;

	w[0] = c;
	w[1] = L'\0';
	if(r->needHexChars)
	{
		if(!isxdigit(c))
		{
			C_buf_write(r, w, A_NORMAL, C_PAIR_ERROR);
			r->needHexChars = 0;
			if(c == L'\"')
				return 1;
		}
		else
		{
			C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
			r->needHexChars--;
			return 0;
		}
	}
	if(c == L'\"' && !r->escaped)
	{
		C_buf_write(r, w, A_NORMAL, C_PAIR_STRING1);
		return 1;
	}
	if(r->escaped)
	{
		switch(c)
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
			C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
			break;
		case L'x':
			r->needHexChars = 2;
			C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
			break;
		case L'u':
			r->needHexChars = 4;
			C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
			break;
		case L'U':
			r->needHexChars = 8;
			C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
			break;
		default:
			C_buf_write(r, w, A_NORMAL, C_PAIR_ERROR);
		}
		r->escaped = 0;
	}
	else if(c == L'\\')
	{
		C_buf_write(r, w, A_NORMAL, C_PAIR_STRING2);
		r->escaped = 1;
	}
	else if(c == L'\n')
	{
		C_buf_write(r, w, A_NORMAL, C_PAIR_STRING1);
		return 1;
	}
	else
	{
		C_buf_write(r, w, A_NORMAL, C_PAIR_STRING1);
	}
	return 0;
}

int
C_state_string(C_tunit_t r,
		int c)
{
	if(C_string(r, c))
		C_popstate(r);
	return 0;
}
