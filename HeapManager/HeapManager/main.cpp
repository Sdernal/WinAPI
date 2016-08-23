#include "HeapManager.h"
#include <iostream>
#include <vector>
#include <time.h>

class TEST {
public:

	friend bool operator <(TEST a, TEST b) {
		return (a.smth[0] - b.smth[0]) > 0;
	}
	static CHeapManager heapManager;
	TEST() {
		smth[0] = rand() % 1000;
	}
	void setSmth(int input) {
		smth[0] = input;
	}
	int getSmth() {
		return smth[0];
	}
	void* operator new(size_t size) {
		return heapManager.Alloc(size);
	}

	void* operator new[](size_t size) {
		return heapManager.Alloc(size);
	}

		void operator delete[](void* ptr) {
		return heapManager.Free(ptr);
	}

		void operator delete (void* ptr) {
		return heapManager.Free(ptr);
	}
private:
	int smth[100];
};

CHeapManager TEST::heapManager;

class TRAIN {
private:
	int smth[100];
};

void pushbackTest() {
	TEST::heapManager.Create(sizeof(TEST) * 1024, 1000 * sizeof(TEST) * 1024);
	std::vector<TEST*> vec;
	std::vector<TRAIN*> vec2;
	auto t1 = clock();
	for (int i = 0; i < 100000; i++) {
		vec.push_back(new TEST);
	}
	auto t2 = clock();
	for (int i = 0; i < 100000; i++) {
		vec2.push_back(new TRAIN);
	}
	auto t3 = clock();
	auto test_time = t2 - t1;
	auto train_time = t3 - t2;
	std::cout << "TEST TIME " << test_time << std::endl;
	std::cout << "TRAIN TIME " << train_time << std::endl;
};

void deleteTest() {
	TEST::heapManager.Create(sizeof(TEST) * 1024, 1000 * sizeof(TEST) * 1024);
	std::vector<TEST*> vec;
	std::vector<TRAIN*> vec2;	
	for (int i = 0; i < 10000; i++) {
		vec.push_back(new TEST);
	}	
	for (int i = 0; i < 10000; i++) {
		vec2.push_back(new TRAIN);
	}	

	auto t1 = clock();
	for (int i = 0; i < 10000; i++) {
		delete(vec[i]);
	}
	auto t2 = clock();
	for (int i = 0; i < 10000; i++) {
		delete(vec2[i]);
	}
	auto t3 = clock();
	auto test_time = t2 - t1;
	auto train_time = t3 - t2;
	std::cout << "TEST TIME " << test_time << std::endl;
	std::cout << "TRAIN TIME " << train_time << std::endl;
}
void randomTest() {
	TEST::heapManager.Create(sizeof(TEST) * 1024, 1000 * sizeof(TEST) * 1024);
	std::set<TEST*> testSet;
	std::set<TRAIN*> trainSet;
	auto t1 = clock();
	for (int i = 0; i < 15000; ++i) {		
		switch (rand() % 3) {
		case 0:
			testSet.insert(new TEST);
			break;
		case 2:
			testSet.insert(new TEST);
			break;
		case 1:
			auto it = testSet.begin();
			if (it != testSet.end()) {
				TEST* toDelete = *it;
				testSet.erase(testSet.begin());
				delete(toDelete);
			}
			break;
		}
	}
	auto t2 = clock();
	for (int i = 0; i < 15000; ++i) {
		switch (rand() % 2) {
		case 0:
			trainSet.insert(new TRAIN);
			break;
		case 2:
			trainSet.insert(new TRAIN);
			break;
		case 1:
			auto it = trainSet.begin();
			if (it != trainSet.end()) {
				TRAIN* toDelete = *it;
				trainSet.erase(trainSet.begin());
				delete(toDelete);
			}
			break;
		}
	}
	auto t3 = clock();
	auto test_time = t2 - t1;
	auto train_time = t3 - t2;
	std::cout << "TEST TIME " << test_time << std::endl;
	std::cout << "TRAIN TIME " << train_time << std::endl;
};


int main() {
	std::cout << "PUSHBACK TEST: " << std::endl;
	pushbackTest();
	std::cout << std::endl;
	std::cout << "DELETE TEST: " << std::endl;
	deleteTest();
	std::cout << std::endl;
	std::cout << "RANDOM TEST: " << std::endl;
	randomTest();
	int adf;
	std::cin >> adf;
	return 0;
}