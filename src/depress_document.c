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

#if defined(_DEBUG) && defined(USE_STB_LEAKCHECK)
#include "third_party/stb_leakcheck.h"
#endif

#if !defined(_WIN32)
#include "unixsupport/wfopen.h"
#endif

#include "../include/depress_document.h"
#include "../include/depress_maker_djvu.h"
#include "../include/interlocked_ptr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MAXULONG_PTR
#define MAXULONG_PTR !((ULONG_PTR)0)
#endif

bool depressDocumentInit(depress_document_type *document, depress_document_flags_type document_flags, depress_maker_type maker, void *maker_ctx)
{
	memset(document, 0, sizeof(depress_document_type));
	
	document->global_error_event = DEPRESS_INVALID_EVENT_HANDLE;

	document->document_flags = document_flags;

	document->maker = maker;
	document->maker_ctx = maker_ctx;

#if defined(_WIN32)
	depressGetProcessGroupFunctions();
#endif

	document->is_init = true;

	return true;
}

bool depressDocumentInitDjvu(depress_document_type *document, depress_document_flags_type document_flags, const wchar_t *output_file)
{
	depress_maker_djvu_ctx_type *djvu_ctx;
	depress_maker_type djvu;
	bool result;

	djvu_ctx = malloc(sizeof(depress_maker_djvu_ctx_type));
	if(!djvu_ctx) return false;
	memset(djvu_ctx, 0, sizeof(depress_maker_djvu_ctx_type));

	// Get paths to djvulibre files
	if(!depressGetDjvulibrePaths(&(djvu_ctx->djvulibre_paths))) {
		wprintf(L"Can't find djvulibre files\n");
		free(djvu_ctx);

		return false;
	}

	if(!depressGetTempFolder(djvu_ctx->temp_path, document_flags.userdef_temp_dir)) {
		wprintf(L"Can't get path for temporary files\n");
		free(djvu_ctx);

		return false;
	}

	djvu_ctx->output_file = output_file;

	memset(&djvu, 0, sizeof(depress_maker_type));
	djvu.convert_ctx = depressMakerDjvuConvertCtx;
	djvu.merge_ctx = depressMakerDjvuMergeCtx;
	djvu.cleanup_ctx = depressMakerDjvuCleanupCtx;
	djvu.finalize_ctx = depressMakerDjvuFinalizeCtx;
	djvu.free_ctx = depressMakerDjvuFreeCtx;

	result = depressDocumentInit(document, document_flags, djvu, djvu_ctx);

	return result;
}

bool depressDocumentDestroy(depress_document_type *document)
{
	if(!document->is_init) return false;
	
	document->maker.free_ctx(document->maker_ctx);
	memset(&(document->maker), 0, sizeof(depress_maker_type));
	document->maker_ctx = 0;

	if(document->tasks) {
		depressDestroyTasks(document->tasks, document->tasks_num);
		
		document->tasks = 0;
		document->tasks_num = document->tasks_max = 0;
	}

	if(document->global_error_event != DEPRESS_INVALID_EVENT_HANDLE) {
		depressCloseEventHandle(document->global_error_event);
		
		document->global_error_event = DEPRESS_INVALID_EVENT_HANDLE;
	}

	depressFreeDocumentFlags(&document->document_flags);

	document->is_init = false;

	return true;
}

