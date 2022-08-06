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

#include "../include/depressed_open.h"

#include <xmllite.h>
#include <Shlwapi.h>
#include <string.h>

namespace Depressed {

	static bool OpenXml(wchar_t *filename, IXmlReader **reader, IStream **filestream)
	{
		HRESULT hr;

		hr = SHCreateStreamOnFileW(filename, STGM_READ, filestream);
		if(FAILED(hr)) return false;

		hr = CreateXmlReader(__uuidof(IXmlReader), (void **)reader, NULL);
		if(FAILED(hr)) {
			(*filestream)->Release();

			return false;
		}

		// We don't need DTD
		hr = (*reader)->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
		if(FAILED(hr)) {
			(*reader)->Release();
			(*filestream)->Release();

			return false;
		}

		hr = (*reader)->SetInput(*filestream);
		if(FAILED(hr)) {
			(*reader)->Release();
			(*filestream)->Release();

			return false;
		}

		return true;
	}

	bool OpenDied(wchar_t *filename, CDocument &document, wchar_t **basepath)
	{
		
		IXmlReader *reader;
		IStream *filestream;

		if(!OpenXml(filename, &reader, &filestream)) return false;

		while(true) {
			const wchar_t *value;
			HRESULT hr;
			XmlNodeType nodetype;

			hr = reader->Read(&nodetype);
			if(hr == S_FALSE) break;
			if(hr == E_PENDING) {
				Sleep(0);
				continue;
			}

			if(nodetype != XmlNodeType_Element) continue;
			reader->GetLocalName(&value, NULL);
			if(wcscmp(value, L"Document") == 0) {
				if(!document.Deserialize(reader, 0)) {
					reader->Release();
					filestream->Release();

					return false;
				}

				break;
			}
		}

		reader->Release();
		filestream->Release();

		return true;
	}
	
	bool SaveDied(wchar_t *filename, CDocument &document, wchar_t **basepath)
	{
		return false;
	}
}
