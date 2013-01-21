#pragma once

#include <vector>
#include "FlatFile.hpp"
#include "VFSFile.h"
using namespace _STL;

// no need to add Windows.h
typedef unsigned long DWORD;
typedef		 void   (__stdcall *  VCALLBACK_CLEARBLANKALL) ( int );

struct VFileInfo
{
	DWORD dwVersion;
	DWORD dwCRC;
};

class CIndex
{
public:
	CIndex(void);
	~CIndex(void);
	bool Open(const char* FileName, const char* Mode = "rb");
	void Close();
	bool Safe();
	bool AddVFS(const char* VfsName);
	short AddFile(const char* VfsName, const char* FileName, const char* TargetName, int version, int crc, bool deleted = false);
	short RemoveFile(const char* FileName);
	static bool NormalizePath(const char* path, char* path2);
	void Defragment(VCALLBACK_CLEARBLANKALL CallBackProc);
	void GetFileInfo(const char* FileName, VFileInfo* FileInfo, bool CalcCrc);
	bool SetFileInfo(const char* FileName, VFileInfo* FileInfo);
	int GetVfsCount() const;
	int GetVFSNames(char** Names, unsigned int Num, short MaxPathLength);
	int GetFileCount(const char* VfsName);
	int GetTotalFileCount();
	int GetFileNames(const char* VfsName, char** FileNames, int NumberToGet, int MaxPathLength);
	int GetBaseVersion() const;
	void SetBaseVersion(int Version);
	int GetCurrentVersion() const;
	void SetCurrentVersion(int Version);
	CVFSFile::File* OpenFile(const char* FileName);
	int GetFileSize(const char* FileName);
	bool FileExists(const char* FileName);
	bool FileExistsInVfs(const char* FileName);
	vector<CVFSFile*>* ListVFS;
private:
	bool changed;
	char* name;
	int baseVersion;
	int currentVersion;	
	int vfsCount; //why such a big lenght for vfsCount :O
	FlatFile* IFile;
};

