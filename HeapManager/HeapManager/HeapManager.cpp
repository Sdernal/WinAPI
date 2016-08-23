#include "HeapManager.h"
#include <iostream>
#include <vector>

CHeapManager::CHeapManager() {
	heapStart = nullptr;
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	pageSize = info.dwPageSize;
	mediumBlockSize = 32 * pageSize;
	memoryBlocks.S = std::set<BLOCK>();
	memoryBlocks.M = std::set<BLOCK>();
	memoryBlocks.L = std::set<BLOCK>();
	memoryBlocks.A = std::set<BLOCK>();
	memoryBlocks.C = std::set<BLOCK>();
}

void CHeapManager::Create(int minSize, int maxSize) {
	resevedSize = round(maxSize, pageSize);
	heapStart = VirtualAlloc(NULL, maxSize, MEM_RESERVE, PAGE_READWRITE);
	if (heapStart == nullptr) {
		throw ("Error Alloc");
	}
	BLOCK block;
	block.ptr = heapStart;
	block.size = resevedSize;
	block.type = 0;
	addBlock(block);
	int commitedSize = round(minSize, pageSize);
	BLOCK blockToCommit = getFreeBlock(commitedSize, RESERVED);
	if (blockToCommit.ptr == nullptr) {
		throw ("Not enough free blocks");
	}
	auto pairBlock = splitBlock(blockToCommit, commitedSize);
	pairBlock.first.type = 1;
	addBlock(pairBlock.first);
	addBlock(pairBlock.second);
	commitBlock(pairBlock.first);
}

int CHeapManager::round(int number, int size) {
	return (number + size - 1) / size * size;
}

void CHeapManager::addBlock(BLOCK block) {
	if (block.type == 1) {
		memoryBlocks.C.insert(block);
	}
	else if (block.type == 2) {
		memoryBlocks.A.insert(block);
	}
	else if (block.size < pageSize) {
		memoryBlocks.S.insert(block);
	}
	else if (block.size < mediumBlockSize) {
		memoryBlocks.M.insert(block);
	}
	else {
		memoryBlocks.L.insert(block);
	}
}

CHeapManager::BLOCK CHeapManager::getFreeBlock(int size, int type) {
	BLOCK temp;
	temp.ptr = nullptr;
	temp.size = 0;
	temp.type = 0;
	if (size < pageSize) {
		for (auto it = memoryBlocks.S.begin(); it != memoryBlocks.S.end(); it++) {
			if (((*it).size > size) && ((*it).type == type)) {
				temp = *it;
				memoryBlocks.S.erase(it);
				break;
			}
		}
	}
	if (temp.ptr == nullptr && size < mediumBlockSize) {
		for (auto it = memoryBlocks.M.begin(); it != memoryBlocks.M.end(); it++) {
			if (((*it).size > size) && ((*it).type == type)) {
				temp = *it;
				memoryBlocks.M.erase(it);
			break;
			}
		}
	}
	if (temp.ptr == nullptr) {
		for (auto it = memoryBlocks.L.begin(); it != memoryBlocks.L.end(); it++) {
			if (((*it).size  > size) && ((*it).type == type)) {
				temp = *it;
				memoryBlocks.L.erase(it);
				break;
			}
		}
	}
	return temp;
}

std::pair<CHeapManager::BLOCK, CHeapManager::BLOCK> CHeapManager::splitBlock(BLOCK block, int size) {
	BLOCK first, second;
	first.ptr = block.ptr;
	first.size = size;
	first.type = block.type;
	second.ptr = static_cast<byte*>(block.ptr) + size;
	second.size = block.size - size;
	second.type = block.type;
	return std::make_pair(first, second);
}

void CHeapManager::commitBlock(BLOCK block) {
	LPVOID commitedPtr = VirtualAlloc(block.ptr, block.size, MEM_COMMIT, PAGE_READWRITE);
	if (commitedPtr == nullptr) {
		throw ("Error Alloc");
	}
}

