int
rwlerror(FILE *file, long pos, const char *msg, int state)
{
	int c;
	int x, y;

	x = y = 1;
	fseek(file, 0, SEEK_SET);
	for(c = fgetc(file); pos; pos--, c = fgetc(file))
	{
		if(c == '\n')
		{
			x = 1;
			y++;
		}
		else
			x++;
	}
	fprintf(stderr, "%d:%d:%s:at %c(%d):state=%d\n", y, x, msg, c, c, state);
	fclose(file);
	return -1;
}
	
int
rwlcompile(const char *fileName, syntax_t syntax)
{
	enum {
		STATELABEL,
		STATEWORD,
		STATESTRING,
		STATEESCAPE,
		STATESTRINGLETTER,
		STATESTRINGTO,
		STATESTRINGTOESCAPE,
		STATEFLOATINGSTRING,
		STATEARROWSTART,
		STATEARROWEND,
		STATEARROW,
		STATEARROWWORD
	};
	FILE *file;
	struct {
		char *name;
		state_t state;
		long pos;
	} *states;
	int nStates, szStates;
	state_t activeState;
	int compileState = STATELABEL;
	char word[512];
	int start;
	int wi;
	int errPos;
	char errMsg[1024];
	condition_t condition;

	file = fopen(fileName, "r");
	if(!file)
		return WARN("file does not exist");

	szStates = 10;
	states = malloc(szStates * sizeof*states);
	nStates = 0;

	activeState = syninitialstate(syntax);

	for(int c = fgetc(file); !feof(file); c = fgetc(file))
	{
		switch(compileState)
		{
		case STATELABEL:
		label:
			if(isspace(c))
				break;
			if(c == '\"')
			{
				compileState = STATESTRING;
				memset(&condition, 0, sizeof condition);
			}
			else if(isalpha(c) || c == '_')
			{
				wi = 1;
				word[0] = c;
				compileState = STATEWORD;
			}
			else if(c == '$')
			{
				compileState = STATEFLOATINGSTRING;
				condition = condefault();
			}
			else
			{
				errPos = ftell(file);
				snprintf(errMsg, sizeof errMsg, "expected string(\"whatever\") or new state(whatever:) after state");
				goto error;
			}
			break;
		case STATEWORD:
			if(isalnum(c) || c == '_')
				word[wi++] = c;
			else if(c == ':')
			{
				word[wi] = 0;
				for(int i = 0; i < nStates; i++)
					if(!strcmp(states[i].name, word))
					{
						activeState = states[i].state;
						goto state_exists;
					}
				if(nStates + 1 > szStates)
				{
					szStates *= 2;
					szStates++;
					states = realloc(states, szStates * sizeof*states);
				}
				states[nStates].name = strdup(word);
				states[nStates].state = activeState = stacreate(0);
				states[nStates].pos = ftell(file) - wi;
				nStates++;
			state_exists:
				compileState = STATELABEL;
			}
			else
			{
				errPos = ftell(file);
				snprintf(errMsg, sizeof errMsg, "expected ':' after word");
				goto error;
			}
			break;
		case STATESTRING:
			if(c == '\\')
				compileState = STATEESCAPE;
			else if(c == '\"')
				compileState = STATEFLOATINGSTRING;
			else
			{
				start = c;
				compileState = STATESTRINGLETTER;
			}
			break;
		case STATEESCAPE:
			switch(c)
			{
			case 'a': start = '\a'; break;
			case 'b': start = '\b'; break;
			case 'e': start = '\e'; break;
			case 'f': start = '\f'; break;
			case 'n': start = '\n'; break;
			case 'r': start = '\r'; break;
			case 't': start = '\t'; break;
			case 'v': start = '\v'; break;
			default:
				start = c;
			}
			compileState = STATESTRINGLETTER;
			break;
		case STATESTRINGLETTER:
			if(c == '-')
				compileState = STATESTRINGTO;
			else if(c == '\\')
			{
				condition = consetbit(condition, c);
				compileState = STATEESCAPE;
			}
			else if(c == '\"')
			{
				condition = consetbit(condition, c);
				compileState = STATEFLOATINGSTRING;
			}
			else
			{
				condition = consetbit(condition, c);
				compileState = STATESTRING;
			}
			break;
		case STATESTRINGTO:
			if(c == '\\')
				compileState = STATESTRINGTOESCAPE;
			else if(c == '\"')
			{
				condition = consetbit(condition, start);
				compileState = STATEFLOATINGSTRING;
			}
			else
			{
				for(; start <= c; start++)
					condition = consetbit(condition, start);
				compileState = STATESTRING;
			}
			break;
		case STATESTRINGTOESCAPE:
			switch(c)
			{
			case 'a': c = '\a'; break;
			case 'b': c = '\b'; break;
			case 'e': c = '\e'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'v': c = '\v'; break;
			}
			for(; start <= c; start++)
				condition = consetbit(condition, start);
			compileState = STATESTRING;
			break;
		case STATEFLOATINGSTRING:
			if(isspace(c))
				break;
			if(c == '-')
			{
				compileState = STATEARROWSTART;
			}
			else
			{
				errPos = ftell(file);
				snprintf(errMsg, sizeof errMsg, "expected arrow after string (path target)");
				goto error;
			}
			break;
		case STATEARROWSTART:
			if(c == '>')
			{
				compileState = STATEARROWEND;
			}
			else
			{
				errPos = ftell(file);
				snprintf(errMsg, sizeof errMsg, "expected '>' after '-' to complete arrow");
				goto error;
			}
			break;
		case STATEARROWEND:
			if(isspace(c))
				break;
			if(isalpha(c) || c == '_')
			{
				wi = 1;
				word[0] = c;
				compileState = STATEARROWWORD;
			}
			else if(c == '$')
			{
				staaddstate(activeState, syninitialstate(syntax), condition);
				compileState = STATELABEL;
			}
			else
			{
				errPos = ftell(file);
				snprintf(errMsg, sizeof errMsg, "expected a word(target state) after ->");
				goto error;
			}
			break;
		case STATEARROWWORD:
			if(isalnum(c) || c == '_')
			{
				word[wi++] = c;
			}
			else
			{
				word[wi] = 0;
				for(int i = 0; i < nStates; i++)
					if(!strcmp(states[i].name, word))
					{
						staaddstate(activeState, states[i].state, condition);
						goto found_state;
					}
				if(nStates + 1 > szStates)
				{
					szStates *= 2;
					szStates++;
					states = realloc(states, szStates * sizeof*states);
				}
				states[nStates].name = strdup(word);
				states[nStates].state = stacreate(0);
				states[nStates].pos = ftell(file) - wi;
				nStates++;
				staaddstate(activeState, states[nStates - 1].state, condition);
			found_state:
				compileState = STATELABEL;
				goto label;
			}
			break;
		}
	}
	if(compileState != STATELABEL)
	{
		errPos = ftell(file);
		snprintf(errMsg, sizeof errMsg, "unexpected end of file(state=%d)", word, compileState);
		goto error;
	}
	for(int i = 0; i < nStates; i++)
		free(states[i].name);
	free(states);
	fclose(file);
	return OK;
error:
	for(int i = 0; i < nStates; i++)
	{
		stafree(states[i].state);
		free(states[i].name);
	}
	free(states);
	free(syntax->initialState.subStates);
	memset(&syntax->initialState, 0, sizeof syntax->initialState);
	return rwlerror(file, errPos, errMsg, compileState);
}
