#include "MemoryManager.h"
#include <algorithm>
#include <vector>
#include <fcntl.h>
//#include <unistd.h>
#include <string>
#include <math.h>
#include <io.h>
using namespace std;

int numHoles;
vector<pair<int, int>> holes; // First int is offset, second int is length
vector<int> offsets;

MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator)
{
	//shutdown();
	this->wSize = wordSize;
	this->currAllocator = allocator;
	memoryBlock = nullptr;
}

MemoryManager::~MemoryManager()
{
	if (memoryBlock != nullptr)
		delete[] memoryBlock;
}

void MemoryManager::initialize(size_t sizeInWords)
{
	if (sizeInWords > 65536)
	{
		cout << "Block can't be larger than 65536, exiting program." << endl;
		delete[] static_cast<char*>(memoryBlock);
		memoryBlock = nullptr;
		return;
	}
	else
	{
		delete[] static_cast<char*>(memoryBlock);
		memoryBlock = nullptr;
		holes.clear();
		offsets.clear();
		byteSize = sizeInWords * wSize; // Initialize byteSize
		memoryBlock = new char[byteSize]; // Initialize memoryBlock as char[byteSize]
		numHoles = 1; // Initialize total number of holes
		holes.push_back(make_pair(0, sizeInWords)); // Initialize first hole in vector pair
		offsets.push_back(0); // Initialize offsets vector (for all hole/non-hole offsets)

	}
}

void MemoryManager::shutdown()
{
	if (memoryBlock == nullptr)
		return;
	delete[] static_cast<char*>(memoryBlock);
	memoryBlock = nullptr;
	// delete the tracking vectors
	holes.clear();
	offsets.clear();
}

void* MemoryManager::allocate(size_t sizeInBytes)
{
	int lenInWords = sizeInBytes / wSize; // Calculate word size from bytes 

	void* list = getList();

	uint32_t holeOffset = currAllocator(lenInWords, list); // Gets hole offset

	delete[] static_cast<uint16_t*>(list);

	if (holeOffset == -1)
		return nullptr;
	else
	{
		// Find new offset for hole being altered by allocate
		int newOffset = holeOffset + lenInWords;
		int index = 0;

		// Find hole with allocator specified offset
		while (holes[index].first != holeOffset)
		{
			index++;
		}

		// If allocate fills up hole completely, delete hole
		if (holes[index].first == holeOffset && holes[index].second == lenInWords)
		{
			holes.erase(holes.begin() + index);
			numHoles--;
		}
		else // Else adjust size of hole according to allocate size
		{
			holes.push_back(make_pair(newOffset, holes[index].second - lenInWords));
			holes.erase(holes.begin() + index);
			std::sort(holes.begin(), holes.end());
		}

		// Offsets are used to keep track of beginnings and ends of holes
		offsets.push_back(newOffset);
		std::sort(offsets.begin(), offsets.end());
		// Get rid of duplicate offsets
		offsets.erase(unique(offsets.begin(), offsets.end()), offsets.end());
	}

	return (memoryBlock + (holeOffset * wSize));
}

void MemoryManager::free(void* address)
{
	if (memoryBlock == nullptr)
		return;

	int offset = ((uint8_t*)address - (uint8_t*)memoryBlock) / wSize;
	int newLength;

	for (int i = 0; i < offsets.size(); i++)
	{
		if (offsets[i] == offset)
			newLength = offsets[i + 1] - offsets[i];
	}

	holes.push_back(make_pair(offset, newLength));
	numHoles++;
	std::sort(holes.begin(), holes.end());

	for (int i = 0; i < holes.size() - 1; i++)
	{
		// Find ending offset of hole
		int holeEnd = holes[i].first + holes[i].second;

		// If hole is adjacent to another hole, combine holes
		if (holeEnd == holes[i + 1].first)
		{
			int totalLength = holes[i].second + holes[i + 1].second;
			holes.push_back(make_pair(holes[i].first, totalLength));
			holes.erase(holes.begin() + i + 1);
			holes.erase(holes.begin() + i);
			std::sort(holes.begin(), holes.end());
			numHoles--;
			i = 0;
		}
	}

	// Run below hole combination algo if only two holes are left
	if (numHoles == 2)
	{
		int holeEnd = holes[0].first + holes[0].second;

		if (holeEnd == holes[1].first)
		{
			int totalLength = holes[0].second + holes[1].second;
			holes.push_back(make_pair(holes[0].first, totalLength));
			holes.erase(holes.begin() + 1);
			holes.erase(holes.begin());
			std::sort(holes.begin(), holes.end());
			numHoles--;
		}
	}
}

void MemoryManager::setAllocator(std::function<int(int, void*)> allocator)
{
	currAllocator = allocator;
}

