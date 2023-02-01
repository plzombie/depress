
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <unistd.h>

wchar_t *_wgetcwd(const wchar_t *buffer, const int maxlen)
{
	char *cbuf;
	size_t cbuf_len;
	
#if defined(_GNU_SOURCE)
	cbuf = get_current_dir_name();
#else
	cbuf = getcwd(0, 0);
#endif
	
	if(!cbuf) return 0;
	
	cbuf_len = strlen(cbuf);
	
	if(cbuf_len+1 > maxlen) {
		free(cbuf);
		
		return 0;
	}
	
	if(mbstowcs(buffer, cbuf, maxlen) == maxlen) {
		free(cbuf);
		
		return 0;
	}

	free(cbuf);
	
	return buffer;
}