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

#include "../include/depress_maker.h"
#include "../include/depress_maker_djvu.h"
#include "../include/depress_converter.h"
#include "../include/depress_threads.h"

#include <stdio.h>
#include <stdlib.h>

#if !defined(_WIN32)
#include "unixsupport/pclose.h"
#include "unixsupport/waccess.h"
#include "unixsupport/wremove.h"
#include "unixsupport/wpopen.h"
#include <unistd.h>
#endif

int depressMakerDjvuConvertCtx(void *ctx, size_t id, depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx)
{
	depress_maker_djvu_ctx_type *djvu_ctx;
	wchar_t temp_file[32768], page_file[32768];

	djvu_ctx = (depress_maker_djvu_ctx_type *)ctx;

	swprintf(temp_file, 32768, L"%ls/temp%llu.ppm", djvu_ctx->temp_path, (unsigned long long)id);

	if(id == 0)
		wcscpy(page_file, djvu_ctx->output_file);
	else
		swprintf(page_file, 32768, L"%ls/temp%llu.djvu", djvu_ctx->temp_path, (unsigned long long)id);

	return depressDjvuConvertPage(flags, load_image, load_image_ctx, id, temp_file, page_file, &(djvu_ctx->djvulibre_paths));
}

bool depressMakerDjvuMergeCtx(void *ctx, size_t id)
{
	bool result = true;
	depress_maker_djvu_ctx_type *djvu_ctx;

	djvu_ctx = (depress_maker_djvu_ctx_type *)ctx;

	if(id > 0) {
		wchar_t *arg0 = 0;
		wchar_t page_file[32768];

		arg0 = malloc((3*32770+1024)*sizeof(wchar_t));
		if(!arg0) return false;

		swprintf(page_file, 32768, L"%ls/temp%llu.djvu", djvu_ctx->temp_path, (unsigned long long)id);
		swprintf(arg0, 3 * 32770 + 1024, L"\"%ls\" -i \"%ls\" \"%ls\"", djvu_ctx->djvulibre_paths.djvm_path, djvu_ctx->output_file, page_file);

		if(depressSpawn(djvu_ctx->djvulibre_paths.djvm_path, arg0, true, true) == DEPRESS_INVALID_PROCESS_HANDLE)
			result = false;

		free(arg0);
	}

	return result;
}

void depressMakerDjvuCleanupCtx(void *ctx, size_t id)
{
	depress_maker_djvu_ctx_type* djvu_ctx;

	djvu_ctx = (depress_maker_djvu_ctx_type*)ctx;

	if(id > 0) {
		wchar_t page_file[32768];

		swprintf(page_file, 32768, L"%ls/temp%llu.djvu", djvu_ctx->temp_path, (unsigned long long)id);

		if(!_waccess(page_file, 06))
			if(_wremove(page_file) == -1)
#if defined(_WIN32)
				Sleep(0);
#else
				usleep(1000);
#endif
	}
}

static void depressDocumentGetTitle(wchar_t *wtitle, char *title, bool use_short_name)
{
	wchar_t temp[32768];
	char *p;
	size_t temp_len, i;
	uint32_t codepoint = 0;

	if(!use_short_name)
		wcscpy(temp, wtitle);
	else {
		wchar_t *last_slash, *last_backslash, *last_dot;

		last_slash = wcsrchr(wtitle, '/');
		if(!last_slash) last_slash = wtitle;
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
			if(codepoint < 1024 && codepoint != 0)
				codepoint = '?';
			else {
				codepoint |= temp[i] - 0xdc00;
				codepoint += 0x10000;
			}
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

static void depressMakerDjvuPrintOutline(depress_maker_djvu_ctx_type *djvu_ctx, depress_outline_type *outline, FILE *djvused)
{
	size_t i;
	bool print_outline = false;
	char *text = 0;

	if(outline->text) {
		size_t len;

		len = wcslen(outline->text);
		if(len < 32768) {
			text = malloc(len*4+1);
			if(text) {
				depressDocumentGetTitle(outline->text, text, false);

				print_outline = true;
			}
		}
	}

	if(print_outline) {
		fprintf(djvused, "(\"%s\" \"%llu\" ", text, (unsigned long long)(outline->page_id));
		free(text);
	}

	for(i = 0; i < outline->nof_suboutlines; i++)
		depressMakerDjvuPrintOutline(djvu_ctx, outline->suboutlines[i], djvused);

	if(print_outline) {
		fprintf(djvused, ")\n");
	}
}

static void depressMakerDjvuPrintOutlines(depress_maker_djvu_ctx_type *djvu_ctx, depress_outline_type *outline, FILE *djvused)
{
	if(!outline) return;

	fprintf(djvused, "set-outline\n(bookmarks \n");

	depressMakerDjvuPrintOutline(djvu_ctx, outline, djvused);

	fprintf(djvused, ")\n.\n");
}

bool depressMakerDjvuFinalizeCtx(void *ctx, const depress_maker_finalize_type finalize)
{
	FILE *djvused;
	wchar_t *opencommand;
	depress_maker_djvu_ctx_type *djvu_ctx;
	char *title;
	size_t i;

	djvu_ctx = (depress_maker_djvu_ctx_type *)ctx;

	opencommand = malloc(65622*sizeof(wchar_t)); //2(whole brackets)+32768+2(brackets)+32768+2(brackets)+80(must be enough for commands)
	if(!opencommand) return false;

	title = malloc(131072); // backslashes and utf8 encoding needs up to 4 bytes
	if(!title) {
		free(opencommand);
		
		return false;
	}

#if defined(WIN32)
	swprintf(opencommand, 65622, L"\"\"%ls\" \"%ls\"\"", djvu_ctx->djvulibre_paths.djvused_path, djvu_ctx->output_file);
#else
	swprintf(opencommand, 65622, L"\"%ls\" \"%ls\"", djvu_ctx->djvulibre_paths.djvused_path, djvu_ctx->output_file);
#endif

	djvused = _wpopen(opencommand, L"wt");
	if(!djvused) {
		free(opencommand);
		free(title);

		return false;
	}

	if(finalize.outline)
		depressMakerDjvuPrintOutlines(djvu_ctx, finalize.outline, djvused);

	for(i = 0; i < finalize.max; i++) {
		if(!finalize.pages[i].page_title) continue;
		if(wcslen(finalize.pages[i].page_title) >= 32768) continue;

		depressDocumentGetTitle(finalize.pages[i].page_title, title, finalize.pages[i].is_page_title_short);

		fprintf(djvused, "select %llu; set-page-title '%s'\n", (unsigned long long)(i+1), title);
	}

	fprintf(djvused, "save\n");

	_pclose(djvused);
	free(opencommand);
	free(title);

	return true;
}

void depressMakerDjvuFreeCtx(void *ctx)
{
	depress_maker_djvu_ctx_type *djvu_ctx;

	djvu_ctx = (depress_maker_djvu_ctx_type*)ctx;

	depressDestroyTempFolder(djvu_ctx->temp_path);

	free(ctx);
}

