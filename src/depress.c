/*
BSD 2-Clause License

Copyright (c) 2021-2023, Mikhail Morozov
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
#define STB_LEAKCHECK_IMPLEMENTATION
#include "third_party/stb_leakcheck.h"
#endif

#if !defined(_WIN32)
#include "unixsupport/waccess.h"
#include "unixsupport/wremove.h"
#else
#include <Windows.h>

#include <fcntl.h>
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#include <wchar.h>
#include <time.h>

#include "../include/depress_document.h"

#define DEPRESS_ARG_PAGETYPE_BW L"-bw"
#define DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF L"-errdiff"
#define DEPRESS_ARG_PAGETYPE_BW_PARAM1_ADAPTIVE L"-adaptive"
#define DEPRESS_ARG_PAGETYPE_LAYERED L"-layered"
#define DEPRESS_ARG_PAGETYPE_LAYERED_PARAM1_DOWNSAMPLEALL L"-laydownall"
#define DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG L"-laydownfg"
#define DEPRESS_ARG_PAGETYPE_PALETTIZED L"-palettized"
#define DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM1_PALCOLORS L"-palcolors"
#define DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_QUANT L"-quant"
#define DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_NOTESHRINK L"-noteshrink"
#define DEPRESS_ARG_PAGETITLEAUTO L"-pta"
#define DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME L"-shortfntitle"
#define DEPRESS_ARG_TEMP L"-temp"
#define DEPRESS_ARG_QUALITY L"-quality"
#define DEPRESS_ARG_DPI L"-dpi"

#if !defined(_WIN32)
#include "unixsupport/wtoi.h"
#endif

int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	wchar_t text_list_filename[32768], text_list_path[32768], *text_list_name_start;
	depress_flags_type flags;
	size_t text_list_fn_length;
	depress_document_type document;
	depress_document_flags_type document_flags;
	bool success = true;
	clock_t time_start;

	depressSetDefaultPageFlags(&flags);

	depressSetDefaultDocumentFlags(&document_flags);

	setlocale(LC_ALL, "");

#ifdef _MSC_VER
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
#endif

	// Reading options
	argsc = argc - 1;
	argsp = argv + 1;
	while(argsc > 0) {
		if((*argsp)[0] != '-')
			break;

		argsc--;

		if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_BW)) {
			flags.type = DEPRESS_PAGE_TYPE_BW;
			flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_SIMPLE;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF)) {
			if(flags.type == DEPRESS_PAGE_TYPE_BW)
				flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_ERRDIFF;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF, DEPRESS_ARG_PAGETYPE_BW);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_BW_PARAM1_ADAPTIVE)) {
			if(flags.type == DEPRESS_PAGE_TYPE_BW)
				flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_ADAPTIVE;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_BW_PARAM1_ADAPTIVE, DEPRESS_ARG_PAGETYPE_BW);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_LAYERED)) {
			flags.type = DEPRESS_PAGE_TYPE_LAYERED;
			flags.param1 = 3;
			flags.param2 = 2;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_LAYERED_PARAM1_DOWNSAMPLEALL)) {
			if(argsc > 0) {
				argsc--;
				if(flags.type == DEPRESS_PAGE_TYPE_LAYERED) {
					flags.param1 = _wtoi(*(++argsp));
					if(flags.param1 < 1) {
						wprintf(L"Warning: downsample rate should be greater than 0\n");
						flags.param2 = 1;
					}
				} else {
					argsp++;
					wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_LAYERED_PARAM1_DOWNSAMPLEALL, DEPRESS_ARG_PAGETYPE_LAYERED);
				}
			} else
				wprintf(L"Warning: argument " DEPRESS_ARG_PAGETYPE_LAYERED_PARAM1_DOWNSAMPLEALL L" should have parameter\n");
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG)) {
			if(argsc > 0) {
				argsc--;
				if(flags.type == DEPRESS_PAGE_TYPE_LAYERED) {
					flags.param2 = _wtoi(*(++argsp));
					if(flags.param2 < 1) {
						wprintf(L"Warning: downsample rate should be greater than 0\n");
						flags.param2 = 1;
					}
				} else {
					argsp++;
					wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG, DEPRESS_ARG_PAGETYPE_LAYERED);
				}
			} else
				wprintf(L"Warning: argument " DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG L" should have parameter\n");
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_PALETTIZED)) {
			flags.type = DEPRESS_PAGE_TYPE_PALETTIZED;
			flags.param1 = 8;
			flags.param2 = 0;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM1_PALCOLORS)) {
			if(argsc > 0) {
				argsc--;
				if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED) {
					flags.param1 = _wtoi(*(++argsp));
					if(flags.param1 < 2) {
						wprintf(L"Warning: downsample rate should be greater than or equal to 2\n");
						flags.param1 = 2;
					}
					if(flags.param1 > 256) {
						wprintf(L"Warning: downsample rate should be less than or equal to 256\n");
						flags.param1 = 256;
					}
				} else {
					argsp++;
					wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM1_PALCOLORS, DEPRESS_ARG_PAGETYPE_PALETTIZED);
				}
			} else
				wprintf(L"Warning: argument " DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG L" should have parameter\n");
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_QUANT)) {
			if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED)
				flags.param2 = DEPRESS_PAGE_TYPE_PALETTIZED_PARAM2_QUANT;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_QUANT, DEPRESS_ARG_PAGETYPE_PALETTIZED);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_NOTESHRINK)) {
			if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED)
				flags.param2 = DEPRESS_PAGE_TYPE_PALETTIZED_PARAM2_NOTESHRINK;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_NOTESHRINK, DEPRESS_ARG_PAGETYPE_PALETTIZED);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO)) {
			document_flags.page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME)) {
			document_flags.page_title_type_flags |= DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_TEMP)) {
			if(argsc > 0) {
				argsc--;
				document_flags.userdef_temp_dir = *(++argsp);
			} else
				wprintf(L"Warning: argument " DEPRESS_ARG_TEMP L" should have parameter\n");
		} else if(!wcscmp(*argsp, DEPRESS_ARG_QUALITY)) {
			if(argsc > 0) {
				argsc--;
				flags.quality = _wtoi(*(++argsp));
				if(flags.quality < 0 || flags.quality > 100) {
					wprintf(L"Warning: quality must be between 0 and 100\n");
					flags.quality = 100;
				}
			} else
				wprintf(L"Warning: argument " DEPRESS_ARG_QUALITY L" should have parameter\n");
		} else if(!wcscmp(*argsp, DEPRESS_ARG_DPI)) {
			if(argsc > 0) {
				argsc--;
				flags.dpi = _wtoi(*(++argsp));
				if(flags.dpi <= 0) {
					wprintf(L"Warning: dpi must be greater than\n");
					flags.dpi = 300;
				}
			}
		} else
			wprintf(L"Warning: unknown argument %ls\n", *argsp);

		argsp++;
	}
	
	if(argsc < 2) {
		wprintf(
			L"\tdepress [options] input.txt output.djvu\n"
			L"\t\toptions:\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_BW L" - create black and white document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF L" - use error diffusion for bw document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_BW_PARAM1_ADAPTIVE L" - use adaptive binarization for bw document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_LAYERED L" - create layered document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_LAYERED_PARAM1_DOWNSAMPLEALL L" ratio - sets downsampling ratio for background and foreground layers\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_LAYERED_PARAM2_DOWNSAMPLEFG L" fgratio - sets further foreground downsampling ratio (ratio*fgratio)\n" 
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_PALETTIZED L" - create palettized document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM1_PALCOLORS L" - number of colors between 2 and 256 (defaults to 8)\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_QUANT L" - use quantization for palettized document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_PALETTIZED_PARAM2_NOTESHRINK L" - use noteshrink for palettized document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETITLEAUTO L" - use file name as page title\n"
			L"\t\t\t" DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME L" - use short file name as page title (when using previous)\n"
			L"\t\t\t" DEPRESS_ARG_TEMP L" tempdir - use tempdir as directory for temporary files\n"
			L"\t\t\t" DEPRESS_ARG_QUALITY L" percents - sets image quality in percents\n"
			L"\t\t\t\t100 is lossless for BW and good for PHOTO\n"
			L"\t\t\t" DEPRESS_ARG_DPI L" - DPI parameter (default to 100)\n\n"
		);

		return 0;
	}

	time_start = clock();

	if(wcslen(*(argsp + 1)) > 32767) {
		wprintf(L"Error: output file name is too long\n");

		return 0;
	}

	// Searching for files list with picture names
	text_list_fn_length = depressGetFilenameToOpen(0, *argsp, L".txt", 32768, text_list_filename, &text_list_name_start);
	if(!text_list_fn_length) {
		wprintf(L"Can't find files list\n");

		return 0;
	}
	if(text_list_fn_length > 32768) {
		wprintf(L"Error: path for files list is too long\n");

		return 0;
	}
	depressGetFilenamePath(text_list_filename, text_list_name_start, text_list_path);

#if defined(_WIN32)
	// Enabling safe search mode
	SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
#endif

	// Initialize document
	if(!depressDocumentInitDjvu(&document, document_flags, *(argsp + 1)))
		return 0;

	// Creating task list from file
	wprintf(L"Opening list: \"%ls\"\n", text_list_filename);

	if(!depressDocumentCreateTasksFromTextFile(&document, text_list_filename, text_list_path, flags)) {
		wprintf(L"Can't create files list\n");

		depressDocumentDestroy(&document);

		return 0;
	}
	
	success = depressDocumentRunTasks(&document);

	// Creating djvu
	if(success) {
		int process_status;

		process_status = depressDocumentProcessTasks(&document);

		wprintf(L"%ls\n", depressGetDocumentProcessStatus(process_status));

		if(process_status != DEPRESS_DOCUMENT_PROCESS_STATUS_OK)
			success = false;
	}

	if(success) {
		if(!depressDocumentFinalize(&document))
			wprintf(L"%ls", L"Warning: can't perform final steps\n");
	}

	depressDocumentDestroy(&document);

	if(!success) {
		wprintf(L"Can't create djvu file\n");
		if(!_waccess(*(argsp + 1), 06))
			_wremove(*(argsp + 1));
	} else
		wprintf(L"Converted in %f s\n", (float)(clock()-time_start)/CLOCKS_PER_SEC);

#if defined(_DEBUG) && defined(USE_STB_LEAKCHECK)
	stb_leakcheck_dumpmem();
#endif

	return 0;
}

