#include <iostream>
#include <windows.h>            //winapi��ͷ�ļ�
//#include <cstring>
using namespace std;
#pragma once

class FileOp
{
public:
	FileOp(void);
	~FileOp(void);
public:
	HANDLE hfile;
	int fileSize;
	int pos;
public:
	int fileIsExist(LPCWSTR fileName);
	int fileIsExist(const char *fileName);	
	int deletefile(LPCWSTR fileName);
	int deletefile(const char *fileName);
	int getfileSize(const char *fileName);
	int getfileContent(const char *fileName, int length, BYTE *buffer);
};