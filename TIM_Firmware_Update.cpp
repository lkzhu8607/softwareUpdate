// TIM_Firmware_Update.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <fstream>
#include <ComDef.h>
#include "fileOp.h"
#include "config.h"
#include "log.h"
#include "SjscOp.h"


using namespace std;
//#define DELETE_FLAG
//#define FOR_NEW
//#define FOR_TAIWAN
FileOp objfileOp;

BYTE proGramData[1024*1024*10] = {0};//分配10M的大小容量为了保存更新程序的数据
CSjscOp objCSjscOp;

/**
param:
	[in] objconfig  配置文件操作对象
	[in] objCSjscOp 串口操作对象
	[in] programLength 更新程序的长度
	[in] program更新程序的保存地址
return:
	0:更新成功
	-1：更新失败
**/
int UpdateProg(Config objconfig, CSjscOp objCSjscOp, int programLength, BYTE *program)
{
	int programPackets = 0;
	int sendProgramErrorFlag = 0;//0表示发送数据过程中更没有产生错误；1表示发送数据过程中产生了错误
	if(programLength ==0 || NULL == program)
	{
		char str[1024] = {0};
		sprintf_s(str,"升级程序文件内无数据");
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
		return -1;
	}
	else
	{
		if(programLength % 64 == 0)
		{
			programPackets = programLength/64;
		}
		else
		{
			programPackets = programLength/64 + 1;
		}
		char str[1024] = {0};
		sprintf_s(str,"将要发送 %d 串程序更新数据",programPackets);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
	}
	//打开串口
	if(objCSjscOp.Start(objconfig, 3) == 1)
	{
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Succ to Open Serial Port!");
	}
	else{
		gp_objTimLog.logError(__FILE__, __LINE__, "Fail to Open Serial Port");
		exit(-1);
	}
	int programTimes = 0;
	while(programTimes < 6)//编程最大的重编次数是6次
	{
		BYTE data;
		int ret = 0;
		char str[1024] = {0};
		sprintf_s(str,"第 %d 程序更新次开始",programTimes);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);

		//发送程序更新命令
		ret = objCSjscOp.OpUpdate(data);
		if(ret == 0){
			if(data == 0){
				//硬币模块编程开始
				Sleep(6000);
				ret = objCSjscOp.OpProgramStart(programLength, data);
				if(ret == 0){
					gp_objTimLog.logInfo(__FILE__, __LINE__, "程序更新开始！");
					Sleep(5000);
					if(data == 0)
					{
						int i = 0;
						BYTE tmpData[64] = {0};
						for(i=0; i<programPackets; i++)
						{
							memset(tmpData,0,64);
							if(i == (programPackets-1))
							{
								memcpy(tmpData, program+(i*64), (programLength - (i * 64)));
								gp_objTimLog.logInfo(__FILE__, __LINE__, "The last program block!");
							}
							else
							{
								memcpy(tmpData, program+(i*64), 64);
							}							
							ret = objCSjscOp.OpSendProgramData(tmpData,data);
							if(ret == 0)
							{
								if(data == 0)
								{
									continue;
								}
								else
								{
									memset(str,'0',1024);
									sprintf_s(str,"硬币模块发送编程数据的第 %d 串产生错误，需要发送的总的更新程序数据串为 %d", i, programPackets);
									gp_objTimLog.logError(__FILE__, __LINE__, str);
									//programTimes++;
									sendProgramErrorFlag = 1;
									break;									
								}
							}
							else
							{
								memset(str,'0',1024);
								sprintf_s(str,"硬币模块发送编程数据的第 %d 串产生错误，需要发送的总的更新程序数据串为 %d", i, programPackets);
								gp_objTimLog.logError(__FILE__, __LINE__, str);
								//programTimes++;
								sendProgramErrorFlag = 1;
								break;	
							}
						}
						if(sendProgramErrorFlag == 0)
						{
							ret = objCSjscOp.OpProgramEnd(data);
							if(ret == 0)
							{
								if(data == 0)
								{
									gp_objTimLog.logInfo(__FILE__, __LINE__, "程序更新成功");
									ret = objCSjscOp.OpProgramSwitchApp(data);
									if(ret == 0)
									{
										if(data == 0)
										{
											gp_objTimLog.logInfo(__FILE__, __LINE__, "成功从编程态切换到应用态");

											unsigned short MajorVersion = 0;
											unsigned short MinorVersion = 0;
											unsigned short slaveMajorVersion = 0;
											unsigned short slaveMinorVersion = 0;
											//gp_objTimLog.logInfo(__FILE__, __LINE__, "程序更新操作成功");
											Sleep(10000);
											ret = objCSjscOp.OpGetVer(&MajorVersion, &MinorVersion);
											if(ret == 0)
											{
												char str[1024] = {0};
												sprintf_s(str,"获取新程序的版本号成功，新程序的版本如下:主版本号 %d, 次版本号 %d",MajorVersion, MinorVersion);
												gp_objTimLog.logInfo(__FILE__, __LINE__, str);
												//return 0;
											}
											else
											{
												ret = objCSjscOp.OpGetVerTW(&MajorVersion, &MinorVersion, &slaveMajorVersion, &slaveMinorVersion);
												if(ret == 0)
												{
													char str[1024] = {0};
													sprintf_s(str,"获取新程序的版本号成功，新程序的版本如下:主边主版本号 %d, 主边次版本号 %d;副边主版本号 %d, 副边次版本号 %d",MajorVersion, MinorVersion, slaveMajorVersion, slaveMinorVersion);
													gp_objTimLog.logInfo(__FILE__, __LINE__, str);
												}
												//else
													//gp_objTimLog.logError(__FILE__, __LINE__, "获取新程序的版本号失败");
											}

											return 0;
										}
										else
										{
											memset(str,'0',1024);
											sprintf_s(str,"硬币模块编程结束换到应用程序命令应答错误，第 %d 次程序更新失败",programTimes);
											gp_objTimLog.logError(__FILE__, __LINE__, str);
											programTimes++;
											continue;
										}
									}
									else
									{
										memset(str,'0',1024);
										sprintf_s(str,"硬币模块编程结束换到应用程序命令应答错误，第 %d 次程序更新失败",programTimes);
										gp_objTimLog.logError(__FILE__, __LINE__, str);
										programTimes++;
										continue;
									}
								}
								else
								{
									memset(str,'0',1024);
									sprintf_s(str,"硬币模块编程结束命令应答错误，第 %d 次程序更新失败",programTimes);
									gp_objTimLog.logError(__FILE__, __LINE__, str);
									programTimes++;
									continue;									
								}
							}
							else
							{
								memset(str,'0',1024);
								sprintf_s(str,"硬币模块编程结束命令应答错误，第 %d 次程序更新失败",programTimes);
								gp_objTimLog.logError(__FILE__, __LINE__, str);
								programTimes++;
								continue;								
							}
						}
						else if(sendProgramErrorFlag ==1)
						{
							programTimes++;
							continue;
						}
					}
					else
					{
						memset(str,'0',1024);
						sprintf_s(str,"硬币模块编程开始命令应答错误，第 %d 次程序更新失败",programTimes);
						gp_objTimLog.logError(__FILE__, __LINE__, str);
						programTimes++;
						continue;	
					}
				}
				else
				{
					memset(str,'0',1024);
					sprintf_s(str,"硬币模块编程开始命令应答错误，第 %d 次程序更新失败",programTimes);
					gp_objTimLog.logError(__FILE__, __LINE__, str);
					programTimes++;
					continue;				
				}
			}
			else
			{
				memset(str,'0',1024);
				sprintf_s(str,"硬币程序更新命令应答错误，第 %d 次程序更新失败",programTimes);
				gp_objTimLog.logError(__FILE__, __LINE__, str);
				programTimes++;
				continue;
			}
		}
		else{
			memset(str,'0',1024);
			sprintf_s(str,"硬币程序更新命令应答错误，第 %d 次程序更新失败",programTimes);
			gp_objTimLog.logError(__FILE__, __LINE__, str);
			programTimes++;
			continue;
		}
	}
	return -1;
	//开始更新程序
	//objCSjscOp.OpUpdate();
}


