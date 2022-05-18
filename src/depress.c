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

#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#include <process.h>
#include <wchar.h>
#include <time.h>

#include <sys/types.h>
#include <fcntl.h>
#include <io.h>

#include "../include/depress_document.h"

#define DEPRESS_ARG_PAGETYPE_BW L"-bw"
#define DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF L"-errdiff"
#define DEPRESS_ARG_PAGETITLEAUTO L"-pta"
#define DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME L"-shortfntitle"

int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	wchar_t text_list_filename[32768], text_list_path[32768], *text_list_name_start;
	size_t text_list_path_size;
	depress_flags_type flags;
	DWORD text_list_fn_length;
	depress_document_type document;
	depress_document_final_flags_type final_flags;
	bool success = true;
	clock_t time_start;

	memset(&flags, 0, sizeof(depress_flags_type));
	flags.type = DEPRESS_PAGE_TYPE_COLOR;

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
			if (flags.type == DEPRESS_PAGE_TYPE_BW)
				flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_ERRDIFF;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_BW_PARAM1_ERRDIFF, DEPRESS_ARG_PAGETYPE_BW);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO)) {
			final_flags.page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME)) {
			final_flags.page_title_type_flags |= DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME;
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
			L"\t\t\t" DEPRESS_ARG_PAGETITLEAUTO L" - use file name as page title\n"
			L"\t\t\t" DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME L" - use short file name as page title (when using previous)\n\n"
		);

		return 0;
	}

	time_start = clock();

	if(wcslen(*(argsp + 1)) > 32767) {
		wprintf(L"Error: output file name is too long\n");

		return 0;
	}

	// Searching for files list with picture names
	text_list_fn_length = SearchPathW(NULL, *argsp, L".txt", 32768, text_list_filename, &text_list_name_start);
	if(!text_list_fn_length) {
		wprintf(L"Can't find files list\n");

		return 0;
	}
	if(text_list_fn_length > 32768) {
		wprintf(L"Error: path for files list is too long\n");

		return 0;
	}
	text_list_path_size = text_list_name_start - text_list_filename;
	wcsncpy(text_list_path, text_list_filename, text_list_path_size);
	text_list_path[text_list_path_size] = 0;

	// Enabling safe search mode
	SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);

	// Initialize document
	if(!depressDocumentInit(&document))
		return 0;

	depressDocumentSetFinalFlags(&document, final_flags);

	// Creating task list from file
	wprintf(L"Opening list: \"%ls\"\n", text_list_filename);

	if(!depressDocumentCreateTasksFromTextFile(&document, text_list_filename, text_list_path, *(argsp + 1), document.temp_path, flags)) {
		wprintf(L"Can't create files list\n");

		depressDocumentDestroy(&document);

		return 0;
	}
	
	success = depressDocumentRunTasks(&document);

	// Creating djvu
	if(success)
		success = depressDocumentProcessTasks(&document);

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

	return 0;
}

