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

#include <stdlib.h>

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
