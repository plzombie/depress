/*
	Файл	: _wtoi.c

	Описание: Реализация _wtoi через atoi

	История	: 29.01.23	Создан

*/

#include "stdlib.h"
#include "wtoi.h"

int _wtoi(wchar_t *ws)
{
	char cs[80];
	size_t ws_len, i;

	ws_len = wcslen(ws)+1;
	if(ws_len > 80) return 0;

	for(i = 0; i < ws_len; i++)
		cs[i] = ws[i];

	return atoi(cs);
}
