int
C_state_commentstart(C_tunit_t r,
		int c)
{
	if(c == '/')
	{
		C_setstate(r, C_STATE_LINECOMMENT);
		C_buf_write(r, L"/", A_NORMAL, C_PAIR_COMMENT1);
		C_buf_write(r, L"/", A_NORMAL, C_PAIR_COMMENT1);
	}
	else if(c == '*')
	{
		C_setstate(r, C_STATE_COMMENT);
		C_buf_write(r, L"/", A_NORMAL, C_PAIR_COMMENT1);
		C_buf_write(r, L"*", A_NORMAL, C_PAIR_COMMENT1);
	}
	else
	{
		C_popstate(r);
		C_buf_write(r, L"/", A_NORMAL, C_PAIR_TEXT);
		return 1;
	}
	return 0;
}

int
C_state_linecomment(C_tunit_t r,
		int c)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	if(!r->escaped)
	{
		C_buf_write(r, w, A_NORMAL, C_PAIR_COMMENT1);
		if(c == '\n')
			C_popstate(r);
		else if(c == '\\')
			r->escaped = 1;
	}
	else
		C_buf_write(r, w, A_NORMAL, C_PAIR_COMMENT1);
	return 0;
}

int
C_state_comment(C_tunit_t r,
		int c)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	if(c == '*')
		C_setstate(r, C_STATE_MAYBECOMMENTEND);
	C_buf_write(r, w, A_NORMAL, C_PAIR_COMMENT1);
	return 0;
}

int
C_state_maybecommentend(C_tunit_t r,
		int c)
{
	if(c == '/')
		C_popstate(r);
	C_buf_write(r, L"/", A_NORMAL, C_PAIR_COMMENT1);
	return 0;
}
