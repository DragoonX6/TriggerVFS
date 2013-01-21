#include "Index.h"
#include <iostream>
#include <algorithm>
#include <cctype>

CIndex::CIndex(void): baseVersion(0), currentVersion(0), vfsCount(0), changed(false)
{
	ListVFS = new vector<CVFSFile*>();
}

CIndex::~CIndex(void)
{
	if(ListVFS)
	{
		for(vector<CVFSFile*>::reverse_iterator i = ListVFS->rbegin(); i != ListVFS->rend(); ++i)
		{
			delete (*i);
			(*i) = NULL;
		}
		delete ListVFS;
		ListVFS = NULL;
	}
	if(IFile)
	{
		if(IFile->IsOpen())
		{
			IFile->Close();
		}
		delete IFile;
		IFile = NULL;
	}
	if(name)
	{
		delete name;
		name = NULL;
	}
}

bool CIndex::Open(const char* FileName, const char* Mode)
{
	IFile = new FlatFile();
	if(!IFile->Open(FileName, Mode))
	{
		return false;
	}
	this->name = (char*)FileName;
	this->baseVersion = IFile->Read<int>();
	this->currentVersion = IFile->Read<int>();
	this->vfsCount = IFile->Read<int>();
	bool HasRoot = false;
	for(int i = 0; i < vfsCount; i++)
	{
		CVFSFile* VfsFile = new CVFSFile();
		short len = IFile->Read<short>();
		VfsFile->SetVFSName((char*)IFile->ReadBytes(len));
		if(!strcmp(VfsFile->GetVFSName(), "ROOT.VFS"))
		{
			HasRoot = true;
		}
		VfsFile->SetDataOffset(IFile->Read<unsigned int>());
		ListVFS->push_back(VfsFile);
	}
	if(HasRoot == true)
	{
		for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
		{
			if(i == ListVFS->end())
			{
				break;
			}
			if(!strcmp((*i)->GetVFSName(), "ROOT.VFS"))
			{
				delete (*i);
				(*i) = NULL;
				i = ListVFS->erase(i);
				vfsCount--;
				break;
			}
		}
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		IFile->Seek((*i)->GetDataOffset());
		(*i)->SetFileCount(IFile->Read<unsigned int>());
		(*i)->SetDeleteCount(IFile->Read<unsigned int>());
		(*i)->SetStartOffset(IFile->Read<unsigned int>());
		unsigned int j = 0;
		while(j < (*i)->GetFileCount())
		{
			CVFSFile::File* RoseFile = new CVFSFile::File();
			RoseFile->vfsIndex = distance(ListVFS->begin(), i);
			RoseFile->data = NULL;
			short len = IFile->Read<short>();
			RoseFile->path = (char*)IFile->ReadBytes(len);
			RoseFile->offset = IFile->Read<int>();
			RoseFile->lenght = IFile->Read<int>();
			int bs = IFile->Read<int>();
			RoseFile->deleted = IFile->Read<bool>();
			bool ct = IFile->Read<bool>();
			bool et = IFile->Read<bool>();
			RoseFile->version = IFile->Read<int>();
			RoseFile->crc = IFile->Read<int>();
			RoseFile->hash = toHash(RoseFile->path);
			(*i)->Files->push_back(RoseFile); // do allow deleted files
			if(!RoseFile->deleted) // but do not make a hash for them
			{
				(*i)->FileTable.addHash(RoseFile->hash, RoseFile);
			}
			j++;
		}
	}
	IFile->Close();
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		(*i)->VFile = new FlatFile();
		(*i)->VFile->Open((*i)->GetVFSName(), "rb+");
	}
	return true;
}

void CIndex::Close()
{
	if(changed == true)
	{
		if(this->Safe())
		{
			this->changed = false;
		}
		IFile->Close();
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		(*i)->VFile->Close();
	}
}

