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
FileOp objfileOp;
//Config objconfig;
//TimLog gp_objTimLog;
//TimLog objTimLog;
BYTE proGramData[1024*1024*10] = {0};//����10M�Ĵ�С����Ϊ�˱�����³��������
CSjscOp objCSjscOp;

/**
param:
	[in] objconfig  �����ļ���������
	[in] objCSjscOp ���ڲ�������
	[in] programLength ���³���ĳ���
	[in] program���³���ı����ַ
return:
	0:���³ɹ�
	-1������ʧ��
**/
int UpdateProg(Config objconfig, CSjscOp objCSjscOp, int programLength, BYTE *program)
{
	int programPackets = 0;
	int sendProgramErrorFlag = 0;//0��ʾ�������ݹ����и�û�в�������1��ʾ�������ݹ����в����˴���
	if(programLength ==0 || NULL == program)
	{
		char str[1024] = {0};
		sprintf_s(str,"���������ļ���������");
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
		sprintf_s(str,"��Ҫ���� %d �������������",programPackets);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
	}
	//�򿪴���
	if(objCSjscOp.Start(objconfig, 3) == 1)
	{
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Succ to Open Serial Port!");
		//return -1;
		//exit(-1);
	}
	else{
		gp_objTimLog.logError(__FILE__, __LINE__, "Fail to Open Serial Port");
		//return -1;
		exit(-1);
	}
	int programTimes = 0;
	while(programTimes < 6)//��������ر������6��
	{
		BYTE data;
		int ret = 0;
		char str[1024] = {0};
		sprintf_s(str,"�� %d ������´ο�ʼ",programTimes);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);

		//���ͳ����������
		ret = objCSjscOp.OpUpdate(data);
		if(ret == 0){
			if(data == 0){
				//Ӳ��ģ���̿�ʼ
				ret = objCSjscOp.OpProgramStart(programLength, data);
				if(ret == 0){
					gp_objTimLog.logInfo(__FILE__, __LINE__, "������¿�ʼ��");
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
									sprintf_s(str,"Ӳ��ģ�鷢�ͱ�����ݵĵ� %d ������������Ҫ���͵��ܵĸ��³������ݴ�Ϊ %d", i, programPackets);
									gp_objTimLog.logError(__FILE__, __LINE__, str);
									//programTimes++;
									sendProgramErrorFlag = 1;
									break;									
								}
							}
							else
							{
								memset(str,'0',1024);
								sprintf_s(str,"Ӳ��ģ�鷢�ͱ�����ݵĵ� %d ������������Ҫ���͵��ܵĸ��³������ݴ�Ϊ %d", i, programPackets);
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
								gp_objTimLog.logInfo(__FILE__, __LINE__, "Succ to send the command of program end!");
								if(data == 0)
								{
									ret = objCSjscOp.OpProgramSwitchApp(data);
									if(ret == 0)
									{
										gp_objTimLog.logInfo(__FILE__, __LINE__, "Succ to send the command of program switch app!");
										if(data == 0)
										{
											memset(str,'0',1024);
											sprintf_s(str,"Ӳ��ģ���̽�������Ӧ�ó�������Ӧ����ȷ���� %d �γ�����³ɹ�",programTimes);
											gp_objTimLog.logError(__FILE__, __LINE__, str);		
											return 0;
										}
										else
										{
											memset(str,'0',1024);
											sprintf_s(str,"Ӳ��ģ���̽�������Ӧ�ó�������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
											gp_objTimLog.logError(__FILE__, __LINE__, str);
											programTimes++;
											continue;
										}
									}
									else
									{
										memset(str,'0',1024);
										sprintf_s(str,"Ӳ��ģ���̽�������Ӧ�ó�������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
										gp_objTimLog.logError(__FILE__, __LINE__, str);
										programTimes++;
										continue;
									}
								}
								else
								{
									memset(str,'0',1024);
									sprintf_s(str,"Ӳ��ģ���̽�������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
									gp_objTimLog.logError(__FILE__, __LINE__, str);
									programTimes++;
									continue;									
								}
							}
							else
							{
								memset(str,'0',1024);
								sprintf_s(str,"Ӳ��ģ���̽�������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
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
						sprintf_s(str,"Ӳ��ģ���̿�ʼ����Ӧ����󣬵� %d �γ������ʧ��",programTimes);
						gp_objTimLog.logError(__FILE__, __LINE__, str);
						programTimes++;
						continue;	
					}
				}
				else
				{
					memset(str,'0',1024);
					sprintf_s(str,"Ӳ��ģ���̿�ʼ����Ӧ����󣬵� %d �γ������ʧ��",programTimes);
					gp_objTimLog.logError(__FILE__, __LINE__, str);
					programTimes++;
					continue;				
				}
			}
			else
			{
				memset(str,'0',1024);
				sprintf_s(str,"Ӳ�ҳ����������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
				gp_objTimLog.logError(__FILE__, __LINE__, str);
				programTimes++;
				continue;
			}
		}
		else{
			memset(str,'0',1024);
			sprintf_s(str,"Ӳ�ҳ����������Ӧ����󣬵� %d �γ������ʧ��",programTimes);
			gp_objTimLog.logError(__FILE__, __LINE__, str);
			programTimes++;
			continue;
		}
	}
	return -1;
	//��ʼ���³���
	//objCSjscOp.OpUpdate();
}


int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 0;
	int programLength = 0;
	

	//ɾ��ǰ�θ��³���ʱ����־
	if(objfileOp.fileIsExist("TIM_Firmware_Update.log") == 0)
	{
		//int l = objfileOp.getfileSize("TIM_Firmware_Update.log");
		//printf("TIM_Firmware_Update.log size is %d",l);
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

	//��������ļ��Ƿ����
	if(objfileOp.fileIsExist(("TIM_Firmware_Update.ini")) == 0)
	{
		objconfig.getConfigInfo();
	}
	else{
		//printf("config file is not exist!\n");
		gp_objTimLog.logError(__FILE__, __LINE__, "config file is not exist!");
		//exit(1);
	}

	//�����³����Ƿ����
	if(objfileOp.fileIsExist(objconfig.ProgramName.c_str()) == 0)
	{
		//printf("program file: %s exist\n",objconfig.ProgramName.c_str());
		char str[1024] = {0};
		sprintf_s(str,"Program file: %s is not exist!",objconfig.ProgramName.c_str());
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
	}
	else{//���³����ļ������ڵĻ���ֱ���˳�����
		//printf("Program file: %s is not exist!\n",objconfig.ProgramName.c_str());
		//string str = "Program file: " + objconfig.ProgramName.c_str() + "is not exist!";
		char str[1024] = {0};
		sprintf_s(str,"Program file: %s is not exist!",objconfig.ProgramName.c_str());
		gp_objTimLog.logError(__FILE__, __LINE__, str);
		exit(1);
	}

	if(programLength = objfileOp.getfileSize(objconfig.ProgramName.c_str()))
	{
		char str[1024] = {0};
		sprintf_s(str,"Program file length is: %d!",programLength);
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);
		objfileOp.getfileContent(objconfig.ProgramName.c_str(), programLength, proGramData);
		//printf("%s content:\n",objconfig.ProgramName.c_str());
		/*for(int i=0;i<programLength;i++)
		{
			printf("%c",proGramData[i]);
		}*/
		if(programLength == sizeof(proGramData))
		{
			//printf("��ȡ�ɹ�");
			gp_objTimLog.logInfo(__FILE__, __LINE__, "�����ȡ�ɹ�");
		}
		if(UpdateProg(objconfig, objCSjscOp, programLength, proGramData) == 0)
		{
			unsigned short MajorVersion;
			unsigned short MinorVersion;
			gp_objTimLog.logInfo(__FILE__, __LINE__, "������²����ɹ�");
			ret = objCSjscOp.OpGetVer(&MajorVersion, &MinorVersion);
			if(ret == 0)
			{
				char str[1024] = {0};
				sprintf_s(str,"��ȡ�³���İ汾�ųɹ����³���İ汾����:���汾�� %d, �ΰ汾�� %d",MajorVersion, MinorVersion);
				gp_objTimLog.logInfo(__FILE__, __LINE__, str);
				objfileOp.deletefile(objconfig.ProgramName.c_str());//���ɾ�����µĳ����ļ�
			}
			else
			{
				//char str[1024] = {0};
				//sprintf_s(str,"��ȡ�³���İ汾��ʧ��");
				gp_objTimLog.logError(__FILE__, __LINE__, "��ȡ�³���İ汾��ʧ��");
			}
		}
	}
	return 0;
}
