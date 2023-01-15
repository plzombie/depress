/*
	Файл	: permissions.h

	Описание: Разрешения для _wopen, _waccess, _wchmod

	История	: 12.06.18	Создан

*/

#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#ifndef _S_IREAD
	#ifndef S_IREAD
		#define _S_IREAD S_IRUSR
	#else
		#define _S_IREAD S_IREAD
	#endif
#endif
#ifndef _S_IWRITE
	#ifndef S_IWRITE
		#define _S_IWRITE S_IWUSR
	#else
		#define _S_IWRITE S_IWRITE
	#endif
#endif
#ifndef _S_IEXEC
	#ifndef S_IEXEC
		#define _S_IEXEC S_IXUSR
	#else
		#define _S_IEXEC S_IEXEC
	#endif
#endif

#endif
