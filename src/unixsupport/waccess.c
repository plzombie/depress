/*
	Файл	: waccess.c

	Описание: Реализация _waccess поверх access

	История	: 04.04.14	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <unistd.h>

int _waccess(const wchar_t *path, int mode)
{
	char *cpath;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, path, 0)+1;

	cpath = malloc(strsize);
	if(!cpath) return -1;

	wcstombsl(cpath, path, strsize);

	result = access(cpath, mode);

	free(cpath);

	return(result);
}
