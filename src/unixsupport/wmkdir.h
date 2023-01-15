/*
	Файл	: wmkdir.h

	Описание: Заголовок для wmkdir

	История	: 18.03.17	Создан

*/

#ifndef WMKDIR_H
#define WMKDIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern int _wmkdir(const wchar_t *dirname);

#ifdef __cplusplus
}
#endif

#endif
