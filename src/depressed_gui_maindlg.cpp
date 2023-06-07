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

#include "../include/depressed_gui_maindlg.h"
#include "../include/depressed_gui_pageflags.h"
#include "../include/depressed_gui_documentflags.h"
#include "../include/depressed_app.h"
#include "../include/depressed_open.h"

#include <iupdraw.h>

static char depressed_text_error_cant_show_pages_name[] = "Error: Can't show page file name";

static int depressedProjectSaveCallback(Ihandle *self);
static int depressedProjectSaveAsCallback(Ihandle *self);

static void depressedSetPagesList(Ihandle *pages_list, Depressed::CDocument &document);

static void depressedPreviewUpdateImage(bool preview_result);

bool depressedDoBeforeNewOrSave(Ihandle *self)
{
	if(depressed_app.document_changed) {
		// TODO: Messagebox with "Project is changed. Do you want to save it first?"
		if(!depressed_app.filename)
			depressedProjectSaveAsCallback(self);
		else if(depressed_app.filename[0] == 0)
			depressedProjectSaveAsCallback(self);
		else
			depressedProjectSaveCallback(self);

		if(!depressed_app.document_changed)
			return true;
		else
			return false;
	}
	depressed_app.document_changed = false;

	return true;
}

static int depressedProjectNewCallback(Ihandle *self)
{
	if(!depressedDoBeforeNewOrSave(self)) return IUP_DEFAULT;

	depressed_app.document.Destroy();
	if (!depressed_app.document.Create()) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't create new document");

		return IUP_CLOSE;
	}

	depressedSetPagesList(depressed_app.pages_list, depressed_app.document);

	return IUP_DEFAULT;
}

static int depressedProjectOpenCallback(Ihandle *self)
{
	Ihandle *opendlg;

	if(!depressedDoBeforeNewOrSave(self)) return IUP_DEFAULT;

	opendlg = IupFileDlg();
	if(!opendlg) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open file dialog");
	}

	IupSetAttribute(opendlg, "DIALOGTYPE", "OPEN");
	IupSetAttribute(opendlg, "EXTFILTER", "DepressED project (*.died)|*.died|All files (*.*)|*.*|");

	IupPopup(opendlg, IUP_CENTER, IUP_CENTER);

	if(IupGetInt(opendlg, "STATUS") != -1) {
		char *cfilename;
		size_t cfilename_length;
		wchar_t *wfilename;

		cfilename = IupGetAttribute(opendlg, "VALUE");
		cfilename_length = strlen(cfilename) + 1;
		wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

		MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

		if(!Depressed::OpenDied(wfilename, depressed_app.document)) {
			IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open project");
			free(wfilename);
		} else {
			if(depressed_app.filename) free(depressed_app.filename);
			depressed_app.filename = wfilename;
			depressed_app.document_changed = false;
			depressedSetPagesList(depressed_app.pages_list, depressed_app.document);
		}
	}

	IupDestroy(opendlg);

	return IUP_DEFAULT;
}

static int depressedProjectSaveCallback(Ihandle *self)
{
	if(!depressed_app.filename) return IUP_DEFAULT;
	if(depressed_app.filename[0] == 0) return IUP_DEFAULT;

	if(!Depressed::SaveDied(depressed_app.filename, depressed_app.document))
	{
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't save file");
	} else
		depressed_app.document_changed = false;

	return IUP_DEFAULT;
}

static int depressedProjectSaveAsCallback(Ihandle *self)
{
	Ihandle *savedlg;

	savedlg = IupFileDlg();
	if(!savedlg) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open file dialog");
	}

	IupSetAttribute(savedlg, "DIALOGTYPE", "SAVE");
	IupSetAttribute(savedlg, "EXTFILTER", "DepressED project (*.died)|*.died|All files (*.*)|*.*|");

	IupPopup(savedlg, IUP_CENTER, IUP_CENTER);

	if(IupGetInt(savedlg, "STATUS") != -1) {
		char *cfilename;
		size_t cfilename_length;
		wchar_t *wfilename;

		cfilename = IupGetAttribute(savedlg, "VALUE");
		cfilename_length = strlen(cfilename) + 1;
		wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

		MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

		if(depressed_app.filename) free(depressed_app.filename);
		depressed_app.filename = wfilename;

		IupDestroy(savedlg);

		return depressedProjectSaveCallback(self);
	}

	IupDestroy(savedlg);

	return IUP_DEFAULT;
}

