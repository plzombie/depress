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

#include "../include/depressed_gui.h"
#include "../include/depressed_app.h"
#include "../include/depressed_open.h"

const char DEPRESSED_APP_TITLE[] = "Depress[ed]";

static int depressedLoadProjectCallback(Ihandle *self)
{
	char *cfilename;
	size_t cfilename_length;
	wchar_t *wfilename;

	cfilename = IupGetAttribute(depressed_app.input_filename, "VALUE");
	cfilename_length = strlen(cfilename) + 1;
	wfilename = (wchar_t *)malloc(cfilename_length * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, cfilename_length);

	IupMessage("Project filename", cfilename);

	if(!Depressed::OpenDied(wfilename, depressed_app.document))
		IupMessage(DEPRESSED_APP_TITLE, "Can't open project");

	free(wfilename);

	return IUP_DEFAULT;
}

static int depressedSaveDjvuCallback(Ihandle *self)
{
	char *cfilename;
	size_t cfilename_length;
	wchar_t *wfilename;

	cfilename = IupGetAttribute(depressed_app.output_filename, "VALUE");
	cfilename_length = strlen(cfilename) + 1;
	wfilename = (wchar_t *)malloc(cfilename_length * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, cfilename, -1, wfilename, cfilename_length);

	IupMessage("Project filename", cfilename);

	if(depressed_app.document.Process(wfilename) == Depressed::DocumentProcessStatus::OK)
		IupMessage(DEPRESSED_APP_TITLE, "DJVU Saved");
	else
		IupMessage(DEPRESSED_APP_TITLE, "Can't save DJVU");

	free(wfilename);

	return IUP_DEFAULT;
}

void depressedRunGui(void)
{
	IupOpen(0, 0);

	depressed_app.input_label = IupLabel("Project file:");
	depressed_app.input_filename = IupText(NULL);
	depressed_app.input_button = IupButton("Load", NULL);
	
	depressed_app.input_box = IupHbox(depressed_app.input_label, depressed_app.input_filename, depressed_app.input_button, NULL);
	
	depressed_app.output_label = IupLabel("Djvu file:");
	depressed_app.output_filename = IupText(NULL);
	depressed_app.output_button = IupButton("Save", NULL);
	depressed_app.output_box = IupHbox(depressed_app.output_label, depressed_app.output_filename, depressed_app.output_button, NULL);

	depressed_app.main_box = IupVbox(depressed_app.input_box, depressed_app.output_box, NULL);
	depressed_app.main_dlg = IupDialog(depressed_app.main_box);
	IupSetAttribute(depressed_app.main_dlg, "TITLE", "Depress[ed]");

	IupSetCallback(depressed_app.input_button, "ACTION", depressedLoadProjectCallback);
	IupSetCallback(depressed_app.output_button, "ACTION", depressedSaveDjvuCallback);

	IupShowXY(depressed_app.main_dlg, IUP_CENTER, IUP_CENTER);

	IupMainLoop();

	IupClose();
}
