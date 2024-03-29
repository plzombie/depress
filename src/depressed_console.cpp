/*
BSD 2-Clause License

Copyright (c) 2023, Mikhail Morozov
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

#include <Windows.h>

#include "../include/depressed_console.h"

HANDLE g_h_stdin = INVALID_HANDLE_VALUE, g_h_stdout = INVALID_HANDLE_VALUE;

bool depressedCreateConsole(void)
{
	if(!AttachConsole(ATTACH_PARENT_PROCESS))
		if(!AllocConsole())
			return false;

	g_h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	g_h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

	if(g_h_stdin == NULL || g_h_stdin == INVALID_HANDLE_VALUE)
		return false;
	if(g_h_stdout == NULL || g_h_stdout == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

void depressedPrint(const wchar_t * const text, bool is_error)
{
	if(g_h_stdout == NULL || g_h_stdout == INVALID_HANDLE_VALUE) {
		MessageBoxW(NULL, text, L"Depressed", MB_OK | MB_TASKMODAL | ( is_error?(MB_ICONSTOP):(MB_ICONINFORMATION) ));
		return;
	}

	WriteConsoleW(g_h_stdout, text, (DWORD)wcslen(text), NULL, NULL);
}

unsigned __stdcall depressedSaverThread(void *param)
{
	void **args;
	Depressed::CDocument *document;
	wchar_t *output_file;
	
	args = (void **)param;
	document = (Depressed::CDocument *)args[0];
	output_file = (wchar_t *)args[1];

	if(document->Process(output_file) == Depressed::DocumentProcessStatus::OK)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

bool depressedProcessBarConsole(Depressed::CDocument &document, const wchar_t *output_file)
{
	HANDLE saver_thread;
	void *args[2];
	DWORD exit_code = EXIT_FAILURE;
	size_t processed_pages = 0, pages_count = document.PagesCount();
	wchar_t tempstr[80];
	
	tempstr[79] = 0;
	args[0] = &document;
	args[1] = (void *)output_file;

	saver_thread = (HANDLE)_beginthreadex(NULL, 0, depressedSaverThread, args, 0, NULL);
	if(!saver_thread) return false;

	depressedPrint(L"Saving djvu:\n");

	while(true) {
		bool exit_loop = WaitForSingleObject(saver_thread, 500) == WAIT_OBJECT_0;
		size_t new_processed_pages = 0;
		
		if(exit_loop)
			new_processed_pages = pages_count;
		else if(document.GetLastDocumentProcessStatus() == Depressed::DocumentProcessStatus::OK)
			new_processed_pages = document.GetPagesProcessed();

		if(processed_pages != new_processed_pages) {
			swprintf(tempstr, 79, L"Processed %f%%\n", (double)new_processed_pages/(double)pages_count*100.0);
			depressedPrint(tempstr);
			processed_pages = new_processed_pages;
		}

		if(exit_loop) break;
	}
	GetExitCodeThread(saver_thread, &exit_code);

	CloseHandle(saver_thread);

	return exit_code == EXIT_SUCCESS;
}