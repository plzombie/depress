/*
BSD 2-Clause License

Copyright (c) 2021-2024, Mikhail Morozov
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

#include "../include/depress_threads.h"

#include <stdlib.h>

#if defined(_WIN32)
GetActiveProcessorGroupCount_type GetActiveProcessorGroupCount_funcptr = 0;
GetActiveProcessorCount_type GetActiveProcessorCount_funcptr = 0;
SetThreadGroupAffinity_type SetThreadGroupAffinity_funcptr;

#include <process.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

depress_process_handle_t depressSpawn(wchar_t *filename, wchar_t *args, bool wait, bool close_handle)
{
#if defined(_WIN32)
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	
	if(!filename && !args) return INVALID_HANDLE_VALUE;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if(!CreateProcessW(
		filename,
		args,
		NULL, // SECURITY_ATTRIBUTES (for process)
		NULL, // SECURITY_ATTRIBUTES (for threads)
		FALSE, // We don't need to inherit handles
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
		NULL, // Environment values
		NULL, // The same current directory
		&si,
		&pi
		)) return INVALID_HANDLE_VALUE;

	if(wait)
		WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);

	if(close_handle)
		CloseHandle(pi.hProcess);

	return pi.hProcess;
#else
	pid_t handle;
	char *cargs;
	depress_process_handle_t depress_handle = 0;
	size_t args_len;

	args_len = wcslen(args);
	if((SIZE_MAX-1)/MB_LEN_MAX < args_len) return DEPRESS_INVALID_PROCESS_HANDLE;
	args_len *= MB_LEN_MAX;

	depress_handle = malloc(sizeof(depress_process_handle_int_t));
	if(!depress_handle) {
		return DEPRESS_INVALID_PROCESS_HANDLE;
	}

	cargs = malloc(args_len+1);
	if(!cargs) {
		free(depress_handle);

		return DEPRESS_INVALID_PROCESS_HANDLE;
	}

	wcstombs(cargs, args, args_len+1);

	handle = fork();

	if(handle) {
		free(cargs);
		depress_handle->handle = handle;
		depress_handle->is_closed = 0;

		if(wait) depressWaitForProcess(depress_handle);
		if(close_handle) depressCloseProcessHandle(depress_handle);

		return depress_handle;
	}

	execlp("/bin/sh", "/bin/sh", "-c", cargs, NULL);

	return DEPRESS_INVALID_PROCESS_HANDLE;
#endif
}

void depressWaitForProcess(depress_process_handle_t handle)
{
#if defined(_WIN32)
	WaitForSingleObject(handle, INFINITE);
#else
	size_t set_closed = 1, size_t prev_closed;
	__atomic_exchange(&handle->is_closed, &set_closed, &prev_closed, __ATOMIC_SEQ_CST);
	if(!prev_closed) waitpid(handle->handle, 0, 0);
#endif
}

void depressCloseProcessHandle(depress_process_handle_t handle)
{
#if defined(_WIN32)
	CloseHandle(handle);
#else
	depressWaitForProcess(handle);
	free(handle);
#endif
}

depress_event_handle_t depressCreateEvent(void)
{
#if defined(_WIN32)
	return CreateEventW(NULL, TRUE, FALSE, NULL);
#else
	size_t *handle;

	handle = malloc(sizeof(size_t));
	if(!handle) return DEPRESS_INVALID_EVENT_HANDLE;

	*handle = 0;

	return handle;
#endif
}

bool depressWaitForEvent(depress_event_handle_t handle, uint32_t milliseconds)
{
#if defined(_WIN32)
	return WaitForSingleObject(handle, milliseconds) == WAIT_OBJECT_0;
#else

	while(1) {
		size_t event_val;

		__atomic_load(handle, &event_val, __ATOMIC_SEQ_CST);
		if(event_val) return true;

		if(milliseconds == 0) break;
		usleep(1000);
		if(milliseconds != DEPRESS_WAIT_TIME_INFINITE) milliseconds -= 1;
	}

	return false;
#endif
}

void depressSetEvent(depress_event_handle_t handle)
{
#if defined(_WIN32)
	SetEvent(handle);
#else
	size_t event_val = 1;

	__atomic_store(handle, &event_val, __ATOMIC_SEQ_CST);
#endif
}

void depressCloseEventHandle(depress_event_handle_t handle)
{
#if defined(_WIN32)
	CloseHandle(handle);
#else
	free(handle);
#endif
}

depress_thread_handle_t depressCreateThread(depress_threadfunc_t threadfunc, void* threadargs)
{
#if defined(_WIN32)
	return (HANDLE)_beginthreadex(0, 0, threadfunc, threadargs, 0, 0);
#else
	pthread_t *thread;
	pthread_attr_t attr;
	int res;

	thread = malloc(sizeof(pthread_t));
	if(!thread) return DEPRESS_INVALID_THREAD_HANDLE;

	if(pthread_attr_init(&attr)) goto LABEL_ERROR;

	res = pthread_create(thread, &attr, threadfunc, threadargs);

	pthread_attr_destroy(&attr);

	if(res) goto LABEL_ERROR;

	return thread;

LABEL_ERROR:

	free(thread);

	return DEPRESS_INVALID_THREAD_HANDLE;
#endif
}

void depressWaitForMultipleThreads(unsigned int threads_num, depress_thread_handle_t *threads)
{
#if defined(_WIN32)
	unsigned int i;
	
	for(i = 0; i < threads_num; i+=MAXIMUM_WAIT_OBJECTS) {
		if(threads_num-i < MAXIMUM_WAIT_OBJECTS)
			WaitForMultipleObjects(threads_num-i, threads+i, TRUE, DEPRESS_WAIT_TIME_INFINITE);
		else
			WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, threads+i, TRUE, DEPRESS_WAIT_TIME_INFINITE);
	}
#else
	unsigned int i;

	for(i = 0; i < threads_num; i++) {
		void *retval;

		if(threads[i] == DEPRESS_INVALID_THREAD_HANDLE) continue;

		pthread_join(*threads[i], &retval);
	}
#endif
}

void depressCloseThreadHandle(depress_thread_handle_t handle)
{
#if defined(_WIN32)
	CloseHandle(handle);
#else
	free(handle);
#endif
}

#if defined(_WIN32)
void depressGetProcessGroupFunctions(void)
{
	HANDLE kernel32_handle;

	kernel32_handle = GetModuleHandleW(L"kernel32.dll");
	if(!kernel32_handle) return;
	GetActiveProcessorGroupCount_funcptr = (GetActiveProcessorGroupCount_type)GetProcAddress(kernel32_handle, "GetActiveProcessorGroupCount");
	GetActiveProcessorCount_funcptr = (GetActiveProcessorCount_type)GetProcAddress(kernel32_handle, "GetActiveProcessorCount");
	SetThreadGroupAffinity_funcptr = (SetThreadGroupAffinity_type)GetProcAddress(kernel32_handle, "SetThreadGroupAffinity");
}

unsigned int depressGetNumberOfThreads(void)
{
	if(GetActiveProcessorGroupCount_funcptr && GetActiveProcessorCount_funcptr) {
		WORD group_count, i;
		DWORD processor_count = 0;

		group_count = GetActiveProcessorGroupCount_funcptr();

		for(i = 0; i < group_count; i++) {
			processor_count += GetActiveProcessorCount_funcptr(i);
		}

		return processor_count;
	} else {
		SYSTEM_INFO si;

		GetSystemInfo(&si);

		return si.dwNumberOfProcessors;
	}
}

KAFFINITY depressGetMaskForProcessorCount(DWORD processor_count)
{
	return ((KAFFINITY)1 << (KAFFINITY)processor_count) - 1;
}
#else
unsigned int depressGetNumberOfThreads(void)
{
	// Should work on linux and freebsd
	return sysconf(_SC_NPROCESSORS_ONLN);
}
#endif