bool CIndex::Safe()
{
	char TmpName[128];
	memset(TmpName, 0, 128);
	strcpy(TmpName, this->name);
	strcat(TmpName, ".tmp");
	if(!IFile->Open(TmpName, "wb"))
	{
		return false;
	}
	IFile->Write(this->baseVersion);
	IFile->Write<int>(this->currentVersion);
	IFile->Write<int>(this->vfsCount);
	int* offsets = new int[vfsCount];
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		short length = 0;
		char* Name = (*i)->GetVFSName();
		length = strlen(Name) + 1;
		IFile->Write<short>(length);
		IFile->WriteData(Name, length);
		offsets[distance(ListVFS->begin(), i)] = IFile->Position();
		IFile->Write<unsigned int>((*i)->GetDataOffset());
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		(*i)->SetDataOffset(IFile->Position());
		IFile->Seek(offsets[distance(ListVFS->begin(), i)]);
		IFile->Write<unsigned int>((*i)->GetDataOffset());
		IFile->Seek((*i)->GetDataOffset());
		IFile->Write<unsigned int>((*i)->GetFileCount());
		IFile->Write<unsigned int>((*i)->GetDeleteCount());
		IFile->Write<unsigned int>((*i)->GetStartOffset());
		for(vector<CVFSFile::File*>::iterator j = (*i)->Files->begin(); j != (*i)->Files->end(); ++j)
		{
			short length = 0;
			length = strlen((*j)->path) + 1;
			IFile->Write<short>(length);
			IFile->WriteData((*j)->path, length);
			IFile->Write<int>((*j)->offset);
			IFile->Write<int>((*j)->lenght);
			IFile->Write<int>((*j)->lenght);
			IFile->Write<bool>((*j)->deleted);
			IFile->Write(false);
			IFile->Write(false);
			IFile->Write<int>((*j)->version);
			if((*j)->crc == 0)
			{
				if(!(*j)->data)
				{
					(*j)->data = new unsigned char[(*j)->lenght];
					(*i)->VFile->Seek((*j)->offset);
					(*i)->VFile->ReadData((*j)->data, (*j)->lenght);
				}
				(*j)->crc = (*i)->CalculateCrc32((*j));
				IFile->Write<int>((*j)->crc);
			}
			else
			{
				IFile->Write<int>((*j)->crc);
			}
			if((*j)->data)
			{
				delete[] (*j)->data;
			}
		}
	}
	delete[] offsets;
	offsets = NULL;
	IFile->Close();
	if(remove(this->name))
	{
		return false;
	}
	if(rename(TmpName, this->name))
	{
		return false;
	}
	changed = false;
	return true;
}

bool CIndex::AddVFS(const char* VfsName)
{
	char* name = new char[strlen(VfsName) + 1];
	CIndex::NormalizePath(VfsName, name);
	short len = strlen(name);
	for(int i = 0; i < len; i++)
	{
		name[i] = toupper(name[i]);
	}
	if(!strcmp(name, "ROOT.VFS"))
	{
		return false;
	} 
	CVFSFile* Vfs = new CVFSFile();
	Vfs->SetVFSName(name);
	this->vfsCount++;
	this->changed = true;
	Vfs->VFile = new FlatFile(name, "wb");
	ListVFS->push_back(Vfs);
	return true;
}

short CIndex::AddFile(const char* VfsName, const char* FileName, const char* TargetName, int version, int crc, bool deleted)
{
	string name = VfsName;
	transform(name.begin(), name.end(), name.begin(), ::toupper);
	if(!name.compare("ROOT.VFS"))
	{
		return 2; // invalid vfs
	}
	bool VfsFound = false;
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		if(!name.compare((*i)->GetVFSName()))
		{
			VfsFound = true;
			if(!FlatFile::Exist((char*)FileName))
			{
				return 5; // doesn't exist
			}
			FlatFile* srcFile = new FlatFile(FileName, "rb");
			if(!srcFile->IsOpen())
			{
				delete srcFile;
				srcFile = NULL;
				return 3; // can't open
			}
			CVFSFile::File* TargetFile = new CVFSFile::File();
			char* NormalizedTargetName = new char[strlen(TargetName) + 1];
			CIndex::NormalizePath(TargetName, NormalizedTargetName);
			TargetFile->path = NormalizedTargetName;
			TargetFile->version = version;
			TargetFile->crc = crc;
			TargetFile->vfsIndex = distance(ListVFS->begin(), i);
			TargetFile->currentPosition = 0;
			TargetFile->deleted = false;
			TargetFile->hash = toHash(TargetFile->path);
			TargetFile->offset = (*i)->VFile->Size();
			TargetFile->lenght = srcFile->Size();
			TargetFile->data = new unsigned char[TargetFile->lenght];
			if(TargetFile->data == NULL || TargetFile->data == (unsigned char*)-1)
			{
				delete TargetFile;
				TargetFile = NULL;
				return 6; // memalloc fail
			}
			if(!srcFile->ReadData(TargetFile->data, TargetFile->lenght))
			{
				delete TargetFile;
				TargetFile = NULL;
				return 6; // memalloc fail (not really but it suits the error the best)
			}
			long pos = (*i)->VFile->Position();
			(*i)->VFile->Seek((*i)->VFile->Size());
			(*i)->VFile->WriteData(TargetFile->data, TargetFile->lenght);
			(*i)->VFile->Seek(pos);
			(*i)->Files->push_back(TargetFile);
			(*i)->FileTable.addHash(TargetFile->hash, TargetFile);
			(*i)->SetFileCount((*i)->GetFileCount() + 1);
			this->changed = true;
			srcFile->Close();
			delete srcFile;
			srcFile = NULL;
		}
	}
	if(VfsFound == false)
	{
		return 2; // invalid vfs
	}
	return 0;
}

