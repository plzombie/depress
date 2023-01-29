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

#ifndef DEPRESS_TASKS_H
#define DEPRESS_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <wchar.h>

#include "depress_flags.h"
#include "depress_threads.h"

typedef struct {
	wchar_t inputfile[32768];
	wchar_t tempfile[32768];
	wchar_t outputfile[32768];
	depress_event_handle_t finished;
	depress_flags_type flags;
	bool is_error;
	bool is_completed;
} depress_task_type;

typedef struct {
	depress_task_type *tasks;
	depress_djvulibre_paths_type *djvulibre_paths;
	size_t tasks_num;
	uintptr_t *tasks_next_to_process;
	int thread_id;
	int threads_num;
	depress_event_handle_t global_error_event;
} depress_thread_task_arg_type;

extern bool depressAddTask(const depress_task_type *task, depress_task_type **tasks_out, size_t *tasks_num_out, size_t *tasks_max_out);
extern void depressDestroyTasks(depress_task_type *tasks, size_t tasks_num);

#if defined(_WIN32)
extern unsigned int __stdcall depressThreadTaskProc(void *args);
#else
extern void *depressThreadTaskProc(void *args);
#endif

#ifdef __cplusplus
}
#endif

#endif
