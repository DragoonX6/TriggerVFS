#include "TriggerVFS.h"
#include "Index.h"

CIndex* idx = 0;

bool __stdcall __ConvertPath(const char * path , char* path2 )
{
	return CIndex::NormalizePath(path, path2);
}

CIndex* __stdcall OpenVFS(const char * FileName, const char * Mode)
{
	if(!idx)
	{
		idx = new CIndex();
	}
	if(!idx->IsOpen())
	{
		if(!idx->Open(FileName, Mode))
		{
			delete idx;
			idx = NULL;
			return NULL;
		}
	}
	return idx;
}

void __stdcall CloseVFS(CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return; // invalid handle
	}
	hVFS->Close();
	delete hVFS;
	hVFS = NULL;
}

bool __stdcall VAddVfs(CIndex* hVFS, const char * VfsName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return false; // invalid handle
	}
	return hVFS->AddVFS(VfsName);
}

short __stdcall VAddFile(CIndex* hVFS, const char * VfsName, const char *FileName, const char * TargetName, DWORD dwVersion, DWORD dwCrc, 
						 BYTE btEncType, BYTE btCompress, bool bUseDel)
{
	UNREFERENCED_PARAMETER(btEncType);
	UNREFERENCED_PARAMETER(btCompress);
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 1; // invalid handle
	}
	return hVFS->AddFile(VfsName, FileName, TargetName, dwVersion, dwCrc, bUseDel);
}

short __stdcall VRemoveFile(CIndex* hVFS, const char *FileName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 4; // invalid handle
	}
	return hVFS->RemoveFile(FileName);
}

void __stdcall VClearBlankAll(CIndex* hVFS, VCALLBACK_CLEARBLANKALL CallBackProc)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return; // invalid handle
	}
	return hVFS->Defragment(CallBackProc);
}

void __stdcall VGetFileInfo(CIndex* hVFS, const char *FileName, VFileInfo* FileInfo, bool bCalCrc)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1) || FileInfo == 0)
	{
		return; // invalid handle
	}
	hVFS->GetFileInfo(FileName, FileInfo, bCalCrc);
}

bool __stdcall VSetFileInfo(CIndex* hVFS, const char *FileName, VFileInfo* FileInfo)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return false; // invalid handle
	}
	return hVFS->SetFileInfo(FileName, FileInfo);
}

int __stdcall VGetVfsCount(CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetVfsCount();
}

int __stdcall VGetVfsNames(CIndex* hVFS, char **ppFiles, DWORD dwNum, short dwMaxPath)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetVFSNames(ppFiles, dwNum, dwMaxPath);
}

int __stdcall VGetFileCount(CIndex* hVFS, const char *VfsName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetFileCount(VfsName);
}

int __stdcall VGetTotFileCount(CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetTotalFileCount();
}

int __stdcall VGetFileNames(CIndex* hVFS, const char *VfsName, char **FileName, int nNum, int nMax)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetFileNames(VfsName, FileName, nNum, nMax);
}

DWORD __stdcall VGetStdVersion(CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetBaseVersion();
}

void __stdcall VSetStdVersion(CIndex* hVFS, DWORD dwVersion)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return; // invalid handle
	}
	hVFS->SetBaseVersion(dwVersion);
}

DWORD __stdcall VGetCurVersion(CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetCurrentVersion();
}

void __stdcall VSetCurVersion(CIndex* hVFS, DWORD dwVersion)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return; // invalid handle
	}
	hVFS->SetCurrentVersion(dwVersion);
}

CVFSFile::File* __stdcall VOpenFile(const char* FileName, CIndex* hVFS)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->OpenFile(FileName);
}

void __stdcall VCloseFile(CVFSFile::File *hVFH)
{
	if(hVFH == 0 || hVFH == (CVFSFile::File*)(-1))
	{
		return; // invalid handle
	}
	delete[] hVFH->data;
	hVFH->data = NULL;
}

size_t __stdcall vfread(void *buffer, size_t size, size_t count, CVFSFile::File* pVFH)
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return 0;
	}

	if((pVFH->currentPosition + (size * count)) <= pVFH->length)
	{
		size_t readsize = size * count;
		if((pVFH->currentPosition + readsize) >= pVFH->lEndOff)
		{
			readsize = (pVFH->lEndOff - pVFH->currentPosition);
		}

		memcpy(buffer, pVFH->data + pVFH->currentPosition, readsize);
		pVFH->currentPosition += readsize;
		return (readsize / size);
	}
	return 0;
}

void* __stdcall vfgetdata(size_t * psize, CVFSFile::File* pVFH)
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return 0;
	}
	if(psize)
	{
		*psize = pVFH->length;
	}
	if(pVFH->data)
	{
		return pVFH->data;
	}
	else
	{
		return 0;
	}
}

int __stdcall vfseek(CVFSFile::File* pVFH, long offset, int origin )
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return 1;
	}
	if(origin == SEEK_SET)
	{
		pVFH->currentPosition = offset;
	}
	else if(origin == SEEK_CUR)
	{
		pVFH->currentPosition += offset;
	}
	else if(origin == SEEK_END)
	{
		pVFH->currentPosition = (pVFH->length - 1) + offset;
	}
	else
	{
		return 11; // io error etc or w/e
	}
	return 0;
}

long __stdcall vftell(CVFSFile::File* pVFH)
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return -1;
	}
	return pVFH->currentPosition;
}

int __stdcall vfeof(CVFSFile::File* pVFH)
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return -1;
	}
	return (pVFH->currentPosition == pVFH->length ? 1 : 0);
}

size_t __stdcall vfgetsize(CVFSFile::File* pVFH)
{
	if(pVFH == NULL || pVFH == (CVFSFile::File*)(-1))
	{
		return 0;
	}
	return pVFH->length;
}

size_t __stdcall VGetFileLength (CIndex* hVFS, const char *FileName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->GetFileSize(FileName);
}

bool __stdcall VFileExists (CIndex* hVFS, const char * FileName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	return hVFS->FileExists(FileName);
}

bool __stdcall VFileExistsInVfs (CIndex* hVFS, const char * FileName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return false; // invalid handle
	}
	return hVFS->FileExistsInVfs(FileName);
}

short __stdcall VGetError (void )
{
	return 0; // no error implementation yet (too lazy)
}

short __stdcall VTestFile (CIndex* hVFS, const char * FileName)
{
	// this is some useless unused function to see if a file exists and if the crc matches
	// and if it is in a folder or not, if there is enough memory to read it
	// not going to code this yet, too useless for everyone
	return 0; // automatic succes
}

DWORD __stdcall ComputeCrc (CIndex* hVFS, const char* FileName)
{
	if(hVFS == 0 || hVFS == (CIndex*)(-1))
	{
		return 0; // invalid handle
	}
	CVFSFile::File* RoseFile = hVFS->OpenFile(FileName);
	if(RoseFile)
	{
		unsigned int crc = CVFSFile::CalculateCrc32(RoseFile);
		VCloseFile(RoseFile);
		return crc;
	}
	return 0;
}