bool depressDocumentRunTasks(depress_document_type *document)
{
	unsigned int i;
	bool success = true;
#if defined(_WIN32)
	// Thread affinity stuff
	DWORD threads_in_current_groups = 0;
	bool need_set_thread_group = false;
	WORD current_thread_affinity = (WORD)-1;
	GROUP_AFFINITY group_affinity;
#endif

	if(document->tasks == 0)
		return false;

	document->threads_num = depressGetNumberOfThreads();
	if(document->threads_num == 0) document->threads_num = 1;
	if(document->threads_num > 64) document->threads_num = 64;
#if defined(__WATCOMC__)
	document->threads_num = 1;
#endif

	document->threads = malloc(sizeof(depress_thread_handle_t) * document->threads_num);
	document->thread_args = malloc(sizeof(depress_thread_task_arg_type) * document->threads_num);
	if(!document->threads || !document->thread_args) {
		wprintf(L"Can't allocate memory\n");

		goto LABEL_ERROR;
	}

	document->global_error_event = depressCreateEvent();
	if(document->global_error_event == NULL) {
		wprintf(L"Can't create event\n");

		goto LABEL_ERROR;
	}

#if defined(_WIN32)
	if(GetActiveProcessorGroupCount_funcptr && GetActiveProcessorCount_funcptr && SetThreadGroupAffinity_funcptr) {
		need_set_thread_group = true;
	
		memset(&group_affinity, 0, sizeof(GROUP_AFFINITY));

		group_affinity.Mask = MAXULONG_PTR;
	}
#endif

	document->tasks_next_to_process = 0;

	for(i = 0; i < document->threads_num; i++) {
		document->thread_args[i].tasks = document->tasks;
		document->thread_args[i].maker = document->maker;
		document->thread_args[i].maker_ctx = document->maker_ctx;
		document->thread_args[i].tasks_num = document->tasks_num;
		document->thread_args[i].tasks_next_to_process = &(document->tasks_next_to_process);
		document->thread_args[i].thread_id = i;
		document->thread_args[i].threads_num = document->threads_num;
		document->thread_args[i].global_error_event = document->global_error_event;

		document->threads[i] = depressCreateThread(depressThreadTaskProc, document->thread_args + i);

		if(document->threads[i] == DEPRESS_INVALID_THREAD_HANDLE) success = false;

#if defined(_WIN32)
		if(success && need_set_thread_group) {
			// We actually should not use this logic on Windows 11 and 2022 because it don't assign thread to specific processor group by default
			// https://docs.microsoft.com/en-us/windows/win32/api/processtopologyapi/nf-processtopologyapi-setthreadgroupaffinity
			if(i >= threads_in_current_groups) {
				DWORD processor_count = GetActiveProcessorCount_funcptr(current_thread_affinity);
				current_thread_affinity += 1;
				threads_in_current_groups += processor_count;
				group_affinity.Mask = depressGetMaskForProcessorCount(processor_count);
				group_affinity.Group = current_thread_affinity;
			}
			if(!SetThreadGroupAffinity_funcptr(document->threads[i], &group_affinity, NULL)) success = false;
		}
#endif

		if(!success) {
			unsigned int j;

			wprintf(L"Can't create thread\n");

			depressWaitForMultipleThreads(i+1, document->threads);

			if(document->threads[i] != DEPRESS_INVALID_THREAD_HANDLE) depressCloseThreadHandle(document->threads[i]);

			for(j = 0; j < i; j++)
				depressCloseThreadHandle(document->threads[j]);

			depressCloseEventHandle(document->global_error_event);
			document->global_error_event = DEPRESS_INVALID_EVENT_HANDLE;

			free(document->threads);
			document->threads = 0;

			free(document->thread_args);
			document->thread_args = 0;

			break;
		}
	}

	return success;

LABEL_ERROR:

	if(document->threads) {
		free(document->threads);
		document->threads = 0;
	}
	if(document->thread_args) {
		free(document->thread_args);
		document->thread_args = 0;
	}
	depressDestroyTasks(document->tasks, document->tasks_num);

	return false;
}

