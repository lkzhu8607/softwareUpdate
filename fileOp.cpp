#include "stdafx.h"
#include <io.h>
#include <iostream>
#include <fstream>
#include <ComDef.h>
#include "fileOp.h"
#include "windows.h"


//using namespace std;
FileOp::FileOp(void)
{
}

FileOp::~FileOp(void)
{
}

int FileOp::fileIsExist(const char *fileName)
{
	ifstream fin(fileName);
	if(!fin)
	{
		//printf ("%s is not exist.\n", fileName);
		return -1;
	}
	else{
		//printf ("%s exist.\n", fileName);
		return 0;
	}
	return 0;
}

int FileOp::fileIsExist(LPCWSTR fileName)
{
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	hFind = FindFirstFile(fileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		//printf ("%s is not exist.\n", fileName);
		return -1;
	 } else {
		//printf ("%s exist.\n", fileName);
		FindClose(hFind);
		return 0;
	 }
	return 0;
}

int FileOp::deletefile(LPCWSTR fileName)
{
	if(!DeleteFile(fileName)){
		//printf("succ to delete %s\n",fileName);
		return -1;
	}
	else
		return 0;
	return 0;
}

int FileOp::deletefile(const char *fileName)
{
	if(remove(fileName)){
		//printf("fail to delete %s\n",fileName);
		return -1;
	}
	else{
		//printf("succ to delete %s\n",fileName);
		return 0;
	}
	return 0;
}

int FileOp::getfileSize(const char *fileName)
{
    FILE * fp = fopen(fileName, "r");
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
	/*std::ifstream t;  
	int length;  
	t.open(fileName);      // open input file  
	t.seekg(0, std::ios::end);    // go to the end  
	length = t.tellg();           // report location (this is the length)  
	t.seekg(0, std::ios::beg);    // go back to the beginning  
	//buffer = new char[length];    // allocate memory for a buffer of appropriate dimension  
	t.read((char *)buffer, length);       // read the whole file into the buffer  
	t.close();                    // close file handle  
	return length;*/
}

int FileOp::getfileContent(const char *fileName, int length, BYTE *buffer)
{
	std::ifstream t;  
	//int length;  
	t.open(fileName);     
	t.read((char *)buffer, length);       // read the whole file into the buffer  
	t.close();                    // close file handle  
	return 0;
}
