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

static void depressedPageChangeTextsByType(SDepressedAppGuiPageFlags &gui_pageflags, int type)
{
	bool show_param1 = false, show_param2 = false;

	switch(type) {
		case 1:
			show_param1 = true;
			IupSetAttribute(gui_pageflags.param1_label, "TITLE", "Type of binarization");
			IupRefresh(gui_pageflags.param1_label);
			IupSetAttribute(gui_pageflags.param1, "TIP", "0 - threshold (default)\n1 - error diffusion\n2 - adaptive");
			break;
		case 2:
			show_param1 = true;
			show_param2 = true;
			IupSetAttribute(gui_pageflags.param1_label, "TITLE", "FG\\BG Downsampling ratio");
			IupRefresh(gui_pageflags.param1_label);
			IupSetAttribute(gui_pageflags.param2_label, "TITLE", "Further FG downsampling");
			IupRefresh(gui_pageflags.param2_label);
			IupSetAttribute(gui_pageflags.param1, "TIP", "1 and more, defaults to 3");
			IupSetAttribute(gui_pageflags.param2, "TIP", "1 and more, defaults to 2");
			break;
		case 3:
			show_param1 = true;
			show_param2 = true;
			IupSetAttribute(gui_pageflags.param1_label, "TITLE", "Colors");
			IupRefresh(gui_pageflags.param1_label);
			IupSetAttribute(gui_pageflags.param2_label, "TITLE", "Algorithm");
			IupRefresh(gui_pageflags.param2_label);
			IupSetAttribute(gui_pageflags.param1, "TIP", "Color number between 2 and 256 (defaults to 8)");
			IupSetAttribute(gui_pageflags.param2, "TIP", "0 - quantization (default)\n1 - noteshrink");
	}

	if(show_param1)
		IupSetAttribute(gui_pageflags.hbox_params_param1, "VISIBLE", "YES");
	else
		IupSetAttribute(gui_pageflags.hbox_params_param1, "VISIBLE", "NO");

	if(show_param2)
		IupSetAttribute(gui_pageflags.hbox_params_param2, "VISIBLE", "YES");
	else
		IupSetAttribute(gui_pageflags.hbox_params_param2, "VISIBLE", "NO");

}

static int depressedPageTypeConvertFromView(int view_type)
{
	switch(view_type) {
		case 1:
		case 2:
		case 3:
		case 4:
			return view_type-1;
		case 5:
			return DEPRESS_PAGE_TYPE_AUTO;
		default:
			return 0;
	}
}

static int depressedPageTypeChangedCallback(Ihandle *self)
{
	int type = depressedPageTypeConvertFromView(IupGetInt(self, "VALUE"));

	depressedPageChangeTextsByType(depressed_app.gui_pageflags, type);

	return 0;
}


