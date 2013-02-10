#pragma once

#include <vector>
#include "FlatFile.hpp"
#include "Hashtable.h"
using namespace std;

class CVFSFile
{
public:
	struct File
	{
		unsigned int currentPosition;
		unsigned char* data;
		char* path;
		unsigned int offset;
		unsigned int lenght;
		unsigned int lEndOff;
		int hash;
		unsigned int crc;
		int version;
		bool deleted;
		int vfsIndex;
		unsigned char btEncrypted;
	};
	CVFSFile(void);
	~CVFSFile(void);
	void SetVFSName(char* Name);
	char* GetVFSName() const;
	void SetDataOffset(unsigned int Offset);
	unsigned int GetDataOffset() const;
	void SetFileCount(unsigned int Count);
	unsigned int GetFileCount() const;
	void SetDeleteCount(unsigned int Count);
	unsigned int GetDeleteCount() const;
	void SetStartOffset(unsigned int Offset);
	unsigned int GetStartOffset() const;
	static unsigned int CalculateCrc32(File* RoseFile);
private:
	char* vfsName;
	unsigned int dataOffset; //offset in data file (data.idx in most cases)
	unsigned int startOffset; // seems to be always 0?
	unsigned int fileCount;
	unsigned int deletedFileCount;
public:
	vector<CVFSFile::File*>* Files;
	CHashTable<CVFSFile::File*> FileTable;
	FlatFile* VFile;
};

