/*
BSD 2-Clause License

Copyright (c) 2022-2023, Mikhail Morozov
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

#include "depress_flags.h"
#include <stdlib.h>
#include <xmllite.h>

#ifndef DEPRESS_PAGE_H
#define DEPRESS_PAGE_H

namespace Depressed {

class CPage
{
	bool m_is_init;
	depress_flags_type m_flags;
	wchar_t *m_filename;
	char *m_filename_u8;
	const size_t m_max_fn_len = 32768;
	
public:
	CPage();
	~CPage();
	void Create(void);
	void Destroy(void);
	void SetFlags(const depress_flags_type& flags) { m_flags = flags; }
	depress_flags_type GetFlags(void) { return m_flags; }
	bool SetFilename(const wchar_t *filename, const wchar_t *basepath = nullptr);
	wchar_t *GetFilename(void) { if(!m_is_init) return 0; return m_filename; }
	char *GetFilenameU8(void);
	bool LoadImageForPage(int *sizex, int *sizey, int *channels, unsigned char **buf);
	static void SetDefaultPageFlags(depress_flags_type *page_flags);
	bool Serialize(IXmlWriter *writer, const wchar_t *basepath, depress_flags_type default_flags);
	bool Deserialize(IXmlReader *reader, const wchar_t *basepath, depress_flags_type default_flags);
	static bool SerializePageFlags(IXmlWriter *writer, depress_flags_type flags);
	static bool DeserializePageFlags(IXmlReader *reader, depress_flags_type *flags);
	bool SerializeIllRect(IXmlWriter *writer, depress_illustration_rect_type *illrect);
	bool DeserializeIllRect(IXmlReader *reader, depress_illustration_rect_type *illrect);
};

}

#endif