int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 0;
	int programLength = 0;

	//删除前次更新程序时的日志
	if(objfileOp.fileIsExist("TIM_Firmware_Update.log") == 0)
	{
		if(objfileOp.deletefile("TIM_Firmware_Update.log") == 0){
			//printf("succ to delete older log file\n");
			gp_objTimLog.logInfo(__FILE__, __LINE__, "succ to delete older log file");
		}
		else{
			//printf("succ to delete older log file: %s\n","TIM_Firmware_Update.log");
			gp_objTimLog.logInfo(__FILE__, __LINE__, "succ to delete older log file: TIM_Firmware_Update.log");
		}
	}
	else
	{
		//printf("log file : %s is not exist\n","TIM_Firmware_Update.log");
		gp_objTimLog.logInfo(__FILE__, __LINE__, "log file : TIM_Firmware_Update.log is not exist");
	}

	//检测配置文件是否存在
	if(objfileOp.fileIsExist(("TIM_Firmware_Update.ini")) == 0)
	{
		objconfig.getConfigInfo();
	}
	else{
		//printf("config file is not exist!\n");
		gp_objTimLog.logError(__FILE__, __LINE__, "config file is not exist!");
		//exit(1);
	}

	//检测更新程序是否存在
	if(objfileOp.fileIsExist(objconfig.ProgramName.c_str()) == 0)
	{
		//printf("program file: %s exist\n",objconfig.ProgramName.c_str());
		char str[1024] = {0};
		sprintf_s(str,"Program file: %s is not exist!",objconfig.ProgramName.c_str());
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
	}
	else{//更新程序文件不存在的话，直接退出程序
		//printf("Program file: %s is not exist!\n",objconfig.ProgramName.c_str());
		char str[1024] = {0};
		sprintf_s(str,"Program file: %s is not exist!",objconfig.ProgramName.c_str());
		gp_objTimLog.logError(__FILE__, __LINE__, str);
		exit(1);
	}

	//if(programLength = objfileOp.getfileSize(objconfig.ProgramName.c_str()))
	//{

		programLength = objfileOp.getfileContent(objconfig.ProgramName.c_str(), proGramData);
		char str[1024] = {0};
		sprintf_s(str,"Program file length is: %d!",programLength);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);

		if(UpdateProg(objconfig, objCSjscOp, programLength, proGramData) == 0)
		{
//#ifdef DELETE_FLAG
			objfileOp.deletefile(objconfig.ProgramName.c_str());//最后删除更新的程序文件
			exit(1);
/*#else
			exit(1);
#endif*/
		}
	//}
	return 0;
}

