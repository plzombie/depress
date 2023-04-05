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

#ifndef DEPRESSED_APP_H
#define DEPRESSED_APP_H

#include <iup.h>

#include "../include/depressed_document.h"

struct SDepressedAppGuiPageFlags
{
	Ihandle *hbox_params_param1;
	Ihandle *hbox_params_param2;
	Ihandle *param1_label, *param1;
	Ihandle *param2_label, *param2;
};

struct SDepressedApp {
	Depressed::CDocument document;
	bool document_changed;

	wchar_t *filename;

	// Main window
	Ihandle *main_dlg, *main_menu, *main_box;
	// Main dialog - Menus
	// Project submenu
	Ihandle *submenu_project;
	Ihandle *menu_project;
	Ihandle *item_project_new;
	Ihandle *item_project_sep1;
	Ihandle *item_project_open, *item_project_save, *item_project_save_as;
	Ihandle *item_project_sep2;
	Ihandle *item_project_document_flags, *item_project_default_page_flags;
	Ihandle *item_project_sep3;
	Ihandle *item_project_create_document;
	Ihandle *item_project_sep4;
	Ihandle *item_project_exit;
	// Pages submenu
	Ihandle *submenu_pages, *menu_pages;
	Ihandle *item_pages_add, *item_pages_delete, *item_pages_insert;
	Ihandle *item_pages_moveup, *item_pages_movedown;
	Ihandle *item_pages_flags;
	// Help menu
	Ihandle *submenu_help, *menu_help;
	Ihandle *item_help_license, *item_help_about;
	// On Main dialog
	Ihandle *input_box, *input_label, *input_filename, *input_button;
	Ihandle *output_box, *output_label, *output_filename, *output_button;

	SDepressedAppGuiPageFlags gui_pageflags;
} ;

extern struct SDepressedApp depressed_app;

extern const char *DEPRESSED_APP_TITLE;

#endif
