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
