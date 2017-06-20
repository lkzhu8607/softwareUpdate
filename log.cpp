#include "stdafx.h"
#include <stdlib.h>
#include <iostream>
/*#include <string>
#include <stdio.h>
#include<ctime>
#include <windows.h>
#include <fstream>*/
#include<ctime>
#include "log.h"

TimLog gp_objTimLog;

std::string getTime()
{
	time_t t;
    time(&t);
    struct tm *now = localtime(&t);
	char timeStr[32] = {0};
	strftime(timeStr,sizeof(timeStr),"%Y-%m-%d %H:%M:%S",now);
	string s(timeStr);
	return s;
}

TimLog::TimLog(void)
{
	fHandle.open("TIM_Firmware_Update.log");
}
TimLog::~TimLog(void)
{
	fHandle.close();
}
void TimLog::logInfo(const char*fileName, int lineNum, string info)
{
	fHandle<<"INFO: "<<getTime()<<" | "<<fileName<<" | "<<lineNum<<" | "<<__FUNCTION__<<" | "<<info<<"\n";
	fHandle << flush;
}
void TimLog::logError(const char*fileName, int lineNum, string info)
{
	fHandle<<"ERROR: "<<getTime()<<" | "<<fileName<<" | "<<lineNum<<" | "<<__FUNCTION__<<" | "<<info<<"\n";
	fHandle << flush;
}
void TimLog::logWarn(const char*fileName, int lineNum, string info)
{
	fHandle<<"WARN: "<<getTime()<<" | "<<fileName<<" | "<<lineNum<<" | "<<__FUNCTION__<<" | "<<info<<"\n";
	fHandle << flush;
}
void TimLog::logFatal(const char*fileName, int lineNum, string info)
{
	fHandle<<"FATAL: "<<getTime()<<" | "<<fileName<<" | "<<lineNum<<" | "<<__FUNCTION__<<" | "<<info<<"\n";
	fHandle << flush;
}
