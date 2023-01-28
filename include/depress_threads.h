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

#ifndef DEPRESS_THREADS_H
#define DEPRESS_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

#include <stdint.h>

#include "depress_tasks.h"
#include "depress_paths.h"

typedef struct {
	depress_task_type *tasks;
	depress_djvulibre_paths_type *djvulibre_paths;
	size_t tasks_num;
	size_t *tasks_next_to_process;
	int thread_id;
	int threads_num;
	HANDLE global_error_event;
} depress_thread_arg_type;

#if defined(_WIN32)
typedef WORD (__stdcall *GetActiveProcessorGroupCount_type)(void);
typedef DWORD (__stdcall *GetActiveProcessorCount_type)(WORD GroupNumber);
typedef BOOL(__stdcall *SetThreadGroupAffinity_type)(HANDLE hThread, const GROUP_AFFINITY *GroupAffinity, PGROUP_AFFINITY PreviousGroupAffinity);

extern GetActiveProcessorGroupCount_type GetActiveProcessorGroupCount_funcptr;
extern GetActiveProcessorCount_type GetActiveProcessorCount_funcptr;
extern SetThreadGroupAffinity_type SetThreadGroupAffinity_funcptr;

typedef HANDLE depress_process_handle_t;

#define DEPRESS_INVALID_PROCESS_HANDLE INVALID_HANDLE_VALUE
#else
typedef void *depress_process_handle_t;

#define DEPRESS_INVALID_PROCESS_HANDLE 0
#endif

extern depress_process_handle_t depressSpawn(wchar_t *filename, wchar_t *args, bool wait, bool close_handle);
extern void depressWaitForProcess(depress_process_handle_t handle);
extern void depressCloseProcessHandle(depress_process_handle_t handle);
extern void depressGetProcessGroupFunctions(void);
extern unsigned int depressGetNumberOfThreads(void);
#if defined(_WIN32)
extern KAFFINITY depressGetMaskForProcessorCount(DWORD processor_count);
extern unsigned int __stdcall depressThreadProc(void *args);
#else
extern void *depressThreadProc(void *args);
#endif

#ifdef __cplusplus
}
#endif

#endif
