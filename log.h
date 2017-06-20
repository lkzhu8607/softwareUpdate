#include "stdafx.h"
#include <stdlib.h>
#include "string"
#include <stdio.h>
#include <windows.h>
#include <fstream>
#include <iostream>

using namespace std;
#pragma once

class TimLog
{
public:
	TimLog(void);
	~TimLog(void);
public:
	ofstream fHandle;
public:
	void logInfo(const char*fileName, int lineNum, string info);
	void logError(const char*fileName, int lineNum, string info);
	void logWarn(const char*fileName, int lineNum, string info);
	void logFatal(const char*fileName, int lineNum, string info);
};

extern TimLog gp_objTimLog;