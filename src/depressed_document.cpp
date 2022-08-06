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

#include "../include/depressed_document.h"

namespace Depressed {

	CDocument::CDocument()
	{
		m_is_init = false;
	}

	CDocument::~CDocument()
	{
		depressDocumentDestroy(&m_document);
	}

	void CDocument::SetDefaultPageFlags(depress_flags_type *page_flags)
	{
		memset(page_flags, 0, sizeof(depress_flags_type));
		page_flags->type = DEPRESS_PAGE_TYPE_COLOR;
		page_flags->quality = 100;
	}

	void CDocument::SetDefaultDocumentFlags(depress_document_flags_type *document_flags)
	{
		memset(document_flags, 0, sizeof(depress_document_flags_type));
		document_flags->page_title_type = DEPRESS_DOCUMENT_PAGE_TITLE_TYPE_NO;
	}

	bool CDocument::Create()
	{
		if(m_is_init) return false;

		SetDefaultPageFlags(&m_global_page_flags);
		SetDefaultDocumentFlags(&m_document_flags);

		if(!depressDocumentInit(&m_document, m_document_flags))
			return false;

		m_is_init = true;

		return true;
	}

	void CDocument::Destroy()
	{
		if(!m_is_init) return;

		depressDocumentDestroy(&m_document);

		m_is_init = false;
	}

	DocumentProcessStatus CDocument::Process(void)
	{
		DocumentProcessStatus status = DocumentProcessStatus::OK;

		if(!depressDocumentInit(&m_document, m_document_flags))
			status = DocumentProcessStatus::CantInitDocument;

		// Add tasks
		// There should be code to add tasks (depressDocumentAddTask)

		// Create threads from tasks
		if(status == DocumentProcessStatus::OK)
			if(!depressDocumentRunTasks(&m_document))
				status = DocumentProcessStatus::CantStartTasks;

		// Creating djvu
		if(status == DocumentProcessStatus::OK)
			if(!depressDocumentProcessTasks(&m_document))
				status = DocumentProcessStatus::CantProcessTasks;

		if(status == DocumentProcessStatus::OK) {
			if(!depressDocumentFinalize(&m_document))
				status = DocumentProcessStatus::CantFinalizeTasks;
		}

		depressDocumentDestroy(&m_document);

		if(!depressDocumentInit(&m_document, m_document_flags))
			if(status == DocumentProcessStatus::OK)
				status = DocumentProcessStatus::CantReInitDocument;

		m_last_document_process_status = status;

		return status;
	}

	size_t CDocument::GetPagesProcessed(void)
	{
		if(!m_is_init) return 0;

		return m_document.tasks_processed;
	}

	bool CDocument::Serialize(void *p)
	{
		if(!m_is_init) return false;

		return false;
	}

	bool CDocument::Deserialize(void *p)
	{
		depress_document_type new_document;
		depress_document_flags_type document_flags;
		bool success = true;

		if(!m_is_init) return false;

		// Здесь должно быть чтение флагов документа и глобальных флагов страницы

		SetDefaultDocumentFlags(&document_flags);
		success = depressDocumentInit(&new_document, document_flags);
		if(!success) return false;

		// Здесь должно быть чтение страниц
		success = false;

		if(success) {
			depressDocumentDestroy(&m_document);
			m_document = new_document;
		} else {
			depressDocumentDestroy(&new_document);
		}

		return success;
	}

}
