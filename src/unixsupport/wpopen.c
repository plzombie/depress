/*
	Файл	: wpopen.c

	Описание: Реализация _wpopen поверх popen

	История	: 15.01.23	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

FILE *_wpopen(const wchar_t *filename, const wchar_t *mode)
{
	char *cfilename, cmode[2];
	FILE *result;
	size_t strsize;

	// Преобразование filename
	strsize = wcstombs(NULL, filename, 0)+1;

	cfilename = malloc(strsize);
	if(!cfilename) return 0;

	wcstombsl(cfilename, filename, strsize);


	// Преобразование mode
	cmode[0] = 0;
	while(*mode) {
		if(*mode == 'r' || *mode == 'w')
			cmode[0] = *mode;
		mode++;
	}
	cmode[1] = 0;


	result = popen(cfilename, cmode);

	free(cfilename);

	return result;
}
