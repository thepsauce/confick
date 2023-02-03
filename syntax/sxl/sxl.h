enum {
	SXL_VAR_BOOL,
	SXL_VAR_STRING,
	SXL_VAR_STRINGLIST,
	SXL_VAR_COLOR,
};
typedef struct sxl_var {
	int type;
	char *name;
	union {
		struct {
			bool b;
		};
		struct {
			char *str;
			int szStr, nStr;
		};
		struct {
			char **strs;
			int szStrs, nStrs;
		};
		struct {
			short color_pair;
			attr_t a;
		};
	};
} SXL_var_t;
void
SXL_var_init(SXL_var_t *var, int type, const char *name)
{
	var->type = type;
	var->name = safe_strdup(name);
}
void
SXL_var_uninit(SXL_var_t *var)
{
	free(var->name);
	switch(var->type)
	{
	case SXL_VAR_STRING: free(var->str); break;
	case SXL_VAR_STRINGLIST:
		for(int i = 0; i < var->nStrs; i++)
			free(var->strs[i]);
		free(var->strs);
		break;
	}
}

enum {
	SXL_PIO_DEFAULT,
	SXL_PIO_IP, // instruction pointer
	SXL_PIO_REF, // variable reference
	SXL_PIO_INSTAREF, // instant refence [label]!
};
typedef struct sxl_pio {
	short type;
	union {
		// ...
		struct {
			char prefix; // &, ?, @, [label]
			char *name;
		};
	};
} SXL_pio_t;
void
SXL_pio_init(SXL_pio_t *pio, int type)
{
	pio->type = type;
}
void
SXL_pio_uninit(SXL_pio_t *pio)
{
	switch(pio->type)
	{
	case SXL_PIO_INSTAREF:
	case SXL_PIO_REF: free(pio->name); break;
	}
}

enum {
	SXL_COND_NONE,
	SXL_COND_MATCH,
	SXL_COND_OF,
};
typedef struct sxl_cond {
	int type;
	union {
		// ...
		struct {
			char *match;
		};
		struct {
			char *left, *right; // &[var1] of &[var2]
		};
	};
} SXL_cond_t;
void
SXL_cond_init(SXL_cond_t *cond, int type)
{
	cond->type = type;
}
void
SXL_cond_uninit(SXL_cond_t *cond)
{
	switch(cond->type)
	{
	case SXL_COND_MATCH: free(cond->match); break;
	case SXL_COND_OF: free(cond->left); free(cond->right); break;
	}
}

typedef struct sxl_instr {
	SXL_cond_t cond;
	struct {
		SXL_pio_t in, out;
		SXL_var_t with;
	} *pios;
	int nPios, szPios;
} SXL_instr_t;
void
SXL_instr_uninit(SXL_instr_t *instr)
{
	SXL_cond_uninit(&instr->cond);
	for(int i = 0; i < instr->nPios; i++)
	{
		SXL_pio_uninit(&instr->pios[i].in);
		SXL_pio_uninit(&instr->pios[i].out);
		SXL_var_uninit(&instr->pios[i].with);
	}
	free(instr->pios);
}
void
SXL_instr_addpio(SXL_instr_t *instr, SXL_pio_t in, SXL_pio_t out, SXL_var_t with)
{
	if(instr->nPios + 1 > instr->szPios)
	{
		instr->szPios *= 2;
		instr->szPios++;
		instr->pios = safe_realloc(instr->pios, instr->szPios * sizeof*instr->pios);
	}
	instr->pios[instr->nPios].in = in;
	instr->pios[instr->nPios].out = out;
	instr->pios[instr->nPios].with = with;
	instr->nPios++;
}

