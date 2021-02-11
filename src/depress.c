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
	bool bw;
} depress_flags_type;

typedef struct {
	wchar_t inputfile[32770];
	wchar_t tempfile[32770];
	wchar_t outputfile[32770];
	bool is_error;
} depress_task_type;

typedef struct {
	depress_task_type *tasks;
	depress_flags_type flags;
	size_t tasks_num;
	int thread_id;
	int threads_num;
} depress_thread_arg_type;

bool depressConvertPage(bool is_bw, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile);
bool depressCreateTasks(wchar_t *textfile, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out);
int depressGetNumberOfThreads(void);
unsigned int __stdcall depressThreadProc(void *args);

int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	depress_flags_type flags;
	size_t filecount = 0;
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;
	int threads_num = 0;
	HANDLE *threads;
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

	// Creating task list from file
	wprintf(L"Opening list: \"%ls\"\n", *argsp);

	if(!depressCreateTasks(*argsp, *(argsp + 1), &tasks, &tasks_num)) {
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
	for(filecount = 0; filecount < tasks_num; filecount++) {
		if(tasks[filecount].is_error) {
			wprintf(L"Error while converting file \"%ls\"\n", tasks[filecount].inputfile);
			is_error = true;
		} else {
			if(is_error == false && filecount > 0) {
				wprintf(L"Merging file \"%ls\"\n", tasks[filecount].inputfile);

				if(_wspawnl(_P_WAIT, L"djvm.exe", L"djvu.exe", L"-i", *(argsp + 1), tasks[filecount].outputfile, 0)) {
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
			if(!depressConvertPage(arg.flags.bw, arg.tasks[i].inputfile, arg.tasks[i].tempfile, arg.tasks[i].outputfile)) {
				arg.tasks[i].is_error = true;
				break;
			}
		} else {
			if(!depressConvertPage(arg.flags.bw, arg.tasks[i].inputfile, arg.tasks[i].tempfile, arg.tasks[i].outputfile)) {
				arg.tasks[i].is_error = true;
				break;
			}
		}
	}

	return 0;
}

bool depressCreateTasks(wchar_t *textfile, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out)
{
	FILE *f;
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;;

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
		if(!fgetws(tasks[tasks_num].inputfile, 32770, f)) {
			if(feof(f))
				break;
			else {
				if(tasks) free(tasks);

				return false;
			}
		}
		if(wcslen(tasks[tasks_num].inputfile) == 32769) {
			if(tasks) free(tasks);

			return false;
		}

		eol = wcsrchr(tasks[tasks_num].inputfile, '\n');
		if(eol) *eol = 0;

		if(*tasks[tasks_num].inputfile == 0)
			continue;

		// Filling task
		swprintf(tasks[tasks_num].tempfile, 32770, L"temp%lld.ppm", (long long)tasks_num);
		if(tasks_num == 0)
			wcscpy(tasks[tasks_num].outputfile, outputfile);
		else
			swprintf(tasks[tasks_num].outputfile, 32770, L"temp%lld.djvu", (long long)tasks_num);

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

bool depressConvertPage(bool is_bw, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile)
{
	FILE *f_in = 0, *f_temp = 0;
	int sizex, sizey, channels;
	unsigned char *buffer = 0;
	bool result = false;

	f_in = _wfopen(inputfile, L"rb");
	f_temp = _wfopen(tempfile, L"wb");
	if(!f_in || !f_temp)
		goto EXIT;

	if(is_bw)
		buffer = stbi_load_from_file(f_in, &sizex, &sizey, &channels, 1);
	else
		buffer = stbi_load_from_file(f_in, &sizex, &sizey, &channels, 0);
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

	if(is_bw) {
		if(_wspawnl(_P_WAIT, L"cjb2.exe", L"cjb2.exe", tempfile, outputfile, 0)) {
			goto EXIT;
		}
	} else {
		if(_wspawnl(_P_WAIT, L"c44.exe", L"c44.exe", tempfile, outputfile, 0)) {
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
	return 16;
}

