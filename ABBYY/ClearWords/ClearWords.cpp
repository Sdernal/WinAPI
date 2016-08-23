#include <Windows.h>
#include <string>
#include <set>


void ClearWords(std::set<std::wstring> dictionary) {
	HANDLE mappedFile;
	LPCTSTR mapBuffer;
	std::wstring fileName = L"File";
	fileName += std::to_wstring(GetCurrentProcessId());

	mappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, (LPCWSTR)fileName.c_str());
	mapBuffer = (LPCTSTR)MapViewOfFile(mappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 1024);

	std::wstring text = mapBuffer;
	//std::string text = std::string(wstrBuffer.begin(), wstrBuffer.end());

	std::wstring outText;
	std::wstring word;
	
	for (int i = 0; i < text.size(); i++) {
		if (text[i] == L' ') {
			if (!word.empty()) {
				auto it = dictionary.find(word);
				if (it != dictionary.end()) {
					outText += word;
					outText += L" ";					
				}
				word.clear();
			}
		}
		else {
			word += text[i];
		}
	}
	if (!word.empty()) {
		auto it = dictionary.find(word);
		if (it != dictionary.end()) {
			outText += word;
			outText += L" ";
		}
	}
	LPTSTR out = (LPTSTR)outText.c_str();
	TCHAR a[1024];
	memset(a, 0, 1024);
	CopyMemory((PVOID)mapBuffer, (PVOID)a, 1024);
	CopyMemory((PVOID)mapBuffer, (PVOID)out, (lstrlen(out) * sizeof(TCHAR)));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {	
	std::set<std::wstring> dictionary;
	dictionary.insert(L"What");
	dictionary.insert(L"period");
	dictionary.insert(L"minutes");
	dictionary.insert(L"when");
	dictionary.insert(L"wander");
	dictionary.insert(L"was");

	std::wstring clearReqName = L"ClearRequest";
	std::wstring termReqName = L"TerminateRequest";
	std::wstring finishAnsName = L"FinishAnswer";

	clearReqName += std::to_wstring(GetCurrentProcessId());
	//termReqName += std::to_wstring(GetCurrentProcessId());
	finishAnsName += std::to_wstring(GetCurrentProcessId());

	HANDLE clearReq = CreateEvent(NULL, FALSE, FALSE, (LPCWSTR)clearReqName.c_str());
	HANDLE termReq = CreateEvent(NULL, TRUE, FALSE, (LPCWSTR)termReqName.c_str());
	HANDLE finishAns = CreateEvent(NULL, FALSE, FALSE, (LPCWSTR)finishAnsName.c_str());
	
	HANDLE events[2];
	events[0] = clearReq;
	events[1] = termReq;
	
	do {
		DWORD waitRes = WaitForMultipleObjects(2, events, FALSE, INFINITE);
		if (waitRes == WAIT_OBJECT_0) {
			//MessageBox(NULL, L"Start clearing" , L"Event handled", MB_OK);
			ClearWords(dictionary);
			SetEvent(finishAns);
		} else if (waitRes == WAIT_OBJECT_0 + 1) {
			//MessageBox(NULL, L"Start clearing", L"Event handled", MB_OK);			
			TerminateProcess(GetCurrentProcess(), 0);
		}
	} while (1);
	return 0;
}