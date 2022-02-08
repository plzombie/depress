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

#include "../include/depress_paths.h"
#include "../include/depress_tasks.h"
#include "../include/depress_threads.h"

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
	depress_task_type *tasks = 0;
	size_t tasks_num = 0;
	DWORD text_list_fn_length;
	int threads_num = 0;
	HANDLE *threads;
	HANDLE global_error_event;
	depress_djvulibre_paths_type djvulibre_paths;
	depress_thread_arg_type *thread_args;
	bool is_error = false;
	int i;
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

		if(!wcscmp(*argsp, L"-bw")) {
			flags.type = DEPRESS_PAGE_TYPE_BW;
			flags.param1 = DEPRESS_PAGE_TYPE_BW_PARAM1_SIMPLE;
		} else
			wprintf(L"Warning: unknown argument %ls\n", *argsp);

		argsp++;
	}

	if(argsc < 2) {
		wprintf(
			L"\tdepress [options] input.txt output.djvu\n"
			L"\t\toptions:\n"
			L"\t\t\t-bw - create black and white document\n\n"
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
	if(!depressGetDjvulibrePaths(&djvulibre_paths)) {
		wprintf(L"Can't find djvulibre files\n");

		return 0;
	}

	// Creating task list from file
	wprintf(L"Opening list: \"%ls\"\n", text_list_filename);

	if(!depressGetTempFolder(temp_path)) {
		wprintf(L"Can't get path for temporary files\n");

		return 0;
	}

	if(!depressCreateTasks(text_list_filename, text_list_path, *(argsp + 1), temp_path, flags, &tasks, &tasks_num)) {
		wprintf(L"Can't create files list\n");

		return 0;
	}

	threads_num = depressGetNumberOfThreads();
	if(threads_num <= 0) threads_num = 1;
	if(threads_num > 64) threads_num = 64;

	threads = malloc(sizeof(HANDLE) * threads_num);
	thread_args = malloc(sizeof(depress_thread_arg_type) * threads_num);
	if(!threads || !thread_args) {
		if(threads) free(threads);
		if(thread_args) free(thread_args);
		depressDestroyTasks(tasks, tasks_num);
		depressDestroyTempFolder(temp_path);

		wprintf(L"Can't allocate memory\n");

		return 0;
	}

	global_error_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if(global_error_event == NULL) {
		free(threads);
		free(thread_args);
		depressDestroyTasks(tasks, tasks_num);
		depressDestroyTempFolder(temp_path);

		wprintf(L"Can't create event\n");

		return 0;
	}

	for(i = 0; i < threads_num; i++) {
		thread_args[i].tasks = tasks;
		thread_args[i].djvulibre_paths = &djvulibre_paths;
		thread_args[i].tasks_num = tasks_num;
		thread_args[i].thread_id = i;
		thread_args[i].threads_num = threads_num;
		thread_args[i].global_error_event = global_error_event;

		threads[i] = (HANDLE)_beginthreadex(0, 0, depressThreadProc, thread_args + i, 0, 0);
		if(!threads[i]) {
			int j;

			WaitForMultipleObjects(i, threads, TRUE, INFINITE);

			for(j = 0; j < i; j++)
				CloseHandle(threads[j]);

			is_error = true;

			break;
		}
	}

	// Creating djvu
	if(!is_error) {
		swprintf(arg1, 32770, L"\"%ls\"", *(argsp + 1));

		for(filecount = 0; filecount < tasks_num; filecount++) {
			if(!is_error)
				if(WaitForSingleObject(global_error_event, 0) == WAIT_OBJECT_0)
					is_error = true;

			while(WaitForSingleObject(tasks[filecount].finished, INFINITE) != WAIT_OBJECT_0);

			if(!tasks[filecount].is_completed)
				continue;

			if(tasks[filecount].is_error) {
				wprintf(L"Error while converting file \"%ls\"\n", tasks[filecount].inputfile);
				is_error = true;
			} else {
				if(is_error == false && filecount > 0) {
					wprintf(L"Merging file \"%ls\"\n", tasks[filecount].inputfile);

					swprintf(arg2, 32770, L"\"%ls\"", tasks[filecount].outputfile);
					swprintf(arg0, 32770, L"\"%ls\"", djvulibre_paths.djvm_path);

					if(_wspawnl(_P_WAIT, djvulibre_paths.djvm_path, arg0, L"-i", arg1, arg2, 0)) {
						wprintf(L"Can't merge djvu files\n");
						is_error = true;
					}
				}
				if(filecount > 0)
					if(!_waccess(tasks[filecount].outputfile, 06))
						_wremove(tasks[filecount].outputfile);
			}
		}
	}

	if(!is_error) {
		WaitForMultipleObjects(threads_num, threads, TRUE, INFINITE);

		for(i = 0; i < threads_num; i++)
			CloseHandle(threads[i]);
	}

	depressDestroyTempFolder(temp_path);

	free(threads);
	free(thread_args);

	CloseHandle(global_error_event);

	if(is_error) {
		wprintf(L"Can't create djvu file\n");
		if(!_waccess(*(argsp + 1), 06))
			_wremove(*(argsp + 1));
	} else
		wprintf(L"Converted in %f s\n", (float)(clock()-time_start)/CLOCKS_PER_SEC);

	depressDestroyTasks(tasks, tasks_num);

	return 0;
}

