/*
	Файл	: pclose.h

	Описание: Заголовок для pclose

	История	: 15.01.23	Создан

*/

#ifndef PCLOSE_H
#define PCLOSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define _pclose(f) pclose(f)

#ifdef __cplusplus
}
#endif

#endif
