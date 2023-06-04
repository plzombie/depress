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

#ifndef DEPRESS_MAKER_H
#define DEPRESS_MAKER_H

#include "depress_image.h"
#include "depress_flags.h"
#include "depress_outlines.h"

typedef struct {
	wchar_t *page_title;
	bool is_page_title_short;
} depress_maker_finalize_page_type;

typedef struct {
	depress_maker_finalize_page_type *pages;
	size_t max;
	depress_outline_type *outline;
} depress_maker_finalize_type;

typedef struct {
	int (* convert_ctx)(void *ctx, size_t id, depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx);
	bool (* merge_ctx)(void* ctx, size_t id);
	void (* cleanup_ctx)(void *ctx, size_t id);
	bool (* finalize_ctx)(void* ctx, depress_maker_finalize_type finalize);
	void (* free_ctx)(void *ctx);
} depress_maker_type;

#endif
