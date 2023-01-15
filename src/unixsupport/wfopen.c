/*
	Файл	: wfopen.c

	Описание: Реализация _wfopen поверх fopen

	История	: 04.04.14	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

FILE *_wfopen(const wchar_t *filename, const wchar_t *mode)
{
	char *cfilename, *cmode;
	FILE *result;
	size_t strsize;

	// Преобразование filename
	strsize = wcstombs(NULL, filename, 0)+1;

	cfilename = malloc(strsize);
	if(!cfilename) return 0;

	wcstombsl(cfilename, filename, strsize);


	// Преобразование mode
	strsize = wcstombs(NULL, mode, 0)+1;

	cmode = malloc(strsize);
	if(!cmode) { free(cfilename); return 0; }

	wcstombsl(cmode, mode, strsize);


	result = fopen(cfilename, cmode);

	free(cfilename);
	free(cmode);

	return result;
}
