
#include <Windows.h>

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	wchar_t **argv, *default_project, *default_output;
	int argc;

	argv = CommandLineToArgvW(pCmdLine, &argc);
	if(!argv) return EXIT_FAILURE;

	if(argc > 0) default_project = argv[0];
	if(argc > 1) default_output = argv[1];

	if(argc < 2) {
		// Run GUI
		MessageBoxW(NULL, L"Depressed GUI unimplemented", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);
	} else if(argc == 2) {
		// Process project file and create djvu
		MessageBoxW(NULL, L"Project processing unimplemented", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONSTOP);
	} else {
		MessageBoxW(NULL, L"Too many arguments", L"Depressed", MB_OK | MB_TASKMODAL | MB_ICONINFORMATION);
	}

	return 0;
}
