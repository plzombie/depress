/*
	Файл	: wremove.c

	Описание: Реализация _wremove поверх remove

	История	: 04.04.14	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int _wremove(const wchar_t *path)
{
	char *cpath;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, path, 0)+1;

	cpath = malloc(strsize);
	if(!cpath) return -1;

	wcstombsl(cpath, path, strsize);

	result = remove(cpath);

	free(cpath);

	return(result);
}
