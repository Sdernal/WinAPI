#include "NotePadWindow.h"
#include "resource.h"
#include <Commctrl.h>
#include <string>

INT_PTR __stdcall CNotePadWindow::DialogProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
	CNotePadWindow* window = (CNotePadWindow*) GetWindowLongPtr(GetParent(handle), GWLP_USERDATA);
	if (message == WM_INITDIALOG) {
		window->InitDialog(handle);
		return TRUE;
	}
	else {
		
		switch (message) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
				window->OnChange();
				EndDialog(handle, 0);
				return TRUE;
			case IDCANCEL:
				window->OnCancel();
				EndDialog(handle, 0);
				return TRUE;
			case IDC_PREVIEW:
				window->OnPrewiew();
				return TRUE;
			case IDC_BACKGROUND:
				window->OnColor(IDC_BACKGROUND);
				return TRUE;
			case IDC_FONTCOLOR:
				window->OnColor(IDC_FONTCOLOR);
				return TRUE;
			}
			break;
		case WM_CLOSE:
			window->OnCancel();
			EndDialog(handle, 0);
			return FALSE;
		case WM_HSCROLL:
			window->OnScroll();
		}
		return FALSE;
	}
}

void CNotePadWindow::InitDialog(HWND handle) {
	dialog = handle;	
	isPreview = false;
	SendMessage(GetDlgItem(dialog, IDC_FONTSIZE), TBM_SETRANGE, 1, MAKELONG(8, 72));
	SendMessage(GetDlgItem(dialog, IDC_TRANSPARENCY), TBM_SETRANGE, 1, MAKELONG(0, 255));
	SendMessage(GetDlgItem(dialog, IDC_FONTSIZE), TBM_SETPOS, 1, currentFontSize);
	SendMessage(GetDlgItem(dialog, IDC_TRANSPARENCY), TBM_SETPOS, 1, currentTransparency);
}

void CNotePadWindow::OnScroll() {
	int fontSize = SendMessage(GetDlgItem(dialog, IDC_FONTSIZE), TBM_GETPOS, 0, 0);
	int transparency = SendMessage(GetDlgItem(dialog, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
	bufferFontSize = fontSize;
	bufferTransparency = transparency;
	if (isPreview) {
		SetFontSize(bufferFontSize);
		SetTransparency(bufferTransparency);
	}
}

void CNotePadWindow::SetFontSize(int fontSize) {	
	LOGFONT logfont;	
	GetObject(font, sizeof(LOGFONT), &logfont);
	logfont.lfHeight = fontSize;	
	DeleteObject(font);
	font = CreateFontIndirect(&logfont);
	SendMessage(editBox, WM_SETFONT, (WPARAM) font, 1);		
}

void CNotePadWindow::OnChange() {
	currentFontSize = bufferFontSize;
	currentTransparency = bufferTransparency;
	currentBackgroundColor = bufferBackgroundColor;
	currentFontColor = bufferFontColor;
	SetFontSize(currentFontSize);
	SetTransparency(currentTransparency);
	InvalidateRect(editBox, NULL, 1);
}

void CNotePadWindow::OnPrewiew() {
	HWND preview = GetDlgItem(dialog, IDC_PREVIEW);
	isPreview = SendMessage(preview, BM_GETCHECK, 0, 0);
	if (isPreview) {
		SetFontSize(bufferFontSize);
		SetTransparency(bufferTransparency);
	}
	else {
		SetFontSize(currentFontSize);
		SetTransparency(currentTransparency);
	}
	InvalidateRect(editBox, NULL, 1);
}

void CNotePadWindow::OnCancel() {
	SetFontSize(currentFontSize);
	SetTransparency(currentTransparency);
	bufferFontSize = currentFontSize;
	bufferTransparency = currentTransparency;
	bufferBackgroundColor = currentBackgroundColor;
	bufferFontColor = currentFontColor;
	InvalidateRect(editBox, NULL, 1);
}

void CNotePadWindow::SetTransparency(int transparency) {
	SetLayeredWindowAttributes(handle, 0, transparency, LWA_ALPHA);	
}

void CNotePadWindow::OnColor(UINT botton) {
	CHOOSECOLOR color;
	static COLORREF colors[16];
	ZeroMemory(&color, sizeof(color));
	color.lStructSize = sizeof(color);
	color.hwndOwner = dialog;
	color.lpCustColors = (LPDWORD) colors;
	color.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&color) == TRUE) {
		switch (botton)	{
		case IDC_BACKGROUND:
			bufferBackgroundColor = color.rgbResult;
			break;
		case IDC_FONTCOLOR:
			bufferFontColor = color.rgbResult;
			break;
		default:
			break;
		}
	}	
	
	if (isPreview) {
		InvalidateRect(editBox, NULL, 1);
	}
}