static int depressedProjectDocumentFlagsCallback(Ihandle *self)
{
	depress_document_flags_type flags;

	flags = depressed_app.document.GetDocumentFlags();
	if(depressedShowDocumentFlagsDlg(flags)) {
		depressed_app.document.SetDocumentFlags(flags);
		depressed_app.document_changed = true;
	}

	return IUP_DEFAULT;
}

static int depressedProjectDefaultPageFlagsCallback(Ihandle *self)
{
	depress_flags_type flags;

	flags = depressed_app.document.GetGlobalPageFlags();

	if(depressedShowPageFlagsDlg(flags)) {
		depressed_app.document.SetGlobalPageFlags(flags);
		depressed_app.document_changed = true;
	}

	return IUP_DEFAULT;
}

static int depressedProjectCreateDocumentCallback(Ihandle *self)
{
	Ihandle *savedlg;

	if(!depressedDoBeforeNewOrSave(self)) return IUP_DEFAULT;

	savedlg = IupFileDlg();
	if(!savedlg) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open file dialog");
	}

	IupSetAttribute(savedlg, "DIALOGTYPE", "SAVE");
	IupSetAttribute(savedlg, "EXTFILTER", "Djvu file (*.djvu)|*.djvu|All files (*.*)|*.*|");

	IupPopup(savedlg, IUP_CENTER, IUP_CENTER);

	if(IupGetInt(savedlg, "STATUS") != -1) {
		char *cfilename;
		size_t cfilename_length;
		wchar_t *wfilename;

		cfilename = IupGetAttribute(savedlg, "VALUE");
		cfilename_length = strlen(cfilename) + 1;
		wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

		MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

		if(depressed_app.document.Process(wfilename) == Depressed::DocumentProcessStatus::OK)
			IupMessage(DEPRESSED_APP_TITLE, "DJVU Saved");
		else
			IupMessage(DEPRESSED_APP_TITLE, "Can't save DJVU");

		free(wfilename);
	}

	IupDestroy(savedlg);

	return IUP_DEFAULT;
}

static int depressedProjectExitCallback(Ihandle *self)
{
	if(!depressedDoBeforeNewOrSave(self)) return IUP_DEFAULT;

	return IUP_CLOSE;
}

static bool depressedPagesInsert(int value)
{
	Ihandle *adddlg;
	bool result = false;

	adddlg = IupFileDlg();
	if(!adddlg) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open file dialog");

		return result;
	}

	IupSetAttribute(adddlg, "DIALOGTYPE", "OPEN");
	IupSetAttribute(adddlg, "EXTFILTER", "Images (*.jpg;*.bmp;*.png)|*.jpg;*.bmp;*.png|All files (*.*)|*.*|");

	IupPopup(adddlg, IUP_CENTER, IUP_CENTER);

	if(IupGetInt(adddlg, "STATUS") != -1) {
		char *cfilename;
		size_t cfilename_length;
		wchar_t *wfilename;
		

		cfilename = IupGetAttribute(adddlg, "VALUE");
		cfilename_length = strlen(cfilename) + 1;
		wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

		MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

		try {
			Depressed::CPage *page;
			depress_flags_type flags, global_flags;

			page = new Depressed::CPage();

			page->Create();

			global_flags = depressed_app.document.GetGlobalPageFlags();
			if(depressCopyPageFlags(&flags, &global_flags)) { // Assume page flags are empty after creation, so no free needed
				page->SetFlags(flags);
			}

			if(page->SetFilename(wfilename, nullptr)) {
				if(value < 1 || (size_t)value > depressed_app.document.PagesCount())
					result = depressed_app.document.PageAdd(page);
				else
					result = depressed_app.document.PageInsert(page, (size_t)(value-1));
			}

			if(!result)
				delete page;
		} catch(std::bad_alloc) {
		}

		free(wfilename);
	}

	IupDestroy(adddlg);

	if(result) {
		char *filename;
		Depressed::CPage *page;
		size_t id;

		if(value < 1 || (size_t)value > depressed_app.document.PagesCount()) {
			id = depressed_app.document.PagesCount()-1;
			value = (int)depressed_app.document.PagesCount();
		} else {
			id = (size_t)value - 1;
		}

		page = depressed_app.document.PageGet((size_t)(value-1));
		if(page) {
			filename = page->GetFilenameU8();
			if(!filename) filename = depressed_text_error_cant_show_pages_name;
		} else
			filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(depressed_app.pages_list, "INSERTITEM", value, filename);
	}

	return result;
}