short CIndex::RemoveFile(const char* FileName)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		CVFSFile::File* RoseFile;
		if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
		{
			RoseFile->deleted = true;
			(*i)->FileTable.removeHash(toHash(NormalizedName));
			(*i)->SetDeleteCount((*i)->GetDeleteCount() + 1);
			return 0; // succes
		}
	}
	return 3; // doesn't exist
}

bool CIndex::NormalizePath(const char* path, char* path2)
{
	int i = 0;
	for(char* p = (char*)path; *p; p++)
	{
		if(*p == '\\')
		{
			if(*(p + 1) == '\\')
			{
				path2[i] = '\\';
				i++;
				p++;
			}
			else
			{
				path2[i] = *p;
				i++;
			}
			continue;
		}
		else if(*p == '/')
		{
			path2[i] = '\\';
			i++;
			continue;
		}
		else
		{
			path2[i] = *p;
			i++;
		}
	}
	path2[i] = 0;
	return true;
}

void CIndex::Defragment(VCALLBACK_CLEARBLANKALL CallBackProc)
{
	if(CallBackProc == NULL)
	{
		return; // no function to call
	}
	int ProcessCount = 0, FilesProcessed = 0, Percent = 0, PercentOld = 0;
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		ProcessCount += ((*i)->GetFileCount() - (*i)->GetDeleteCount());
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		short len = strlen((*i)->GetVFSName());
		char TmpVfsName[128];
		memset(TmpVfsName, 0, 128);
		strcpy(TmpVfsName, (*i)->GetVFSName());
		strcat(TmpVfsName, ".TMP");
		FlatFile* TmpVfsFile = new FlatFile(TmpVfsName, "wb+");
		for(vector<CVFSFile::File*>::iterator j = (*i)->Files->begin(); j != (*i)->Files->end(); ++j)
		{
			if(!(*j)->deleted)
			{
				if(!(*j)->data)
				{
					(*j)->data = new unsigned char[(*j)->lenght];
					(*i)->VFile->Seek((*j)->offset);
					(*i)->VFile->ReadData((*j)->data, (*j)->lenght);
				}
				(*j)->offset = TmpVfsFile->Position();
				TmpVfsFile->WriteData((*j)->data, (*j)->lenght);
				(*j)->crc = CVFSFile::CalculateCrc32((*j));
				delete[] (*j)->data;
				(*j)->data = NULL;
				FilesProcessed++;
			}
			Percent = ((static_cast<double>(FilesProcessed) / static_cast<double>(ProcessCount)) * 100);
			if(Percent > PercentOld)
			{
				CallBackProc(Percent);
				PercentOld = Percent;
			}
		}
		TmpVfsFile->Close();
		(*i)->VFile->Close();
		(*i)->VFile->Delete();
		rename(TmpVfsName, (*i)->GetVFSName());
		(*i)->VFile->Open((*i)->GetVFSName(), "rb+");
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		int FileCount = (*i)->GetFileCount();
		int DeleteCount = (*i)->GetDeleteCount();
		for(vector<CVFSFile::File*>::iterator j = (*i)->Files->begin(); j != (*i)->Files->end(); ++j)
		{
			if((*j)->deleted)
			{
				delete (*j);
				(*j) = NULL;
				j = (*i)->Files->erase(j);
				FileCount--;
				DeleteCount--;
			}
			if(DeleteCount == 0)
			{
				break;
			}
			if(j == (*i)->Files->end())
			{
				break;
			}
		}
		(*i)->SetFileCount(FileCount);
		(*i)->SetDeleteCount(DeleteCount);
	}
	changed = true;
	return; // done (I think)
}

void CIndex::GetFileInfo(const char* FileName, VFileInfo* FileInfo, bool CalcCrc)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		CVFSFile::File* RoseFile;
		if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
		{
			FileInfo->dwVersion = RoseFile->version;
			if(CalcCrc)
			{
				FileInfo->dwCRC = (*i)->CalculateCrc32(RoseFile);
				RoseFile->crc = FileInfo->dwCRC;
				return;
			}
			else
			{
				FileInfo->dwCRC = RoseFile->crc;
			}
		}
	}
	return;
}

bool CIndex::SetFileInfo(const char* FileName, VFileInfo* FileInfo)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		CVFSFile::File* RoseFile;
		if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
		{
			RoseFile->crc = FileInfo->dwCRC;
			RoseFile->version = FileInfo->dwVersion;
			return true;
		}
	}
	return false;
}