int MemoryManager::dumpMemoryMap(char* filename)
{
	int fd;
	int ret;
	string s = "";

	for (int i = 0; i < holes.size(); i++)
	{
		s = s + "[" + to_string(holes[i].first) + ", " + to_string(holes[i].second) + "]";

		// If this is not the last hole
		if (i != holes.size() - 1)
			s = s + " - ";
	}

	int len = s.length();
	const char* str = s.c_str();

	fd = _open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);

	if (fd == -1)
	{
		perror("Error opening file.");
		return -1;
	}

	if ((ret = _write(fd, str, len)) == -1)
	{
		perror("Error writing to file.");
		_close(fd);
		return -1;
	}

	_close(fd);
	return 0;
}

void* MemoryManager::getList()
{
	uint16_t* holesList = new uint16_t[numHoles * 2 + 1];
	int n = 1;

	// if memory has not been allocated 
	if (memoryBlock == nullptr)
		return nullptr;
	else
	{
		// Number of holes will be first element
		holesList[0] = numHoles; 

		for (int i = 0; i < holes.size(); i++)
		{
			holesList[n] = holes[i].first;
			holesList[n + 1] = holes[i].second;
			n = n + 2;
		}

		return (void*)holesList;
	}

	return nullptr;
}


void* MemoryManager::getBitmap()
{
	vector<int> bitVec;
	int sizeInWords = byteSize / wSize;
	vector<int> holeStarts;

	for (int i = 0; i < holes.size(); i++)
	{
		holeStarts.push_back(holes[i].first);
	}

	for (int i = 0; i < offsets.size(); i++)
	{
		int length;
		if ((i + 1) < offsets.size())
			length = offsets[i + 1] - offsets[i];
		else
			length = sizeInWords - offsets[i];

		if (find(holeStarts.begin(), holeStarts.end(), offsets[i]) != holeStarts.end())
		{

			for (int j = 0; j < length; j++)
			{
				bitVec.push_back(0); // Add 0's for holes
			}
		}
		else
		{
			for (int j = 0; j < length; j++)
			{
				bitVec.push_back(1); // Add 1's for non-holes
			}
		}
	}

	while (bitVec.size() % 8 != 0)
		bitVec.push_back(0); // Fill out with zeroes if it's not divisible by 8

	// Reverse vector
	std::reverse(bitVec.begin(), bitVec.end());

	int bitmapSize = bitVec.size() / 8 + 2;

	// Create bitmap to store ints in
	uint8_t* bitmap = new uint8_t[bitmapSize];

	// Set first bitmap element to number of bitmap elements
	bitmap[0] = bitmapSize - 2;
	bitmap[1] = 0;
	int index = bitmapSize - 1;

	while (bitVec.size() != 0)
	{
		int result = 0;

		// divide out into groups of 8 bits in binary
		for (int i = 0; i < 8; i++)
		{
			result = result * 10 + bitVec[i];
		}

		// Add group into bitmap array after converting to decimal from binary
		bitmap[index] = binaryToDecimal(result);
		index--;

		// Remove 8 bits that were copied into bitmap from vector
		bitVec.erase(bitVec.begin(), bitVec.begin() + 8);
	}

	return bitmap;
}

unsigned MemoryManager::getWordSize()
{
	return wSize;
}

void* MemoryManager::getMemoryStart()
{
	return memoryBlock;
}

unsigned MemoryManager::getMemoryLimit()
{
	return (unsigned)byteSize;
}

int bestFit(int sizeInWords, void* list)
{
	uint16_t* holesArray = (uint16_t*)list;

	int bestOffset = -1;
	int bestLength = 65536;
	int element = 1;

	for (int i = 0; i < holesArray[0]; i++)
	{
		if (sizeInWords <= holesArray[element + 1])
		{
			if (holesArray[element + 1] < bestLength)
			{
				bestLength = holesArray[element + 1];
				bestOffset = holesArray[element];
			}
		}
		element = element + 2;
	}

	return bestOffset;
}

int worstFit(int sizeInWords, void* list)
{
	uint16_t* holesArray = (uint16_t*)list;

	int worstOffset = -1;
	int worstLength = -1;
	int element = 1;

	for (int i = 0; i < holesArray[0]; i++)
	{
		if (sizeInWords <= holesArray[element + 1])
		{
			if (holesArray[element + 1] > worstLength)
			{
				worstLength = holesArray[element + 1];
				worstOffset = holesArray[element];
			}
		}
		element = element + 2;
	}

	return worstOffset;
}

// Function to convert binary to decimal
int binaryToDecimal(int n)
{
	int dec = 0;
	int exp = 0;
	int remainder;

	while (n != 0)
	{
		remainder = n % 10;
		n /= 10;

		dec += remainder * pow(2, exp);
		exp++;
	}

	return dec;
}







