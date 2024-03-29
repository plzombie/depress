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

#ifndef DEPRESS_MAKER_DJVU_H
#define DEPRESS_MAKER_DJVU_H

#include <wchar.h>

#include "depress_paths.h"
#include "depress_maker.h"

typedef struct {
	depress_djvulibre_paths_type djvulibre_paths;
	wchar_t temp_path[32768];
	const wchar_t *output_file;
} depress_maker_djvu_ctx_type;

extern int depressMakerDjvuConvertCtx(void *ctx, size_t id, depress_flags_type flags, depress_load_image_type load_image, void *load_image_ctx);
extern bool depressMakerDjvuMergeCtx(void *ctx, size_t id);
extern void depressMakerDjvuCleanupCtx(void *ctx, size_t id);
extern bool depressMakerDjvuFinalizeCtx(void *ctx, const depress_maker_finalize_type finalize);
extern void depressMakerDjvuFreeCtx(void *ctx);

#endif
