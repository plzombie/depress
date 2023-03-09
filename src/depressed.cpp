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
#include <stdio.h>
FILE *g_stb_leakcheck_f;
#define STB_LEAKCHECK_IMPLEMENTATION
#define STB_LEAKCHECK_OUTPUT_PIPE g_stb_leakcheck_f
extern "C" {
#include "third_party/stb_leakcheck.h"
}
#endif

#include <iup.h>

#include <Windows.h>
#include <process.h>

#include "../include/depressed_document.h"
#include "../include/depressed_open.h"
#include "../include/depressed_console.h"
#include "../include/depressed_gui.h"
#include "../include/depressed_app.h"

struct SDepressedApp depressed_app;

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	wchar_t **argv = 0, *default_project = 0, *default_output = 0;
	int argc = 0;

	if(*pCmdLine) {
		argv = CommandLineToArgvW(pCmdLine, &argc);
		if(!argv) return EXIT_FAILURE;
	}

	if(argc > 0) default_project = argv[0];
	if(argc > 1) default_output = argv[1];

	SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(!depressed_app.document.Create()) {
		MessageBoxW(NULL, L"Can't create empty document, probably not enough memory", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);

		return EXIT_FAILURE;
	}

	if(argc < 2) {
		// Run GUI
		if(default_project) Depressed::OpenDied(default_project, depressed_app.document);
		
		depressedRunGui();
	} else if(argc == 2) {
		// Process project file and create djvu

		if(!depressedCreateConsole())
			depressedPrint(L"Can't create console\n", true);

		if(Depressed::OpenDied(default_project, depressed_app.document)) {
			if(depressed_app.document.PagesCount() == 0)
				depressedPrint(L"No pages in document");
			else if(!depressedProcessBarConsole(depressed_app.document, default_output))
				depressedPrint(L"Can't save djvu file", true);
			else
				depressedPrint(L"File saved");
#if 0
			wchar_t *new_fn;
			new_fn = (wchar_t *)malloc((wcslen(default_project) + 5 + 1)*sizeof(wchar_t));
			if(new_fn) {
				wcscpy(new_fn, default_project);
				wcscat(new_fn, L".copy");

				if(Depressed::SaveDied(new_fn, document))
					MessageBoxW(NULL, L"Project copy saved", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONINFORMATION);
				else
					MessageBoxW(NULL, L"Can't create project copy", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);

				free(new_fn);
			}
#endif
		} else
			MessageBoxW(NULL, L"Can't open project file", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);
		//MessageBoxW(NULL, L"Project processing unimplemented", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);
	} else {
		MessageBoxW(NULL, L"Too many arguments", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONINFORMATION);
	}
	
	depressed_app.document.Destroy();

	if(argv) LocalFree(argv);

#if defined(_DEBUG) && defined(USE_STB_LEAKCHECK)
	g_stb_leakcheck_f = fopen("depressed_stb_leakcheck.txt", "w");
	if(g_stb_leakcheck_f) {
		stb_leakcheck_dumpmem();
		fclose(g_stb_leakcheck_f);
	}
#endif

	return 0;
}
