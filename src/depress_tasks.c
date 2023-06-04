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
#include "third_party/stb_leakcheck.h"
#endif

#include "../include/depress_tasks.h"
#include "../include/depress_converter.h"

#include "../include/interlocked_ptr.h"

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
	tasks[tasks_num].process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_OK;
	tasks[tasks_num].is_completed = false;

	tasks[tasks_num].finished = depressCreateEvent();
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

	for(i = 0; i < tasks_num; i++) {
		tasks[i].load_image.free_ctx(tasks[i].load_image_ctx, i);
		depressFreePageFlags(&tasks[i].flags);

		depressCloseEventHandle(tasks[i].finished);
	}

	free(tasks);
}


void depressSetDefaultPageFlags(depress_flags_type *flags)
{
	memset(flags, 0, sizeof(depress_flags_type));
	flags->type = DEPRESS_PAGE_TYPE_COLOR;
	flags->quality = 100;
	flags->dpi = 100;
}

void depressFreePageFlags(depress_flags_type *flags)
{
	if(!flags->keep_data) {
		if(flags->page_title) free(flags->page_title);
		if(flags->nof_illrects) free(flags->illrects);
	}

	memset(flags, 0, sizeof(depress_flags_type));
}

bool depressCopyPageFlags(depress_flags_type *dst, depress_flags_type *src)
{
	memcpy(dst, src, sizeof(depress_flags_type));

	dst->page_title = 0;
	dst->illrects = 0;
	dst->nof_illrects = 0;
	if(src->page_title) {
		dst->page_title = malloc((wcslen(src->page_title)+1)*sizeof(wchar_t));
		if(!dst->page_title) goto FAILURE;
		wcscpy(dst->page_title, src->page_title);
	}
	if(src->nof_illrects) {
		dst->illrects = malloc(src->nof_illrects*sizeof(depress_illustration_rect_type));
		if(!dst->illrects) goto FAILURE;
		memcpy(dst->illrects, src->illrects, src->nof_illrects*sizeof(depress_illustration_rect_type));
		dst->nof_illrects = src->nof_illrects;
	}

	return true;

FAILURE:
	if(dst->page_title) free(dst->page_title);
	if(dst->illrects) free(dst->illrects);

	return false;
}


#if defined(_WIN32)
unsigned int __stdcall depressThreadTaskProc(void *args)
#else
void *depressThreadTaskProc(void *args)
#endif
{
	size_t i;
	depress_thread_task_arg_type arg;
	bool global_error = false;

	arg = *((depress_thread_task_arg_type *)args);

	//for(i = arg.thread_id; i < arg.tasks_num; i += arg.threads_num) {
	while((i = InterlockedExchangeAddPtr(arg.tasks_next_to_process, 1)) < arg.tasks_num) {
		if(global_error == false)
			if(depressWaitForEvent(arg.global_error_event, 0))
				global_error = true;

		if(global_error == false) {
			int convert_status;

			convert_status = arg.maker.convert_ctx(arg.maker_ctx, i, arg.tasks[i].flags, arg.tasks[i].load_image, arg.tasks[i].load_image_ctx);

			switch(convert_status) {
				case DEPRESS_CONVERT_PAGE_STATUS_OK:
					break;
				case DEPRESS_CONVERT_PAGE_STATUS_GENERIC_ERROR:
				default:
					arg.tasks[i].process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_GENERIC_ERROR;
					break;
				case DEPRESS_CONVERT_PAGE_STATUS_CANT_ALLOC_MEMORY:
					arg.tasks[i].process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_ALLOC_MEMORY;
					break;
				case DEPRESS_CONVERT_PAGE_STATUS_CANT_OPEN_IMAGE:
					arg.tasks[i].process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_OPEN_IMAGE;
					break;
				case DEPRESS_CONVERT_PAGE_STATUS_CANT_SAVE_PAGE:
					arg.tasks[i].process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_SAVE_PAGE;
					break;
			}

			if(arg.tasks[i].process_status != DEPRESS_DOCUMENT_PROCESS_STATUS_OK)
				depressSetEvent(arg.global_error_event);

			arg.tasks[i].is_completed = true;
		}

		depressSetEvent(arg.tasks[i].finished);
	}

	return 0;
}
