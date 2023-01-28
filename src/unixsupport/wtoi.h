/*
	Файл	: wtoi.h

	Описание: Заголовок для _wtoi

	История	: 29.01.23	Создан

*/

#ifndef WTOI_H
#define WTOI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>

extern int _wtoi(wchar_t *ws);

#ifdef __cplusplus
}
#endif

#endif
