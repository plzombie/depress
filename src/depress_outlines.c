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

#include "../include/depress_outlines.h"

#if !defined(_WIN32)
#include "unixsupport/wfopen.h"
#include "unixsupport/wtoi.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool depressOutlineLoadFromFile(depress_outline_type **outline, wchar_t *filename)
{
	FILE *f = 0;
	depress_outline_type *o = 0;
	wchar_t *s = 0;

	f = _wfopen(filename, L"r");
	if(!f) goto FAILURE;

	o = malloc(sizeof(depress_outline_type));
	if(!o) goto FAILURE;

	memset(o, 0, sizeof(depress_outline_type));

	s = malloc(32768*sizeof(wchar_t));
	if(!s) goto FAILURE;

	while(!feof(f)) {
		depress_outline_type *new_o;
		wchar_t *sep, *sep2;
		size_t page_id, text_len;
		unsigned int nesting;

		if(!fgetws(s, 32768, f)) {
			if(feof(f))
				break;
			else
				goto FAILURE;
		}
		if(wcslen(s) == 32767) goto FAILURE;

		sep = wcschr(s, '\n');
		if(sep) *sep = 0;
		if(wcslen(s) == 0) continue;

		sep = wcschr(s, '|');
		if(!sep) goto FAILURE;
		*sep = 0;
		page_id = _wtoi(s);

		sep++;
		sep2 = wcschr(sep, '|');
		if(!sep2) goto FAILURE;
		*sep2 = 0;
		nesting = _wtoi(sep);
		sep2++;

		new_o = malloc(sizeof(depress_outline_type));
		if(!new_o) goto FAILURE;
		memset(new_o, 0, sizeof(depress_outline_type));
		new_o->page_id = page_id;
		text_len = wcslen(sep2)+1;
		new_o->text = malloc(text_len*sizeof(wchar_t));
		if(!new_o->text) {
			free(new_o);

			goto FAILURE;
		}
		memcpy(new_o->text, sep2, text_len*sizeof(wchar_t));

		if(!depressOutlineAdd(&o, new_o, nesting)) {
			depressOutlineDestroy(new_o);

			goto FAILURE;
		}
	}

	fclose(f);
	free(s);
	*outline = o;

	return true;

FAILURE:

	if(f) fclose(f);
	if(o) depressOutlineDestroy(o);
	if(s) free(s);

	return false;
}

bool depressOutlineAdd(depress_outline_type **outline_source, depress_outline_type *outline_add, unsigned int nesting)
{
	size_t nof_suboutlines;

	if(*outline_source == 0) return false;
	if(!outline_add) return false;

	nof_suboutlines = (*outline_source)->nof_suboutlines;

	if(nesting == 0) {
		depress_outline_type **_suboutlines;
		
		if(nof_suboutlines == SIZE_MAX) return false;
		nof_suboutlines++;

		if(nof_suboutlines == 1)
			_suboutlines = malloc(sizeof(depress_outline_type *));
		else
			_suboutlines = realloc((*outline_source)->suboutlines, sizeof(depress_outline_type *)*nof_suboutlines);
		
		if(!_suboutlines) return false;

		_suboutlines[nof_suboutlines-1] = outline_add;
		(*outline_source)->nof_suboutlines = nof_suboutlines;
		(*outline_source)->suboutlines = _suboutlines;

		return true;
	} else {
		if(nof_suboutlines == 0) return false; // No nesting

		return depressOutlineAdd((depress_outline_type **)((*outline_source)->suboutlines+nof_suboutlines-1), outline_add, nesting-1);
	}

	return false;
}

void depressOutlineDestroy(depress_outline_type *outline)
{
	size_t i;

	if(!outline) return;

	for(i = 0; i < outline->nof_suboutlines; i++)
		depressOutlineDestroy(outline->suboutlines[i]);

	if(outline->text) free(outline->text);
	
	if(outline->nof_suboutlines)
		free(outline->suboutlines);

	free(outline);
}