static int depressedPagesAddCallback(Ihandle *self)
{
	if(!depressedPagesInsert(0)) {
		IupMessage(DEPRESSED_APP_TITLE, "Page wasn't added");
	}

	return IUP_DEFAULT;
}

static int depressedPagesDeleteCallback(Ihandle *self)
{
	int value;
	size_t id;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if(value < 1 || (size_t)value > depressed_app.document.PagesCount()) return IUP_DEFAULT;
	id = value - 1;

	if(depressed_app.document.PageDelete(id)) {
		IupSetInt(depressed_app.pages_list, "REMOVEITEM", value);
	}

	return IUP_DEFAULT;
}

static int depressedPagesInsertCallback(Ihandle *self)
{
	int value;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if(!depressedPagesInsert(value)) {
		IupMessage(DEPRESSED_APP_TITLE, "Page wasn't inserted");
	}

	return IUP_DEFAULT;
}

static int depressedPagesMoveUpCallback(Ihandle *self)
{
	int value;
	size_t id;
	Depressed::CPage *page;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if(value <= 1 || (size_t)value > depressed_app.document.PagesCount()) return IUP_DEFAULT;
	id = value - 1;

	if(!depressed_app.document.PageSwap(id-1, id)) return IUP_DEFAULT;

	page = depressed_app.document.PageGet(id);
	if(page) {
		char *filename;
		filename = page->GetFilenameU8();
		if(!filename) filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(depressed_app.pages_list, "", value, filename);
	} else
		IupSetAttributeId(depressed_app.pages_list, "", value, depressed_text_error_cant_show_pages_name);

	page = depressed_app.document.PageGet(id-1);
	if(page) {
		char *filename;
		filename = page->GetFilenameU8();
		if(!filename) filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(depressed_app.pages_list, "", value-1, filename);
	} else
		IupSetAttributeId(depressed_app.pages_list, "", value-1, depressed_text_error_cant_show_pages_name);

	IupSetInt(depressed_app.pages_list, "VALUE", value-1);

	return IUP_DEFAULT;
}

static int depressedPagesMoveDownCallback(Ihandle *self)
{
	int value;
	size_t id;
	Depressed::CPage *page;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if (value < 1 || (size_t)value >= depressed_app.document.PagesCount()) return IUP_DEFAULT;
	id = value - 1;

	if(!depressed_app.document.PageSwap(id, id+1)) return IUP_DEFAULT;

	page = depressed_app.document.PageGet(id);
	if(page) {
		char *filename;
		filename = page->GetFilenameU8();
		if(!filename) filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(depressed_app.pages_list, "", value, filename);
	} else
		IupSetAttributeId(depressed_app.pages_list, "", value, depressed_text_error_cant_show_pages_name);

	page = depressed_app.document.PageGet(id+1);
	if(page) {
		char *filename;
		filename = page->GetFilenameU8();
		if(!filename) filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(depressed_app.pages_list, "", value+1, filename);
	} else
		IupSetAttributeId(depressed_app.pages_list, "", value+1, depressed_text_error_cant_show_pages_name);

	IupSetInt(depressed_app.pages_list, "VALUE", value+1);

	return IUP_DEFAULT;
}

static int depressedPagesFlagsCallback(Ihandle *self)
{
	int value;
	size_t id;
	depress_flags_type flags;
	Depressed::CPage *page;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if(value < 1 || (size_t)value > depressed_app.document.PagesCount()) return IUP_DEFAULT;
	id = value - 1;

	page = depressed_app.document.PageGet(id);
	if(!page) return IUP_DEFAULT;

	flags = page->GetFlags();
	if(depressedShowPageFlagsDlg(flags)) {
		char* value;

		page->SetFlags(flags);
		depressed_app.document_changed = true;

		value = IupGetAttribute(depressed_app.preview_toggle, "VALUE"); // ON, OFF
		if(strcmp(value, "ON") == 0) depressedPreviewUpdateImage(true);
	}

	return IUP_DEFAULT;
}


static int depressedHelpLicenseCallback(Ihandle *self)
{
	IupMessage(DEPRESSED_APP_TITLE,
		"Depress[ed] - BSD\n"
		"Djvul - Unlicense\n"
		"stb - Unlicense/MIT\n"
		"noteshrink-c - MIT\n"
		"djvulibre - GNU GPL 2"
	);

	return IUP_DEFAULT;
}

