/*
	Файл	: wrmdir.h

	Описание: Заголовок для wrmdir

	История	: 18.03.17	Создан

*/

#ifndef WRMDIR_H
#define WRMDIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern int _wrmdir(const wchar_t *dirname);

#ifdef __cplusplus
}
#endif

#endif
