#include "stdafx.h"
#include <io.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <ComDef.h>
#include "fileOp.h"
#include "config.h"
#include "string"
#include<vector>
#include "stdio.h"
#include "log.h"

Config objconfig;
Config::Config()
{
	//默认的串口参数
	Port = 2;
	Baudrate = 57600; 
	Databits = 8;
	Parity = 0;
	StopBits = 0;
	ConfigFileName = "TIM_Firmware_Update.ini";
}

Config::~Config()
{}

void Config::splitStringByDelimiter(const string& str,const string & strDelimiter,vector<string>& vtSubStr)  
{  
    int iLen = str.length();  
    int isize = str.size();  
    int iDelLen = strDelimiter.length();  
      
    int iFPos = 0; //First position  
    int iSPos=str.find(strDelimiter); //second position  
    while ( iSPos >= 0){  
        string substr = str.substr(iFPos,iSPos-iFPos);  
        if ( !substr.empty() ){  
            vtSubStr.push_back(substr);  
        }  
        iFPos = iSPos+iDelLen;  
        iSPos = str.find(strDelimiter,iFPos);  
    }  
    string substr = str.substr(iFPos,iLen-iFPos);  
    if ( !substr.empty() ){  
        vtSubStr.push_back(substr);  
    }  
    return;  
}  

int Config::getConfigInfo()
{
	char buffer[256];
	fstream file;
	file.open(ConfigFileName,ios::in);
	cout<<ConfigFileName<<" 的内容如下:"<<endl;
	while(!file.eof())
	{
	   file.getline(buffer,256,'\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
	   string s(buffer);
	   if(s.find("COM") != string::npos)
	   {
			cout<<s<<endl;
			vector<string> vtSubStr;
			splitStringByDelimiter(s, ",", vtSubStr);
			Port = static_cast<int>(strtol(vtSubStr[0].substr(3).c_str(),NULL,10));
			Baudrate = static_cast<int>(strtol(vtSubStr[1].c_str(),NULL,10)); 
	        Databits = static_cast<int>(strtol(vtSubStr[2].c_str(),NULL,10));
	        Parity = static_cast<int>(strtol(vtSubStr[3].c_str(),NULL,10));
	        StopBits = static_cast<int>(strtol(vtSubStr[4].c_str(),NULL,10));
			char str[1024] = {0};
			sprintf_s(str,"COM Config Into port: %s, baudrate: %s, databits: %s, parity: %s, stopbits: %s",vtSubStr[0].c_str(),vtSubStr[1].c_str(),vtSubStr[2].c_str(),vtSubStr[3].c_str(),vtSubStr[4].c_str());
			gp_objTimLog.logInfo(__FILE__, __LINE__, str);
			printf("port: %s, baudrate: %s, databits: %s, parity: %s, stopbits: %s\n",vtSubStr[0].c_str(),vtSubStr[1].c_str(),vtSubStr[2].c_str(),vtSubStr[3].c_str(),vtSubStr[4].c_str());
	   }
	   else if(s.find(".hex") != string::npos){
			ProgramName = s;
			//cout<<"ProgramName: "<<ProgramName<<endl;
			char str[1024] = {0};
			sprintf_s(str,"ProgramName: %s",ProgramName);
			gp_objTimLog.logInfo(__FILE__, __LINE__, str);
	   }
	}
	file.close();
	return 0;
}


