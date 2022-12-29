/*
BSD 2-Clause License

Copyright (c) 2020, Mikhail Morozov
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

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <stdio.h>

int wmain(int argc, wchar_t **argv);

int main(int argc, char **argv)
{
	int i, retval;
	size_t argv_len;
	wchar_t **wstr_argv;

	wstr_argv = malloc((argc+1)*sizeof(wchar_t *));
	if(!wstr_argv)
		exit(EXIT_FAILURE);

	setlocale(LC_ALL, "");

	for(i = 0; i < argc; i++) {
		argv_len = strlen(argv[i]);
		wstr_argv[i] = malloc((argv_len+1)*sizeof(wchar_t));
		if(wstr_argv[i] == 0) {
			int j;

			for(j = 0; j < i; j++)
				free(wstr_argv[j]);

			return EXIT_FAILURE;
		}
		mbstowcs(wstr_argv[i], argv[i], argv_len);
		wstr_argv[i][argv_len] = 0;
	}

	wstr_argv[argc] = 0;

	setlocale(LC_ALL, "C");

	retval = wmain(argc, wstr_argv);

	for(i = 0; i < argc; i++)
		free(wstr_argv[i]);
	free(wstr_argv);

	return retval;
}
