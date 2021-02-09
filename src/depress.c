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
} depress_task_type;

bool depressConvertPage(bool is_bw, wchar_t *inputfile, wchar_t *tempfile, wchar_t *outputfile);
bool depressCreateTasks(wchar_t *textfile, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out);


int wmain(int argc, wchar_t **argv)
{
	wchar_t **argsp;
	int argsc;
	depress_flags_type flags;
	size_t filecount = 0;
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;;

	flags.bw = false;

	setlocale(LC_ALL, "");

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

	wprintf(L"Opening list: \"%ls\"\n", *argsp);

	if(!depressCreateTasks(*argsp, *(argsp + 1), &tasks, &tasks_num)) {
		wprintf(L"Can't create files list\n");

		return 0;
	}

	for(filecount = 0; filecount < tasks_num; filecount++) {
		wprintf(L"Processing file \"%ls\"\n", tasks[filecount].inputfile);

		if(!filecount) {
			if(!depressConvertPage(flags.bw, tasks[filecount].inputfile, tasks[filecount].tempfile, tasks[filecount].outputfile)) {
				wprintf(L"Can't save djvu\n");
				break;
			}
		}
		else {
			if(!depressConvertPage(flags.bw, tasks[filecount].inputfile, tasks[filecount].tempfile, tasks[filecount].outputfile)) {
				wprintf(L"Can't save djvu\n");
				break;
			}
			if(_wspawnl(_P_WAIT, L"djvm.exe", L"djvu.exe", L"-i", *(argsp + 1), tasks[filecount].outputfile, 0)) {
				wprintf(L"Can't merge djvu files\n");
				_wremove(tasks[filecount].outputfile);
				break;
			}
			_wremove(tasks[filecount].outputfile);
		}
	}

	if(tasks)
		free(tasks);

	return 0;
}

bool depressCreateTasks(wchar_t *textfile, wchar_t *outputfile, depress_task_type **tasks_out, size_t *tasks_num_out)
{
	FILE *f;
	wchar_t filename[32770];
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;;

	*tasks_out = 0;
	*tasks_num_out = 0;

	f = _wfopen(textfile, L"r");
	if(!f) return false;

	while(1) {
		wchar_t *eol;

		if(!fgetws(filename, 32770, f)) {
			if(feof(f))
				break;
			else {
				if(tasks) free(tasks);

				return false;
			}
		}
		if(wcslen(filename) == 32769) {
			if(tasks) free(tasks);

			return false;
		}

		eol = wcsrchr(filename, '\n');
		if(eol) *eol = 0;

		if(*filename == 0)
			continue;

		if(tasks_max == 0) {
			tasks = malloc(sizeof(depress_task_type) * 2);
			if(tasks) {
				tasks_max = 2;
			} else {
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

		wcscpy(tasks[tasks_num].inputfile, filename);
		swprintf(tasks[tasks_num].tempfile, 32770, L"temp%lld.ppm", (long long)tasks_num);
		if(tasks_num == 0)
			wcscpy(tasks[tasks_num].outputfile, outputfile);
		else
			swprintf(tasks[tasks_num].outputfile, 32770, L"temp%lld.djvu", (long long)tasks_num);

		tasks_num++;
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

	_wremove(tempfile);

	return result;

}

