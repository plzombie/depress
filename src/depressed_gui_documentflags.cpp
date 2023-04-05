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

#include "../include/depressed_app.h"
#include "../include/depressed_gui_documentflags.h"

static int depressedDocumentFlagsOkCallback(Ihandle *self)
{
	IupSetAttribute(IupGetDialog(self), "STATUS", "1");

	return IUP_CLOSE;
}

static int depressedDocumentFlagsCancelCallback(Ihandle *self)
{
	IupSetAttribute(IupGetDialog(self), "STATUS", "0");

	return IUP_CLOSE;
}

bool depressedShowDocumentFlagsDlg(depress_document_flags_type &flags)
{
	bool result = false;
	Ihandle *dlg = 0, *vbox_main = 0,
		*hbox_ptt,
		*hbox_buttons = 0,
		*ptt, *ptt_flags,
		*btn_fill = 0, *btn_ok = 0, *btn_cancel = 0;

	ptt = IupToggle("Use authomatic page title (APT)", NULL);
	ptt_flags = IupToggle("Short APT", NULL);

	btn_ok = IupButton("OK", NULL);
	btn_cancel = IupButton("Cancel", NULL);

	IupSetCallback(btn_ok, "ACTION", (Icallback)depressedDocumentFlagsOkCallback);
	IupSetCallback(btn_cancel, "ACTION", (Icallback)depressedDocumentFlagsCancelCallback);

	hbox_ptt = IupHbox(ptt, ptt_flags, NULL);

	if((btn_fill = IupFill()) == NULL) goto FAIL;
	hbox_buttons = IupHbox(btn_fill, btn_ok, btn_cancel, NULL);
	IupSetAttributes(hbox_buttons, "NORMALIZESIZE=HORIZONTAL");

	vbox_main = IupVbox(hbox_ptt, hbox_buttons, NULL);

	// Set default values
	IupSetInt(ptt, "VALUE", flags.page_title_type);
	IupSetInt(ptt_flags, "VALUE", flags.page_title_type_flags);

	dlg = IupDialog(vbox_main);

	IupSetAttribute(dlg, "TITLE", "Document flags");
	IupSetAttributeHandle(dlg, "DEFAULTENTER", btn_ok);
	IupSetAttributeHandle(dlg, "DEFAULTESC", btn_cancel);

	IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if(IupGetInt(dlg, "STATUS") == 1) {
		result = true;

		flags.page_title_type = IupGetInt(ptt, "VALUE");
		flags.page_title_type_flags = IupGetInt(ptt_flags, "VALUE");
	}

	IupDestroy(dlg);

	return result;

FAIL:
	if(vbox_main) {
		IupDestroy(vbox_main);
	} else {
		if(hbox_ptt) {
			IupDestroy(hbox_ptt);
		} else {
			if(ptt) IupDestroy(ptt);
			if(ptt_flags) IupDestroy(ptt_flags);
		}

		if(hbox_buttons) {
			IupDestroy(hbox_buttons);
		} else {
			if(btn_fill) IupDestroy(btn_fill);
			if(btn_ok) IupDestroy(btn_ok);
			if(btn_cancel) IupDestroy(btn_cancel);
		}
	}

	MessageBoxW(NULL, L"Depressed", L"Can't open window", MB_ICONERROR | MB_OK);

	return false;
}