void* CHeapManager::Alloc(int size) {
	int requairedSize = round(size, sizeof(int));
	int roundedSize = requairedSize + sizeof(int);
	BLOCK allocatedBlock = getCommitedBlock(roundedSize);
	if (allocatedBlock.ptr == nullptr) {
		int roundedTo100Page = round(roundedSize, pageSize * 100);
		BLOCK blockToCommit = getFreeBlock(roundedTo100Page, RESERVED);
		if (blockToCommit.ptr == nullptr) {
		throw ("Not enough free blocks");
		}
		auto pairBlock = splitBlock(blockToCommit, roundedTo100Page);
		pairBlock.first.type = 1;
		addBlock(pairBlock.first);
		addBlock(pairBlock.second);
		commitBlock(pairBlock.first);

		allocatedBlock = getCommitedBlock(roundedSize);
		if (allocatedBlock.ptr == nullptr) {
			throw ("Something get wrong");
		}
	}
	BLOCK remainderBlock;
	remainderBlock.ptr = static_cast<byte*>(allocatedBlock.ptr) + roundedSize;
	remainderBlock.size = allocatedBlock.size - roundedSize;
	remainderBlock.type = COMMITED;
	allocatedBlock.size = roundedSize;
	memcpy(allocatedBlock.ptr, &requairedSize, sizeof(int));
	allocatedBlock.type = ALLOCATED;
	addBlock(allocatedBlock);
	if (remainderBlock.size != 0) {
		addBlock(remainderBlock);
	}
	return static_cast<byte*>(allocatedBlock.ptr) + sizeof(int);
}

CHeapManager::BLOCK CHeapManager::getCommitedBlock(int size) {
	BLOCK temp;
	temp.ptr = nullptr;
	temp.size = 0;
	temp.type = 0;
	for (auto it = memoryBlocks.C.begin(); it != memoryBlocks.C.end(); it++) {
		if (((*it).size > size)) {
			temp = *it;
			memoryBlocks.C.erase(it);
			break;
		}
	}
	return temp;
}

void CHeapManager::Free(void* mem) {
	mem = static_cast<byte*>(mem) - sizeof(int);
	int blockSize;
	memcpy(&blockSize, mem, sizeof(int));
	BLOCK blockToFree;
	blockToFree.ptr = mem;
	blockToFree.size = blockSize + sizeof(int);
	blockToFree.type = 2;
	deAllocateBlock(blockToFree);
}

void CHeapManager::deAllocateBlock(BLOCK block) {
	auto it = memoryBlocks.A.find(block);
	if (it != memoryBlocks.A.end()) {
		BLOCK blockToFree;
		blockToFree.ptr = it->ptr;
		blockToFree.size = it->size;
		blockToFree.type = COMMITED;
		memoryBlocks.A.erase(it);
		addBlock(blockToFree);
		mergeBlocks(blockToFree);
	}
	else {
		throw ("invalid pointer");
	}
}

void CHeapManager::mergeBlocks(BLOCK block) {
	auto it = memoryBlocks.C.find(block); 
	if (it != memoryBlocks.C.end()) {
		auto foundedIt = it;
		auto currentIt = it;
		auto nextIt = it;
		if (currentIt != --memoryBlocks.C.end()) {
			nextIt++;
			while ((static_cast<byte*>(nextIt->ptr) == static_cast<byte*>(currentIt->ptr) + currentIt->size) && (nextIt->type == COMMITED)) {
				currentIt++;
				if (currentIt != --memoryBlocks.C.end()) {
					nextIt++;
				}
			}
		}
		auto rightIt = currentIt;
		currentIt = foundedIt;
		nextIt = currentIt;
		if (currentIt != memoryBlocks.C.begin()) {
			nextIt--;
			while ((static_cast<byte*>(nextIt->ptr) == static_cast<byte*>(currentIt->ptr) - nextIt->size) && (nextIt->type == COMMITED)) {
				currentIt--;
				if (currentIt != memoryBlocks.C.begin()) {
					nextIt--;
				}
			}
		}
		auto leftIt = currentIt;
		BLOCK leftBlock = *leftIt;
		BLOCK rightBlock = *rightIt;
		rightIt++;
		memoryBlocks.C.erase(leftIt, rightIt);


		int leftShift = static_cast<byte*>(leftBlock.ptr) - static_cast<byte*>(heapStart);
		int rightShift = static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(heapStart);

		int pageCount = (static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(leftBlock.ptr) + rightBlock.size) / pageSize;
		if (pageCount < 1) {
			BLOCK newBlock;
			newBlock.ptr = leftBlock.ptr;
			newBlock.size = static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(leftBlock.ptr) + rightBlock.size;
			newBlock.type = 1;
			addBlock(newBlock);
			return;
		}
		else {
			LPVOID DecommitPoint = static_cast<byte*>(heapStart) + round(leftShift, pageSize);
			int DecommitSize = ((static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(DecommitPoint) + rightBlock.size) / pageSize) * pageSize;
			VirtualFree(DecommitPoint, DecommitSize, MEM_DECOMMIT);
			if (DecommitPoint == leftBlock.ptr && DecommitSize ==
				static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(leftBlock.ptr) + rightBlock.size) {
				BLOCK newBlock;
				newBlock.ptr = DecommitPoint;
				newBlock.size = DecommitSize;
				newBlock.type = RESERVED;
				addBlock(newBlock);
				freeBlock(newBlock);
				return;
			}
			else if (DecommitPoint == leftBlock.ptr) {
				BLOCK newBlock1;
				BLOCK newBlock2;
				newBlock1.ptr = DecommitPoint;
				newBlock1.size = DecommitSize;
				newBlock1.type = RESERVED;
				addBlock(newBlock1);
				freeBlock(newBlock1);
				newBlock2.ptr = static_cast<byte*>(DecommitPoint) + DecommitSize;
				newBlock2.size = static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(leftBlock.ptr) + rightBlock.size - DecommitSize;
				newBlock2.type = COMMITED;
				addBlock(newBlock2);
			}
			else if (DecommitSize == static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(DecommitPoint) + rightBlock.size) {
				BLOCK newBlock1;
				BLOCK newBlock2;
				newBlock1.ptr = leftBlock.ptr;
				newBlock1.size = static_cast<byte*>(DecommitPoint) - static_cast<byte*>(leftBlock.ptr);
				newBlock1.type = COMMITED;
				addBlock(newBlock1);
				newBlock2.ptr = DecommitPoint;
				newBlock2.size = DecommitSize;
				newBlock2.type = RESERVED;
				addBlock(newBlock2);
				freeBlock(newBlock2);
			}
			else {
				BLOCK newBlock1;
				BLOCK newBlock2;
				BLOCK newBlock3;
				newBlock1.ptr = leftBlock.ptr;
				newBlock1.size = static_cast<byte*>(DecommitPoint) - static_cast<byte*>(leftBlock.ptr);
				newBlock1.type = COMMITED;
				addBlock(newBlock1);
				newBlock2.ptr = DecommitPoint;
				newBlock2.size = DecommitSize;
				newBlock2.type = RESERVED;
				addBlock(newBlock2);
				newBlock3.ptr = static_cast<byte*>(DecommitPoint) + DecommitSize;
				newBlock3.size = static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(DecommitPoint) + rightBlock.size - DecommitSize;
				newBlock3.type = COMMITED;
				addBlock(newBlock3);
			}
		}
	}
	else {
		throw ("invalid pointer");
	}
}
void CHeapManager::freeBlock(BLOCK block) {	
	if (block.size < pageSize) {
		deCommitBlock(block, memoryBlocks.S);
	}
	else if (block.size < mediumBlockSize) {
		deCommitBlock(block, memoryBlocks.M);
	}
	else {
		deCommitBlock(block, memoryBlocks.L);
	}
}


