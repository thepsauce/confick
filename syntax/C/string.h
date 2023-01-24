// 0 - char received
// 1 - end of string
int C_string(C_receiver_t r, int c)
{
	if(!r->escaped)
		recvc(r, c, 0, C_PAIR_STRING1);
	if(r->needHexChars)
	{
		if(!isxdigit(c))
		{
			if(c == '\"')
			{
				r->needHexChars = 0;
				return 1;
			}
		}
		else
			r->needHexChars--;
		return 0;
	}
	if(c == '\"' && r->escaped)
		return 1;
	if(c == '\\')
		r->escaped = 1;
	else 
		r->escaped = 0;
	if(r->escaped)
	{
		switch(c)
		{
		case 'a':
		case 'b':
		case 'e':
		case 'f':
		case 'n':
		case 'm':
		case 'v':
			recvc(r, c, 0, C_PAIR_STRING2);
			break;
		case 'x':
			r->needHexChars = 2;
			recvc(r, c, 0, C_PAIR_STRING2);
			break;
		case 'u':
			r->needHexChars = 4;
			recvc(r, c, 0, C_PAIR_STRING2);
			break;
		case 'U':
			r->needHexChars = 8;
			recvc(r, c, 0, C_PAIR_STRING2);
			break;
		}
		recvc(r, c, 0, C_PAIR_STRINGERROR);
		r->escaped = 0;
	}
	return 0;
}