typedef struct sxl_label {
	char *name;
	SXL_instr_t *instrs;
	int nInstrs, szInstrs;
} SXL_label_t;
void
SXL_label_init(SXL_label_t *label, const char *name)
{
	label->name = safe_strdup(name);
	label->instrs = NULL;
	label->nInstrs = 0;
	label->szInstrs = 0;
}
void
SXL_label_uninit(SXL_label_t *label)
{
	free(label->name);
	for(int i = 0; i < label->nInstrs; i++)
		SXL_instr_uninit(label->instrs + i);
	free(label->instrs);
}
void
SXL_label_addinstr(SXL_label_t *label, SXL_instr_t instr)
{
	if(label->nInstrs + 1 > label->szInstrs)
	{
		label->szInstrs *= 2;
		label->szInstrs++;
		label->instrs = safe_realloc(label->instrs, label->szInstrs * sizeof*label->instrs);
	}
	label->instrs[label->nInstrs++] = instr;
}

enum {
	SXL_STATE_GLOBAL,
	SXL_STATE_VAR,
	SXL_STATE_VARVALSTRING,
	SXL_STATE_VARVALSTRINGLIST,
	SXL_STATE_VARVALCOLORFG,
	SXL_STATE_VARVALCOLORBG,
	SXL_STATE_VARVALCOLORATTR,
	SXL_STATE_COLORORPIO
	SXL_STATE_VARVALBOOL,
	SXL_STATE_MAX,
};
typedef struct sxl_tunit {
	_TUNIT_HEADER;
	cchar_t data[1024];
	int iWrite, iRead;
	int state, prevState;
	int varType;

	SXL_label_t *labels;
	int nLabels, szLabels;
} *SXL_tunit_t;
void
SXL_tunit_add(SXL_tunit_t sxl, SXL_label_t label)
{
	if(sxl->nLabels + 1 > sxl->szLabels)
	{
		sxl->szLabels *= 2;
		sxl->szLabels++;
		sxl->labels = realloc(sxl->labels, sxl->szLabels * sizeof*sxl->labels);
	}
	sxl->labels[sxl->nLabels++] = label;
}

int
SXL_init(SXL_tunit_t sxl)
{
	memset((void*) sxl + sizeof(struct tunit), 0, sizeof(*sxl) - sizeof(struct tunit));
	return OK;
}

int
SXL_destroy(SXL_tunit_t sxl)
{
	for(int i = 0; i < sxl->nLabels; i++)
		SXL_label_uninit(sxl->labels + i);
	free(sxl->labels);
	free(sxl);
	return OK;
}

void
SXL_buf_out(SXL_tunit_t sxl,
		int c,
		attr_t a,
		short color_pair)
{
	wchar_t w[2];

	w[0] = c;
	w[1] = L'\0';
	setcchar(sxl->data + sxl->iWrite, w, a, color_pair, NULL);
	sxl->iWrite++;
	sxl->iWrite %= ARRLEN(sxl->data);
}

int
SXL_state_global(SXL_tunit_t sxl,
		int c)
{
	switch(c)
	{
	case ' ': case '\r': case '\n': case '\t': case '\v': case '\f':
		SXL_buf_out(sxl, c, 0, C_PAIR_TEXT);
		break;
	case '&':
	case '@':
	case '?':
		sxl->varType = c;
		sxl->state = SXL_STATE_VARIABLE;
		break;
	case '$':
		
		break;
	case '\"':
		
		break;
	default:
		SXL_buf_out(sxl, c, 0, C_PAIR_ERROR);
	}
	return 0;
}

int
SXL_write(SXL_tunit_t sxl,
		int c)
{
	int (*states[SXL_STATE_MAX])(SXL_tunit_t sxl, int c) = {
		[C_STATE_GLOBAL] = SXL_state_global,
	};
	while(states[sxl->state](sxl, c));
	return OK;
}

int
SXL_read(SXL_tunit_t sxl,
		cchar_t *cc)
{
	if(sxl->iWrite == sxl->iRead)
		return EOF;
	memcpy(cc, sxl->data + sxl->iRead, sizeof*cc);
	sxl->iRead++;
	sxl->iRead %= ARRLEN(sxl->data);
	return OK;
}
