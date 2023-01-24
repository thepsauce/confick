int C_state_maybecomment(C_receiver_t r, int c)
{
	if(c == '/')
	{
		r->state = C_STATE_LINECOMMENT;
		recvs(r, "//", 0, C_PAIR_COMMENT);
	}
	else if(c == '*')
	{
		r->state = C_STATE_COMMENT;
		recvs(r, "/*", 0, C_PAIR_COMMENT);
	}
	else
	{
		r->state = C_STATE_DEFAULT;
		recvc(r, '/', 0, C_PAIR_TEXT);
		return 1;
	}
	return 0;
}

int C_state_linecomment(C_receiver_t r, int c)
{
	if(!r->escaped)
	{
		if(c == '\n')
			r->state = r->prevState;
		else if(c == '\\')
			r->escaped = 1;
	}
	else
		recvc(r, c, 0, C_PAIR_COMMENT);
	return 0;
}

int C_state_comment(C_receiver_t r, int c)
{
	if(c == '*')
		r->state = C_STATE_MAYBECOMMENTEND;
	recv(r, c, 0, C_PAIR_COMMENT);
	return 0;
}

int C_state_maybecommentend(C_receiver_t r, int c)
{
	if(c == '/')
		r->state = r->prevState;
	recv(r, c, 0, C_PAIR_COMMENT);
	return 0;
}