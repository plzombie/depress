/*
	Файл	: pclose.h

	Описание: Заголовок для pclose

	История	: 15.01.23	Создан

*/

#ifndef WPOPEN_H
#define WPOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define _pclose(f) pclose(f)

#ifdef __cplusplus
}
#endif

#endif
