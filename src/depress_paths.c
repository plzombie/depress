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

#include "../include/depress_paths.h"

#include <Windows.h>
#include <ShlObj.h>

#include <direct.h>

#include <io.h>

bool depressGetDjvulibrePaths(depress_djvulibre_paths_type *djvulibre_paths)
{
	DWORD filename_len;

	filename_len = SearchPathW(NULL, L"c44.exe", NULL, 32768, djvulibre_paths->c44_path, NULL);
	if (filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"cjb2.exe", NULL, 32768, djvulibre_paths->cjb2_path, NULL);
	if (filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvm.exe", NULL, 32768, djvulibre_paths->djvm_path, NULL);
	if (filename_len == 0 || filename_len > 32768) return false;

	return true;
}

bool depressGetTempFolder(wchar_t *temp_path)
{
	wchar_t appdatalocalpath[MAX_PATH], tempstr[30];
	long long counter = 0;

	if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appdatalocalpath) != S_OK) appdatalocalpath[0] = 0;

	// 260 is much greater than 32768, no checks needed

	while (1) {
		wcscpy(temp_path, appdatalocalpath);
		wcscat(temp_path, L"\\Temp");
		if (_waccess(temp_path, 06))
			return false;

		swprintf(tempstr, 30, L"\\depress%lld", counter);

		wcscat(temp_path, tempstr);

		if (_waccess(temp_path, 06)) { // Folder doesnt exist
			if (!_wmkdir(temp_path))
				return true;
		}

		counter++;
	}
}

void depressDestroyTempFolder(wchar_t *temp_path)
{
	_wrmdir(temp_path);
}
