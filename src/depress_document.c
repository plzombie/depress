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

#include <stdio.h>

static depressDocumentGetTitleFromFilename(wchar_t* fname, char* title, bool use_full_name)
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
}

bool depressDocumentFinalize(depress_document_type *document, wchar_t *fname)
{
	FILE *djvused;
	wchar_t opencommand[65622]; //2(whole brackets)+32768+2(brackets)+32768+2(brackets)+80(must be enough for commands)
	char title[131072]; // backslashes and utf8 encoding needs up to 4 bytes

	if(document->page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO) // Check if there are some post processing
		return true; // Nothing to be done

	swprintf(opencommand, 65622, L"\"\"%ls\" %ls\"", document->djvulibre_paths.djvused_path, fname);

	djvused = _wpopen(opencommand, L"wt");
	if(!djvused)
		return false;

	if(document->page_title_type == DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_AUTOMATIC) {
		size_t i;
		bool full_name;

		full_name = !(document->page_title_type_flags & DEPRESS_DOCUMENT_PAGE_TITLE_AUTOMATIC_USE_SHORT_NAME);

		for(i = 0; i < document->tasks_num; i++) {
			depressDocumentGetTitleFromFilename(document->tasks[i].inputfile, title, full_name);
			fwprintf(djvused, L"select %lld; set-page-title '%hs'\n", (long long)(i+1), title);
		}
	}

	fwprintf(djvused, L"save\n");

	_pclose(djvused);

	return true;
}
