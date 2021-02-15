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

#include "../include/ppm_save.h"

#include <Windows.h>

#include <stdlib.h>

#include <locale.h>

#include <process.h>
#include <wchar.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

typedef struct {
	wchar_t cjb2_path[32768];
	wchar_t c44_path[32768];
	wchar_t djvm_path[32768];
} depress_djvulibre_paths_type;

typedef struct {
	bool bw;
} depress_flags_type;

typedef struct {
	wchar_t inputfile[32768];
	wchar_t tempfile[32768];
	wchar_t outputfile[32768];
	bool is_error;
	bool is_completed;
} depress_task_type;

typedef struct {
	depress_task_type *tasks;
	depress_djvulibre_paths_type *djvulibre_paths;
	depress_flags_type flags;
	size_t tasks_num;
	int thread_id;
	int threads_num;
} depress_thread_arg_type;

bool depressConvertPage(bool is_bw, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths);
bool depressCreateTasks(wchar_t *textfile, wchar_t *textfilepath, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out);
int depressGetNumberOfThreads(void);
bool depressGetDjvulibrePaths(depress_djvulibre_paths_type *djvulibre_paths);
unsigned int __stdcall depressThreadProc(void *args);

int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	wchar_t text_list_filename[32768], text_list_path[32768], *text_list_name_start;
	size_t text_list_path_size;
	wchar_t arg1[32770], arg2[32770];
	depress_flags_type flags;
	size_t filecount = 0;
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;
	DWORD text_list_fn_length;
	int threads_num = 0;
	HANDLE *threads;
	depress_djvulibre_paths_type djvulibre_paths;
	depress_thread_arg_type *thread_args;
	bool is_error = false;
	int i;

	flags.bw = false;

	setlocale(LC_ALL, "");

	// Reading options
	argsc = argc - 1;
	argsp = argv + 1;
	while(argsc > 0) {
		if((*argsp)[0] != '-')
			break;

		argsc--;

		if(!wcscmp(*argsp, L"-bw"))
			flags.bw = true;
		else
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

	if(!depressCreateTasks(text_list_filename, text_list_path, *(argsp + 1), &tasks, &tasks_num)) {
		wprintf(L"Can't create files list\n");

		return 0;
	}

	threads_num = depressGetNumberOfThreads();
	if(threads_num > 64) threads_num = 64;

	threads = malloc(sizeof(HANDLE) * threads_num);
	thread_args = malloc(sizeof(depress_thread_arg_type) * threads_num);
	if(!threads || !thread_args) {
		if(threads) free(threads);
		if(thread_args) free(thread_args);

		wprintf(L"Can't allocate memory\n");

		return 0;
	}

	for(i = 0; i < threads_num; i++) {
		thread_args[i].tasks = tasks;
		thread_args[i].djvulibre_paths = &djvulibre_paths;
		thread_args[i].flags = flags;
		thread_args[i].tasks_num = tasks_num;
		thread_args[i].thread_id = i;
		thread_args[i].threads_num = threads_num;

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

	if(!is_error) {
		WaitForMultipleObjects(threads_num, threads, TRUE, INFINITE);

		for(i = 0; i < threads_num; i++)
			CloseHandle(threads[i]);
	}

	free(threads);
	free(thread_args);

	// Creating djvu
	swprintf(arg1, 32770, L"\"%ls\"", *(argsp + 1));

	for(filecount = 0; filecount < tasks_num; filecount++) {
		if(!tasks[filecount].is_completed)
			continue;

		if(tasks[filecount].is_error) {
			wprintf(L"Error while converting file \"%ls\"\n", tasks[filecount].inputfile);
			is_error = true;
		} else {
			if(is_error == false && filecount > 0) {
				wprintf(L"Merging file \"%ls\"\n", tasks[filecount].inputfile);

				swprintf(arg2, 32770, L"\"%ls\"", tasks[filecount].outputfile);

				if(_wspawnl(_P_WAIT, djvulibre_paths.djvm_path, djvulibre_paths.djvm_path, L"-i", arg1, arg2, 0)) {
					wprintf(L"Can't merge djvu files\n");
					is_error = true;
				}
			}
			if(filecount > 0)
				if(!_waccess(tasks[filecount].outputfile, 06))
					_wremove(tasks[filecount].outputfile);
		}
	}

	if(is_error) {
		wprintf(L"Can't create djvu file\n");
		if(!_waccess(*(argsp + 1), 06))
			_wremove(*(argsp + 1));
	}

	if(tasks)
		free(tasks);

	return 0;
}

unsigned int __stdcall depressThreadProc(void *args)
{
	size_t i;
	depress_thread_arg_type arg;

	arg = *((depress_thread_arg_type *)args);

	for(i = arg.thread_id; i < arg.tasks_num; i += arg.threads_num) {
		if(!i) {
			if(!depressConvertPage(arg.flags.bw, arg.tasks[i].inputfile, arg.tasks[i].tempfile, arg.tasks[i].outputfile, arg.djvulibre_paths)) {
				arg.tasks[i].is_error = true;
				arg.tasks[i].is_completed = true;
				break;
			}
		} else {
			if(!depressConvertPage(arg.flags.bw, arg.tasks[i].inputfile, arg.tasks[i].tempfile, arg.tasks[i].outputfile, arg.djvulibre_paths)) {
				arg.tasks[i].is_error = true;
				arg.tasks[i].is_completed = true;
				break;
			}
		}
		arg.tasks[i].is_completed = true;
	}

	return 0;
}

bool depressCreateTasks(wchar_t *textfile, wchar_t *textfilepath, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out)
{
	FILE *f;
	size_t task_inputfile_length;
	wchar_t inputfile[32770];
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;

	*tasks_out = 0;
	*tasks_num_out = 0;

	f = _wfopen(textfile, L"r");
	if(!f) return false;

	while(1) {
		wchar_t *eol;

		if(tasks_max == 0) {
			tasks = malloc(sizeof(depress_task_type) * 2);
			if(tasks) {
				tasks_max = 2;
			}
			else {
				return false;
			}
		}
		else if(tasks_num == tasks_max) {
			depress_task_type *_tasks;
			size_t _tasks_max;

			_tasks_max = tasks_max * 2;
			_tasks = realloc(tasks, _tasks_max * sizeof(depress_task_type));
			if(!_tasks) {
				free(tasks);

				return false;
			}
			tasks = _tasks;
			tasks_max = _tasks_max;
		}

		// Reading line
		if(!fgetws(inputfile, 32770, f)) {
			if(feof(f))
				break;
			else {
				if(tasks) free(tasks);

				return false;
			}
		}
		if(wcslen(inputfile) == 32769) {
			free(tasks);

			return false;
		}

		eol = wcsrchr(inputfile, '\n');
		if(eol) *eol = 0;

		if(*inputfile == 0)
			continue;

		// Adding textfile path to inputfile
		task_inputfile_length = SearchPathW(textfilepath, inputfile, NULL, 32768, tasks[tasks_num].inputfile, NULL);
		if(task_inputfile_length > 32768 || task_inputfile_length == 0) {
			free(tasks);

			return false;
		}

		// Filling task
		swprintf(tasks[tasks_num].tempfile, 32768, L"temp%lld.ppm", (long long)tasks_num);
		if(tasks_num == 0)
			wcscpy(tasks[tasks_num].outputfile, outputfile);
		else
			swprintf(tasks[tasks_num].outputfile, 32768, L"temp%lld.djvu", (long long)tasks_num);

		tasks[tasks_num].is_error = false;
		tasks[tasks_num].is_completed = false;

		tasks_num++;
	}

	if(tasks_num == 0 && tasks_max > 0) {
		free(tasks);
		tasks = 0;
	}

	*tasks_out = tasks;
	*tasks_num_out = tasks_num;

	fclose(f);

	return true;
}

bool depressConvertPage(bool is_bw, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile, depress_djvulibre_paths_type *djvulibre_paths)
{
	FILE *f_in = 0, *f_temp = 0;
	int sizex, sizey, channels;
	wchar_t arg1[32770], arg2[32770];
	unsigned char *buffer = 0;
	bool result = false;

	f_in = _wfopen(inputfile, L"rb");
	f_temp = _wfopen(tempfile, L"wb");
	if(!f_in || !f_temp)
		goto EXIT;

	buffer = stbi_load_from_file(f_in, &sizex, &sizey, &channels, is_bw?1:0);
	if(!buffer)
		goto EXIT;

	fclose(f_in); f_in = 0;

	if(is_bw) {
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

	if(is_bw) {
		if(_wspawnl(_P_WAIT, djvulibre_paths->cjb2_path, djvulibre_paths->cjb2_path, arg1, arg2, 0)) {
			goto EXIT;
		}
	} else {
		if(_wspawnl(_P_WAIT, djvulibre_paths->c44_path, djvulibre_paths->c44_path, arg1, arg2, 0)) {
			goto EXIT;
		}
	}

	result = true;

EXIT:
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

int depressGetNumberOfThreads(void)
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

bool depressGetDjvulibrePaths(depress_djvulibre_paths_type *djvulibre_paths)
{
	DWORD filename_len;

	filename_len = SearchPathW(NULL, L"c44.exe", NULL, 32768, djvulibre_paths->c44_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"cjb2.exe", NULL, 32768, djvulibre_paths->cjb2_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvm.exe", NULL, 32768, djvulibre_paths->djvm_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	return true;
}

