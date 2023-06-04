/*
BSD 2-Clause License

Copyright (c) 2022-2023, Mikhail Morozov
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#include "depress_tasks.h"
#include "depress_threads.h"
#include "depress_maker.h"
#include "depress_outlines.h"

enum {
	DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO,
	DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC
};

enum {
	DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME = 0x1
};

typedef struct {
	wchar_t *userdef_temp_dir;
	int page_title_type;
	unsigned int page_title_type_flags;
	depress_outline_type *outline;
	bool keep_data;
} depress_document_flags_type;

typedef struct {
	// Tasks
	depress_task_type *tasks;
	size_t tasks_num;
	size_t tasks_max;
	size_t tasks_processed;
	uintptr_t tasks_next_to_process;
	// Threads
	depress_thread_handle_t *threads;
	depress_thread_task_arg_type *thread_args;
	unsigned int threads_num;
	// Document wide flags
	depress_document_flags_type document_flags;
	// Handles
	depress_event_handle_t global_error_event;
	// Converter
	depress_maker_type maker;
	void *maker_ctx;
	// Init flag
	bool is_init;
} depress_document_type;

extern bool depressDocumentInit(depress_document_type *document, depress_document_flags_type document_flags, depress_maker_type maker, void *maker_ctx);
extern bool depressDocumentInitDjvu(depress_document_type *document, depress_document_flags_type document_flags, const wchar_t *output_file);
extern bool depressDocumentDestroy(depress_document_type *document);
extern bool depressDocumentRunTasks(depress_document_type *document);
extern int depressDocumentProcessTasks(depress_document_type *document);
extern const wchar_t* depressGetDocumentProcessStatus(int process_status);
extern bool depressDocumentFinalize(depress_document_type *document);
extern size_t depressDocumentGetPagesProcessed(depress_document_type *document);
extern bool depressDocumentAddTask(depress_document_type *document, const depress_load_image_type load_image, void *load_image_ctx, const depress_flags_type flags);
extern bool depressDocumentAddTaskFromImageFile(depress_document_type *document, const wchar_t *inputfile, const depress_flags_type flags);
extern bool depressDocumentCreateTasksFromTextFile(depress_document_type *document, const wchar_t *textfile, const wchar_t *textfilepath, depress_flags_type flags);
extern void depressSetDefaultDocumentFlags(depress_document_flags_type *document_flags);
extern void depressFreeDocumentFlags(depress_document_flags_type *document_flags);

#ifdef __cplusplus
}
#endif

#endif
