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

#include "../include/depress_tasks.h"

#include <stdio.h>
#include <stdlib.h>

bool depressAddTask(const depress_task_type *task, depress_task_type **tasks_out, size_t *tasks_num_out, size_t *tasks_max_out)
{
	depress_task_type *tasks = 0;
	size_t tasks_num, tasks_max = 0;
	bool success = true;

	tasks = *tasks_out;
	tasks_num = *tasks_num_out;
	tasks_max = *tasks_max_out;

	if(tasks_max == 0) {
		tasks = malloc(sizeof(depress_task_type) * 2);
		if(tasks) {
			tasks_max = 2;
		}
		else {
			return false;
		}
	} else if(tasks_num == tasks_max) {
		depress_task_type *_tasks;
		size_t _tasks_max;

		_tasks_max = tasks_max * 2;
		_tasks = realloc(tasks, _tasks_max * sizeof(depress_task_type));
		if(!_tasks) {
			return false;
		}
		tasks = _tasks;
		tasks_max = _tasks_max;
	}


	tasks[tasks_num] = *task;
	tasks[tasks_num].is_error = false;
	tasks[tasks_num].is_completed = false;

	tasks[tasks_num].finished = CreateEventW(NULL, TRUE, FALSE, NULL);
	if(!tasks[tasks_num].finished)
		success = false;

	if(success)
		tasks_num++;

	*tasks_out = tasks;
	*tasks_num_out = tasks_num;
	*tasks_max_out = tasks_max;

	if(success)
		return true;
	else
		return false;
}

void depressDestroyTasks(depress_task_type *tasks, size_t tasks_num)
{
	size_t i;

	for(i = 0; i < tasks_num; i++)
		CloseHandle(tasks[i].finished);

	free(tasks);
}
