/*
BSD 2-Clause License

Copyright (c) 2022, Mikhail Morozov
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

#ifndef DEPRESS_DOCUMENT_H
#define DEPRESS_DOCUMENT_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#include <Windows.h>

#include "../include/depress_tasks.h"
#include "../include/depress_threads.h"
#include "../include/depress_paths.h"

enum {
	DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO,
	DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC
};

enum {
	DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME = 0x1
};

typedef struct {
	// Tasks
	depress_task_type *tasks;
	size_t tasks_num;
	size_t tasks_max;
	// Threads
	HANDLE *threads;
	depress_thread_arg_type *thread_args;
	int threads_num;
	// Paths
	depress_djvulibre_paths_type djvulibre_paths;
	// Final flags
	int page_title_type;
	unsigned int page_title_type_flags;
	// Paths
	wchar_t temp_path[32768];
	wchar_t *output_file;
	// Handles
	HANDLE global_error_event;
} depress_document_type;

extern bool depressDocumentInit(depress_document_type *document);
extern bool depressDocumentDestroy(depress_document_type *document);
extern bool depressDocumentRunTasks(depress_document_type *document);
extern bool depressDocumentProcessTasks(depress_document_type *document);
extern bool depressDocumentFinalize(depress_document_type *document);

#endif
