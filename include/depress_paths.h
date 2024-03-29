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

#ifndef DEPRESS_PATHS_H
#define DEPRESS_PATHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <wchar.h>

typedef struct {
	wchar_t cjb2_path[32768];
	wchar_t c44_path[32768];
	wchar_t cpaldjvu_path[32768];
	wchar_t csepdjvu_path[32768];
	wchar_t djvm_path[32768];
	wchar_t djvused_path[32768];
	wchar_t djvuextract_path[32768];
	wchar_t djvumake_path[32768];
} depress_djvulibre_paths_type;

extern size_t depressGetFilenameToOpen(const wchar_t *inp_path, const wchar_t *inp_filename, const wchar_t *file_ext, size_t buflen, wchar_t *out_filename, wchar_t **out_filename_start);
extern void depressGetFilenamePath(const wchar_t *filename, const wchar_t *filename_start, wchar_t *filepath);
extern bool depressGetDjvulibrePaths(depress_djvulibre_paths_type *djvulibre_paths);
extern bool depressGetTempFolder(wchar_t *temp_path, wchar_t *userdef_temp_dir);
extern void depressDestroyTempFolder(wchar_t *temp_path);

#ifdef __cplusplus
}
#endif

#endif
