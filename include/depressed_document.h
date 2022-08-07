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

#ifndef DEPRESSED_DOCUMENT_H
#define DEPRESSED_DOCUMENT_H

#include "depressed_page.h"
#include "depress_document.h"
#include "depress_flags.h"
#include <xmllite.h>
#include <vector>

namespace Depressed {

enum class DocumentProcessStatus {
	OK,
	CantInitDocument,
	CantAddTask,
	CantStartTasks,
	CantProcessTasks,
	CantFinalizeTasks,
	CantReInitDocument
};

class CDocument {
	depress_document_type m_document;
	depress_document_flags_type m_document_flags;
	depress_flags_type m_global_page_flags;
	bool m_is_init;
	DocumentProcessStatus m_last_document_process_status;
	std::vector<CPage *> m_pages;

public:
	CDocument();
	~CDocument();
	static void SetDefaultDocumentFlags(depress_document_flags_type *document_flags);
	bool Create(void);
	void Destroy(void);
	void DestroyPages(void);
	DocumentProcessStatus Process(const wchar_t *outputfile);
	size_t GetPagesProcessed(void);
	DocumentProcessStatus GetLastDocumentProcessStatus(void) { return m_last_document_process_status; }
	size_t PagesCount(void);
	CPage *PageGet(size_t id);
	bool PageAdd(CPage *page);
	bool PageDelete(size_t id);
	bool PageSwap(size_t id1, size_t id2);
	bool Serialize(IXmlWriter *writer, const wchar_t *basepath);
	bool Deserialize(IXmlReader *reader, const wchar_t *basepath);
	static bool SerializeDocumentFlags(IXmlWriter *writer, depress_document_flags_type document_flags);
	static bool DeserializeDocumentFlags(IXmlReader *reader, depress_document_flags_type *document_flags);
};

}

#endif