static int depressedHelpAboutCallback(Ihandle *self)
{
	IupMessage(DEPRESSED_APP_TITLE,
		"Depress[ed] (c) Morozov Mikhail and contributors\n\n"
		"Software to automatize creation of djvu files"
	);

	return IUP_DEFAULT;
}

static void depressedPreviewUpdateImage(bool preview_result)
{
	int value;
	Depressed::CPage *page = 0;
	wchar_t *filename = 0;
	Ihandle *new_image = 0;

	value = IupGetInt(depressed_app.pages_list, "VALUE");

	if(value > 0 && (size_t)value <= depressed_app.document.PagesCount()) {
		size_t id;

		id = (size_t)value - 1;
		page = depressed_app.document.PageGet(id);
	}

	if(page) {
		filename = page->GetFilename();
	}

	if(filename) {
		unsigned char *buf;
		int sizex, sizey, channels;

		if(!preview_result) {
			FILE* f;
			f = _wfopen(filename, L"rb");
			if(!f) goto FINAL;
			buf = depressLoadImage(f, &sizex, &sizey, &channels, 3);
			if(!buf) goto FINAL;
		} else {
			if(!depressLoadImageFromFileAndApplyFlags(filename, &sizex, &sizey, &channels, &buf, page->GetFlags())) goto FINAL;
		}

		if(channels == 1) {
#if 0
			new_image = IupImage(sizex, sizey, buf);
			for(int i = 0; i < 256; i++) {
				char col[16], index[16];

				sprintf(index, "%d", i);
				sprintf(col, "%d %d %d", i, i, i);
				IupSetAttribute(new_image, index, col);
			}
#else
			unsigned char *new_buf;

			if(sizex*3 > INT_MAX || INT_MAX/sizey < sizex*3) {
				free(buf);

				goto FINAL;
			}
			new_buf = (unsigned char *)malloc(sizex*sizey*3);
			if(!new_buf) {
				free(buf);

				goto FINAL;
			}
			for(int i = 0; i < sizex*sizey; i++) {
				new_buf[i*3] = new_buf[i*3+1] = new_buf[i*3+2] = buf[i];
			}
			free(buf);
			buf = new_buf;
			new_image = IupImageRGB(sizex, sizey, buf);
#endif
		} else if(channels == 3) {
			new_image = IupImageRGB(sizex, sizey, buf);
		} else {
			free(buf);
			goto FINAL;
		}

		free(buf);
	}

FINAL:
	IupSetHandle("preview_image", NULL);
	if(depressed_app.preview_image) IupDestroy(depressed_app.preview_image);
	auto h = IupSetHandle("preview_image", new_image);
	IupUpdate(depressed_app.preview_canvas);
	//if(depressed_app.preview_image) IupDestroy(depressed_app.preview_image);
	depressed_app.preview_image = new_image;
}

static int depressedPreviewToggleCallback(Ihandle *self)
{
	char *value;
	
	value = IupGetAttribute(self, "VALUE"); // ON, OFF

	depressedPreviewUpdateImage(strcmp(value, "ON") == 0);

	return IUP_DEFAULT;
}

static int depressedPreviewCanvasActionCallback(Ihandle *self, float posx, float posy)
{
	int w, h;

	IupDrawBegin(self);

	IupDrawParentBackground(self);

	IupDrawGetSize(self, &w, &h);

	IupDrawImage(self, "preview_image", 0, 0, w, h);

	IupDrawEnd(self);

	return IUP_DEFAULT;
}

static int depressedPagesListValueChangedCallback(Ihandle* self)
{
	char *value;

	(void)self;

	value = IupGetAttribute(depressed_app.preview_toggle, "VALUE"); // ON, OFF

	depressedPreviewUpdateImage(strcmp(value, "ON") == 0);

	return IUP_DEFAULT;
}

