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

#ifndef DEPRESS_IMAGE_H
#define DEPRESS_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../include/depress_flags.h"

#include <stdio.h>
#include <stdbool.h>

typedef struct {
	bool (* load_from_ctx)(void *ctx, size_t id, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags);
	void (* free_ctx)(void *ctx, size_t id);
	wchar_t *(* get_name)(void *ctx, size_t id);
} depress_load_image_type;

extern bool depressImageLoadFromCtx(void *ctx, size_t id, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags);
extern void depressImageFreeCtx(void *ctx, size_t id);
extern wchar_t *depressImageGetNameCtx(void *ctx, size_t id);

extern bool depressLoadImageForPreview(wchar_t *filename, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags);
extern bool depressLoadImageFromFileAndApplyFlags(wchar_t *filename, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags);
extern unsigned char *depressLoadImage(FILE *f, int *sizex, int *sizey, int *channels, int desired_channels);
extern void depressImageApplyErrorDiffusion(unsigned char* buf, int sizex, int sizey);
extern bool depressImageApplyAdaptiveBinarization(unsigned char* buf, int sizex, int sizey);
extern bool depressImageApplyQuantization(unsigned char* buf, int sizex, int sizey, int colors);
extern bool depressImageApplyNoteshrink(unsigned char* buf, int sizex, int sizey, int colors);

#ifdef __cplusplus
}
#endif

#endif
