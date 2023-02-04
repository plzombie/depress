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

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, cfilename_length);

	IupMessage("Project filename", cfilename);

	if (!Depressed::OpenDied(wfilename, depressed_app.document))
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

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, cfilename_length);

	IupMessage("Project filename", cfilename);

	if (depressed_app.document.Process(wfilename) == Depressed::DocumentProcessStatus::OK)
		IupMessage(DEPRESSED_APP_TITLE, "DJVU Saved");
	else
		IupMessage(DEPRESSED_APP_TITLE, "Can't save DJVU");

	free(wfilename);

	return IUP_DEFAULT;
}

bool depressedDoBeforeNewOrSave(void)
{
	if (depressed_app.document_changed) return true;

	depressed_app.document_changed = false;

	return true;
}

static int depressedProjectNewCallback(Ihandle *self)
{
	if (!depressedDoBeforeNewOrSave()) return IUP_DEFAULT;

	depressed_app.document.Destroy();
	if (!depressed_app.document.Create()) {
		IupMessage(DEPRESSED_APP_TITLE, "Error: Can't create new document");

		return IUP_CLOSE;
	}

	return IUP_DEFAULT;
}

static int depressedProjectCreateDocumentCallback(Ihandle *self)
{
	return depressedSaveDjvuCallback(self);
}

static int depressedProjectExitCallback(Ihandle *self)
{
	if (!depressedDoBeforeNewOrSave()) return IUP_DEFAULT;

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
