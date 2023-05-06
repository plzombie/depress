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


#if !defined(_WIN32)
#include "unixsupport/waccess.h"
#include "unixsupport/wremove.h"
#include "unixsupport/wfopen.h"
#include <unistd.h>
#else
#include <io.h>
#endif

#include "../include/depress_converter.h"
#include "../include/depress_image.h"
#include "../include/depress_flags.h"
#include "../include/depress_threads.h"
#include "../include/ppm_save.h"

#include <stdlib.h>
#include <string.h>

#define DJVUL_IMPLEMENTATION
#define THRESHOLD_IMPLEMENTATION
#include "third_party/djvul.h"

int depressDjvuConvertLayeredPage(const depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx, size_t load_image_id, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths);

int depressDjvuConvertPage(depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx, size_t load_image_id, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	FILE *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t *arg0 = 0, *arg_options, *arg_temp = 0, *djvulibre_path;
	unsigned char *buffer = 0;
	const size_t arg0_size = 3*32768+1536; // 2*3(braces)+3(spaces)+1024(options)<1536
	int convert_status = DEPRESS_CONVERT_PAGE_STATUS_OK;

	// Checking for modes that needed separate complex functions
	if(flags.type == DEPRESS_PAGE_TYPE_LAYERED)
		return depressDjvuConvertLayeredPage(flags, load_image, load_image_ctx, load_image_id, tempfile, outputfile, djvulibre_paths);

	arg0 = malloc((arg0_size+1024+80)*sizeof(wchar_t)); // 

	if(!arg0) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_ALLOC_MEMORY;

		goto EXIT;
	} else {
		arg_options = arg0 + arg0_size;
		arg_temp = arg_options + 1024;
	}

	f_temp = _wfopen(tempfile, L"wb");
	if(!f_temp) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

		goto EXIT;
	}

	if(!load_image.load_from_ctx(load_image_ctx, load_image_id, &sizex, &sizey, &channels, &buffer, flags)) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_OPEN_IMAGE;

		goto EXIT;
	}

	if(flags.type == DEPRESS_PAGE_TYPE_BW) {
		if(!flags.nof_illrects) {
			if(!pbmSave(sizex, sizey, buffer, f_temp)) {
				convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

				goto EXIT;
			}
		} else {
			if(!ppmSave(sizex, sizey, channels, buffer, f_temp)) {
				convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

				goto EXIT;
			}
		}
	} else {
		if(!ppmSave(sizex, sizey, channels, buffer, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
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

		swprintf(arg0, arg0_size, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
	} else if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED) {
		int colors;

		djvulibre_path = djvulibre_paths->cpaldjvu_path;

		colors = flags.param1;
		if(colors < 2) colors = 2;
		if(colors > 256) colors = 256;

		swprintf(arg_temp, 80, L"-colors %d", colors);
		wcscat(arg_options, arg_temp);

		if(flags.dpi > 0) {
			swprintf(arg_temp, 80, L" -dpi %d", flags.dpi);
			wcscat(arg_options, arg_temp);
		}

		swprintf(arg0, arg0_size, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
	} else {
		int quality;

		djvulibre_path = djvulibre_paths->c44_path;

		quality = flags.quality + 30;
		if(quality < 30) quality = 30;
		if(quality > 130) quality = 130;

		swprintf(arg_temp, 80, L"-slice %d,%d,%d", quality-25, quality-15, quality);
		wcscat(arg_options, arg_temp);

		if(flags.dpi > 0) {
			swprintf(arg_temp, 80, L" -dpi %d", flags.dpi);
			wcscat(arg_options, arg_temp);
		}

		swprintf(arg0, arg0_size, L"\"%ls\" %ls \"%ls\" \"%ls\"", djvulibre_path, arg_options, tempfile, outputfile);
	}

	if(depressSpawn(djvulibre_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

		goto EXIT;
	}

EXIT:
	if(arg0) free(arg0);

	if(f_temp) fclose(f_temp);
	if(buffer) free(buffer);

	while(1) {
		if(!_waccess(tempfile, 06)) {
			if(_wremove(tempfile) == -1)
#if defined(_WIN32)
				Sleep(0);
#else
				usleep(1000);
#endif
		} else break;
	}

	return convert_status;
}

int depressDjvuConvertLayeredPage(const depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx, size_t load_image_id, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	FILE *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t *arg0 = 0, *arg_options, *arg_temp = 0, *arg_sjbz = 0, *arg_fg44 = 0, *arg_bg44 = 0;
	size_t outputfile_length = 0;
	unsigned char *buffer = 0, *buffer_mask = 0, *buffer_bg = 0, *buffer_fg = 0;
	const size_t arg0_size = 5*32768+1536; // 2*3(braces)+3(spaces)+1024(options)<1536
	int convert_status = DEPRESS_CONVERT_PAGE_STATUS_OK;

	outputfile_length = wcslen(outputfile);
	if(outputfile_length > (32768-5-1)) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_GENERIC_ERROR;

		goto EXIT;
	}

	arg0 = malloc((arg0_size+1024+80+3*32768)*sizeof(wchar_t));

	if(!arg0) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_ALLOC_MEMORY;

		goto EXIT;
	} else {
		arg_options = arg0 + arg0_size;
		arg_temp = arg_options + 1024;
		arg_sjbz = arg_temp + 80;
		arg_fg44 = arg_sjbz + 32768;
		arg_bg44 = arg_fg44 + 32768;
	}

	memcpy(arg_sjbz, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_sjbz+outputfile_length, L".sjbz"); // Mask chunk (also I used this filename for bg/fg mask pbm)
	memcpy(arg_fg44, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_fg44+outputfile_length, L".fg44"); // Foreground chunk
	memcpy(arg_bg44, outputfile, (outputfile_length+1)*sizeof(wchar_t));
	wcscpy(arg_bg44+outputfile_length, L".bg44"); // Background chunk

	if(!load_image.load_from_ctx(load_image_ctx, load_image_id, &sizex, &sizey, &channels, &buffer, flags)) {
		convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_OPEN_IMAGE;

		goto EXIT;
	}

	{
		unsigned int bg_downsample, fg_downsample;
		unsigned int bg_width, bg_height, fg_width, fg_height;
		size_t i;
		int quality;

		quality = flags.quality + 30;
		if(quality < 30) quality = 30;
		if(quality > 130) quality = 130;

		if(flags.param1 < 1) bg_downsample = 1;
		else bg_downsample = (unsigned int)flags.param1;
		if(flags.param2 < 1) fg_downsample = 1;
		else fg_downsample = (unsigned int)flags.param2;

		bg_width = sizex/bg_downsample+(sizex%bg_downsample>0);
		bg_height = sizey/bg_downsample+(sizey%bg_downsample>0);
		fg_width = bg_width/fg_downsample+(bg_width%fg_downsample>0);
		fg_height = bg_height/fg_downsample+(bg_height%fg_downsample>0);

		buffer_mask = malloc((size_t)sizex*(size_t)sizey);
		buffer_bg = malloc(bg_width*bg_height*channels);
		buffer_fg = malloc(bg_width*bg_height*channels);
		if(!buffer_mask || !buffer_bg || !buffer_fg) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_ALLOC_MEMORY;

			goto EXIT;
		}

		ImageDjvulThreshold(buffer, (bool *)buffer_mask, buffer_bg, buffer_fg, sizex, sizey, channels,
			bg_downsample, 0, 1, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f);

		for(i = 0; i < (size_t)sizex*(size_t)sizey; i++)
			if(buffer_mask[i]) buffer_mask[i] = 0; else buffer_mask[i] = 255;

		if(fg_downsample > 1)
			ImageFGdownsample(buffer_fg, bg_width, bg_height, channels, flags.param2);

		// Save background
		f_temp = _wfopen(tempfile, L"wb");
		if(!f_temp) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		if(!ppmSave(bg_width, bg_height, channels, buffer_bg, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		fclose(f_temp); f_temp = 0;
		// Save background mask
		f_temp = _wfopen(arg_sjbz, L"wb");
		if(!f_temp) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		memset(buffer_bg, 0, bg_width*bg_height);
		for(i = 0; i < (size_t)sizey; i++) {
			size_t j;

			for(j = 0; j < (size_t)sizex; j++)
				if(buffer_mask[i*sizex+j])
					buffer_bg[(i*bg_height/sizey)*bg_width+(j*bg_width/sizex)] = 255;
		}
		if(!pbmSave(bg_width, bg_height, buffer_bg, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		free(buffer_bg); buffer_bg = 0;
		fclose(f_temp); f_temp = 0;
		// Convert background
		swprintf(arg0, arg0_size, L"\"%ls\" -slice %d,%d,%d -mask \"%ls\" \"%ls\" \"%ls\"", djvulibre_paths->c44_path, quality-25, quality-15, quality, arg_sjbz, tempfile, outputfile);
		if(depressSpawn(djvulibre_paths->c44_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;
			
			goto EXIT;
		}
		swprintf(arg0, arg0_size, L"\"%ls\" \"%ls\" \"BG44=%ls\"", djvulibre_paths->djvuextract_path, outputfile, arg_bg44);
		if(depressSpawn(djvulibre_paths->djvuextract_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE; 
			
			goto EXIT;
		}

		// Save foreground
		f_temp = _wfopen(tempfile, L"wb");
		if(!f_temp) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		if(!ppmSave(fg_width, fg_height, channels, buffer_fg, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		fclose(f_temp); f_temp = 0;
		// Save background mask
		f_temp = _wfopen(arg_sjbz, L"wb");
		if(!f_temp) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		memset(buffer_fg, 0, fg_width*fg_height);
		for(i = 0; i < (size_t)sizey; i++) {
			size_t j;

			for(j = 0; j < (size_t)sizex; j++)
				if(!buffer_mask[i*sizex+j])
					buffer_fg[(i*fg_height/sizey)*fg_width+(j*fg_width/sizex)] = 255;
		}
		if(!pbmSave(fg_width, fg_height, buffer_fg, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		free(buffer_fg); buffer_fg = 0;
		fclose(f_temp); f_temp = 0;
		// Convert foreground
		swprintf(arg0, arg0_size, L"\"%ls\" -slice %d -mask \"%ls\" \"%ls\" \"%ls\"", djvulibre_paths->c44_path, quality, arg_sjbz, tempfile, outputfile);
		if(depressSpawn(djvulibre_paths->c44_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE; 
			
			goto EXIT;
		}
		swprintf(arg0, arg0_size, L"\"%ls\" \"%ls\" \"BG44=%ls\"", djvulibre_paths->djvuextract_path, outputfile, arg_fg44);
		if(depressSpawn(djvulibre_paths->djvuextract_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;
			
			goto EXIT;
		}

		// Save mask
		f_temp = _wfopen(tempfile, L"wb");
		if(!f_temp) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		if(!pbmSave(sizex, sizey, buffer_mask, f_temp)) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		free(buffer_mask); buffer_mask = 0;
		fclose(f_temp); f_temp = 0;
		swprintf(arg0, arg0_size, L"\"%ls\" \"%ls\" \"%ls\"", djvulibre_paths->cjb2_path, tempfile, outputfile);
		if(depressSpawn(djvulibre_paths->cjb2_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}
		swprintf(arg0, arg0_size, L"\"%ls\" \"%ls\" \"Sjbz=%ls\"", djvulibre_paths->djvuextract_path, outputfile, arg_sjbz);
		if(depressSpawn(djvulibre_paths->djvuextract_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;

			goto EXIT;
		}

		swprintf(arg_options, 1024, L"INFO=,,%d", flags.dpi);
		swprintf(arg0, arg0_size, L"\"%ls\" \"%ls\" %ls \"Sjbz=%ls\" \"FG44=%ls\" \"BG44=%ls\"", djvulibre_paths->djvumake_path,
			outputfile, arg_options, arg_sjbz, arg_fg44, arg_bg44);
		if(depressSpawn(djvulibre_paths->djvumake_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE) {
			convert_status = DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE;
			
			goto EXIT;
		}
	}

EXIT:
	if(f_temp) fclose(f_temp);
	if(buffer) free(buffer);
	if(buffer_mask) free(buffer_mask);
	if(buffer_bg) free(buffer_bg);
	if(buffer_fg) free(buffer_fg);

	{
		bool del_temp = true, del_sjbz = false, del_fg44 = false, del_bg44 = false;

		if(arg_sjbz) del_sjbz = true;
		if(arg_fg44) del_fg44 = true;
		if(arg_bg44) del_bg44 = true;

		while(del_temp || del_sjbz || del_fg44 || del_bg44) {
			if(!_waccess(tempfile, 06)) {
				if(_wremove(tempfile) == -1)
#if defined(_WIN32)
					Sleep(0);
#else
					usleep(1000);
#endif
			} else del_temp = false;

			if(arg_sjbz) {
				if(!_waccess(arg_sjbz, 06)) {
					if(_wremove(arg_sjbz) == -1)
#if defined(_WIN32)
						Sleep(0);
#else
						usleep(1000);
#endif
				} else del_sjbz = false;
			}

			if(arg_fg44) {
				if(!_waccess(arg_fg44, 06)) {
					if(_wremove(arg_fg44) == -1)
#if defined(_WIN32)
						Sleep(0);
#else
						usleep(1000);
#endif
				} else del_fg44 = false;
			} 

			if(arg_bg44) {
				if(!_waccess(arg_bg44, 06)) {
					if(_wremove(arg_bg44) == -1)
#if defined(_WIN32)
						Sleep(0);
#else
						usleep(1000);
#endif
				} else del_bg44 = false;
			}
		}
	}

	if(arg0) free(arg0);

	return convert_status;
}
