#ifndef __TRIGGERVFS_H__
#define __TRIGGERVFS_H__

#include <Windows.h>

#ifdef TRIGGERVFS_EXPORTS
#include"VFSFile.h"
#include "Index.h"
#else
class CIndex;
class CVFSFile
{
public:
	struct File
	{
		unsigned int currentPosition;
		unsigned char* data;
		char* path;
		unsigned int offset;
		unsigned int length;
		unsigned int lEndOff;
		int hash;
		unsigned int crc;
		int version;
		bool deleted;
		int vfsIndex;
		unsigned char btEncrypted;
	};
};
struct VFileInfo
{
	DWORD dwVersion;
	DWORD dwCRC;
};
#endif

#ifdef TRIGGERVFS_EXPORTS
	#define TRIGGERVFS_API __declspec(dllexport)
#else
	#define TRIGGERVFS_API __declspec(dllimport)
#endif

typedef		 void   (__stdcall *  VCALLBACK_CLEARBLANKALL) ( int );

/**********************************************************************************************
 *                                      VFS Interface
 **********************************************************************************************
 */
#ifdef	__cplusplus
extern "C" {
#endif

/**
 * vfseek
 */
#define VFSEEK_CUR SEEK_CUR
#define VFSEEK_END SEEK_END
#define VFSEEK_SET SEEK_SET

/*! @brief Converts a path to a path suited for loading and searching in the VFS
 * @param path, IN, String that needs to be converted to a correct path for file loading
 * @param path2, OUT, Converted string
 * @return always true
 */
TRIGGERVFS_API bool __stdcall __ConvertPath (const char * path , char * path2 );

/*! @brief Open the Rose filesystem
 * @param FileName, IN
 * @param Mode, Ignored
 * @return Pointer of CIndex* idx
 */
TRIGGERVFS_API CIndex* __stdcall OpenVFS (const char * FileName, const char * Mode = "r");

/*! @brief Close the Rose filesystem
 * @param hVFS, IN, put here the pointer obtained from OpenVFS
 */
TRIGGERVFS_API void __stdcall CloseVFS (CIndex* hVFS);

/*! @brief Add VFS file to the filesystem
 * @param hVFS, IN, OpenVFS pointer
 * @VfsName, IN, The name of the new VFSFile
 * @return Fails if CIndex pointer is invalid or VfsName == ROOT.VFS
 */
TRIGGERVFS_API bool __stdcall VAddVfs (CIndex* hVFS, const char * VfsName);

/*! @brief Adds a file to the specified VFS
 * @param hVFS, IN, OpenVFS pointer
 * @param VfsName, IN
 * @param FileName, IN, the name of the file in the folder
 * @param TargetName, IN, the path of the file in the VFS
 * @param dwVersion, IN, the version of the file, presumably currentverion + 1
 * @param dwCrc, IN, throw CalcCrc in here
 * @param btEncType, Ignored
 * @param btCompress, Ignored
 * @param bUseDel, IN, specifies if the file is initially deleted or not, you may want to set this to false
 * @return
 * returns VADDFILE_SUCCESS when everything went ok
 * VADDFILE_INVALIDHANDLE if the pointer gotten from OpenVFS is invalid (param 1)
 * VADDFILE_INVALIDVFS if the VFS is nonexistent
 * VADDFILE_CANTOPENFILE if it can't open FileName
 * VADDFILE_EXISTSALREADY if the file already exists
 * VADDFILE_DONTEXIST if FileName doesn't exist
 * VADDFILE_MEMALLOCFAIL if it can't allocate memory for the file
 * VADDFILE_ZEROFILESIZE if the file is empty
 */
TRIGGERVFS_API short __stdcall VAddFile (CIndex* hVFS, const char * VfsName, const char *FileName, const char * TargetName, DWORD dwVersion, DWORD dwCrc, 
			   BYTE btEncType, BYTE btCompress, bool bUseDel);

/*! @brief Removes a file from the specified VFS
 * @param hVFS, IN, OpenVFS pointer
 * @param FileName, IN, file to remove
 * @return 
 * returns VRMVFILE_SUCCESS when everything went ok
 * VRMVFILE_DONTEXIST if the file doesn't exist
 * VRMVFILE_INVALIDHANDLE if the pointer gotten from OpenVFS is invalid (param 1)
 */
TRIGGERVFS_API short __stdcall VRemoveFile (CIndex* hVFS, const char *FileName);

/*! @brief Defragment the Rose filesystem
 * See the example on osrose on how to work with this function
 * @param hVFS, IN, OpenVFS pointer
 * @param CallBackProc, IN, address of the callback function
 */
TRIGGERVFS_API void __stdcall VClearBlankAll (CIndex* hVFS, VCALLBACK_CLEARBLANKALL CallBackProc = NULL);

/*! @brief Get the version and the crc of a file
 * @param hVFS, IN, OpenVFS pointer
 * @param FileName, IN
 * @param FileInfo, REF
 * @param CalcCrc, specify if the crc has to be calculated for the file.
 */
TRIGGERVFS_API void __stdcall VGetFileInfo (CIndex* hVFS, const char *FileName, VFileInfo* FileInfo, bool bCalCrc);

/*! @brief Set the version and the crc of a file
 * @param hVFS, IN, OpenVFS pointer
 * @param FileName, IN
 * @param FileInfo, IN
 * @return true at succes, false at fail
 */
TRIGGERVFS_API bool __stdcall VSetFileInfo (CIndex* hVFS, const char *FileName, VFileInfo* FileInfo);

/*! @brief 
 */
TRIGGERVFS_API int __stdcall VGetVfsCount (CIndex* hVFS);

TRIGGERVFS_API int __stdcall VGetVfsNames (CIndex* hVFS, char **ppFiles, DWORD dwNum, short dwMaxPath);

TRIGGERVFS_API int __stdcall VGetFileCount (CIndex* hVFS, const char *VfsName);

TRIGGERVFS_API int __stdcall VGetTotFileCount (CIndex* hVFS);

TRIGGERVFS_API int __stdcall VGetFileNames (CIndex* hVFS, const char *VfsName, char **FileName, int nNum, int nMax);

TRIGGERVFS_API DWORD __stdcall VGetStdVersion (CIndex* hVFS);

TRIGGERVFS_API void __stdcall VSetStdVersion (CIndex* hVFS, DWORD dwVersion);

TRIGGERVFS_API DWORD __stdcall VGetCurVersion (CIndex* hVFS);

TRIGGERVFS_API void __stdcall VSetCurVersion (CIndex* hVFS, DWORD dwVersion);

TRIGGERVFS_API CVFSFile::File* __stdcall VOpenFile (const char* FileName, CIndex* hVFS);

TRIGGERVFS_API void __stdcall VCloseFile (CVFSFile::File *hVFH);

TRIGGERVFS_API size_t __stdcall vfread (void *buffer, size_t size, size_t count, CVFSFile::File* pVFH);

TRIGGERVFS_API void* __stdcall vfgetdata (size_t * psize, CVFSFile::File* pVFH);

TRIGGERVFS_API int __stdcall vfseek (CVFSFile::File* pVFH, long offset, int origin );

TRIGGERVFS_API long __stdcall vftell (CVFSFile::File* pVFH);

TRIGGERVFS_API int __stdcall vfeof (CVFSFile::File* pVFH);

TRIGGERVFS_API size_t __stdcall vfgetsize (CVFSFile::File* pVFH);

TRIGGERVFS_API size_t __stdcall VGetFileLength (CIndex* hVFS, const char *FileName);

TRIGGERVFS_API bool __stdcall VFileExists (CIndex* hVFS, const char * FileName);

TRIGGERVFS_API bool __stdcall VFileExistsInVfs (CIndex* hVFS, const char * FileName);

TRIGGERVFS_API short __stdcall VGetError (void );

TRIGGERVFS_API short __stdcall VTestFile (CIndex* hVFS, const char * FileName);

TRIGGERVFS_API DWORD __stdcall ComputeCrc (CIndex* hVFS, const char * FileName);

#ifdef	__cplusplus
}
#endif


