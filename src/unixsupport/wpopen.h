/*
	Файл	: wpopen.h

	Описание: Заголовок для wpopen

	История	: 15.01.23	Создан

*/

#ifndef WPOPEN_H
#define WPOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <wchar.h>

extern FILE *_wpopen(const wchar_t *filename, const wchar_t *mode);

#ifdef __cplusplus
}
#endif

#endif
