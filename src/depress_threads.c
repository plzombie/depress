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

#include "../include/depress_threads.h"

#include "../include/depress_converter.h"

unsigned int __stdcall depressThreadProc(void *args)
{
	size_t i;
	depress_thread_arg_type arg;
	bool global_error = false;

	arg = *((depress_thread_arg_type *)args);

	for(i = arg.thread_id; i < arg.tasks_num; i += arg.threads_num) {
		if(global_error == false)
			if(WaitForSingleObject(arg.global_error_event, 0) == WAIT_OBJECT_0)
				global_error = true;

		if(global_error == false) {
			if(!depressConvertPage(arg.tasks[i].flags.type, arg.tasks[i].inputfile, arg.tasks[i].tempfile, arg.tasks[i].outputfile, arg.djvulibre_paths))
				arg.tasks[i].is_error = true;

			if(arg.tasks[i].is_error == true)
				SetEvent(arg.global_error_event);

			arg.tasks[i].is_completed = true;
		}

		SetEvent(arg.tasks[i].finished);
	}

	return 0;
}

int depressGetNumberOfThreads(void)
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}
