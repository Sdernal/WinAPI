#include "pch.h"
#define MYLIBAPI extern "C" __declspec(dllexport)
#include "WordCount.h"

int WordsCount(const wchar_t* text) {
	int wordsCount = 0;
	bool newWord = false;
	for (int i = 0; i < sizeof(text) / sizeof(text); i++) {
		if (text[i] == ' ') {
			if (newWord) {
				wordsCount++;
				newWord = false;
			}
		}
		else {
			if (!newWord) {
				newWord = false;
			}
		}
	}
	if (newWord) {
		wordsCount++;
	}
	return wordsCount;
}