LRESULT CNotePadWindow::EditColor(WPARAM wParam) {	
	COLORREF fontColor;
	COLORREF backgroundColor;
	if (isPreview) {
		fontColor = bufferFontColor;
		backgroundColor = bufferBackgroundColor;
		DeleteObject(bkBrush);
		bkBrush = CreateSolidBrush(backgroundColor);
	}
	else {
		fontColor = currentFontColor;
		backgroundColor = currentBackgroundColor;
		DeleteObject(bkBrush);
		bkBrush = CreateSolidBrush(backgroundColor);
	}
	SetTextColor((HDC) wParam, fontColor);
	SetBkColor((HDC) wParam, backgroundColor);

	return (LRESULT) bkBrush;
}

LRESULT __stdcall CNotePadWindow::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {

	CNotePadWindow* window;
	static HWND hwndEdit;
	HWND hwndButton;
	if (message == WM_NCCREATE) {
		window = (CNotePadWindow*) ((CREATESTRUCT*) lParam)->lpCreateParams;
		if (window != 0) {
			SetWindowLongPtr(handle, GWLP_USERDATA, (LONG) window);
			window->OnNCCreate(handle);
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		window = (CNotePadWindow*) GetWindowLongPtr(handle, GWLP_USERDATA);
		switch (message) {
		case WM_CREATE: {
			auto smth = (CREATESTRUCT*)lParam;
			wchar_t title[10];
			GetWindowText(handle, title, 10);
			window->OnCreate(handle);
			return 1;
		}
		case WM_SIZE:
			window->OnSize();
			break;
		case WM_DESTROY:
			window->OnDestroy();
			break;
		case WM_CLOSE:
			window->OnClose();
			break;
		case WM_COMMAND:
			if (HIWORD(wParam) == EN_CHANGE) {
				window->OnCommand();
			}
			else if (LOWORD(wParam) == ID_40002) {
				window->OnSave();
			}
			else if (LOWORD(wParam) == ID_FILE_EXIT) {
				window->OnClose();
			}
			else if (LOWORD(wParam) == ID_VIEW_SETTINGS) {
				window->OnSettings();
			}
			else if (LOWORD(wParam) == ID_ACCELERATOR40004) {
				window->OnDestroy();
			}
			else if (LOWORD(wParam) == ID_WORDSCOUNT) {
				window->OnWordsCount();
			}
			else if (LOWORD(wParam) == ID_FILE_OPEN) {
				window->OpenFile();
			}
			else if (LOWORD(wParam) == ID_GODMODE) {
				STARTUPINFO cif;
				ZeroMemory(&cif, sizeof(STARTUPINFO));
				PROCESS_INFORMATION pi;
				CreateProcess(L"c:\\windows\\notepad.exe", NULL,
					NULL, NULL, FALSE, NULL, NULL, NULL, &cif, &pi);
			}
			else if (LOWORD(wParam) == ID_FILE_CLEAR) {
				window->ClearWords();
			}
			break;
		case WM_CTLCOLOREDIT:
			{
				window->bkBrush = (HBRUSH) window->EditColor(wParam);
				return (LRESULT) window->bkBrush;
			}

		default:
			return DefWindowProc(handle, message, wParam, lParam);
		}
	}
	return 0;
}

void CNotePadWindow::OnSettings() {
	dialog = CreateDialog(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1), handle, CNotePadWindow::DialogProc);
	ShowWindow(dialog, SW_SHOW);
}

CNotePadWindow::CNotePadWindow() {
	isChanged = false;
	isPreview = false;
	currentTransparency = 255;
	bufferTransparency = 255;	
	currentBackgroundColor = WHITE_BRUSH;
	bufferBackgroundColor = WHITE_BRUSH;
}

bool CNotePadWindow::RegisterClass(){
	WNDCLASSEX windowClass;
	::ZeroMemory(&windowClass, sizeof(windowClass));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.hIcon = (HICON) LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), 1, 32, 32, 0);
	windowClass.lpszClassName = L"NotePadWindow";	
	windowClass.hIconSm = (HICON) LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), 1, 16, 16, 0);
	if (!RegisterClassEx(&windowClass)) {
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!dasd"),
			_T("Win32 Guided Tour"),
			NULL);
		return 1;
	}
	else {
		return 0;
	}
}

