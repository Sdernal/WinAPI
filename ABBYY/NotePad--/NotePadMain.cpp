#include "NotePadWindow.h"
#include <string>
#include "resource.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {	
	CNotePadWindow::RegisterClass();
	CNotePadWindow mainWindow;
	mainWindow.Create();
	mainWindow.Show(nCmdShow);
	auto acceleratorTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(mainWindow.GetHandle(), acceleratorTable, &msg) &&
			!IsDialogMessage(mainWindow.GetDialogHandle(), &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	::DestroyAcceleratorTable(acceleratorTable);
	return (int) msg.wParam;
}


