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

#if defined(_DEBUG) && defined(USE_STB_LEAKCHECK)
extern "C" {
#include "third_party/stb_leakcheck.h"
}
#endif

#include "../include/depressed_document.h"

namespace Depressed {

	CDocument::CDocument()
	{
		m_is_init = false;
	}

	CDocument::~CDocument()
	{
		Destroy();
	}

	void CDocument::SetDefaultDocumentFlags(depress_document_flags_type *document_flags)
	{
		depressSetDefaultDocumentFlags(document_flags);
	}

	bool CDocument::Create()
	{
		if(m_is_init) return false;

		memset(&m_global_page_flags, 0, sizeof(depress_flags_type));
		CPage::SetDefaultPageFlags(&m_global_page_flags);
		SetDefaultDocumentFlags(&m_document_flags);

		memset(&m_document, 0, sizeof(depress_document_type));

		m_is_init = true;

		return true;
	}

	void CDocument::Destroy()
	{
		if(!m_is_init) return;

		DestroyPages();

		depressDocumentDestroy(&m_document);

		if(m_global_page_flags.nof_illrects) {
			free(m_global_page_flags.illrects);
			m_global_page_flags.illrects = 0;
			m_global_page_flags.nof_illrects = 0;
		}
		if(m_global_page_flags.page_title) {
			free(m_global_page_flags.page_title);

			m_global_page_flags.page_title = 0;
		}

		depressFreePageFlags(&m_global_page_flags);
		depressFreeDocumentFlags(&m_document_flags);

		m_is_init = false;
	}

	void CDocument::DestroyPages(void)
	{
		if(!m_is_init) return;

		for(auto page : m_pages) {
			page->Destroy();

			delete page;
		}

		m_pages.clear();
	}