//extern short g_nLastErr;

#define VERR_NONE					0
#define VERR_DELETE_CANTFIND		1
#define VERR_MEM_ALLOCFAIL			2


/***************************************************************
 * VTestFile
 */
#define	VTEST_SUCCESS				0
#define	VTEST_INVALIDHANDLE			1
#define	VTEST_CANTOPEN				2
#define	VTEST_NOTENOUGHMEM			3
#define	VTEST_LENGTHNOTMATCH		4
#define	VTEST_CRCNOTMATCH			5
#define	VTEST_FILENOTEXISTS			6
#define	VTEST_INVALIDRANGE			7
#define VTEST_OUTFILE				8
#define VTEST_CANTKNOWVFSINFO		9
#define VTEST_ZEROLENGTH			10


/******************************************************************
 * VAddFile  Error Code
 */
#define	VADDFILE_SUCCESS				0
#define	VADDFILE_INVALIDHANDLE			1
#define	VADDFILE_INVALIDVFS				2
#define	VADDFILE_CANTOPENFILE			3
#define	VADDFILE_EXISTSALREADY			4
#define	VADDFILE_DONTEXIST				5
#define	VADDFILE_MEMALLOCFAIL			6
#define	VADDFILE_CANTWRITE				7 // probably no rights to write
#define	VADDFILE_CANTFLUSH				8
#define	VADDFILE_CVFS_AddFile_FAILURE	9 // unable to add file
#define	VADDFILE_ZEROFILESIZE			10
#define	VADDFILE_FIOERROR_ETC			11 // unhandled file error
#define VADDFILE_CVFS_AddFile_NONOE		12 // ?

#define VADDFILE_FIXEVALUE_CANTWRITE		15000
#define VADDFILE_FIXEVALUE_CANTFLUSH		20000


/******************************************************************
 * VRemoveFile  Error Code
 */
#define VRMVFILE_SUCCESS			0
#define VRMVFILE_INVALIDVFS			1
#define VRMVFILE_MEMALLOCFAIL		2
#define VRMVFILE_DONTEXIST			3
#define VRMVFILE_INVALIDHANDLE		4

#endif