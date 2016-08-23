//#pragma comment(lib, "WC.lib")
//#include "../WC/WordCount.h"
#include <Windows.h>
#include <tchar.h>
#include <map>
#include <vector>

class CNotePadWindow{
public:
	CNotePadWindow();
	~CNotePadWindow(){};
	// ���������������� ����� ����
	static bool RegisterClass();
	void OnNCCreate(HWND _hanlde);
	// ������� ��������� ����
	bool Create();
	// �������� ����
	void Show(int cmdShow);
	// �������� ������� �����
	void OnCreate(HWND handle);
	// ��������� ��������� �������� ����
	void OnSize();
	// ��������� �������� ����
	void OnClose();
	// ��������� �������� � �������� �����
	void OnCommand();
	// ���������� � ����
	void OnSave();
	void OnSettings();
	void OpenFile();
	
	// ��������� ������� ����
	HWND GetHandle() const;
	HWND GetDialogHandle();

	void OnScroll();
	void InitDialog(HWND handle);
	void SetFontSize(int fontSize);
	void SetTransparency(int transparency);
	void OnChange();
	void OnPrewiew();
	void OnCancel();
	void OnColor(UINT botton);
	LRESULT EditColor(WPARAM wParam);
	
	void OnWordsCount();

	void CWPrepare();
	void ClearWords();
	std::vector<std::wstring> TextToWords(std::wstring text);
protected:
	void OnDestroy();

private:	
	struct WORKER {
		PROCESS_INFORMATION prInfo;
		HANDLE mappedFile;
		LPCWSTR mappedFileName;
		HANDLE request;
		HANDLE answer;
	};
	std::vector<WORKER> workers;
	typedef int(*MYFUNC)(const wchar_t*); //remove to use static metod
	MYFUNC func; //remove to use static metod
	HINSTANCE library;
	HBRUSH bkBrush;
	HFONT font;
	HWND editBox;
	bool isChanged;
	bool isPreview;
	int currentFontSize;
	int bufferFontSize;
	int currentTransparency;
	int bufferTransparency;
	COLORREF currentBackgroundColor;
	COLORREF currentFontColor;
	COLORREF bufferBackgroundColor;
	COLORREF bufferFontColor;
	HWND handle; // ����� ����	
	HWND dialog;
	void onFile(OPENFILENAME* ofn);
	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);	
	static INT_PTR __stdcall DialogProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};