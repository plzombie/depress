/*
	Файл	: wcstombsl.c

	Описание: Версия wcstombs, где max задаёт размер буфера dest, а не макс. количество записанных туда символов. В случае ошибки, в dest[0] также записывается 0. Если dest == NULL или src == NULL, возвращает (size_t)-1

	История	: 19.04.15	Создан

*/

#include "wcstombsl.h"

#include <stdlib.h>

size_t wcstombsl(char *dest, const wchar_t *src, size_t max)
{
	size_t ret;

	if(max == 0) return 0;
	if(src == NULL || dest == NULL) return (size_t)-1;

	ret = wcstombs(dest, src, max-1);

	if(ret > 0 && ret < max)
		dest[ret] = 0;

	if(ret == (size_t)(-1))
		dest[0] = 0;

	return ret;
}
