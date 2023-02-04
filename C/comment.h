bool
c_state_maybecomment(struct c_state_info *si)
{
	switch(si->w)
	{
	case L'/':
		siaddword((struct state_info*) si, L"//", A_DIM, C_PAIR_COMMENT1);
		sisetstate((struct state_info*) si, C_STATE_LINECOMMENT);
		break;
	case L'*':
		siaddword((struct state_info*) si, L"/*", A_DIM, C_PAIR_COMMENT1);
		sisetstate((struct state_info*) si, C_STATE_COMMENT);
		break;
	default:
		siadd((struct state_info*) si, L'/', A_DIM, C_PAIR_CHAR);
		sipopstate((struct state_info*) si);
		return 1;
	}
	return 0;
}

bool
c_state_linecomment(struct c_state_info *si)
{
	if(si->w != EOF)
		siadd((struct state_info*) si, si->w, A_DIM, C_PAIR_COMMENT1);
	switch(si->w)
	{
	case L'\\':
		si->escaped = !si->escaped;
		break;
	case EOF:
	case L'\n':
		if(!si->escaped)
			sipopstate((struct state_info*) si);
	default:
		si->escaped = 0;
	}
	return 0;
}

bool
c_state_comment(struct c_state_info *si)
{
	if(si->w != EOF)
		siadd((struct state_info*) si, si->w, A_DIM, C_PAIR_COMMENT1);
	if(si->escaped && si->w == L'/')
		sipopstate((struct state_info*) si);
	si->escaped = si->w == L'*';
	return 0;
}