int depressDocumentProcessTasks(depress_document_type *document)
{
	bool success = true;
	int process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_OK;
	size_t filecount = 0;
	unsigned int i;

	if(document->tasks == 0 || document->threads == 0 || document->thread_args == 0)
		return false;

	for(filecount = 0; filecount < document->tasks_num; filecount++) {
		if(success)
			if(depressWaitForEvent(document->global_error_event, 0))
				if(process_status == DEPRESS_DOCUMENT_PROCESS_STATUS_OK)
					process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_GENERIC_ERROR;

		while(!depressWaitForEvent(document->tasks[filecount].finished, DEPRESS_WAIT_TIME_INFINITE));

		if(!document->tasks[filecount].is_completed)
			continue;

		if(document->tasks[filecount].process_status != DEPRESS_DOCUMENT_PROCESS_STATUS_OK) {
			wprintf(L"Error while converting file \"%ls\"\n", document->tasks[filecount].load_image.get_name(document->tasks[filecount].load_image_ctx, filecount));
			if(process_status == DEPRESS_DOCUMENT_PROCESS_STATUS_OK || process_status == DEPRESS_DOCUMENT_PROCESS_STATUS_GENERIC_ERROR)
				process_status = document->tasks[filecount].process_status;
		} else {
			if(process_status == DEPRESS_DOCUMENT_PROCESS_STATUS_OK && filecount > 0) {
				wprintf(L"Merging file \"%ls\"\n", document->tasks[filecount].load_image.get_name(document->tasks[filecount].load_image_ctx, filecount));

				if(!document->maker.merge_ctx(document->maker_ctx, filecount)) {
					depressSetEvent(document->global_error_event);
					process_status = DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_ADD_PAGE;
				} else
					InterlockedExchangePtr((uintptr_t *)(&document->tasks_processed), filecount);
			}
			if(filecount > 0)
				document->maker.cleanup_ctx(document->maker_ctx, filecount);
		}
	}

	depressWaitForMultipleThreads(document->threads_num, document->threads);

	for(i = 0; i < document->threads_num; i++)
		depressCloseThreadHandle(document->threads[i]);

	free(document->threads);
	free(document->thread_args);
	document->threads = 0;
	document->thread_args = 0;

	return process_status;
}

const wchar_t *depressGetDocumentProcessStatus(int process_status)
{
	switch(process_status) {
		case DEPRESS_DOCUMENT_PROCESS_STATUS_OK:
			return L"Document processed successfully";
		case DEPRESS_DOCUMENT_PROCESS_STATUS_GENERIC_ERROR:
		default:
			return L"Can't process document";
		case DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_ALLOC_MEMORY:
			return L"Can't allocate memory while processing document";
		case DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_OPEN_IMAGE:
			return L"Can't open image while processing document";
		case DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_SAVE_PAGE:
			return L"Can't save page while processing document";
		case DEPRESS_DOCUMENT_PROCESS_STATUS_CANT_ADD_PAGE:
			return L"Can't add processed page to the document";
	}
}

bool depressDocumentFinalize(depress_document_type *document)
{
	bool result;
	depress_maker_finalize_type finalize;
	size_t i;

	if(document->document_flags.page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO && !document->document_flags.outline) // Check if there are some post processing
		return true; // Nothing to be done
	
	if(SIZE_MAX/sizeof(depress_maker_finalize_page_type) < document->tasks_num) return false;

	finalize.pages = malloc(document->tasks_num*sizeof(depress_maker_finalize_page_type));
	if(!finalize.pages) return false;
	finalize.max = document->tasks_num;
	finalize.outline = document->document_flags.outline;

	for(i = 0; i < document->tasks_num; i++) {
		memset(finalize.pages+i, 0, sizeof(depress_maker_finalize_page_type));

		finalize.pages[i].page_title = document->tasks[i].flags.page_title;
		finalize.pages[i].is_page_title_short = false;
	}

	if(document->document_flags.page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC) {
		bool is_page_title_short;

		is_page_title_short = (document->document_flags.page_title_type_flags & DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME) > 0;

		for(i = 0; i < document->tasks_num; i++) {
			if(finalize.pages[i].page_title) continue;

			finalize.pages[i].page_title = document->tasks[i].load_image.get_name(document->tasks[i].load_image_ctx, i);
			finalize.pages[i].is_page_title_short = is_page_title_short;
		}
	}

	result = document->maker.finalize_ctx(document->maker_ctx, finalize);

	free(finalize.pages);

	return result;
}

size_t depressDocumentGetPagesProcessed(depress_document_type *document)
{
	return InterlockedExchangeAddPtr((uintptr_t *)(&document->tasks_processed), 0);
}

