/* Condition */
condition_t
conset(const char *str)
{
	condition_t condition;
	memset(&condition, 0, sizeof condition);

	if(!str)
		ERROR("string is null");

	for(int c = 0, prev; prev = c, c = *str; str++)
	{
		if(c == '-' && prev && str[1])
		{
			str++;
			for(int i = prev, n = *str; i <= n; i++)
				condition.set[i >> 4] |= 1 << (i & 0xF);
		}
		else
		{
			condition.set[c >> 4] |= 1 << (c & 0xF);
		}
	}

	return condition;
}

condition_t
consetbit(condition_t condition,
		int c)
{
	condition.set[c >> 4] |= 1 << (c & 0xF);
	return condition;
}

bool
conisset(condition_t condition, int c)
{
	return condition.set[c >> 4] & (1 << (c & 0xF));
}

condition_t
condefault(void)
{
	condition_t condition;
	memset(&condition, 0xFF, sizeof condition);
	return condition;
}

condition_t
connegate(condition_t condition)
{
	for(int i = 0; i < ARRLEN(condition.set); i++)
		condition.set[i] = ~condition.set[i];
	return condition;
}

condition_t
conor(condition_t c1, condition_t c2)
{
	for(int i = 0; i < ARRLEN(c1.set); i++)
		c1.set[i] |= c2.set[i];
	return c1;
}

condition_t
conand(condition_t c1, condition_t c2)
{
	for(int i = 0; i < ARRLEN(c1.set); i++)
		c1.set[i] &= c2.set[i];
	return c1;
}

condition_t
conxor(condition_t c1, condition_t c2)
{
	for(int i = 0; i < ARRLEN(c1.set); i++)
		c1.set[i] ^= c2.set[i];
	return c1;
}

/* State */
state_t
stacreate(chtype cht)
{
	state_t state;
	state = malloc(sizeof*state);
	if(!state)
		return ERROR("out of memory", NULL);
	memset(state, 0, sizeof*state);
	state->cht = cht;
	return state;
}

int
stafree(state_t state)
{
	if(!state)
		return ERROR("syntax is null");
	free(state->subStates);
	free(state);
	return OK;
}

int
staaddstate(state_t parent, state_t child, condition_t condition)
{
	if(!parent || !child)
		return ERROR(!parent ? "parent is null" : "child is null");
	
	if(parent->nSubStates + 1 > parent->szSubStates)
	{
		parent->szSubStates *= 2;
		parent->szSubStates++;
		parent->subStates = realloc(parent->subStates, 
				parent->szSubStates * sizeof*parent->subStates);
		if(!parent->subStates)
			return ERROR("out of memory");
	}
	parent->subStates[parent->nSubStates].condition = condition;;
	parent->subStates[parent->nSubStates].nextState = child;
	parent->nSubStates++;
	return OK;
}

/* Syntax */
syntax_t
syncreate(chtype cht)
{
	syntax_t syntax;
	syntax = malloc(sizeof*syntax);
	if(!syntax)
		return ERROR("out of memory", NULL);
	memset(syntax, 0, sizeof*syntax);
	syntax->initialState.cht = cht;
	syntax->currentState = &syntax->initialState;
	return syntax;
}

int
synfree(syntax_t syntax)
{
	if(!syntax)
		return ERROR("syntax is null");
	free(syntax->initialState.subStates);
	free(syntax);
	return OK;
}

int
synreset(syntax_t syntax)
{
	if(!syntax)
		return ERROR("syntax is null");
	syntax->currentState = &syntax->initialState;
	return OK;
}

int
synaddstate(syntax_t syntax,
		state_t state,
		condition_t condition)
{
	if(!syntax)
		return ERROR("syntax is null");
	return staaddstate(&syntax->initialState, state, condition);
}

state_t
syninitialstate(syntax_t syntax)
{
	if(!syntax)
		return ERROR("syntax is null", NULL);
	return &syntax->initialState;
}

chtype
synfeed(syntax_t syntax,
		char ch)
{
	chtype cht;
	state_t state;

	if(!syntax)
		return ERROR("syntax is null");
	
	state = syntax->currentState;
	while(1)
	{
		for(int i = 0; i < state->nSubStates; i++)
		{
			if(conisset(state->subStates[i].condition, ch))
			{
				state = state->subStates[i].nextState;
				cht = state->cht;
				goto state_set;
			}
		}
		if(state == &syntax->initialState)
		{
			cht = state->cht;
			goto state_set;
		}
		state = &syntax->initialState;
	}
state_set:
	syntax->currentState = state;
	return cht | ch;
}