bool depressedShowPageFlagsDlg(depress_flags_type &flags)
{
	bool result = false;
	Ihandle *dlg = 0, *vbox_main = 0,
		*hbox_params_type = 0,
		*hbox_params_quality = 0,
		*hbox_params_dpi = 0,
		*hbox_buttons = 0,
		*main_fill = 0,
		*type_label = 0, *type_fill = 0, *type = 0,
		*param1_fill = 0, *param2_fill = 0,
		*quality_label = 0, *quality_fill = 0, *quality = 0,
		*dpi_label = 0, *dpi_fill = 0, *dpi = 0,
		*btn_fill = 0, *btn_ok = 0, *btn_cancel = 0;

	memset(&depressed_app, 0, sizeof(SDepressedAppGuiPageFlags));

	// Define parameters

	// Define Type parameter
	if((type_label = IupLabel("Page type:")) == NULL) goto FAIL;
	if((type = IupList(NULL)) == NULL) goto FAIL;
	IupSetAttribute(type, "DROPDOWN", "YES");
	IupSetAttribute(type, "1", "Color");
	IupSetAttribute(type, "2", "Black&White");
	IupSetAttribute(type, "3", "Layered");
	IupSetAttribute(type, "4", "Palettized");
	IupSetAttribute(type, "5", "Auto");
	IupSetCallback(type, "VALUECHANGED_CB", (Icallback)depressedPageTypeChangedCallback);
	IupSetAttribute(type, "VISIBLECOLUMNS", "10");

	// Define Parameter 1
	if((depressed_app.gui_pageflags.param1_label = IupLabel("Parameter 1:")) == NULL) goto FAIL;
	if((depressed_app.gui_pageflags.param1 = IupText(NULL)) == NULL) goto FAIL;
	IupSetAttribute(depressed_app.gui_pageflags.param1, "MASK", IUP_MASK_INT);
	IupSetAttribute(depressed_app.gui_pageflags.param1, "VISIBLECOLUMNS", "3");

	// Define Parameter 2
	if((depressed_app.gui_pageflags.param2_label = IupLabel("Parameter 2:")) == NULL) goto FAIL;
	if((depressed_app.gui_pageflags.param2 = IupText(NULL)) == NULL) goto FAIL;
	IupSetAttribute(depressed_app.gui_pageflags.param2, "MASK", IUP_MASK_INT);
	IupSetAttribute(depressed_app.gui_pageflags.param2, "VISIBLECOLUMNS", "3");

	// Define Quality parameter
	if((quality_label = IupLabel("Quality (1..100):")) == NULL) goto FAIL;
	if((quality = IupText(NULL)) == NULL) goto FAIL;
	IupSetAttribute(quality, "MASK", IUP_MASK_UINT);
	IupSetAttribute(quality, "VISIBLECOLUMNS", "3");

	// Define DPI parameter
	if((dpi_label = IupLabel("DPI:")) == NULL) goto FAIL;
	if((dpi = IupText(NULL)) == NULL) goto FAIL;
	IupSetAttribute(dpi, "MASK", IUP_MASK_INT);
	IupSetAttribute(dpi, "VISIBLECOLUMNS", "4");

	// Define buttons
	if((btn_ok = IupButton("OK", NULL)) == NULL) goto FAIL;
	if((btn_cancel = IupButton("Cancel", NULL)) == NULL) goto FAIL;

	IupSetCallback(btn_ok, "ACTION", (Icallback)depressedPageFlagsOkCallback);
	IupSetCallback(btn_cancel, "ACTION", (Icallback)depressedPageFlagsCancelCallback);

	// Define parameter boxes
	type_fill = IupFill();
	if((hbox_params_type = IupHbox(type_label, type_fill, type, NULL)) == NULL) goto FAIL;
	IupSetAttributes(hbox_params_type, "NORMALIZESIZE=VERTICAL");

	if((param1_fill = IupFill()) == NULL) goto FAIL;
	if((depressed_app.gui_pageflags.hbox_params_param1 = 
		IupHbox(depressed_app.gui_pageflags.param1_label,
			param1_fill,
			depressed_app.gui_pageflags.param1,
			NULL)) == NULL) goto FAIL;
	IupSetAttributes(depressed_app.gui_pageflags.hbox_params_param1, "NORMALIZESIZE=VERTICAL");
	
	if((param2_fill = IupFill()) == NULL) goto FAIL;
	if((depressed_app.gui_pageflags.hbox_params_param2 =
		IupHbox(depressed_app.gui_pageflags.param2_label,
			param2_fill,
			depressed_app.gui_pageflags.param2,
			NULL)) == NULL) goto FAIL;
	IupSetAttributes(depressed_app.gui_pageflags.hbox_params_param2, "NORMALIZESIZE=VERTICAL");
	
	if((quality_fill = IupFill()) == NULL) goto FAIL;
	if((hbox_params_quality = IupHbox(quality_label, quality_fill, quality, NULL)) == NULL) goto FAIL;
	IupSetAttributes(hbox_params_quality, "NORMALIZESIZE=VERTICAL");
	
	if((dpi_fill = IupFill()) == NULL) goto FAIL;
	if((hbox_params_dpi = IupHbox(dpi_label, dpi_fill, dpi, NULL)) == NULL) goto FAIL;
	IupSetAttributes(hbox_params_dpi, "NORMALIZESIZE=VERTICAL");

	// Define button box
	if((btn_fill = IupFill()) == NULL) goto FAIL;
	if((hbox_buttons = IupHbox(btn_fill, btn_ok, btn_cancel, NULL)) == NULL) goto FAIL;
	IupSetAttributes(hbox_buttons, "NORMALIZESIZE=HORIZONTAL");

	main_fill = IupFill();
	vbox_main = IupVbox(
		hbox_params_type,
		depressed_app.gui_pageflags.hbox_params_param1,
		depressed_app.gui_pageflags.hbox_params_param2,
		hbox_params_quality,
		hbox_params_dpi,
		main_fill,
		hbox_buttons,
		NULL);

	// Set default values
	switch(flags.type) {
		case DEPRESS_PAGE_TYPE_COLOR:
		case DEPRESS_PAGE_TYPE_BW:
		case DEPRESS_PAGE_TYPE_LAYERED:
		case DEPRESS_PAGE_TYPE_PALETTIZED:
			IupSetInt(type, "VALUE", flags.type+1);
			break;
		case DEPRESS_PAGE_TYPE_AUTO:
			IupSetInt(type, "VALUE", 5);
			break;
		default:
			IupSetInt(type, "VALUE", 1);
	}
	IupSetInt(depressed_app.gui_pageflags.param1, "VALUE", flags.param1);
	IupSetInt(depressed_app.gui_pageflags.param2, "VALUE", flags.param2);
	IupSetInt(quality, "VALUE", flags.quality);
	IupSetInt(dpi, "VALUE", flags.dpi);

	depressedPageChangeTextsByType(depressed_app.gui_pageflags, flags.type);

	dlg = IupDialog(vbox_main);

	IupSetAttribute(dlg, "TITLE", "Page flags");
	IupSetAttributeHandle(dlg, "DEFAULTENTER", btn_ok);
	IupSetAttributeHandle(dlg, "DEFAULTESC", btn_cancel);

	IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

	if(IupGetInt(dlg, "STATUS") == 1) {
		result = true;

		flags.type = depressedPageTypeConvertFromView(IupGetInt(type, "VALUE"));
		flags.param1 = IupGetInt(depressed_app.gui_pageflags.param1, "VALUE");
		flags.param2 = IupGetInt(depressed_app.gui_pageflags.param2, "VALUE");
		flags.quality = IupGetInt(quality, "VALUE");
		flags.dpi = IupGetInt(dpi, "VALUE");
	}

	IupDestroy(dlg);

	return result;

FAIL:
	if(vbox_main) {
		IupDestroy(vbox_main);
	} else {
		if(hbox_params_type) {
			IupDestroy(hbox_params_type);
		} else {
			if(type_label) IupDestroy(type_label);
			if(type) IupDestroy(type);
			if(type_fill) IupDestroy(type_fill);
		}

		if(depressed_app.gui_pageflags.hbox_params_param1) {
			IupDestroy(depressed_app.gui_pageflags.hbox_params_param1);
		} else {
			if(depressed_app.gui_pageflags.param1_label) IupDestroy(depressed_app.gui_pageflags.param1_label);
			if(depressed_app.gui_pageflags.param1) IupDestroy(depressed_app.gui_pageflags.param1);
			if(param1_fill) IupDestroy(param1_fill);
		}

		if(depressed_app.gui_pageflags.hbox_params_param2) {
			IupDestroy(depressed_app.gui_pageflags.hbox_params_param2);
		} else {
			if(depressed_app.gui_pageflags.param2_label) IupDestroy(depressed_app.gui_pageflags.param2_label);
			if(depressed_app.gui_pageflags.param2) IupDestroy(depressed_app.gui_pageflags.param2);
			if(param2_fill) IupDestroy(param2_fill);
		}

		if(hbox_params_quality) {
			IupDestroy(hbox_params_quality);
		} else {
			if(quality_label) IupDestroy(quality_label);
			if(quality) IupDestroy(quality);
			if(quality_fill) IupDestroy(quality_fill);
		}

		if(hbox_params_dpi) {
			IupDestroy(hbox_params_dpi);
		} else {
			if(dpi_label) IupDestroy(dpi_label);
			if(dpi) IupDestroy(dpi);
			if(dpi_fill) IupDestroy(dpi_fill);
		}

		if(main_fill) IupDestroy(main_fill);

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
