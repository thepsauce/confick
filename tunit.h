struct {
	tunitname_t *d;
	int n, sz;	
} UnitNames;

int
tuaddname(const char *name,
		size_t size,
		int (*init)(tunit_t tunit),
		int (*write)(tunit_t tunit, int c),
		int (*read)(tunit_t tunit, void *ptr),
		int (*destroy)(tunit_t tunit))
{
	tunitname_t tname;

	if(!name)
		return ERROR("name is null");
	if(!init || !write || !read || !destroy)
		return ERROR("a function is null");

	tname = malloc(sizeof*tname);
	if(!tname)
		return ERROR("out of memory");
	if(UnitNames.n + 1 > UnitNames.sz)
	{
		UnitNames.sz *= 2;
		UnitNames.sz++;
		UnitNames.d = realloc(UnitNames.d, UnitNames.sz * sizeof*UnitNames.d);
		if(!UnitNames.d)
		{
			free(tname);
			return ERROR("out of memory");
		}
	}
	if(!(tname->name = strdup(name)))
	{
		free(tname);
		return ERROR("out of memory");
	}
	UnitNames.d[UnitNames.n++] = tname;
	tname->size = size;
	tname->init = init;
	tname->write = write;
	tname->read = read;
	tname->destroy = destroy;
	return OK;
}

tunitname_t tugetname(const char *name)
{
	for(int i = 0; i < UnitNames.n; i++)
		if(!strcmp(UnitNames.d[i]->name, name))
			return UnitNames.d[i];
	return WARN("base was not found", NULL);
}

tunit_t
tucreate(const char *name)
{
	tunitname_t tname;
	tunit_t tunit;

	if(!(tname = tugetname(name)))
		return NULL;
	
	tunit = malloc(tname->size);
	if(!tunit)
		return ERROR("out of memory", NULL);
	tunit->write = tname->write;
	tunit->read = tname->read;
	tunit->destroy = tname->destroy;
	if(tname->init(tunit))
	{
		free(tunit);
		return NULL;
	}
	return tunit;
}
