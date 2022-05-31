/*
BSD 2-Clause License

Copyright (c) 2021, Mikhail Morozov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../include/depress_converter.h"
#include "../include/depress_image.h"
#include "../include/depress_flags.h"
#include "../include/ppm_save.h"

#include <Windows.h>

#include <io.h>
#include <process.h>

bool depressConvertPage(depress_flags_type flags, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	FILE *f_in = 0, *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t *arg0, *arg1, *arg2;
	unsigned char *buffer = 0;
	bool result = false;

	arg0 = malloc(3*32770*sizeof(wchar_t));

	if(!arg0)
		goto EXIT;
	else {
		arg1 = arg0 + 32770;
		arg2 = arg1 + 32770;
	}

	f_in = _wfopen(inputfile, L"rb");
	f_temp = _wfopen(tempfile, L"wb");
	if(!f_in || !f_temp)
		goto EXIT;

	buffer = depressLoadImage(f_in, &sizex, &sizey, &channels, flags.type == DEPRESS_PAGE_TYPE_BW);
	if(!buffer)
		goto EXIT;

	if(flags.type == DEPRESS_PAGE_TYPE_BW && flags.param1 == DEPRESS_PAGE_TYPE_BW_PARAM1_ERRDIFF)
		depressImageApplyErrorDiffusion(buffer, sizex, sizey);

	fclose(f_in); f_in = 0;

	if(flags.type == DEPRESS_PAGE_TYPE_BW) {
		if(!pbmSave(sizex, sizey, buffer, f_temp))
			goto EXIT;
	} else {
		if(!ppmSave(sizex, sizey, channels, buffer, f_temp))
			goto EXIT;
	}

	free(buffer); buffer = 0;
	fclose(f_temp); f_temp = 0;

	swprintf(arg1, 32770, L"\"%ls\"", tempfile);
	swprintf(arg2, 32770, L"\"%ls\"", outputfile);

	if(flags.type == DEPRESS_PAGE_TYPE_BW) {
		swprintf(arg0, 32770, L"\"%ls\"", djvulibre_paths->cjb2_path);
		if(_wspawnl(_P_WAIT, djvulibre_paths->cjb2_path, arg0, arg1, arg2, 0)) goto EXIT;
	} else {
		swprintf(arg0, 32770, L"\"%ls\"", djvulibre_paths->c44_path);
		if(_wspawnl(_P_WAIT, djvulibre_paths->c44_path, arg0, arg1, arg2, 0)) goto EXIT;
	}

	result = true;

EXIT:
	if(arg0) free(arg0);

	if(f_in) fclose(f_in);
	if(f_temp) fclose(f_temp);
	if(buffer) free(buffer);

	while(1) {
		if(!_waccess(tempfile, 06)) {
			if(_wremove(tempfile) == -1)
				Sleep(0);
		} else break;
	}

	return result;

}