bool CNotePadWindow::Create() {
	TCHAR bufferTitle[MAX_PATH];
	LoadString(GetModuleHandle(0), IDS_STRING103, bufferTitle, MAX_PATH);
	CreateWindowEx(WS_EX_LAYERED, L"NotePadWindow", _T("fsdf"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		::LoadMenu( ::GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1)), GetModuleHandle(0), this);
	if (!handle) {
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);
		return 1;
	}
	else {
		return 0;
	}	
}

void CNotePadWindow::OnNCCreate(HWND _handle) {
	handle = _handle;
}

void CNotePadWindow::Show(int cmdShow) {
	ShowWindow(handle, cmdShow);
}

void CNotePadWindow::OnDestroy() {
	DeleteObject(bkBrush);
	DeleteObject(font);
	FreeLibrary(library);
	HANDLE ret = OpenEvent(EVENT_ALL_ACCESS, false, L"TerminateRequest");
	SetEvent(ret);

	HANDLE prHandles[4];
	for (int i = 0; i < 4; i++)
		prHandles[i] = (HANDLE)workers[i].prInfo.hProcess;
	auto ans = WaitForMultipleObjects(4, prHandles, TRUE, INFINITE);
	switch (ans) {
	case WAIT_OBJECT_0:
			MessageBox(NULL, L"WAIT_OBJECT_0", L"WAIT_OBJECT_0", NULL);
			break;
	case WAIT_ABANDONED_0:
		MessageBox(NULL, L"WAIT_ABANDONED_0", L"WAIT_ABANDONED_0", NULL);
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, L"WAIT_TIMEOUT", L"WAIT_TIMEOUT", NULL);
		break;
	case WAIT_FAILED:
		MessageBox(NULL, L"WAIT_FAILED", L"WAIT_FAILED", NULL);
		break;
	}
	if (ans != WAIT_OBJECT_0) {
		for (int i = 0; i < 4; i++)
			TerminateProcess(prHandles[i], 0);
	}
	PostQuitMessage(0);	
}


HWND CNotePadWindow::GetHandle() const {
	return handle;
}

HWND CNotePadWindow::GetDialogHandle() {
	return dialog;
}

void CNotePadWindow::OnCreate(HWND parentHandle) {
	RECT parentRect;
	GetClientRect(parentHandle, &parentRect);	
	editBox = CreateWindow(L"Edit", NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_EX_LAYERED,
		parentRect.left, parentRect.top,
		parentRect.right - parentRect.left,
		parentRect.bottom - parentRect.top,
		parentHandle, (HMENU) 1,
		GetModuleHandle(0), this);
	HRSRC startTextRC = FindResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_CUSTOM1), L"CUSTOM");
	HGLOBAL startTextGRC = LoadResource(GetModuleHandle(0), startTextRC);
	void* textPtr = LockResource(startTextGRC);
	SetWindowText(editBox, LPCTSTR(textPtr));	
	font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT logfont;	
	GetObject(font, sizeof(LOGFONT), &logfont);	
	logfont.lfItalic = TRUE;
	logfont.lfUnderline = TRUE;
	DeleteObject(font);
	font = CreateFontIndirect(&logfont);
	SendMessage(editBox, WM_SETFONT, (WPARAM) font, 1);	
	currentFontSize = logfont.lfHeight;
	bkBrush = CreateSolidBrush(currentBackgroundColor);	
	SetLayeredWindowAttributes(handle, 0,255, LWA_ALPHA);
	library = LoadLibrary(TEXT("WC.dll"));	//remove to use static metod
	func = (MYFUNC)GetProcAddress(library, "WordsCount"); // remove to use static metod

	CWPrepare();
}

void CNotePadWindow::OnSize() {
	RECT rect;
	GetClientRect(handle, &rect);
	int wigth = (rect.right - rect.left);
	int heigh = (rect.bottom - rect.top);
	SetWindowPos(editBox, 0,
		rect.left, rect.top, wigth, heigh, 0);
}

void CNotePadWindow::OnClose() {
	if (isChanged == true) {
		int result = MessageBox(handle,
			_T("Do you want to save changes?"),
			_T("Exit"),
			MB_YESNOCANCEL);
		if (result == IDCANCEL) {
			return;
		}
		else if (result == IDYES) {
			OnSave();
		}
	}
	DestroyWindow(handle);
}

void CNotePadWindow::OnCommand() {
	isChanged = true;
}

