/*
BSD 2-Clause License

Copyright (c) 2021-2023, Mikhail Morozov
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

#include "../include/depress_paths.h"

#if defined(_WIN32)
#include <Windows.h>
#include <ShlObj.h>

#include <direct.h>

#include <io.h>
#else
#include <limits.h>

#include "unixsupport/waccess.h"
#include "unixsupport/wmkdir.h"
#include "unixsupport/wrmdir.h"
#include "unixsupport/wgetcwd.h"
#endif

size_t depressGetFilenameToOpen(const wchar_t *inp_path, const wchar_t *inp_filename, const wchar_t *file_ext, size_t buflen, wchar_t *out_filename, wchar_t **out_filename_start)
{
#if defined(_WIN32)
	DWORD fn_length;
	
	if(buflen > MAXDWORD) return 0;

	fn_length = SearchPathW(inp_path, inp_filename, file_ext, (DWORD)buflen, out_filename, out_filename_start);
	
	return fn_length;
#else
	size_t inp_len;

	inp_len = wcslen(inp_filename);

	if(inp_filename[0] != '.' && inp_filename[0] != '/') {
		bool out_filename_found = false;

		if(inp_len >= buflen) return 0;

		if(inp_path) {
			size_t inppath_len;

			inppath_len = wcslen(inp_path);

			if(inppath_len + inp_len + 2 < buflen) {
				out_filename[0] = 0;
				wcscpy(out_filename, inp_path);
				wcscpy(out_filename + inppath_len, L"/");
				wcscpy(out_filename + inppath_len + 1, inp_filename);
				if(!_waccess(out_filename, 04)) out_filename_found = true;
			}
		}

		if(!out_filename_found) {
			wchar_t cwd[PATH_MAX+1];

			if(_wgetcwd(cwd, PATH_MAX + 1)) {
				size_t cwd_len;

				cwd_len = wcslen(cwd);

				if(cwd_len + inp_len + 2 < buflen) {
					out_filename[0] = 0;
					wcscpy(out_filename, cwd);
					wcscpy(out_filename + cwd_len, L"/");
					wcscpy(out_filename + cwd_len + 1, inp_filename);
					if(!_waccess(out_filename, 04)) out_filename_found = true;
				}
			}
		}
		

		if(!out_filename_found) wcscpy(out_filename, inp_filename);
	} else {
		if(inp_len >= buflen) return 0;

		// Should use realpath here
		out_filename[0] = 0;
		wcscpy(out_filename, inp_filename);
	}

	if(out_filename_start) {
		*out_filename_start = wcsrchr(out_filename, L'/');

		if(!out_filename_start) *out_filename_start = out_filename;
	}

	return inp_len;
#endif
}

void depressGetFilenamePath(const wchar_t *filename, const wchar_t *filename_start, wchar_t *filepath)
{
	size_t path_size;
	
	path_size = filename_start - filename;
	wcsncpy(filepath, filename, path_size);
	filepath[path_size] = 0;
}

bool depressGetDjvulibrePaths(depress_djvulibre_paths_type *djvulibre_paths)
{
#if defined(_WIN32)
	DWORD filename_len;

	filename_len = SearchPathW(NULL, L"c44.exe", NULL, 32768, djvulibre_paths->c44_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"cjb2.exe", NULL, 32768, djvulibre_paths->cjb2_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"cpaldjvu.exe", NULL, 32768, djvulibre_paths->cpaldjvu_path, NULL);
	if (filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"csepdjvu.exe", NULL, 32768, djvulibre_paths->csepdjvu_path, NULL);
	if (filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvm.exe", NULL, 32768, djvulibre_paths->djvm_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvused.exe", NULL, 32768, djvulibre_paths->djvused_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvuextract.exe", NULL, 32768, djvulibre_paths->djvuextract_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;

	filename_len = SearchPathW(NULL, L"djvumake.exe", NULL, 32768, djvulibre_paths->djvumake_path, NULL);
	if(filename_len == 0 || filename_len > 32768) return false;
#else
	wcscpy(djvulibre_paths->c44_path, L"c44");
	wcscpy(djvulibre_paths->cjb2_path, L"cjb2");
	wcscpy(djvulibre_paths->cpaldjvu_path, L"cpaldjvu");
	wcscpy(djvulibre_paths->csepdjvu_path, L"csepdjvu");
	wcscpy(djvulibre_paths->djvm_path, L"djvm");
	wcscpy(djvulibre_paths->djvused_path, L"djvused");
	wcscpy(djvulibre_paths->djvuextract_path, L"djvuextract");
	wcscpy(djvulibre_paths->djvumake_path, L"djvumake");
#endif

	return true;
}

bool depressGetTempFolder(wchar_t *temp_path, wchar_t *userdef_temp_dir)
{
	wchar_t tempstr[30], *temp_path_end;
	long long counter = 0;

	if(!userdef_temp_dir) { // Temp dir not defined, use user-wide temp path
#if defined(_WIN32)
		wchar_t appdatalocalpath[MAX_PATH];

		if(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appdatalocalpath) != S_OK) return false;

		// 32768 is much greater than 260, no checks needed
		wcscpy(temp_path, appdatalocalpath);
		wcscat(temp_path, L"\\Temp");
		if(_waccess(temp_path, 06))
			return false;
#else
		// Todo: XDG directories here
		wcscpy(temp_path, L"/tmp");
#endif
	} else { // Temp dir defined
		if(wcslen(userdef_temp_dir) >= 32700) return false; // Path too long

		if(_waccess(userdef_temp_dir, 06))
			return false;

		wcscpy(temp_path, userdef_temp_dir);
	}

	temp_path_end = temp_path + wcslen(temp_path);

	while(1) {
		*temp_path_end = 0;

		swprintf(tempstr, 30, L"/depress%lld", counter);

		wcscat(temp_path, tempstr);

		if(_waccess(temp_path, 06)) { // Folder doesnt exist
			if(!_wmkdir(temp_path))
				return true;
		}

		counter++;
	}
}

void depressDestroyTempFolder(wchar_t *temp_path)
{
	_wrmdir(temp_path);
}
