/*
BSD 2-Clause License

Copyright (c) 2021-2025, Mikhail Morozov
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

#if !defined(_WIN32)
#include "unixsupport/wfopen.h"
#endif

#include "../include/depress_image.h"

#include "third_party/noteshrink.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

bool depressImageLoadFromCtx(void *ctx, size_t id, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags)
{
	(void)id;

	return depressLoadImageFromFileAndApplyFlags((wchar_t *)ctx, sizex, sizey, channels, buf, flags);
}

void depressImageFreeCtx(void *ctx, size_t id)
{
	(void)id;

	free(ctx);
}

wchar_t *depressImageGetNameCtx(void *ctx, size_t id)
{
	(void)id;

	return (wchar_t *)ctx;
}
//#include <time.h>
//#include <Windows.h>
bool depressLoadImageForPreview(wchar_t *filename, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags)
{
	bool result = false;
	//clock_t c0, c1, c2; wchar_t tempstr[256];

	if(flags.type == 3) flags.param2 = 1; // We don't have default quantization algorithm here we will use noteshrink

	//c0 = clock();

	result = depressLoadImageFromFileAndApplyFlags(filename, sizex, sizey, channels, buf, flags);
	if(!result) return result;

	//c1 = clock();
	
	if(flags.type == 1 && flags.nof_illrects == 0) { // Need to binarize image
		size_t i, len;
		unsigned char *p;

		p = *buf;
		len = (size_t)(*sizex)*(size_t)(*sizey);
		for(i = 0; i < len; i++) {
			if(*p > 127) *p = 255; else *p = 0;
			p++;
		}
	}

	//c2 = clock();

	//swprintf(tempstr, 256, L"%u %u %u (%u %u)", c0, c1, c2, (c1 - c0), (c2 - c0));
	//MessageBoxW(NULL, tempstr, L"Measurement", MB_OK);

	return result;
}

bool depressLoadImageFromFileAndApplyFlags(wchar_t *filename, int *sizex, int *sizey, int *channels, unsigned char **buf, depress_flags_type flags)
{
	FILE *f = 0;
	int desired_channels = 0;

	f = _wfopen(filename, L"rb");
	if(!f)
		return false;

	if(flags.type == DEPRESS_PAGE_TYPE_BW) {
		if(!flags.nof_illrects) desired_channels = 1;
	} else if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED) {
		desired_channels = 3;
	}

	*buf = depressLoadImage(f, sizex, sizey, channels, desired_channels);

	fclose(f);

	if(!(*buf))
		return false;

	if(flags.type == DEPRESS_PAGE_TYPE_PALETTIZED) {
		if(flags.param2 == DEPRESS_PAGE_TYPE_PALETTIZED_PARAM2_QUANT) {
			if(!depressImageApplyQuantization(*buf, *sizex, *sizey, flags.param1)) {
				free(*buf);

				return false;
			}
		} else if(flags.param2 == DEPRESS_PAGE_TYPE_PALETTIZED_PARAM2_NOTESHRINK) {
			if(!depressImageApplyNoteshrink(*buf, *sizex, *sizey, flags.param1)) {
				free(*buf);

				return false;
			}
		}
	}

	if(!flags.nof_illrects) {
		if(flags.type == DEPRESS_PAGE_TYPE_BW && flags.param1 == DEPRESS_PAGE_TYPE_BW_PARAM1_ERRDIFF)
			depressImageApplyErrorDiffusion(*buf, *sizex, *sizey);
		else if(flags.type == DEPRESS_PAGE_TYPE_BW && flags.param1 == DEPRESS_PAGE_TYPE_BW_PARAM1_ADAPTIVE) {
			if(!depressImageApplyAdaptiveBinarization(*buf, *sizex, *sizey))
				return false;
		}
	}

	return true;
}

unsigned char *depressLoadImage(FILE *f, int *sizex, int *sizey, int *channels, int desired_channels)
{
	unsigned char *buf = 0;

	if(desired_channels) {
		buf = stbi_load_from_file(f, sizex, sizey, channels, desired_channels);
		*channels = desired_channels;
	} else if(stbi_info_from_file(f, sizex, sizey, channels)) {
		switch(*channels) {
			case 1:
			case 3:
				buf = stbi_load_from_file(f, sizex, sizey, channels, 0);
				break;
			case 2:
				buf = stbi_load_from_file(f, sizex, sizey, channels, 1);
				*channels = 1;
				break;
			case 4:
				buf = stbi_load_from_file(f, sizex, sizey, channels, 3);
				*channels = 3;
				break;
		}
	}

	return buf;
}

int depressImageDetectType(int sizex, int sizey, int channels, const unsigned char *buf)
{
	int type = DEPRESS_PAGE_TYPE_COLOR;
	const unsigned char v_in_bw_min = 5, v_in_bw_max = 250;
	const double s_in_bw_min = 1.0/48.0;
	const double v_out_in_percentage = 1.0/256.0;
	const double s_out_in_percentage = 1.0/128.0;
	size_t v_in_bw_ranges = 0, v_out_bw_ranges = 0; // Количество значений Value (HSV) в пределах диапазона для ЧБ изображений и за пределами диапазона
	size_t s_in_bw_ranges = 0, s_out_bw_ranges = 0; // Количество значений Saturation (HSV) в пределах диапазона для ЧБ изображений и за пределами диапазона
	
	if(sizex < 1 || sizey < 1 || channels < 1) return type;
	if(SIZE_MAX/(size_t)sizex < (size_t)sizey) return type;
	if(SIZE_MAX/((size_t)sizex*(size_t)sizey) < (size_t)channels) return type;

	if(channels == 1) {
		size_t i;
		type = DEPRESS_PAGE_TYPE_BW;
		for(i = 0; i < (size_t)sizex*(size_t)sizey; i++) {
			if(buf[i] <= v_in_bw_min || buf[i] >= v_in_bw_max) v_in_bw_ranges++; else v_out_bw_ranges++;
		}
	} else {
		size_t i;

		type = DEPRESS_PAGE_TYPE_BW;
		for(i = 0; i < (size_t)sizex*(size_t)sizey; i++) {
			unsigned char min, max;
			size_t j;
			double s = 0.0;

			min = max = buf[i*(size_t)channels];

			for(j = 1; j < (size_t)channels; j++) {
				if(buf[i*(size_t)channels+j] < min) {
					min = buf[i*(size_t)channels+j];
				} else if(buf[i*(size_t)channels+j] > max) {
					max = buf[i*(size_t)channels+j];
				}
			}

			if(max != 0) s = 1.0 - (double)min/(double)max;

			if(max <= v_in_bw_min || max >= v_in_bw_max) v_in_bw_ranges++; else v_out_bw_ranges++;
			if(s <= s_in_bw_min) s_in_bw_ranges++; else s_out_bw_ranges++;
		}
	}

	//wprintf(L"v in %u v out %u s in %u s out %u\n", (unsigned int)v_in_bw_ranges, (unsigned int)v_out_bw_ranges, (unsigned int)s_in_bw_ranges, (unsigned int)s_out_bw_ranges);

	if(v_in_bw_ranges == 0)
		type = DEPRESS_PAGE_TYPE_COLOR;
	else if((double)v_out_bw_ranges > (double)v_in_bw_ranges*v_out_in_percentage)
		type = DEPRESS_PAGE_TYPE_COLOR;
	else if(s_in_bw_ranges != 0 || s_out_bw_ranges != 0) {
		if(s_in_bw_ranges == 0)
			type = DEPRESS_PAGE_TYPE_COLOR;
		else if((double)s_out_bw_ranges > (double)s_in_bw_ranges*s_out_in_percentage)
			type = DEPRESS_PAGE_TYPE_COLOR;
	}

	return type;
}

void depressImageSimplyBinarize(unsigned char **buf, int sizex, int sizey, int channels)
{
	unsigned char *orig_buf, *new_buf;
	size_t i;

	if(sizex < 1 || sizey < 1 || channels < 2) return;
	if(SIZE_MAX/(size_t)sizex < (size_t)sizey) return;
	if(SIZE_MAX/((size_t)sizex*(size_t)sizey) < (size_t)channels) return;

	orig_buf = *buf;
	for(i = 1; i < (size_t)sizex*(size_t)sizey; i++) {
		orig_buf[i] = orig_buf[i*(size_t)channels];
		if(orig_buf[i] >= 128) orig_buf[i] = 255; else orig_buf[i] = 0;
	}

	new_buf = realloc(orig_buf, (size_t)sizex*(size_t)sizey);
	if(new_buf) *buf = new_buf;
}

void depressImageApplyErrorDiffusion(unsigned char* buf, int sizex, int sizey)
{
	int i, j, acc = 0;
	unsigned char *p;

	if(!buf || sizex <= 0 || sizey <= 0) return;

	p = buf;

	for(i = 0; i < sizey; i++) {
		for(j = 0; j < sizex; j++) {
			acc += *p;
			
			if(acc >= 255) {
				acc -= 255;
				*(p++) = 255;
			} else
				*(p++) = 0;
		}
	}
}

//#include <time.h>

bool depressImageApplyAdaptiveBinarization(unsigned char* buf, int sizex, int sizey)
{
	int window_size = 33, window_size_half = 16;
	int i;
	unsigned char *old_buf, *p, *p1;
	//clock_t time_start;

	if(!buf || sizex <= 0 || sizey <= 0) return false;

	if(INT_MAX-sizex < window_size_half*2) return false;
	if(INT_MAX-sizey < window_size_half*2) return false;
	if(INT_MAX/(window_size_half*2+sizex) < (window_size_half*2+sizey)) return false;

	old_buf = malloc((window_size_half*2+sizex)*(window_size_half*2+sizey));
	if(!old_buf) return false;

	//time_start = clock();

	// Fill copy of image
	p = buf;
	p1 = old_buf;
	for(i = -window_size_half; i < sizey+window_size_half; i++) {
		int k;
		unsigned char b;
		
		for(k = 0; k < window_size_half; k++) {
			*p1 = *p;
			p1++;
		}

		memcpy(p1, p, sizex);
		p1 += sizex;

		b = p[sizex-1];
		for(k = 0; k < window_size_half; k++) {
			*p1 = b;
			p1++;
		}

		if(i >= 0 && i < sizey-1) p += sizex;
	}

	// Perform adaptive binarization
	
#pragma omp parallel for
	for(i = 0; i < sizey; i++) {
		unsigned char *p, *p1;
		int j;

		p = buf+(size_t)i*(size_t)sizex; // New buffer
		p1 = old_buf+(size_t)i*((size_t)sizex+2*window_size_half); // Buffer with old data
		for(j = 0; j < sizex; j++) {
			unsigned int sum = 0;
			int k;
			unsigned char *p2;

			p2 = p1; // Window buffer with old data
			for(k = 0; k < window_size; k++) {
				int l;

				for(l = 0; l < window_size; l++) {
					int b;

					b = (int)(*p2);
					sum += b*b;
					p2++;
				}

				p2 += sizex-1; // Jump to the next window line
			}

			sum /= window_size*window_size;
			sum = (unsigned int)sqrt(sum);

			p1++;
			if(*p >= sum) *p = 255; else *p = 0;
			p++;
		}
		p1 += window_size_half * 2; // Pad for the next line
	}

	//wprintf(L"time %d\n", clock() - time_start);
	free(old_buf);

	return true;
}

bool depressImageApplyQuantization(unsigned char *buf, int sizex, int sizey, int colors)
{
	(void)buf;
	(void)sizex;
	(void)sizey;

	if(colors < 2) colors = 2;
	if(colors > 256) colors = 256;

	return true;
}

bool depressImageApplyNoteshrink(unsigned char *buf, int sizex, int sizey, int colors)
{
	float *palette = 0, *p3;
	unsigned char *newbuf = 0, *p, *p2;
	NSHOption option;
	bool success = true;
	size_t i;

	if(INT_MAX/sizex < sizey) return false;
	if(SIZE_MAX/(3*sizeof(float)) < colors) return false;

	if(colors < 2) colors = 2;
	if(colors > 256) colors = 256;
	option = NSHMakeDefaultOption();

	palette = malloc(3*colors*sizeof(float));
	if(!palette) success = false;

	if(success) {
		newbuf = malloc(sizex*sizey);
		if(!newbuf) success = false;
	}

	if(success)
		success = NSHPaletteCreate(buf, sizex, sizey, 3, option, palette, colors);

	if(success)
		success = NSHPaletteApply(buf, sizex, sizey, 3, palette, colors, option, newbuf);

	if(success) {
		if(option.Saturate)
			success = NSHPaletteSaturate(palette, colors, 3);
	}

	if(success) {
		if(option.Norm)
			success = NSHPaletteNorm(palette, colors, 3);
	}

	if(success) {
		if(option.WhiteBackground) {
			palette[0] = 255;
			palette[1] = 255;
			palette[2] = 255;
		}
	}

	if(success) {
		for(i = 0; i < 3*(size_t)(colors); i++) {
			palette[i] += 0.5f;
			if(palette[i] < 0) palette[i] = 0;
			if(palette[i] > 255) palette[i] = 255;
		}

		p = buf;
		p2 = newbuf;
		for(i = 0; i < (size_t)(sizex*sizey); i++) {
			p3 = palette+(*p2)*3;
			*p = (unsigned int)(*p3); p++; p3++;
			*p = (unsigned int)(*p3); p++; p3++;
			*p = (unsigned int)(*p3); p++; p3++;
			p2++;
		}
	}

	if(palette) free(palette);
	if(newbuf) free(newbuf);

	return success;
}
