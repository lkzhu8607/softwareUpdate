#include "stdafx.h"
#include <io.h>
#include <iostream>
#include <fstream>
#include <ComDef.h>
#include "fileOp.h"
#include "windows.h"
#include "string"

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

int FileOp::getfileContent(const char *fileName, BYTE *buffer)
{
	//std::ifstream t;
	/*unsigned char szbuff[1024] = {0};
	while(!t.eof())
	{
			t.getline(szbuff,1024);
	}
	//int length;  
	t.open(fileName);     
	t.read((char *)buffer, length); */      // read the whole file into the buffer  
	int count = 0;
	int address = 0;
	int size = 0;
	ifstream file;  
	file.open(fileName,ios::in);  
  
	if(!file.is_open())  
		return 0;
	std::string strLine;  
	int j = 0;
    while(getline(file,strLine))  
    {  
		count = 0;
		address = 0;
		std::string dataStr;
		if(strLine.empty())  
			continue;
		sscanf(strLine.substr(1,2).c_str(),"%02X",&count);
		sscanf(strLine.substr(3,4).c_str(),"%02X",&address);
		
		if(count == 0)
			break;
		if(address == 0)
			continue;
		size += count;
		dataStr = strLine.substr(9,count*2);
		for(int i = 0;i < count; i++){
			sscanf(dataStr.substr(i*2,2).c_str(),"%02X",&buffer[j]);
			//cout<<buffer[j] <<endl;
			//printf("%02X", buffer[j]);
			j++;
		}
		//printf("\n");
		//cout<<count <<endl;
		//cout<<size <<endl;
		//cout<<dataStr<<endl; 
		//cout<<strLine <<endl;                
    }
	//cout<<size <<endl;
	file.close();                    // close file handle  
	//length = size;
	return size;
}
