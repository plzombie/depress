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
#include "../include/depressed_gui_pageflags.h"

static int depressedPageFlagsOkCallback(Ihandle *self)
{
	IupSetAttribute(IupGetDialog(self), "STATUS", "1");

	return IUP_CLOSE;
}

static int depressedPageFlagsCancelCallback(Ihandle *self)
{
	IupSetAttribute(IupGetDialog(self), "STATUS", "0");

	return IUP_CLOSE;
}


bool depressedShowPageFlagsDlg(depress_flags_type &flags)
{
	bool result = false;
	Ihandle *dlg = 0, *vbox_main = 0,
		*hbox_params_type = 0,
		*hbox_params_param1 = 0,
		*hbox_params_param2 = 0,
		*hbox_params_quality = 0,
		*hbox_params_dpi = 0,
		*hbox_buttons = 0,
		*type_label = 0, *type = 0,
		*param1_label = 0, *param1 = 0,
		*param2_label = 0, *param2 = 0,
		*quality_label = 0, *quality = 0,
		*dpi_label, *dpi,
		*btn_ok = 0, *btn_cancel = 0;

	type_label = IupLabel("Page type:");
	IupSetAttribute(type_label, "EXPAND", "YES");
	type = IupText(NULL);
	IupSetAttribute(type, "MASK", IUP_MASK_UINT);
	IupSetAttribute(type, "VISIBLECOLUMNS", "3");

	param1_label = IupLabel("Parameter 1:");
	IupSetAttribute(param1_label, "EXPAND", "YES");
	param1 = IupText(NULL);
	IupSetAttribute(param1, "MASK", IUP_MASK_INT);
	IupSetAttribute(param1, "VISIBLECOLUMNS", "3");

	param2_label = IupLabel("Parameter 2:");
	IupSetAttribute(param2_label, "EXPAND", "YES");
	param2 = IupText(NULL);
	IupSetAttribute(param2, "MASK", IUP_MASK_INT);
	IupSetAttribute(param2, "VISIBLECOLUMNS", "3");

	quality_label = IupLabel("Quality (1..100):");
	IupSetAttribute(quality_label, "EXPAND", "YES");
	quality = IupText(NULL);
	IupSetAttribute(quality, "MASK", IUP_MASK_UINT);
	IupSetAttribute(quality, "VISIBLECOLUMNS", "3");

	dpi_label = IupLabel("DPI:");
	IupSetAttribute(dpi_label, "EXPAND", "YES");
	dpi = IupText(NULL);
	IupSetAttribute(dpi, "MASK", IUP_MASK_INT);
	IupSetAttribute(dpi, "VISIBLECOLUMNS", "4");

	btn_ok = IupButton("OK", NULL);
	btn_cancel = IupButton("Cancel", NULL);

	IupSetCallback(btn_ok, "ACTION", (Icallback)depressedPageFlagsOkCallback);
	IupSetCallback(btn_cancel, "ACTION", (Icallback)depressedPageFlagsCancelCallback);
	
	hbox_params_type = IupHbox(type_label, type, NULL);
	hbox_params_param1 = IupHbox(param1_label, param1, NULL);
	hbox_params_param2 = IupHbox(param2_label, param2, NULL);
	hbox_params_quality = IupHbox(quality_label, quality, NULL);
	hbox_params_dpi = IupHbox(dpi_label, dpi, NULL);

	hbox_buttons = IupHbox(IupFill(), btn_ok, btn_cancel, NULL);
	IupSetAttributes(hbox_buttons, "NORMALIZESIZE=HORIZONTAL");

	vbox_main = IupVbox(hbox_params_type, hbox_params_param1, hbox_params_param2, hbox_params_quality, hbox_params_dpi, hbox_buttons, NULL);

// Set default values
	IupSetInt(type, "VALUE", flags.type);
	IupSetInt(param1, "VALUE", flags.param1);
	IupSetInt(param2, "VALUE", flags.param2);
	IupSetInt(quality, "VALUE", flags.quality);
	IupSetInt(dpi, "VALUE", flags.dpi);

	dlg = IupDialog(vbox_main);

	IupSetAttribute(dlg, "TITLE", "Page flags");
	IupSetAttributeHandle(dlg, "DEFAULTENTER", btn_ok);
	IupSetAttributeHandle(dlg, "DEFAULTESC", btn_cancel);

	IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if(IupGetInt(dlg, "STATUS") == 1) {
		result = true;

		flags.type = IupGetInt(type, "VALUE");
		flags.param1 = IupGetInt(param1, "VALUE");
		flags.param2 = IupGetInt(quality, "VALUE");
		flags.quality = IupGetInt(quality, "VALUE");
		flags.dpi = IupGetInt(dpi, "VALUE");
	}

	IupDestroy(dlg);

	return result;
}