void CHeapManager::deCommitBlock(BLOCK block, std::set<BLOCK>& blockSet) {
	auto it = blockSet.find(block);
	if (it != blockSet.end()) {
		auto foundedIt = it;
		auto currentIt = it;
		auto nextIt = it;
		if (currentIt != --blockSet.end()) {
			nextIt++;
			while ((static_cast<byte*>(nextIt->ptr) == static_cast<byte*>(currentIt->ptr) + currentIt->size) && (nextIt->type == RESERVED)) {
				currentIt++;
				if (currentIt != --blockSet.end()) {
					nextIt++;
				}
			}
		}
		auto rightIt = currentIt;
		currentIt = foundedIt;
		nextIt = currentIt;
		if (currentIt != blockSet.begin()) {
			nextIt--;
			while ((static_cast<byte*>(nextIt->ptr) == static_cast<byte*>(currentIt->ptr) - currentIt->size) && (nextIt->type == RESERVED)) {
				currentIt--;
				if (currentIt != blockSet.begin()) {
					nextIt--;
				}
			}
		}
		auto leftIt = currentIt;
		BLOCK leftBlock = *leftIt;
		BLOCK rightBlock = *rightIt;
		bool isOneBlock = false;
		if (leftIt == rightIt)
			isOneBlock = true;
		rightIt++;
		blockSet.erase(leftIt, rightIt);
		BLOCK newBlock;
		newBlock.ptr = leftBlock.ptr;
		newBlock.size = static_cast<byte*>(rightBlock.ptr) - static_cast<byte*>(leftBlock.ptr) + rightBlock.size;
		newBlock.type = RESERVED;
		addBlock(newBlock);
		if (!isOneBlock)
			freeBlock(newBlock);
	}
	else {
		throw ("invalid pointer");
	}
}

void CHeapManager::Destroy() {
	std::cout << "ALLOCATED POINTERS: " << std::endl;
	std::vector<BLOCK> blocksToFree;
	for (auto it = memoryBlocks.A.begin(); it != memoryBlocks.A.end(); it++) {
		std::cout << "POINTER: " << it->ptr << " SIZE: " << it->size << std::endl;
		blocksToFree.push_back(*it);
	}
	for (int i = 0; i < blocksToFree.size(); i++) {
		deAllocateBlock(blocksToFree[i]);
	}
}