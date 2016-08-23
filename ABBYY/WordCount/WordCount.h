#ifndef MYLIBAPI 
#define MYLIBAPI extern "C" __declspec(dllimport)
#endif

MYLIBAPI int WordsCount(const wchar_t* text);