bool depressedCreateMainDlgMenu(void)
{
	// Project submenu
	depressed_app.item_project_new = IupItem("New", NULL);
	depressed_app.item_project_sep1 = IupSeparator();
	depressed_app.item_project_open = IupItem("Open", NULL);
	depressed_app.item_project_save = IupItem("Save", NULL);
	depressed_app.item_project_save_as = IupItem("Save As", NULL);
	depressed_app.item_project_sep2 = IupSeparator();
	depressed_app.item_project_document_flags = IupItem("Document flags", NULL);
	depressed_app.item_project_default_page_flags = IupItem("Default page flags", NULL);
	depressed_app.item_project_sep3 = IupSeparator();
	depressed_app.item_project_create_document = IupItem("Create Document", NULL);
	depressed_app.item_project_sep4 = IupSeparator();
	depressed_app.item_project_exit = IupItem("Exit", NULL);

	if(!depressed_app.item_project_new || !depressed_app.item_project_sep1 ||
		!depressed_app.item_project_open || !depressed_app.item_project_save || !depressed_app.item_project_save_as ||
		!depressed_app.item_project_sep2 || !depressed_app.item_project_document_flags || !depressed_app.item_project_default_page_flags ||
		!depressed_app.item_project_sep3 || !depressed_app.item_project_create_document ||
		!depressed_app.item_project_sep4 || !depressed_app.item_project_exit)
		return false;
	
	IupSetCallback(depressed_app.item_project_new, "ACTION", (Icallback)depressedProjectNewCallback);
	IupSetCallback(depressed_app.item_project_open, "ACTION", (Icallback)depressedProjectOpenCallback);
	IupSetCallback(depressed_app.item_project_save, "ACTION", (Icallback)depressedProjectSaveCallback);
	IupSetCallback(depressed_app.item_project_save_as, "ACTION", (Icallback)depressedProjectSaveAsCallback);
	IupSetCallback(depressed_app.item_project_document_flags, "ACTION", (Icallback)depressedProjectDocumentFlagsCallback);
	IupSetCallback(depressed_app.item_project_default_page_flags, "ACTION", (Icallback)depressedProjectDefaultPageFlagsCallback);
	IupSetCallback(depressed_app.item_project_create_document, "ACTION", (Icallback)depressedProjectCreateDocumentCallback);
	IupSetCallback(depressed_app.item_project_exit, "ACTION", (Icallback)depressedProjectExitCallback);

	depressed_app.menu_project = IupMenu(
		depressed_app.item_project_new,
		depressed_app.item_project_sep1,
		depressed_app.item_project_open,
		depressed_app.item_project_save,
		depressed_app.item_project_save_as,
		depressed_app.item_project_sep2,
		depressed_app.item_project_document_flags,
		depressed_app.item_project_default_page_flags,
		depressed_app.item_project_sep3,
		depressed_app.item_project_create_document,
		depressed_app.item_project_sep4,
		depressed_app.item_project_exit,
		NULL
	);
	if(!depressed_app.menu_project) return false;
	depressed_app.submenu_project = IupSubmenu("Project", depressed_app.menu_project);
	if(!depressed_app.submenu_project) return false;

	// Page submenu
	depressed_app.item_pages_add = IupItem("Add", NULL);
	depressed_app.item_pages_delete = IupItem("Delete", NULL);
	depressed_app.item_pages_insert = IupItem("Insert", NULL);
	depressed_app.item_pages_moveup = IupItem("Move Up", NULL);
	depressed_app.item_pages_movedown = IupItem("Move Down", NULL);
	depressed_app.item_pages_flags = IupItem("Flags", NULL);

	if(!depressed_app.item_pages_add || !depressed_app.item_pages_delete || !depressed_app.item_pages_insert ||
		!depressed_app.item_pages_moveup || !depressed_app.item_pages_movedown ||
		!depressed_app.item_pages_flags)
		return false;

	depressed_app.menu_pages = IupMenu(
		depressed_app.item_pages_add,
		depressed_app.item_pages_delete,
		depressed_app.item_pages_insert,
		depressed_app.item_pages_moveup,
		depressed_app.item_pages_movedown,
		depressed_app.item_pages_flags,
		NULL
	);
	if(!depressed_app.menu_pages) return false;
	depressed_app.submenu_pages = IupSubmenu("Pages", depressed_app.menu_pages);
	if(!depressed_app.submenu_pages) return false;

	IupSetCallback(depressed_app.item_pages_add, "ACTION", (Icallback)depressedPagesAddCallback);
	IupSetCallback(depressed_app.item_pages_delete, "ACTION", (Icallback)depressedPagesDeleteCallback);
	IupSetCallback(depressed_app.item_pages_insert, "ACTION", (Icallback)depressedPagesInsertCallback);
	IupSetCallback(depressed_app.item_pages_moveup, "ACTION", (Icallback)depressedPagesMoveUpCallback);
	IupSetCallback(depressed_app.item_pages_movedown, "ACTION", (Icallback)depressedPagesMoveDownCallback);
	IupSetCallback(depressed_app.item_pages_flags, "ACTION", (Icallback)depressedPagesFlagsCallback);

	// Help menu
	depressed_app.item_help_license = IupItem("License", NULL);
	depressed_app.item_help_about = IupItem("About", NULL);

	if(!depressed_app.item_help_license || !depressed_app.item_help_about) return false;

	IupSetCallback(depressed_app.item_help_license, "ACTION", (Icallback)depressedHelpLicenseCallback);
	IupSetCallback(depressed_app.item_help_about, "ACTION", (Icallback)depressedHelpAboutCallback);

	depressed_app.menu_help = IupMenu(
		depressed_app.item_help_license,
		depressed_app.item_help_about,
		NULL
	);
	if(!depressed_app.menu_help) return false;
	depressed_app.submenu_help = IupSubmenu("Help", depressed_app.menu_help);
	if(!depressed_app.submenu_help) return false;

	depressed_app.main_menu = IupMenu(
		depressed_app.submenu_project,
		depressed_app.submenu_pages,
		depressed_app.submenu_help,
		NULL
	);
	if(!depressed_app.main_menu) return false;

	return true;
}

