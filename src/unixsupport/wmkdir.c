/*
	Файл	: wmkdir.c

	Описание: Реализация _wmkdir поверх mkdir

	История	: 17.03.17	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wchar.h>

int _wmkdir(const wchar_t *dirname)
{
	char *cdirname;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, dirname, 0)+1;

	cdirname = malloc(strsize);
	if(!cdirname) return -1;

	wcstombsl(cdirname, dirname, strsize);

	result = mkdir(cdirname, S_IRWXU);

	free(cdirname);

	return(result);
}
