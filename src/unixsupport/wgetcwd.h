/*
	Файл	: wgetcwd.h

	Описание: Заголовок для wgetcwd

	История	: 01.02.23	Создан

*/

#ifndef WGETCWD_H
#define WGETCWD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>

extern wchar_t *_wgetcwd(wchar_t *buffer, const int maxlen);

#ifdef __cplusplus
}
#endif

#endif