/*
BSD 2-Clause License

Copyright (c) 2021-2022, Mikhail Morozov
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

#if defined(_DEBUG) && defined(USE_STB_LEAKCHECK)
#include "third_party/stb_leakcheck.h"
#endif

#include "../include/depress_converter.h"
#include "../include/depress_image.h"
#include "../include/depress_flags.h"
#include "../include/depress_threads.h"
#include "../include/ppm_save.h"

#include <Windows.h>

#include <io.h>
#include <process.h>

#define DJVUL_IMPLEMENTATION
#include "third_party/djvul.h"

bool depressConvertLayeredPage(const depress_flags_type flags, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths);

bool depressConvertPage(depress_flags_type flags, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	FILE *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t *arg0 = 0, *arg_options, *arg_temp = 0, *djvulibre_path;
	unsigned char *buffer = 0;
	bool result = false;

	// Checking for modes that needed separate complex functions
	if(flags.type == DEPRESS_PAGE_TYPE_LAYERED)
		return depressConvertLayeredPage(flags, inputfile, tempfile, outputfile, djvulibre_paths);

	arg0 = malloc((3*32768+1536+1024+80)*sizeof(wchar_t)); // 2*3(braces)+3(spaces)+1024(options)<1566

	if(!arg0)
		goto EXIT;
	else {
		arg_options = arg0 + 3 * 32768 + 1536;
		arg_temp = arg_options + 1024;
	}

	f_temp = _wfopen(tempfile, L"wb");
	if(!f_temp)
		goto EXIT;

	if(!depressLoadImageFromFileAndApplyFlags(inputfile, &sizex, &sizey, &channels, &buffer, flags))
		goto EXIT;

	if(flags.type == DEPRESS_PAGE_TYPE_BW) {
		if(!flags.nof_illrects) {
			if(!pbmSave(sizex, sizey, buffer, f_temp))
				goto EXIT;
		} else {
			if(!ppmSave(sizex, sizey, channels, buffer, f_temp))
				goto EXIT;
		}
	} else {
		if(!ppmSave(sizex, sizey, channels, buffer, f_temp))
			goto EXIT;
	}

	free(buffer); buffer = 0;
	fclose(f_temp); f_temp = 0;

	*arg_options = 0;

	if(flags.type == DEPRESS_PAGE_TYPE_BW && !flags.nof_illrects) {
		djvulibre_path = djvulibre_paths->cjb2_path;

		if(flags.quality >= 0 && flags.quality <= 100) {
			int q;

			q = flags.quality;
			q = 200 - 2 * q; // 0 - 100%, 200 - 0%

			swprintf(arg_temp, 80, L"-losslevel %d", q);
			wcscat(arg_options, arg_temp);
		}

		if(flags.dpi > 0) {
			swprintf(arg_temp, 80, L" -dpi %d", flags.dpi);
			wcscat(arg_options, arg_temp);
		}

		swprintf(arg0, 32770, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
	} else {
		int q;

		djvulibre_path = djvulibre_paths->c44_path;

		q = flags.quality/10;
		if(q < 1) q = 1;
		if(q > 10) q = 10;

		swprintf(arg_temp, 80, L"-percent %d", q);
		wcscat(arg_options, arg_temp);

		if(flags.dpi > 0) {
			swprintf(arg_temp, 80, L" -dpi %d", flags.dpi);
			wcscat(arg_options, arg_temp);
		}

		swprintf(arg0, 32770, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
	}

	if(depressSpawn(djvulibre_path, arg0, true, true) == INVALID_HANDLE_VALUE) goto EXIT;

	result = true;

EXIT:
	if(arg0) free(arg0);

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

bool depressConvertLayeredPage(const depress_flags_type flags, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	//FILE *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t *arg0 = 0, *arg_options, *arg_temp = 0, *arg_sjbz = 0, *arg_fg44 = 0, *arg_bg44 = 0, *djvulibre_path;
	size_t outputfile_length = 0;
	unsigned char *buffer = 0;
	bool result = false;

	outputfile_length = wcslen(outputfile);
	if(outputfile_length > (32768-5-1)) goto EXIT;

	arg0 = malloc((3*32768+1536+1024+80+3*32768)*sizeof(wchar_t)); // 2*3(braces)+3(spaces)+1024(options)<1566

	if(!arg0)
		goto EXIT;
	else {
		arg_options = arg0 + 3 * 32768 + 1536;
		arg_temp = arg_options + 1024;
		arg_sjbz = arg_temp + 80;
		arg_fg44 = arg_sjbz + 32768;
		arg_bg44 = arg_fg44 + 32768;
	}

	memcpy(arg_sjbz, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_sjbz+outputfile_length, L".sjbz");
	memcpy(arg_fg44, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_fg44+outputfile_length, L".fg44");
	memcpy(arg_bg44, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_bg44+outputfile_length, L".bg44");

	if(!depressLoadImageFromFileAndApplyFlags(inputfile, &sizex, &sizey, &channels, &buffer, flags))
		goto EXIT;

	// Start temporary code
	*arg_options = 0;

	{
		FILE *f_temp = 0;
		int q;

		f_temp = _wfopen(tempfile, L"wb");
		if(!f_temp)
			goto EXIT;

		if(!ppmSave(sizex, sizey, channels, buffer, f_temp))
			goto EXIT;

		free(buffer); buffer = 0;
		fclose(f_temp); f_temp = 0;

		djvulibre_path = djvulibre_paths->c44_path;

		q = flags.quality/10;
		if(q < 1) q = 1;
		if(q > 10) q = 10;

		swprintf(arg_temp, 80, L"-percent %d", q);
		wcscat(arg_options, arg_temp);

		if(flags.dpi > 0) {
			swprintf(arg_temp, 80, L" -dpi %d", flags.dpi);
			wcscat(arg_options, arg_temp);
		}

		swprintf(arg0, 32770, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
		//wprintf(L"dlp %ls arg0 %ls\n", djvulibre_path, arg0);
	}

	if(depressSpawn(djvulibre_path, arg0, true, true) == INVALID_HANDLE_VALUE) goto EXIT;
	// End temporary code

	result = true;

EXIT:
	if(buffer) free(buffer);

	{
		bool del_temp = true, del_sjbz = false, del_fg44 = false, del_bg44 = false;

		if(arg_sjbz) del_sjbz = true;
		if(arg_fg44) del_fg44 = true;
		if(arg_bg44) del_bg44 = true;

		while(del_temp || del_sjbz || del_fg44 || del_bg44) {
			if(!_waccess(tempfile, 06)) {
				if(_wremove(tempfile) == -1)
					Sleep(0);
			} else del_temp = false;

			if(arg_sjbz) {
				if(!_waccess(arg_sjbz, 06)) {
					if(_wremove(arg_sjbz) == -1)
						Sleep(0);
				} else del_sjbz = false;
			}

			if(arg_fg44) {
				if(!_waccess(arg_fg44, 06)) {
					if(_wremove(arg_fg44) == -1)
						Sleep(0);
				} else del_fg44 = false;
			} 

			if(arg_bg44) {
				if(!_waccess(arg_bg44, 06)) {
					if(_wremove(arg_bg44) == -1)
						Sleep(0);
				} else del_bg44 = false;
			}
		}
	}

	if(arg0) free(arg0);

	return result;
}