void CNotePadWindow::onFile(OPENFILENAME* ofn) {	
	TCHAR fileName[MAX_PATH];	
	memset(fileName, 0, sizeof(TCHAR)*MAX_PATH);
	memset(ofn, 0, sizeof(OPENFILENAME));
	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = NULL;
	ofn->lpstrFile = fileName;
	ofn->nMaxFile = sizeof(fileName);
	ofn->lpstrFilter = L"Text files(*.txt)\0*.txt\0JPEG files(*.jpg)\0*.jpg\0All files(*.*)\0*.*\0\0";
	ofn->nFilterIndex = 1;
	ofn->lpstrTitle = L"Save as";
	ofn->lpstrInitialDir = L"c:\\";
	ofn->lpstrDefExt = ofn->lpstrFilter;
	ofn->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	MessageBox(NULL, ofn->lpstrTitle, L"on onFile", MB_OK);
}

void CNotePadWindow::OnSave() {
	int len = GetWindowTextLength(editBox);
	TCHAR* buffer;
	buffer = (TCHAR*) calloc(len + 1, sizeof(TCHAR));
	GetWindowText(editBox, buffer, len + 1);

	OPENFILENAME ofn;
	TCHAR fileName[MAX_PATH];
	*fileName = 0;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrFilter = L"Text files(*.txt)\0*.txt\0JPEG files(*.jpg)\0*.jpg\0All files(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = L"Save as";
	ofn.lpstrInitialDir = L"c:\\";
	ofn.lpstrDefExt = ofn.lpstrFilter;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;	
	if (GetSaveFileName(&ofn))
		MessageBox(NULL, ofn.lpstrFile, L"FIle chosen", MB_OK);

	HANDLE hOut = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD writtenBytes;
	WORD uCode = 0xFEFF;
	WriteFile(hOut, &uCode, 2, &writtenBytes, NULL);
	WriteFile(hOut, buffer, 2 * len, &writtenBytes, NULL);
	CloseHandle(hOut);
	isChanged = false;
}

void CNotePadWindow::OnWordsCount() {
	int len = GetWindowTextLength(editBox);
	wchar_t* buffer;
	buffer = (wchar_t*)calloc(len + 1, sizeof(wchar_t));
	GetWindowText(editBox, buffer, len + 1);
	//int wordsCount = WordsCount(buffer);
	int wordsCount = (func)(buffer);
	wchar_t buffer2[32];
	_itow_s(wordsCount, buffer2, 10);
	MessageBox(NULL, buffer2, L"WordsCount", MB_OK);
}

void CNotePadWindow::OpenFile() {
	if (isChanged) {
		int result = MessageBox(handle,
			_T("Do you want to save changes?"),
			_T("Exit"),
			MB_YESNOCANCEL);		
		if (result == IDYES) {
			OnSave();
		}
		else if (result == IDCANCEL) {
			return;
		}
	}
	OPENFILENAME ofn;
	TCHAR fileName[MAX_PATH];
	*fileName = 0;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrFilter = L"Text files(*.txt)\0*.txt\0JPEG files(*.jpg)\0*.jpg\0All files(*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = L"Open";
	ofn.lpstrInitialDir = L"c:\\";
	ofn.lpstrDefExt = ofn.lpstrFilter;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
		MessageBox(NULL, ofn.lpstrFile, L"FIle chosen", MB_OK);
	HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	wchar_t* buffer;
	auto fileSize = GetFileSize(hFile, NULL);
	DWORD newsize = 0;
	DWORD dwBR;
	buffer = (wchar_t*)malloc(fileSize);
	ReadFile(hFile, buffer, fileSize, &newsize, NULL);

	/*std::string str;
	char ch;
	ReadFile(hFile, &ch, 1, &dwBR, NULL) && dwBR && (ch != '\n');
	while (ReadFile(hFile, &ch, 1, &dwBR, NULL) && dwBR)
	{
		str += ch;
	}
	if (str.size() && (str[str.size() - 1] == '\r'))
		str.erase(str.end() - 1);
	LPWSTR bu;
	bu = (LPWSTR)calloc(str.size(), sizeof(WCHAR));
	MultiByteToWideChar(CP_OEMCP, NULL, str.c_str(), str.size(), bu, str.size());*/

	//ReadFile(hFile, buffer, fileSize, &newsize, NULL);
	
	SetWindowTextW(editBox, buffer);
	CloseHandle(hFile);
	isChanged = false;
}

