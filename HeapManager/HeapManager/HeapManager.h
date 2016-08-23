#pragma once
#include <Windows.h>
#include <set>

#define RESERVED 0
#define COMMITED 1
#define ALLOCATED 2

class CHeapManager {
public:
	CHeapManager();
	void Create(int minSize, int maxSize);
	void* Alloc(int size);
	void Free(void* mem);
	void Destroy();

private:
	int pageSize;
	int resevedSize;
	int mediumBlockSize;
	LPVOID heapStart;

	struct BLOCK {

		friend bool operator <(BLOCK a, BLOCK b) {
			return (static_cast<byte*>(b.ptr) - static_cast<byte*>(a.ptr)) > 0;
		}

		LPVOID ptr;
		int size;
		int type; // 0 - reserved
				  // 1 - commited
				  // 2 - allocated
	};

	struct MemoryBlocks {
		std::set<BLOCK> A; //Allocated
		std::set<BLOCK> C; //Commited
		std::set<BLOCK> S; //Small reseved
		std::set<BLOCK> M; //Medium reserved
		std::set<BLOCK> L; //Large reserved
	} memoryBlocks;

	int round(int number, int size);
	void addBlock(BLOCK block);
	BLOCK getFreeBlock(int size, int type);
	BLOCK getCommitedBlock(int size);
	std::pair<BLOCK, BLOCK> splitBlock(BLOCK block, int size);
	void commitBlock(BLOCK block);
	void freeBlock(BLOCK block);
	void deAllocateBlock(BLOCK block);
	void mergeBlocks(BLOCK block);
	void deCommitBlock(BLOCK block, std::set<BLOCK>& blockSet);
};
