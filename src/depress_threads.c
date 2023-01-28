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

#include "../include/depress_threads.h"

#if defined(_WIN32)
GetActiveProcessorGroupCount_type GetActiveProcessorGroupCount_funcptr = 0;
GetActiveProcessorCount_type GetActiveProcessorCount_funcptr = 0;
SetThreadGroupAffinity_type SetThreadGroupAffinity_funcptr;

#include <process.h>
#endif

depress_process_handle_t depressSpawn(wchar_t *filename, wchar_t *args, bool wait, bool close_handle)
{
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
}

void depressWaitForProcess(depress_process_handle_t handle)
{
#if defined(_WIN32)
	WaitForSingleObject(handle, INFINITE);
#else
	waitpid(handle, 0, 0);
#endif
}

void depressCloseProcessHandle(depress_process_handle_t handle)
{
#if defined(_WIN32)
	CloseHandle(handle);
#else
	depressWaitForProcess(handle);
#endif
}

depress_event_handle_t depressCreateEvent(void)
{
	return CreateEventW(NULL, TRUE, FALSE, NULL);
}

bool depressWaitForEvent(depress_event_handle_t handle, uint32_t milliseconds)
{
	return WaitForSingleObject(handle, milliseconds) == WAIT_OBJECT_0;
}

void depressSetEvent(depress_event_handle_t handle)
{
	SetEvent(handle);
}

void depressCloseEventHandle(depress_event_handle_t handle)
{
	CloseHandle(handle);
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

	res = pthread_create(&thread, &attr, threadfunc, threadargs));

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
	WaitForMultipleObjects(threads_num, threads, TRUE, DEPRESS_WAIT_TIME_INFINITE);
#else
	unsigned int i;

	for(i = 0; i < threads_num; i++) {
		void *retval;

		if(threads[i] == DEPRESS_INVALID_THREAD_HANDLE) continue;

		pthread_join(*threads[i], *retval);
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

