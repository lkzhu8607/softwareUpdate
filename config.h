#include <stdlib.h>
#include <iostream>
#include "string"
#include <stdio.h>
#include <vector>
using namespace std;
#pragma once

class Config
{
public:
	Config(void);
	~Config(void);
public:
	int Port;
	int Baudrate;
	int Databits;
	int Parity;
	int StopBits;
	string  ProgramName;
	string ConfigFileName;
public:
	int getConfigInfo();
	void splitStringByDelimiter(const string& str,const string & strDelimiter,vector<string>& vtSubStr);

};

extern Config objconfig;
