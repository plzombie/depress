/*
	Файл	: wremove.h

	Описание: Заголовок для wremove

	История	: 18.03.17	Создан

*/

#ifndef WREMOVE_H
#define WREMOVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern int _wremove(const wchar_t *path);

#ifdef __cplusplus
}
#endif

#endif