	DocumentProcessStatus CDocument::Process(const wchar_t *outputfile)
	{
		DocumentProcessStatus status = DocumentProcessStatus::OK;
		depress_document_flags_type document_flags;

		if(!m_is_init) return DocumentProcessStatus::DocumentNotInit;

		document_flags = m_document_flags;
		document_flags.keep_data = true;

		if(!depressDocumentInitDjvu(&m_document, document_flags, outputfile))
			status = DocumentProcessStatus::CantInitDocument;

		// Add tasks
		for(auto page : m_pages) {
			depress_flags_type page_flags;

			page_flags = page->GetFlags();
			page_flags.keep_data = true;

			if(!depressDocumentAddTaskFromImageFile(&m_document, page->GetFilename(), page_flags)) {
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
			if(depressDocumentProcessTasks(&m_document) != DEPRESS_DOCUMENT_PROCESS_STATUS_OK)
				status = DocumentProcessStatus::CantProcessTasks;

		if(status == DocumentProcessStatus::OK) {
			if(!depressDocumentFinalize(&m_document))
				status = DocumentProcessStatus::CantFinalizeTasks;
		}

		depressDocumentDestroy(&m_document);

		m_last_document_process_status = status;

		return status;
	}

	size_t CDocument::GetPagesProcessed(void)
	{
		if(!m_is_init) return 0;

		return depressDocumentGetPagesProcessed(&m_document);
	}

	size_t CDocument::PagesCount(void)
	{
		if(!m_is_init) return 0;

		return m_pages.size();
	}

	CPage *CDocument::PageGet(size_t id)
	{
		if(!m_is_init) return 0;

		if(id >= m_pages.size())
			return 0;

		return m_pages[id];
	}

	bool CDocument::PageAdd(CPage *page)
	{
		if(!m_is_init) return false;

		try {
			m_pages.push_back(page);

			return true;
		} catch(std::bad_alloc) { // Why not ...? Because help says what it's only bad_alloc in allocator. So any others are not standard situation.
			return false;
		}
	}

	bool CDocument::PageInsert(CPage *page, size_t pos)
	{
		if(!m_is_init) return false;
		if(pos >= m_pages.size()) return false;

		try {
			m_pages.insert(m_pages.begin() + pos, page);

			return true;
		} catch(std::bad_alloc) { // Why not ...? Because help says what it's only bad_alloc in allocator. So any others are not standard situation.
			return false;
		}
	}

	bool CDocument::PageDelete(size_t id)
	{
		if(!m_is_init) return false;

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
		if(!m_is_init) return false;

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
		if(hr != S_OK) return false;
		// End of pages

		if(m_document_flags.outline) {
			hr = writer->WriteStartElement(NULL, L"Outlines", NULL);
			if(hr != S_OK) return false;

			SerializeOutline(writer, m_document_flags.outline);

			hr = writer->WriteEndElement();
			if(hr != S_OK) return false;
		}

		hr = writer->WriteEndElement();
		if(hr != S_OK) return false;

		return true;
	}

	bool CDocument::Deserialize(IXmlReader *reader, const wchar_t *basepath)
	{
		depress_document_flags_type document_flags;
		depress_flags_type flags;
		bool success = true;
		std::vector<CPage *> pages;
		enum class ERead {
			DEFAULT,
			PAGES,
			OUTLINES
		} read = ERead::DEFAULT;

		if(!m_is_init) return false;

		memset(&flags, 0, sizeof(depress_flags_type));
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

				if(read == ERead::DEFAULT) {
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
						read = ERead::PAGES;
					} else if(wcscmp(value, L"Outlines") == 0) {
						read = ERead::OUTLINES;
					} else {
						success = false;
						break;
					}
				} else if(read == ERead::PAGES) {
					if(wcscmp(value, L"Page") == 0) {
						CPage *page = new CPage();

						page->Create();

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
				} else if(read == ERead::OUTLINES) {
					if(!document_flags.outline) {
						document_flags.outline = (depress_outline_type *)malloc(sizeof(depress_outline_type));
						if(!document_flags.outline) {
							success = false;
							break;
						}
						memset(document_flags.outline, 0, sizeof(depress_outline_type));
					}

					if(wcscmp(value, L"Outline") == 0) {
						if(!DeserializeOutline(reader, document_flags.outline)) {
							success = false;
							break;
						}
					} else {
						success = false;
						break;
					}
				}
			} else if(nodetype == XmlNodeType_EndElement) {
				if(read != ERead::DEFAULT) {
					if(reader->GetLocalName(&value, NULL) != S_OK) {
						success = false;
						break;
					}

					if(read == ERead::PAGES && wcscmp(value, L"Pages") == 0) {
						read = ERead::DEFAULT;
					} else if(read == ERead::OUTLINES && wcscmp(value, L"Outlines") == 0) {
						read = ERead::DEFAULT;
					} else {
						success = false;
						break;
					}
				}
			}
		}

		if(success) {
			for(auto page : m_pages) {
				page->Destroy();

				delete page;
			}
			m_pages = pages;
			m_document_flags = document_flags;
			m_global_page_flags = flags;
		} else {
			for(auto page : pages)
				delete page;

			depressFreeDocumentFlags(&document_flags);
			depressFreePageFlags(&flags);
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

	bool CDocument::SerializeOutline(IXmlWriter *writer, depress_outline_type *outline)
	{
		wchar_t value[100];
		HRESULT hr;

		if(outline->text) {
			hr = writer->WriteStartElement(NULL, L"Outline", NULL);
			if(hr != S_OK) return false;

			hr = writer->WriteAttributeString(NULL, L"text", NULL, outline->text);
			if(hr != S_OK) return false;

			hr = writer->WriteAttributeString(NULL, L"page_id", NULL, _itow(outline->page_id, value, 10));
			if(hr != S_OK) return false;
		}

		for(size_t i = 0; i < outline->nof_suboutlines; i++) {
			if(!SerializeOutline(writer, (depress_outline_type *)outline->suboutlines[i])) return false;
		}

		if(outline->text) {
			hr = writer->WriteEndElement();
			if(hr != S_OK) return false;
		}

		return true;
	}

	bool CDocument::DeserializeOutline(IXmlReader *reader, depress_outline_type *sup_outline)
	{
		depress_outline_type *cur_outline;
		void **suboutlines;
		bool is_not_empty;

		cur_outline = (depress_outline_type *)malloc(sizeof(depress_outline_type));
		if(!cur_outline) return false;
		memset(cur_outline, 0, sizeof(depress_outline_type));

		suboutlines = (void **)realloc(sup_outline->suboutlines, (sup_outline->nof_suboutlines+1)*sizeof(void *));
		if(!suboutlines) {
			free(cur_outline);

			return false;
		}
		sup_outline->suboutlines = suboutlines;
		sup_outline->suboutlines[sup_outline->nof_suboutlines] = cur_outline;
		sup_outline->nof_suboutlines++;

		is_not_empty = !reader->IsEmptyElement();

		if(reader->MoveToFirstAttribute() != S_OK) return true;
		while(1) {
			const wchar_t *value;

			if(reader->GetLocalName(&value, NULL) != S_OK) goto PROCESSING_FAILED;
				
			if(wcscmp(value, L"text") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;

				cur_outline->text = _wcsdup(value);
				if(!cur_outline->text) goto PROCESSING_FAILED;
			} else if(wcscmp(value, L"page_id") == 0) {
				if(reader->GetValue(&value, NULL) != S_OK) goto PROCESSING_FAILED;
				
				cur_outline->page_id = _wtoi(value);
			}

			if(reader->MoveToNextAttribute() != S_OK) break;
		}

		if(is_not_empty) {
			while(true) {
				const wchar_t *value;
				HRESULT hr;
				XmlNodeType nodetype;


				hr = reader->Read(&nodetype);
				if(hr != S_OK) break;

				if(nodetype == XmlNodeType_Element) {
					if(reader->GetLocalName(&value, NULL) != S_OK) goto PROCESSING_FAILED;

					if(wcscmp(value, L"Outline") == 0) {
						if(!CDocument::DeserializeOutline(reader, cur_outline)) goto PROCESSING_FAILED;
					} else
						goto PROCESSING_FAILED;
				} else if(nodetype == XmlNodeType_EndElement) {
					if(reader->GetLocalName(&value, NULL) != S_OK) goto PROCESSING_FAILED;

					if(wcscmp(value, L"Outline") == 0)
						break;
					else
						goto PROCESSING_FAILED;
				}
			}
		}

		return true;

	PROCESSING_FAILED:

		return false;
	}
}
