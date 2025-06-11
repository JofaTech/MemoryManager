#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <stdlib.h>
#include <map>
using namespace std;

int bestFit(int sizeInWords, void* list);
int worstFit(int sizeInWords, void* list);
int binaryToDecimal(int n);
//void addNode(int offset, int length);
//struct node* findNode(int offset);
//void deleteNode(int offset);
//void insertNode(int offset, int length);

//struct node
//{
//	node* prev;
//	node* next;
//	bool isHole;
//	int offset;
//	int length;
//};

//struct hole
//{
//	int offset;
//	int length;
//};


class MemoryManager
{
	unsigned int wSize;
	unsigned int byteSize;
	char* memoryBlock;
	function<int(int, void*)> currAllocator;


public:
	MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
	~MemoryManager();
	void initialize(size_t sizeInWords);
	void shutdown();
	void* allocate(size_t sizeInBytes);
	void free(void* address);
	void setAllocator(std::function<int(int, void*)> allocator);
	int dumpMemoryMap(char* filename);
	void* getList();
	void* getBitmap();
	unsigned getWordSize();
	void* getMemoryStart();
	unsigned getMemoryLimit();

};

