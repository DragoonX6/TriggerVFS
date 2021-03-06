#ifndef FLAT_FILE_H
#define FLAT_FILE_H

#include "File.hpp"
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <map>
#include <share.h>
using namespace std;

class FlatFile : public File {
public:
	FlatFile() : fh(0) {}

	FlatFile(const char* path, const char* method){
		Open(path, method);
	}

	~FlatFile(){
		Close();
	}

	static bool Exist(char* path)
	{
		if(_access(path, 4) && errno != EACCES)
		{
			return false;
		}
		return true;
	}

	bool Open(const char* path, const char* method)
	{
		fopen_s(&fh, path, method); 
		file = path;
		return IsOpen();
	}

	bool IsOpen(){
		return (fh != NULL);
	}

	void Close(){
		if(!fh) return;
		fclose(fh);
	}

	void Delete()
	{
		remove(file);
	}

	int ReadData(void* data, int size){
		return fread(data, size, 1, fh);
	}

	int WriteData(void* data, int size){
		return fwrite(data, size, 1, fh);
	}

	void Skip(int bytes){
		fseek(fh, bytes, SEEK_CUR);
	}

	void Seek(int position){
		fseek(fh, position, SEEK_SET);
	}

	void Seek(int position, int origin)
	{
		fseek(fh, position, origin);
	}

	long Position(){
		return ftell(fh);
	}

	long Size(){
		long pos = Position();
		fseek(fh, 0, SEEK_END);
		long pos2 = Position();
		Seek(pos);
		return pos2;
	}

	bool IsEOF(){
		return feof(fh) != 0;
	}

private:
	FILE* fh;
	const char* file;
};

#endif
