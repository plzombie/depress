/*
BSD 2-Clause License

Copyright (c) 2022, Mikhail Morozov
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

#include <stdio.h>
#include <wchar.h>

#include "../include/depress_image.h"

#include "../include/depressed_page.h"

namespace Depressed {
	bool CPage::LoadImageForPage(int *sizex, int *sizey, int *channels, unsigned char **buf)
	{
		if(!depressLoadImageFromFileAndApplyFlags(m_filename, sizex, sizey, channels, buf, m_flags))
			return false;

	}

	bool CPage::Serialize(void *p, wchar_t *basepath)
	{
		return false;
	}
	
	bool CPage::Deserialize(void *p, wchar_t *basepath)
	{
		return false;
	}

	bool CPage::SerializePageFlags(void *p, depress_flags_type flags)
	{
		return false;
	}

	bool CPage::DeserializePageFlags(void *p, depress_flags_type *flags)
	{
		return false;
	}
}

