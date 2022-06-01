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

#include "../include/depress_image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"



unsigned char *depressLoadImage(FILE *f, int *sizex, int *sizey, int *channels, bool is_bw)
{
	unsigned char *buf = 0;

	if(is_bw) {
		buf = stbi_load_from_file(f, sizex, sizey, channels, 1);
		*channels = 1;
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

bool depressImageApplyAdaptiveBinarization(unsigned char* buf, int sizex, int sizey)
{
	int window_size = 33, window_size_half = 16;
	int i, j, k;
	unsigned char *old_buf, *p, *p1;

	if(!buf || sizex <= 0 || sizey <= 0) return false;

	old_buf = malloc((window_size_half*2+sizex)*(window_size_half*2+sizey));
	if(!old_buf) return false;

	// Fill copy of image
	p = buf;
	p1 = old_buf;
	for(i = -window_size_half; i < sizey+window_size_half; i++) {
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
	p = buf;
	for(i = 0; i < sizey; i++) {
		for(j = 0; j < sizex; j++) {
			unsigned int sum = 0;
			int l;
			unsigned char *p2;

			p1 = old_buf+i*(window_size_half*2+sizex)+j;

			for(k = 0; k < window_size; k++) {
				p2 = p1;

				for(l = 0; l < window_size; l++) {
					sum += (*p2) * (*p2);
					p2++;
				}

				p1 += window_size_half*2+sizex;
			}

			sum /= window_size*window_size;
			sum = (unsigned int)sqrt(sum);
			//wprintf(L"%d ", sum);
			if(*p >= sum) *p = 255; else *p = 0;
			p++;
		}
	}

	free(old_buf);

	return true;
}
