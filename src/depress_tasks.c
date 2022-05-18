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

#include "../include/depress_tasks.h"

#include <stdio.h>
#include <stdlib.h>

bool depressCreateTasks(wchar_t *textfile, wchar_t *textfilepath, wchar_t *outputfile, wchar_t *temppath, depress_flags_type flags, depress_task_type **tasks_out, size_t *tasks_num_out, size_t *tasks_max_out)
{
	FILE *f;
	size_t task_inputfile_length;
	wchar_t inputfile[32770];
	wchar_t tempstr[32];
	depress_task_type *tasks = 0;
	size_t tasks_num = 0, tasks_max = 0;

	*tasks_out = 0;
	*tasks_num_out = 0;

#ifdef _MSC_VER
	f = _wfopen(textfile, L"rt, ccs=UTF-8");
#else
	f = _wfopen(textfile, L"rt");
#endif
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
				size_t i;

				for(i = 0; i < tasks_num; i++)
					CloseHandle(tasks[i].finished);
					
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
				size_t i;

				for(i = 0; i < tasks_num; i++)
					CloseHandle(tasks[i].finished);
						
				free(tasks);

				return false;
			}
		}
		if(wcslen(inputfile) == 32769) {
			size_t i;

			for(i = 0; i < tasks_num; i++)
				CloseHandle(tasks[i].finished);
				
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
			size_t i;

			for(i = 0; i < tasks_num; i++)
				CloseHandle(tasks[i].finished);
				
			free(tasks);

			return false;
		}

		// Filling task

		swprintf(tempstr, 32, L"\\temp%lld.ppm", (long long)tasks_num);
		wcscpy(tasks[tasks_num].tempfile, temppath);
		wcscat(tasks[tasks_num].tempfile, tempstr);
		if(tasks_num == 0)
			wcscpy(tasks[tasks_num].outputfile, outputfile);
		else {
			swprintf(tempstr, 32, L"\\temp%lld.djvu", (long long)tasks_num);
			wcscpy(tasks[tasks_num].outputfile, temppath);
			wcscat(tasks[tasks_num].outputfile, tempstr);
		}

		tasks[tasks_num].flags = flags;
		tasks[tasks_num].is_error = false;
		tasks[tasks_num].is_completed = false;

		tasks[tasks_num].finished = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(!tasks[tasks_num].finished) {
			size_t i;

			for(i = 0; i < tasks_num; i++)
				CloseHandle(tasks[i].finished);

			free(tasks);

			return false;
		}

		tasks_num++;
	}

	if(tasks_num == 0 && tasks_max > 0) {
		free(tasks);
		tasks = 0;
	}

	*tasks_out = tasks;
	*tasks_num_out = tasks_num;
	*tasks_max_out = tasks_max;

	fclose(f);

	return true;
}

void depressDestroyTasks(depress_task_type *tasks, size_t tasks_num)
{
	size_t i;

	for(i = 0; i < tasks_num; i++)
		CloseHandle(tasks[i].finished);

	free(tasks);
}
