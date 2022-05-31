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

#include "../include/depress_document.h"

#include <io.h>
#include <process.h>
#include <stdio.h>

static void depressDocumentGetTitleFromFilename(wchar_t* fname, char* title, bool use_full_name)
{
	wchar_t temp[32768];
	char *p;
	size_t temp_len, i;
	uint32_t codepoint;

	if(use_full_name)
		wcscpy(temp, fname);
	else {
		wchar_t *last_slash, *last_backslash, *last_dot;

		last_slash = wcsrchr(fname, '/');
		if(!last_slash) last_slash = fname;
		else last_slash++;

		last_backslash = wcsrchr(last_slash, '\\');
		if(!last_backslash) last_backslash = last_slash;
		else last_backslash++;

		wcscpy(temp, last_backslash);

		last_dot = wcsrchr(temp, '.');
		if(last_dot) *last_dot = 0;
	}

	temp_len = wcslen(temp);

	p = title;
	for(i = 0; i < temp_len; i++) {
		if(temp[i] < 0xd800 || temp[i] > 0xdfff)
			codepoint = temp[i];
		else if(temp[i] < 0xdc00) { // high surrogate
			codepoint = temp[i] - 0xd800;
			codepoint = codepoint << 10;
			continue;
		} else { // low surrogate
			codepoint |= temp[i] - 0xdc00;
			codepoint += 0x10000;
		}

		if(codepoint == '\\')
			*(p++) = '\\';

		if(codepoint <= 0x7f)
			*(p++) = codepoint;
		else if(codepoint <= 0x7ff) {
			*(p++) = 0xc0 | ((codepoint >> 6) &0x1f);
			*(p++) = 0x80 | (codepoint & 0x3f);
		} else if(codepoint <= 0xffff) {
			*(p++) = 0xe0 | ((codepoint >> 12) & 0xf);
			*(p++) = 0x80 | ((codepoint >> 6) & 0x3f);
			*(p++) = 0x80 | (codepoint & 0x3f);
		} else if(codepoint <= 0x10ffff) {
			/**(p++) = 0xf | ((codepoint >> 18) & 0x7);
			*(p++) = 0x80 | ((codepoint >> 12) & 0x3f);
			*(p++) = 0x80 | ((codepoint >> 6) & 0x3f);
			*(p++) = 0x80 | (codepoint & 0x3f);*/
			*(p++) = '?';
		}
	}
	*p = 0;
}

bool depressDocumentInit(depress_document_type *document, depress_document_flags_type document_flags)
{
	memset(document, 0, sizeof(depress_document_type));
	
	document->global_error_event = INVALID_HANDLE_VALUE;

	document->document_flags = document_flags;

	// Get paths to djvulibre files
	if(!depressGetDjvulibrePaths(&document->djvulibre_paths)) {
		wprintf(L"Can't find djvulibre files\n");

		return false;
	}

	if(!depressGetTempFolder(document->temp_path, document->document_flags.userdef_temp_dir)) {
		wprintf(L"Can't get path for temporary files\n");

		return false;
	}

	return true;
}

bool depressDocumentDestroy(depress_document_type *document)
{
	depressDestroyTempFolder(document->temp_path);

	if(document->tasks) {
		depressDestroyTasks(document->tasks, document->tasks_num);
		
		document->tasks = 0;
		document->tasks_num = document->tasks_max = 0;
	}

	if(document->global_error_event != INVALID_HANDLE_VALUE) {
		CloseHandle(document->global_error_event);
		
		document->global_error_event = INVALID_HANDLE_VALUE;
	}

	return true;
}

