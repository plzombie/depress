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

	void CDocument::SetDefaultDocumentFlags(depress_document_flags_type *document_flags)
	{
		depressSetDefaultDocumentFlags(document_flags);
	}

	bool CDocument::Create()
	{
		if(m_is_init) return false;

		CPage::SetDefaultPageFlags(&m_global_page_flags);
		SetDefaultDocumentFlags(&m_document_flags);

		if(!depressDocumentInit(&m_document, m_document_flags))
			return false;

		m_is_init = true;

		return true;
	}

	void CDocument::Destroy()
	{
		if(!m_is_init) return;

		DestroyPages();

		depressDocumentDestroy(&m_document);

		m_is_init = false;
	}

	void CDocument::DestroyPages(void)
	{
		if(!m_is_init) return;

		for(auto page : m_pages)
			delete page;

		m_pages.clear();
	}

	DocumentProcessStatus CDocument::Process(const wchar_t *outputfile)
	{
		DocumentProcessStatus status = DocumentProcessStatus::OK;

		if(!depressDocumentInit(&m_document, m_document_flags))
			status = DocumentProcessStatus::CantInitDocument;

		m_document.output_file = outputfile;

		// Add tasks
		for(auto page : m_pages) {
			if(!depressDocumentAddTask(&m_document, page->GetFilename(), page->GetFlags())) {
				status = DocumentProcessStatus::CantAddTask;

				break;
			}
		}

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

#if defined(_M_AMD64) || defined(_M_ARM64) 
		return InterlockedAdd64((LONG64 *)(&m_document.tasks_processed), 0);
#elif defined(_M_IX86) || defined(_M_ARM)
		return InterlockedAdd((LONG *)(&m_document.tasks_processed), 0);
#else
#error Define specific interlocked operation here
#endif
	}

	size_t CDocument::PagesCount(void)
	{
		return m_pages.size();
	}

	CPage *CDocument::PageGet(size_t id)
	{
		if(id >= m_pages.size())
			return 0;

		return m_pages[id];
	}

	bool CDocument::PageAdd(CPage *page)
	{
		try {
			m_pages.push_back(page);

			return true;
		} catch(std::bad_alloc) { // Why not ...? Because help says what it's only bad_alloc in allocator. So any others are not standard situation.
			return false;
		}
	}

	bool CDocument::PageDelete(size_t id)
	{
		if(id >= m_pages.size())
			return false;

		delete m_pages[id];

		if(id == m_pages.size() - 1)
			m_pages.pop_back();
		else
			m_pages.erase(m_pages.begin() + id);

		return true;
	}

	bool CDocument::PageSwap(size_t id1, size_t id2)
	{
		CPage *temp;
		if(id1 >= m_pages.size()) return false;
		if(id2 >= m_pages.size()) return false;

		temp = m_pages[id1];
		m_pages[id1] = m_pages[id2];
		m_pages[id2] = temp;

		return true;
	}

	bool CDocument::Serialize(IXmlWriter *writer, const wchar_t *basepath)
	{
		HRESULT hr;

		if(!m_is_init) return false;

		hr = writer->WriteStartElement(NULL, L"Document", NULL);
		if(hr != S_OK) return false;

		if(!CPage::SerializePageFlags(writer, m_global_page_flags)) return false;
		if(!SerializeDocumentFlags(writer, m_document_flags)) return false;

		// Write pages
		hr = writer->WriteStartElement(NULL, L"Pages", NULL);
		if(hr != S_OK) return false;

		for(auto page : m_pages) {
			if(!page->Serialize(writer, basepath, m_global_page_flags))
				return false;
		}

		hr = writer->WriteEndElement();
		// End of pages

		hr = writer->WriteEndElement();

		return true;
	}

	bool CDocument::Deserialize(IXmlReader *reader, const wchar_t *basepath)
	{
		depress_document_flags_type document_flags;
		depress_flags_type flags;
		bool success = true, read_pages = false;
		std::vector<CPage *> pages;

		if(!m_is_init) return false;

		CPage::SetDefaultPageFlags(&flags);
		SetDefaultDocumentFlags(&document_flags);

		while(true) {
			const wchar_t *value;
			HRESULT hr;
			XmlNodeType nodetype;

			hr = reader->Read(&nodetype);
			if(hr != S_OK) break;

			if(nodetype == XmlNodeType_Element) {
				if(reader->GetLocalName(&value, NULL) != S_OK) {
					success = false;
					break;
				}

				if(read_pages == false) {
					if(wcscmp(value, L"DocumentFlags") == 0) {
						if(!DeserializeDocumentFlags(reader, &document_flags)) {
							success = false;
							break;
						}
					} else if(wcscmp(value, L"Flags") == 0) {
						if(!CPage::DeserializePageFlags(reader, &flags)) {
							success = false;
							break;
						}
					} else if(wcscmp(value, L"Pages") == 0) {
						read_pages = true;
					} else {
						success = false;
						break;
					}
				} else {
					if(wcscmp(value, L"Page") == 0) {
						CPage *page = new CPage();

						if(!page->Deserialize(reader, basepath, flags)) {
							delete page;
							success = false;
							break;
						}

						pages.push_back(page);
					} else {
						success = false;
						break;
					}
				}
			} else if(nodetype == XmlNodeType_EndElement) {
				if(read_pages == true) {
					if(reader->GetLocalName(&value, NULL) != S_OK) {
						success = false;
						break;
					}

					if(wcscmp(value, L"Pages") == 0) {
						read_pages = false;
					} else {
						success = false;
						break;
					}
				}
			}
		}

		if(success) {
			for(auto page : m_pages)
				delete page;
			m_pages = pages;
			m_document_flags = document_flags;
			m_global_page_flags = flags;
		} else {
			for(auto page : pages)
				delete page;
		}

		return success;
	}

	bool CDocument::SerializeDocumentFlags(IXmlWriter *writer, depress_document_flags_type document_flags)
	{
		wchar_t value[100];
		HRESULT hr;

		hr = writer->WriteStartElement(NULL, L"DocumentFlags", NULL);
		if(hr != S_OK) return false;

		hr = writer->WriteAttributeString(NULL, L"ptt", NULL, _itow(document_flags.page_title_type, value, 10));
		if(hr != S_OK) return false;

		hr = writer->WriteAttributeString(NULL, L"ptt_flags", NULL, _itow(document_flags.page_title_type_flags, value, 10));
		if(hr != S_OK) return false;

		hr = writer->WriteEndElement();
		if(hr != S_OK) return false;

		return true;
	}

	bool CDocument::DeserializeDocumentFlags(IXmlReader *reader, depress_document_flags_type *document_flags)
	{
		const wchar_t *value;

		SetDefaultDocumentFlags(document_flags);
			
		if(reader->MoveToFirstAttribute() != S_OK) return true;
		while(1) {
			if(reader->GetLocalName(&value, NULL) != S_OK) goto PROCESSING_FAILED;
				
			if(wcscmp(value, L"ptt") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;

				document_flags->page_title_type = _wtoi(value);
			} else if(wcscmp(value, L"ptt_flags") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;
				
				document_flags->page_title_type_flags = _wtoi(value);
			}

			if(reader->MoveToNextAttribute() != S_OK) break;
		}

		return true;

	PROCESSING_FAILED:
		SetDefaultDocumentFlags(document_flags);
		return false;
	}

}
