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
	if(strlen(name) > TUNAME_MAX)
		return ERROR("name is too long");
	if(!init || !write || !read || !destroy)
		return ERROR("a function is null");

	if(UnitNames.n + 1 > UnitNames.sz)
	{
		UnitNames.sz *= 2;
		UnitNames.sz++;
		UnitNames.d = safe_realloc(UnitNames.d, UnitNames.sz * sizeof*UnitNames.d);
	}
	tname = safe_malloc(sizeof*tname);
	UnitNames.d[UnitNames.n++] = tname;
	strcpy(tname->name, name);
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
	
	tunit = safe_malloc(tname->size);
	tunit->write = tname->write;
	tunit->read = tname->read;
	tunit->destroy = tname->destroy;
	if(tname->init(tunit) == ERR)
	{
		free(tunit);
		return NULL;
	}
	return tunit;
}
