/*
	Файл	: wfopen.h

	Описание: Заголовок для wfopen

	История	: 18.03.18	Создан

*/

#ifndef WFOPEN_H
#define WFOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <wchar.h>

extern FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);

#ifdef __cplusplus
}
#endif

#endif
