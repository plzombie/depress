/*
	Файл	: waccess.h

	Описание: Заголовок для waccess

	История	: 18.03.17	Создан

*/

#ifndef WACCESS_H
#define WACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "permissions.h"

extern int _waccess(const wchar_t *path, int mode);

#ifdef __cplusplus
}
#endif

#endif
