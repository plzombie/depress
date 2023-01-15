/*
	Файл	: wrmdir.c

	Описание: Реализация _wrmdir поверх rmdir

	История	: 17.03.17	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wchar.h>
#include <unistd.h>

int _wrmdir(const wchar_t *dirname)
{
	char *cdirname;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, dirname, 0)+1;

	cdirname = malloc(strsize);
	if(!cdirname) return -1;

	wcstombsl(cdirname, dirname, strsize);

	result = rmdir(cdirname);

	free(cdirname);

	return(result);
}
