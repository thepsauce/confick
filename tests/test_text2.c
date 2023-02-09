#include "test.h"
#include "../wdgbase.h"
#include "../text2.h"

int
main(void)
{
	text_t text;
	wchar_t str[8000];

	txinit(&text);

	for(int i = 0; i < sizeof(str)/sizeof*(str); i++)
		str[i] = L'A';
	
	txaddnstr(&text, str, sizeof(str)/sizeof*(str));
	text.cur = text.first;
	text.iCur = TXBUFSIZE - 2;
	for(int i = 0; i < 10000; i++)
		txdelete(&text);

	txdiscard(&text);

	return 0;
}