bool depressDocumentAddTask(depress_document_type *document, const depress_load_image_type load_image, void *load_image_ctx, const depress_flags_type flags)
{
	depress_task_type task;

	if(!document->is_init) return false;

	memset(&task, 0, sizeof(depress_task_type));

	// Filling task
	task.load_image = load_image;
	task.load_image_ctx = load_image_ctx;
	task.flags = flags;

	if(depressAddTask(&task, &(document->tasks), &(document->tasks_num), &(document->tasks_max)))
		return true;
	else
		return false;
}

bool depressDocumentAddTaskFromImageFile(depress_document_type *document, const wchar_t *inputfile, const depress_flags_type flags)
{
	depress_load_image_type load_image;
	void *load_image_ctx;
	size_t inputfile_size;
	bool result;

	if(!document->is_init) return false;
	
	inputfile_size = wcslen(inputfile);
	if(inputfile_size >= 32768) return false;
	if((SIZE_MAX-sizeof(wchar_t))/inputfile_size < sizeof(wchar_t)) return false;
	inputfile_size = inputfile_size*sizeof(wchar_t)+sizeof(wchar_t);
	load_image_ctx = malloc(inputfile_size);
	if(!load_image_ctx) return false;
	memcpy(load_image_ctx, inputfile, inputfile_size);

	memset(&load_image, 0, sizeof(depress_load_image_type));
	load_image.load_from_ctx = depressImageLoadFromCtx;
	load_image.free_ctx = depressImageFreeCtx;
	load_image.get_name = depressImageGetNameCtx;
	
	result = depressDocumentAddTask(document, load_image, load_image_ctx, flags);

	if(!result) free(load_image_ctx);

	return result;
}

bool depressDocumentCreateTasksFromTextFile(depress_document_type *document, const wchar_t *textfile, const wchar_t *textfilepath, depress_flags_type flags)
{
	FILE *f;
	size_t task_inputfile_length;
	wchar_t *inputfile;
	wchar_t *inputfile_fullname;

	if(document->tasks)
		depressDestroyTasks(document->tasks, document->tasks_num);

	inputfile = malloc((32770+32768)*sizeof(wchar_t));
	if(!inputfile)
		return false;
	else
		inputfile_fullname = inputfile+32770;

	document->tasks = 0;
	document->tasks_num = 0;
	document->tasks_max = 0;
	InterlockedExchangePtr((uintptr_t *)(&document->tasks_processed), 0);

#ifdef _MSC_VER
	f = _wfopen(textfile, L"rt, ccs=UTF-8");
#else
	f = _wfopen(textfile, L"rt");
#endif
	if(!f) {
		free(inputfile);
		return false;
	}

	while(1) {
		wchar_t *eol;

		// Reading line
		if(!fgetws(inputfile, 32770, f)) {
			if(feof(f))
				break;
			else 
				goto LABEL_ERROR;
		}
		if(wcslen(inputfile) == 32769) goto LABEL_ERROR;

		eol = wcsrchr(inputfile, '\n');
		if(eol) *eol = 0;

		if(*inputfile == 0)
			continue;

		// Adding textfile path to inputfile
		task_inputfile_length = depressGetFilenameToOpen(textfilepath, inputfile, 0, 32768, inputfile_fullname, 0);
		
		if(task_inputfile_length >= 32768 || task_inputfile_length == 0) goto LABEL_ERROR;

		if(!depressDocumentAddTaskFromImageFile(document, inputfile_fullname, flags)) goto LABEL_ERROR;
	}

	free(inputfile);
	fclose(f);

	return true;

LABEL_ERROR:

	free(inputfile);
	fclose(f);

	{
		size_t i;

		for (i = 0; i < document->tasks_num; i++)
			depressCloseEventHandle(document->tasks[i].finished);
	}

	free(document->tasks);

	document->tasks = 0;
	document->tasks_num = document->tasks_max = 0;

	return false;
}

void depressSetDefaultDocumentFlags(depress_document_flags_type *document_flags)
{
	memset(document_flags, 0, sizeof(depress_document_flags_type));
	document_flags->page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO;
}

void depressFreeDocumentFlags(depress_document_flags_type *document_flags)
{
	if(!document_flags->keep_data) {
		if(document_flags->outline) depressOutlineDestroy(document_flags->outline);
	}

	memset(document_flags, 0, sizeof(depress_document_flags_type));
}

