#include <cfk/wdg.h>

void *keeperPiece;

int
safe_init(void)
{
	keeperPiece = malloc(4096);
	return keeperPiece ? 0 : -1;
}
	
void
heavy_breathing(void)
{
	endwin();
	free(keeperPiece);
	keeperPiece = NULL;
	printf("Ran out of memory, using keeper's memory for last breath\nWhat's your call?\n");
options:
	printf("1. Exit quietly\n");
	printf("2. Retry\n");
	int c = getchar();
	while(getchar() != '\n');
	switch(c)
	{
	case '1':
		fcloseall();
		wdgmgrdiscard();
		break;
	case '2':
		if(!(keeperPiece = malloc(4096)))
		{
			free(keeperPiece);
			printf("Retry failed!\n");
			goto options;
		}
		printf("Retry successful!\n");
		sleep(1);
		refresh();
		break;
	default:
		goto options;
	}
}

void *
safe_malloc(size_t size)
{
	void *ptr;
	while(!(ptr = malloc(size)))
		heavy_breathing();
	return ptr;
}

void *
safe_realloc(void *ptr, size_t size)
{
	while(!(ptr = realloc(ptr, size)))
		heavy_breathing();
	return ptr;
}

void *
safe_strdup(const char *str)
{
	void *ptr;
	while(!(ptr = strdup(str)))
		heavy_breathing();
	return ptr;
}

void *
safe_strndup(const char *str, size_t nStr)
{
	void *ptr;
	while(!(ptr = strndup(str, nStr)))
		heavy_breathing();
	return ptr;
}
