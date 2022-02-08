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
#define DEPRESS_ARG_PAGETYPE_BW_PARAM1_DITHERING L"-dith"
#define DEPRESS_ARG_PAGETITLEAUTO L"-pta"
#define DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME L"-shortfntitle"

int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	wchar_t text_list_filename[32768], text_list_path[32768], *text_list_name_start;
	size_t text_list_path_size;
	wchar_t temp_path[32768];
	wchar_t arg0[32770], arg1[32770], arg2[32770];
	depress_flags_type flags;
	size_t filecount = 0;
	DWORD text_list_fn_length;
	HANDLE global_error_event;
	depress_document_type document;
	bool is_error = false;
	int i;
	clock_t time_start;

	memset(&flags, 0, sizeof(depress_flags_type));
	flags.type = DEPRESS_PAGE_TYPE_COLOR;

	memset(&document, 0, sizeof(depress_document_type));
	document.page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO;

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
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETYPE_BW_PARAM1_DITHERING)) {
			if (flags.type == DEPRESS_PAGE_TYPE_BW)
				flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_DITHERING;
			else
				wprintf(L"Warning: argument %ls can be set only with %ls\n", DEPRESS_ARG_PAGETYPE_BW_PARAM1_DITHERING, DEPRESS_ARG_PAGETYPE_BW);
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO)) {
			document.page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC;
		} else if(!wcscmp(*argsp, DEPRESS_ARG_PAGETITLEAUTO_SHORTNAME)) {
			document.page_title_type_flags |= DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME;
		} else
			wprintf(L"Warning: unknown argument %ls\n", *argsp);

		argsp++;
	}

	if(argsc < 2) {
		wprintf(
			L"\tdepress [options] input.txt output.djvu\n"
			L"\t\toptions:\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_BW L" - create black and white document\n"
			L"\t\t\t" DEPRESS_ARG_PAGETYPE_BW_PARAM1_DITHERING L" - use dithering for bw document\n"
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

	// Get paths to djvulibre files
	if(!depressGetDjvulibrePaths(&document.djvulibre_paths)) {
		wprintf(L"Can't find djvulibre files\n");

		return 0;
	}

	// Creating task list from file
	wprintf(L"Opening list: \"%ls\"\n", text_list_filename);

	if(!depressGetTempFolder(temp_path)) {
		wprintf(L"Can't get path for temporary files\n");

		return 0;
	}

	if(!depressCreateTasks(text_list_filename, text_list_path, *(argsp + 1), temp_path, flags, &document.tasks, &document.tasks_num)) {
		wprintf(L"Can't create files list\n");

		return 0;
	}

	document.threads_num = depressGetNumberOfThreads();
	if(document.threads_num <= 0) document.threads_num = 1;
	if(document.threads_num > 64) document.threads_num = 64;

	document.threads = malloc(sizeof(HANDLE) * document.threads_num);
	document.thread_args = malloc(sizeof(depress_thread_arg_type) * document.threads_num);
	if(!document.threads || !document.thread_args) {
		if(document.threads) free(document.threads);
		if(document.thread_args) free(document.thread_args);
		depressDestroyTasks(document.tasks, document.tasks_num);
		depressDestroyTempFolder(temp_path);

		wprintf(L"Can't allocate memory\n");

		return 0;
	}

	global_error_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if(global_error_event == NULL) {
		free(document.threads);
		free(document.thread_args);
		depressDestroyTasks(document.tasks, document.tasks_num);
		depressDestroyTempFolder(temp_path);

		wprintf(L"Can't create event\n");

		return 0;
	}

	for(i = 0; i < document.threads_num; i++) {
		document.thread_args[i].tasks = document.tasks;
		document.thread_args[i].djvulibre_paths = &document.djvulibre_paths;
		document.thread_args[i].tasks_num = document.tasks_num;
		document.thread_args[i].thread_id = i;
		document.thread_args[i].threads_num = document.threads_num;
		document.thread_args[i].global_error_event = global_error_event;

		document.threads[i] = (HANDLE)_beginthreadex(0, 0, depressThreadProc, document.thread_args + i, 0, 0);
		if(!document.threads[i]) {
			int j;

			WaitForMultipleObjects(i, document.threads, TRUE, INFINITE);

			for(j = 0; j < i; j++)
				CloseHandle(document.threads[j]);

			is_error = true;

			break;
		}
	}

	// Creating djvu
	if(!is_error) {
		swprintf(arg1, 32770, L"\"%ls\"", *(argsp + 1));

		for(filecount = 0; filecount < document.tasks_num; filecount++) {
			if(!is_error)
				if(WaitForSingleObject(global_error_event, 0) == WAIT_OBJECT_0)
					is_error = true;

			while(WaitForSingleObject(document.tasks[filecount].finished, INFINITE) != WAIT_OBJECT_0);

			if(!document.tasks[filecount].is_completed)
				continue;

			if(document.tasks[filecount].is_error) {
				wprintf(L"Error while converting file \"%ls\"\n", document.tasks[filecount].inputfile);
				is_error = true;
			} else {
				if(is_error == false && filecount > 0) {
					wprintf(L"Merging file \"%ls\"\n", document.tasks[filecount].inputfile);

					swprintf(arg2, 32770, L"\"%ls\"", document.tasks[filecount].outputfile);
					swprintf(arg0, 32770, L"\"%ls\"", document.djvulibre_paths.djvm_path);

					if(_wspawnl(_P_WAIT, document.djvulibre_paths.djvm_path, arg0, L"-i", arg1, arg2, 0)) {
						wprintf(L"Can't merge djvu files\n");
						is_error = true;
					}
				}
				if(filecount > 0)
					if(!_waccess(document.tasks[filecount].outputfile, 06))
						_wremove(document.tasks[filecount].outputfile);
			}
		}
	}

	if(!is_error) {
		WaitForMultipleObjects(document.threads_num, document.threads, TRUE, INFINITE);

		for(i = 0; i < document.threads_num; i++)
			CloseHandle(document.threads[i]);
	}

	if(!is_error) {
		if(!depressDocumentFinalize(&document, arg1))
			wprintf(L"%ls", L"Warning: can't perform final steps\n");
	}

	depressDestroyTempFolder(temp_path);

	free(document.threads);
	free(document.thread_args);

	CloseHandle(global_error_event);

	if(is_error) {
		wprintf(L"Can't create djvu file\n");
		if(!_waccess(*(argsp + 1), 06))
			_wremove(*(argsp + 1));
	} else
		wprintf(L"Converted in %f s\n", (float)(clock()-time_start)/CLOCKS_PER_SEC);

	depressDestroyTasks(document.tasks, document.tasks_num);

	return 0;
}