bool depressDocumentRunTasks(depress_document_type *document)
{
	int i;
	bool success = true;

	if(document->tasks == 0)
		return false;

	document->threads_num = depressGetNumberOfThreads();
	if(document->threads_num <= 0) document->threads_num = 1;
	if(document->threads_num > 64) document->threads_num = 64;

	document->threads = malloc(sizeof(HANDLE) * document->threads_num);
	document->thread_args = malloc(sizeof(depress_thread_arg_type) * document->threads_num);
	if(!document->threads || !document->thread_args) {
		wprintf(L"Can't allocate memory\n");

		goto LABEL_ERROR;
	}

	document->global_error_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if(document->global_error_event == NULL) {
		wprintf(L"Can't create event\n");

		goto LABEL_ERROR;
	}

	for(i = 0; i < document->threads_num; i++) {
		document->thread_args[i].tasks = document->tasks;
		document->thread_args[i].djvulibre_paths = &document->djvulibre_paths;
		document->thread_args[i].tasks_num = document->tasks_num;
		document->thread_args[i].thread_id = i;
		document->thread_args[i].threads_num = document->threads_num;
		document->thread_args[i].global_error_event = document->global_error_event;

		document->threads[i] = (HANDLE)_beginthreadex(0, 0, depressThreadProc, document->thread_args + i, 0, 0);
		if(document->threads[i] == INVALID_HANDLE_VALUE) {
			int j;

			wprintf(L"Can't create thread\n");

			WaitForMultipleObjects(i, document->threads, TRUE, INFINITE);

			for (j = 0; j < i; j++)
				CloseHandle(document->threads[j]);

			fclose(document->global_error_event);
			document->global_error_event = INVALID_HANDLE_VALUE;

			free(document->threads);
			document->threads = 0;

			free(document->thread_args);
			document->thread_args = 0;

			success = false;

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
	depressDestroyTempFolder(document->temp_path);

	return false;
}

bool depressDocumentProcessTasks(depress_document_type *document)
{
	bool success = true;
	size_t filecount = 0;
	int i;
	wchar_t *arg0 = 0, *arg1 = 0, *arg2 = 0;

	if(document->tasks == 0 || document->threads == 0 || document->thread_args == 0)
		return false;

	arg0 = malloc(3*32770*sizeof(wchar_t));
	if(!arg0) return false;
	else {
		arg1 = arg0 + 32770;
		arg2 = arg1 + 32770;
	}

	swprintf(arg1, 32770, L"\"%ls\"", document->output_file);

	for(filecount = 0; filecount < document->tasks_num; filecount++) {
		if(success)
			if(WaitForSingleObject(document->global_error_event, 0) == WAIT_OBJECT_0)
				success = false;

		while(WaitForSingleObject(document->tasks[filecount].finished, INFINITE) != WAIT_OBJECT_0);

		if(!document->tasks[filecount].is_completed)
			continue;

		if(document->tasks[filecount].is_error) {
			wprintf(L"Error while converting file \"%ls\"\n", document->tasks[filecount].inputfile);
			success = false;
		}
		else {
			if(success && filecount > 0) {
				wprintf(L"Merging file \"%ls\"\n", document->tasks[filecount].inputfile);

				swprintf(arg2, 32770, L"\"%ls\"", document->tasks[filecount].outputfile);
				swprintf(arg0, 32770, L"\"%ls\"", document->djvulibre_paths.djvm_path);

				if(_wspawnl(_P_WAIT, document->djvulibre_paths.djvm_path, arg0, L"-i", arg1, arg2, 0)) {
					wprintf(L"Can't merge djvu files\n");
					success = false;
				}
			}
			if(filecount > 0)
				if(!_waccess(document->tasks[filecount].outputfile, 06))
					_wremove(document->tasks[filecount].outputfile);
		}
	}

	WaitForMultipleObjects(document->threads_num, document->threads, TRUE, INFINITE);

	for(i = 0; i < document->threads_num; i++)
		CloseHandle(document->threads[i]);

	free(arg0);
	free(document->threads);
	free(document->thread_args);
	document->threads = 0;
	document->thread_args = 0;

	return success;
}

bool depressDocumentFinalize(depress_document_type *document)
{
	FILE *djvused;
	wchar_t *opencommand;
	char *title;

	if(document->document_flags.page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO) // Check if there are some post processing
		return true; // Nothing to be done

	opencommand = malloc(65622*sizeof(wchar_t)); //2(whole brackets)+32768+2(brackets)+32768+2(brackets)+80(must be enough for commands)
	if(!opencommand) return false;
	title = malloc(131072); // backslashes and utf8 encoding needs up to 4 bytes
	if(!title) {
		free(opencommand);
		return false;
	}

	swprintf(opencommand, 65622, L"\"\"%ls\" \"%ls\"\"", document->djvulibre_paths.djvused_path, document->output_file);

	djvused = _wpopen(opencommand, L"wt");
	if(!djvused) {
		free(title);
		free(opencommand);
		return false;
	}

	if(document->document_flags.page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC) {
		size_t i;
		bool full_name;

		full_name = !(document->document_flags.page_title_type_flags & DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME);

		for(i = 0; i < document->tasks_num; i++) {
			depressDocumentGetTitleFromFilename(document->tasks[i].inputfile, title, full_name);
			fwprintf(djvused, L"select %lld; set-page-title '%hs'\n", (long long)(i+1), title);
		}
	}

	fwprintf(djvused, L"save\n");

	_pclose(djvused);
	free(opencommand);
	free(title);

	return true;
}

bool depressDocumentAddTask(depress_document_type *document, wchar_t *inputfile, depress_flags_type flags)
{
	depress_task_type task;
	wchar_t tempstr[32];

	memset(&task, 0, sizeof(depress_task_type));

	// Filling task
	wcscpy(task.inputfile, inputfile);
	swprintf(tempstr, 32, L"\\temp%lld.ppm", (long long)(document->tasks_num));
	wcscpy(task.tempfile, document->temp_path);
	wcscat(task.tempfile, tempstr);
	if(document->tasks_num == 0)
		wcscpy(task.outputfile, document->output_file);
	else {
		swprintf(tempstr, 32, L"\\temp%lld.djvu", (long long)(document->tasks_num));
		wcscpy(task.outputfile, document->temp_path);
		wcscat(task.outputfile, tempstr);
	}
	task.flags = flags;

	if(depressAddTask(&task, &(document->tasks), &(document->tasks_num), &(document->tasks_max)))
		return true;
	else
		return false;
}

bool depressDocumentCreateTasksFromTextFile(depress_document_type *document, wchar_t *textfile, wchar_t *textfilepath, wchar_t *outputfile, depress_flags_type flags)
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
	document->output_file = outputfile;

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
		task_inputfile_length = SearchPathW(textfilepath, inputfile, NULL, 32768, inputfile_fullname, NULL);
		if(task_inputfile_length > 32768 || task_inputfile_length == 0) goto LABEL_ERROR;

		if(!depressDocumentAddTask(document, inputfile_fullname, flags)) goto LABEL_ERROR;
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
			CloseHandle(document->tasks[i].finished);
	}

	free(document->tasks);

	document->tasks = 0;
	document->tasks_num = document->tasks_max = 0;
	document->output_file = 0;

	return false;
}

