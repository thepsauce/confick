#define _XOPEN_SOURCE_EXTENDED
#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))

#define min(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__a:__b; \
})

#define max(a, b) \
({ \
	__auto_type __a = (a); \
	__auto_type __b = (b); \
	__a<__b?__b:__a; \
}

#define CURSEDRGB(color) ((color>>16)&0xFF)*1000/256, ((color>>8)&0xFF)*1000/256, (color&0xFF)*1000/256

void *keeperPiece;

void __attribute__((cold))
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

void * __attribute__((returns_nonnull))
safe_malloc(size_t size)
{
	void *ptr;
	while(!(ptr = malloc(size)))
		heavy_breathing();
	return ptr;
}

void * __attribute__((returns_nonnull))
safe_realloc(void *ptr, size_t size)
{
	while(!(ptr = realloc(ptr, size)))
		heavy_breathing();
	return ptr;
}

void * __attribute__((returns_nonnull))
safe_strdup(const char *str)
{
	void *ptr;
	while(!(ptr = strdup(str)))
		heavy_breathing();
	return ptr;
}

void * __attribute__((returns_nonnull))
safe_strndup(const char *str, size_t nStr)
{
	void *ptr;
	while(!(ptr = strndup(str, nStr)))
		heavy_breathing();
	return ptr;
}
