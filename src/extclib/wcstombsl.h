/*
	Файл	: wcstombsl.h

	Описание: Заголовок для wcstombsl

	История	: 19.04.15	Создан

*/

#ifndef WCSTOMBSL_H
#define WCSTOMBSL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern size_t wcstombsl(char *dest, const wchar_t *src, size_t max);

#ifdef __cplusplus
}
#endif

#endif
