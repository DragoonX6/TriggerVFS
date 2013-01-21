#ifndef FILE_H
#define FILE_H

#pragma warning(disable:4267)

class File {
public:
	File(){}
	virtual ~File(){}

	virtual bool Open(const char* path, const char* method) = 0;
	virtual void Close() = 0;
	//virtual void Delete() = 0;

	virtual int ReadData(void* data, int size) = 0;
	virtual int WriteData(void* data, int size) = 0;
	virtual int WriteData(void* data, int size, int pos){
		long start = Position();
		Seek(pos);
		int wrote = WriteData(data, size);
		Seek(start);
		return wrote;
	}

	virtual bool IsOpen() = 0;

	virtual long Position() = 0;
	virtual long Size() = 0;

	virtual void Skip(int bytes) = 0;
	virtual void Seek(int position) = 0;

	virtual bool IsEOF() = 0;

	template <class T> T Read(){
		T val;
		ReadData(&val, sizeof(T));
		return val;
	}

	template <class T> void Read(T& val){
		ReadData(&val, sizeof(T));
	}

	template <class T> void Write(T val){
		WriteData(&val, sizeof(T));
	}

	char* ReadString(unsigned int len){
		char *buf = new char[256];;
		ReadData(buf, len);
		buf[len] = 0;
		return buf;
	}

	unsigned char* ReadBytes(int length){
		unsigned char* buffer = new unsigned char[length];
		int size = length;
		if(ReadData(buffer, size))
			return buffer;

		delete [] buffer;
		return NULL;
	}

	unsigned char* ReadWholeFile(){
		Seek(0);
		long size = Size();
		unsigned char* buffer = new unsigned char[size + 1];
		ReadData(buffer, size);
		buffer[size] = 0;
		return buffer;
	}

	template <class T> char* ReadStringLength(){
		T length;
		ReadData(&length, sizeof(T));
		char buf[255];
		ReadData(buf, length);
		buf[length] = 0;
		return buf;
	}

	char* ReadTerminatedString(char terminator = 0){
		char *buffer = new char[255];
		for(int i = 0; i <= 255; ++i){
			if(i == 255) return 0;

			if(ReadData(&buffer[i], 1) != 1){
				if(i == 0){
					return 0;
				}else{
					buffer[i] = 0;
					break;
				}
			}

			if(buffer[i] == terminator) break;
		}

		return buffer;
	}

	char* ReadLine(){
		char *buffer = new char[255];
		for(int i = 0; i <= 255; ++i){
			if(i == 255) return 0;

			if(ReadData(&buffer[i], 1) != 1){
				if(i == 0){
					return 0;
				}else{
					buffer[i] = 0;
					break;
				}
			}

			if(buffer[i] == 0xA || buffer[i] == 0xD){
				buffer[i] = 0;
				break;
			}
		}

		char tmp;
		int pos = Position();
		ReadData(&tmp, 1);
		if(tmp != 0xA && tmp != 0xD) Seek(pos);

		return buffer;
	}

	char* ReadPascalString(){
		unsigned int length;
		unsigned char curBLen;
		ReadData(&curBLen, 1);
		length = curBLen & 0x7F;
		if(curBLen & 0x80){
			ReadData(&curBLen, 1);
			length |= (curBLen & 0x7F) << 7;
		}

		char *buf = new char[512];
		ReadData(buf, length);
		buf[length] = 0;

		return buf;
	}

	void ReadPascalString(char* str){
		unsigned int length;
		unsigned char curBLen;
		ReadData(&curBLen, 1);
		length = curBLen & 0x7F;
		if(curBLen & 0x80){
			ReadData(&curBLen, 1);
			length |= (curBLen & 0x7F) << 7;
		}

		ReadData(str, length);
		str[length] = 0;
	}

	int ReadPascalStringLen(){
		unsigned int length;
		unsigned char curBLen;
		ReadData(&curBLen, 1);
		length = curBLen & 0x7F;
		if(curBLen & 0x80){
			ReadData(&curBLen, 1);
			length |= (curBLen & 0x7F) << 7;
		}

		return length;
	}

	void WriteTerminatedString(const char* str, char terminator = 0){
		WriteData((void*)(const char*)str, strlen(str));
		WriteData(&terminator, 1);
	}

	template <class T> void WriteStringLength(const char* str){
		T length;
		length = strlen(str);
		WriteData(&length, sizeof(T));
		WriteData((void*)(const char*)str, length);
	}

	void WriteLine(const char* str){
		static const char* endl = "\r\n";

		WriteData((void*)str, strlen(str));
		WriteData((void*)endl, 2);
	}
};

#endif