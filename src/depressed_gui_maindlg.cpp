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

static int depressedLoadProjectCallback(Ihandle *self)
{
	char* cfilename;
	size_t cfilename_length;
	wchar_t* wfilename;

	cfilename = IupGetAttribute(depressed_app.input_filename, "VALUE");
	cfilename_length = strlen(cfilename) + 1;
	wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

	IupMessage("Project filename", cfilename);

	if(!Depressed::OpenDied(wfilename, depressed_app.document))
		IupMessage(DEPRESSED_APP_TITLE, "Can't open project");

	free(wfilename);

	return IUP_DEFAULT;
}

static int depressedSaveDjvuCallback(Ihandle *self)
{
	char* cfilename;
	size_t cfilename_length;
	wchar_t* wfilename;

	cfilename = IupGetAttribute(depressed_app.output_filename, "VALUE");
	cfilename_length = strlen(cfilename) + 1;
	wfilename = (wchar_t*)malloc(cfilename_length * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, (int)cfilename_length);

	IupMessage("Project filename", cfilename);

	if (depressed_app.document.Process(wfilename) == Depressed::DocumentProcessStatus::OK)
		IupMessage(DEPRESSED_APP_TITLE, "DJVU Saved");
	else
		IupMessage(DEPRESSED_APP_TITLE, "Can't save DJVU");

	free(wfilename);

	return IUP_DEFAULT;
}

static int depressedProjectSaveCallback(Ihandle *self);
static int depressedProjectSaveAsCallback(Ihandle *self);

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
		}

		IupDestroy(opendlg);
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
	} else
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't open file dialog");

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
		IupDestroy(savedlg);
	}

	IupDestroy(savedlg);

	return IUP_DEFAULT;
}

static int depressedProjectExitCallback(Ihandle *self)
{
	if(!depressedDoBeforeNewOrSave(self)) return IUP_DEFAULT;

	return IUP_CLOSE;
}

static int depressedHelpLicenseCallback(Ihandle *self)
{
	IupMessage(DEPRESSED_APP_TITLE,
		"Depress[ed] - BSD\n"
		"Djvul - Unlicense\n"
		"stb - Unlicense/MIT\n"
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

bool depressedCreateMainDlg(void)
{
	if(!depressedCreateMainDlgMenu()) return false;

	depressed_app.input_label = IupLabel("Project file:");
	depressed_app.input_filename = IupText(NULL);
	IupSetAttribute(depressed_app.input_filename, "EXPAND", "YES");
	depressed_app.input_button = IupButton("Load", NULL);
	
	depressed_app.input_box = IupHbox(depressed_app.input_label, depressed_app.input_filename, depressed_app.input_button, NULL);
	
	depressed_app.output_label = IupLabel("Djvu file:");
	depressed_app.output_filename = IupText(NULL);
	IupSetAttribute(depressed_app.output_filename, "EXPAND", "YES");
	depressed_app.output_button = IupButton("Save", NULL);
	depressed_app.output_box = IupHbox(depressed_app.output_label, depressed_app.output_filename, depressed_app.output_button, NULL);

	depressed_app.main_box = IupVbox(depressed_app.input_box, depressed_app.output_box, NULL);
	depressed_app.main_dlg = IupDialog(depressed_app.main_box);
	IupSetAttribute(depressed_app.main_dlg, "TITLE", DEPRESSED_APP_TITLE);
	IupSetAttributeHandle(depressed_app.main_dlg, "MENU", depressed_app.main_menu);

	IupSetCallback(depressed_app.input_button, "ACTION", depressedLoadProjectCallback);
	IupSetCallback(depressed_app.output_button, "ACTION", depressedSaveDjvuCallback);

	return true;
}