int CIndex::GetVfsCount() const
{
	return this->vfsCount;
}

int CIndex::GetVFSNames(char** Names, unsigned int Num, short MaxPathLength)
{
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		Names[distance(ListVFS->begin(), i)] = (*i)->GetVFSName();
		Names[distance(ListVFS->begin(), i)][MaxPathLength - 1] = 0;
	}
	return vfsCount;
}

int CIndex::GetFileCount(const char* VfsName)
{
	string Name = VfsName;
	transform(Name.begin(), Name.end(), Name.begin(), ::toupper); // This might not be needed, but you never know
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		if(!strcmp((*i)->GetVFSName(), Name.c_str()))
		{
			return ((*i)->GetFileCount() - (*i)->GetDeleteCount());
		}
	}
	return 0;
}

int CIndex::GetTotalFileCount()
{
	int FileCount = 0;
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		FileCount += ((*i)->GetFileCount() - (*i)->GetDeleteCount());
	}
	return FileCount;
}

int CIndex::GetFileNames(const char* VfsName, char** FileNames, int NumberToGet, int MaxPathLength)
{
	string Name = VfsName;
	transform(Name.begin(), Name.end(), Name.begin(), ::toupper);
	int Files = 0;
	int MaxCount = GetFileCount(Name.c_str());
	if(MaxCount == 0)
	{
		return 0;
	}
	if(NumberToGet > MaxCount)
	{
		NumberToGet = MaxCount;
	}
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		if(!strcmp((*i)->GetVFSName(), Name.c_str()))
		{
			for(int j = 0; Files < NumberToGet && j < MaxCount; j++)
			{
				if(!(*i)->Files->at(j)->deleted)
				{
					FileNames[distance(ListVFS->begin(), i)] = (*i)->Files->at(j)->path;
					FileNames[distance(ListVFS->begin(), i)][MaxPathLength - 1] = 0;
					Files++;
				}
			}
		}
	}
	return Files;
}

int CIndex::GetBaseVersion() const
{
	return this->baseVersion;
}

void CIndex::SetBaseVersion(int Version)
{
	this->baseVersion = Version;
}

int CIndex::GetCurrentVersion() const
{
	return this->currentVersion;
}

void CIndex::SetCurrentVersion(int Version)
{
	this->currentVersion = Version;
}

CVFSFile::File* CIndex::OpenFile(const char* FileName)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	if(FlatFile::Exist(NormalizedName))
	{
		FlatFile* FFile = new FlatFile(NormalizedName, "rb");
		if(FFile->IsOpen())
		{
			CVFSFile::File* RoseFile = new CVFSFile::File();
			RoseFile->path = NormalizedName;
			RoseFile->currentPosition = 0;
			RoseFile->deleted = false;
			RoseFile->lenght = FFile->Size();
			RoseFile->data = new unsigned char[RoseFile->lenght];
			FFile->ReadData(RoseFile->data, RoseFile->lenght);
			RoseFile->crc = 0;
			RoseFile->hash = 0;
			RoseFile->version = this->GetCurrentVersion();
			RoseFile->vfsIndex = 0;
			FFile->Close();
			delete FFile;
			FFile = NULL;
			return RoseFile;
		}
		else
		{
			delete FFile;
			FFile = NULL;
		}
	}
	else
	{
		for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
		{
			CVFSFile::File* RoseFile;
			if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
			{
				RoseFile->data = new unsigned char[RoseFile->lenght];
				(*i)->VFile->Seek(RoseFile->offset);
				(*i)->VFile->ReadData(RoseFile->data, RoseFile->lenght);
				RoseFile->currentPosition = 0;
				return RoseFile;
			}
		}
	}
	return 0;
}

int CIndex::GetFileSize(const char* FileName)
{
	CVFSFile::File* RoseFile = OpenFile(FileName);
	if(RoseFile)
	{
		return RoseFile->lenght;
	}
	return 0;
}

bool CIndex::FileExists(const char* FileName)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	if(FlatFile::Exist(NormalizedName))
	{
		return true;
	}
	else
	{
		for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
		{
			CVFSFile::File* RoseFile;
			if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
			{
				return true;
			}
		}
	}
	return false;
}

bool CIndex::FileExistsInVfs(const char* FileName)
{
	char* NormalizedName = new char[strlen(FileName) + 1];
	CIndex::NormalizePath(FileName, NormalizedName);
	for(vector<CVFSFile*>::iterator i = ListVFS->begin(); i != ListVFS->end(); ++i)
	{
		CVFSFile::File* RoseFile;
		if((*i)->FileTable.getHash(toHash(NormalizedName), &RoseFile))
		{
			return true;
		}
	}
	return false;
}