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

		return true;
	}

	void CPage::SetDefaultPageFlags(depress_flags_type *page_flags)
	{
		memset(page_flags, 0, sizeof(depress_flags_type));
		page_flags->type = DEPRESS_PAGE_TYPE_COLOR;
		page_flags->quality = 100;
	}

	bool CPage::Serialize(void *p, wchar_t *basepath)
	{
		return false;
	}
	
	bool CPage::Deserialize(IXmlReader *reader, wchar_t *basepath, depress_flags_type flags)
	{
		bool read_filename = false;

		m_flags = flags;

		while(true) {
			const wchar_t *value;
			HRESULT hr;
			XmlNodeType nodetype;


			hr = reader->Read(&nodetype);
			if(hr != S_OK) break;
			

			if(read_filename == false) {
				if(nodetype == XmlNodeType_Element) {
					if(reader->GetLocalName(&value, NULL) != S_OK) return false;

					if(wcscmp(value, L"Flags") == 0) DeserializePageFlags(reader, &m_flags);
					else if(wcscmp(value, L"Filename") == 0) read_filename = true;
				} if(nodetype == XmlNodeType_EndElement) {
					if(reader->GetLocalName(&value, NULL) != S_OK) return false;

					if(wcscmp(value, L"Page") == 0) break; // Finished processing of the tag
				}
			} else {
				if(nodetype == XmlNodeType_Text) {
					if(reader->GetValue(&value, NULL) != S_OK) return false;

					m_filename = (wchar_t *)malloc((wcslen(value) + 1)*sizeof(wchar_t));
					if(!m_filename) return false;

					wcscpy(m_filename, value);
				} else if(nodetype == XmlNodeType_EndElement) {
					if(reader->GetLocalName(&value, NULL) != S_OK) return false;

					if(wcscmp(value, L"Filename") == 0)
						read_filename = false;
					else
						return false;
				} else if(nodetype == XmlNodeType_Element) return false;
			}
			
		}

		return true;
	}

	bool CPage::SerializePageFlags(void *p, depress_flags_type flags)
	{
		return false;
	}

	bool CPage::DeserializePageFlags(IXmlReader *reader, depress_flags_type *flags)
	{
		const wchar_t *value;
		memset(flags, 0, sizeof(depress_flags_type));

		if(reader->MoveToFirstAttribute() != S_OK) return true;
		while(1) {
			if(reader->GetLocalName(&value, NULL) != S_OK) goto PROCESSING_FAILED;

			if(wcscmp(value, L"type") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;

				flags->type = _wtoi(value);
			} else if(wcscmp(value, L"param1") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;

				flags->param1 = _wtoi(value);
			} else if(wcscmp(value, L"quality") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;

				flags->quality = _wtoi(value);
			}

			if(reader->MoveToNextAttribute() != S_OK) break;
		}

		return true;

	PROCESSING_FAILED:
		memset(flags, 0, sizeof(depress_flags_type));
		return false;
	}
}