void CNotePadWindow::ClearWords() {
	for (int i = 0; i < 4; i++) {
		PROCESS_INFORMATION pi = workers[i].prInfo;

		std::wstring clearReqName = L"ClearRequest";
		std::wstring finishAnsName = L"FinishAnswer";

		clearReqName += std::to_wstring(pi.dwProcessId);
		finishAnsName += std::to_wstring(pi.dwProcessId);

		HANDLE clearReq = OpenEvent(EVENT_ALL_ACCESS, false, (LPCWSTR)clearReqName.c_str());
		HANDLE finishAns = OpenEvent(EVENT_ALL_ACCESS, false, (LPCWSTR)finishAnsName.c_str());

		workers[i].request = clearReq;
		workers[i].answer = finishAns;
	}

	UINT len = GetWindowTextLength(editBox);
	
	if (len > 0) {
		TCHAR* buffer;
		buffer = (TCHAR*)calloc(len + 1, sizeof(TCHAR));
		GetWindowText(editBox, buffer, len + 1);
		std::wstring text(buffer); 
		std::vector<std::wstring> words = TextToWords(text);

		std::vector<std::wstring> parts;
		parts.resize(4);

		int sizeofPart = words.size() / 4;
		for (int i = 0; i < words.size(); i++) {
			if (i <= sizeofPart) {
				parts[0] += words[i];
				parts[0] += L" ";
			} else if (i > sizeofPart && i <= 2*sizeofPart) {
				parts[1] += words[i];
				parts[1] += L" ";
			}
			else if (i > 2 * sizeofPart && i <= 3 * sizeofPart) {
				parts[2] += words[i];
				parts[2] += L" ";
			}
			else {
				parts[3] += words[i];
				parts[3] += L" ";
			}
		}
		
		for (int i = 0; i < 4; i++) {
			LPCTSTR mapBuffer;
			mapBuffer = (LPTSTR)MapViewOfFile(workers[i].mappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 256);
			
			char a[256];
			memset(a, 0, 256);
			CopyMemory((PVOID)mapBuffer, (PVOID)a, 256);
			LPCTSTR textBuffer = (LPTSTR)parts[i].c_str();
			CopyMemory((PVOID)mapBuffer, (PVOID)textBuffer, (lstrlen(textBuffer) * sizeof(TCHAR)));
			SetEvent(workers[i].request);
		}
		
		std::vector<bool> answerFlags;
		answerFlags.resize(4, false);
		HANDLE ansEvents[4];
		std::vector<LPTSTR> receivedParts;
		receivedParts.resize(4);
		for (int i = 0; i < 4; i++) {
			ansEvents[i] = workers[i].answer;
		}

		DWORD waitResult;
		waitResult = WaitForMultipleObjects(4, ansEvents, TRUE, INFINITE);
		if (waitResult == WAIT_OBJECT_0) {
			for (int i = 0; i < 4; i++) {
				receivedParts[i] = (LPTSTR)MapViewOfFile(workers[i].mappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 256);
			}
		}
	
		std::wstring result = receivedParts[0];
		for (int i = 1;i < 4; i++) {
			std::wstring wstrPart = receivedParts[i];			
			if (wstrPart.size()) {
				result += L" ";
				result += wstrPart;				
			}
		}

		SetWindowText(editBox, (LPCWSTR)result.c_str());
	}
	
}

void CNotePadWindow::CWPrepare() {
	workers.resize(4);
	for (int i = 0; i < 4; i++) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		CreateProcessW(L"C:\\Users\\Sdern_000\\Desktop\\ABBYY\\Debug\\ClearWords.exe",
			NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		workers[i].prInfo = pi;

		std::wstring fileName = L"File";
		fileName += std::to_wstring(pi.dwProcessId);
		workers[i].mappedFileName = fileName.c_str();

		HANDLE fm;
		fm = CreateFileMapping(INVALID_HANDLE_VALUE,
			NULL, PAGE_READWRITE, 0, 256, (LPTSTR)workers[i].mappedFileName);
		workers[i].mappedFile = fm;
	}
}

std::vector<std::wstring> CNotePadWindow::TextToWords(std::wstring text) {
	std::vector<std::wstring> outText;
	std::wstring word;

	for (int i = 0; i < text.size(); i++) {
		if (text[i] == L' ') {
			if (!word.empty()) {				
				outText.push_back(word);			
				word.clear();
			}
		}
		else {
			word += text[i];
		}
	}
	if (!word.empty()) {
		outText.push_back(word);
	}
	return outText;
}