static void depressedSetPagesList(Ihandle *pages_list, Depressed::CDocument &document)
{
	//IupSetAttribute(pages_list, "1", NULL);
	IupSetAttribute(pages_list, "REMOVEITEM", "ALL");

	for(size_t i = 0; i < document.PagesCount(); i++) {
		char *filename;
		Depressed::CPage *page;

		page = document.PageGet(i);
		filename = page->GetFilenameU8();
		if(!filename) filename = depressed_text_error_cant_show_pages_name;
		IupSetAttributeId(pages_list, "", i+1, filename);
	}
}

bool depressedCreateMainDlg(void)
{
	if(!depressedCreateMainDlgMenu()) return false;

	if((depressed_app.pages_list = IupList(NULL)) == NULL) goto FAIL;
	IupSetAttribute(depressed_app.pages_list, "EXPAND", "VERTICAL");
	IupSetAttribute(depressed_app.pages_list, "SIZE", "320x");
	IupSetCallback(depressed_app.pages_list, "VALUECHANGED_CB", depressedPagesListValueChangedCallback);

	if((depressed_app.preview_flags_button = IupButton("Flags", NULL)) == NULL) goto FAIL;
	IupSetCallback(depressed_app.preview_flags_button, "ACTION", depressedPagesFlagsCallback);

	if((depressed_app.preview_toggle = IupToggle("Preview", NULL)) == NULL) goto FAIL;
	IupSetCallback(depressed_app.preview_toggle, "VALUECHANGED_CB", depressedPreviewToggleCallback);

	if((depressed_app.preview_canvas = IupCanvas(NULL)) == NULL) goto FAIL;
	IupSetAttribute(depressed_app.preview_canvas, "DRAWUSEDIRECT2D", "YES");
	IupSetCallback(depressed_app.preview_canvas, "ACTION", (Icallback)depressedPreviewCanvasActionCallback);

	if((depressed_app.preview_inner_hbox = IupHbox(depressed_app.preview_toggle, depressed_app.preview_flags_button, NULL)) == NULL) goto FAIL;
	if((depressed_app.preview_outer_vbox = IupVbox(depressed_app.preview_inner_hbox, depressed_app.preview_canvas, NULL)) == NULL) goto FAIL;

	if((depressed_app.main_hbox = IupHbox(depressed_app.pages_list, depressed_app.preview_outer_vbox, NULL)) == NULL) goto FAIL;

	if((depressed_app.main_dlg = IupDialog(depressed_app.main_hbox)) == NULL) goto FAIL;
	IupSetAttribute(depressed_app.main_dlg, "TITLE", DEPRESSED_APP_TITLE);
	IupSetAttributeHandle(depressed_app.main_dlg, "MENU", depressed_app.main_menu);

	depressedSetPagesList(depressed_app.pages_list, depressed_app.document);

	return true;

FAIL:
	if(depressed_app.main_dlg) IupDestroy(depressed_app.main_dlg);
	else {
		if(depressed_app.main_hbox) IupDestroy(depressed_app.main_hbox);
		else {
			if(depressed_app.pages_list) IupDestroy(depressed_app.pages_list);

			if(depressed_app.preview_outer_vbox) IupDestroy(depressed_app.preview_outer_vbox);
			else {
				if(depressed_app.preview_inner_hbox) IupDestroy(depressed_app.preview_inner_hbox);
				else {
					if(depressed_app.preview_toggle) IupDestroy(depressed_app.preview_toggle);
					if(depressed_app.preview_flags_button) IupDestroy(depressed_app.preview_flags_button);
				}
			}
		}
	}

	return false;
}
