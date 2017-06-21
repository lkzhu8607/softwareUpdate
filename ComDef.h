#include "StdAfx.h"
#include "windows.h"
#include "WinBase.h"
#pragma once
//typedef unsigned char BYTE;
//typedef unsigned int UINT;
//typedef int BOOL;

//RECV DATA STATE
const BYTE STATE_IDLE		= 0;
const BYTE STATE_RECVDATA	= 1;

//const CString	SJSC_API_VERSION = _T("DLL API Ver: Sjsc-API$Ver1.6.3.0");
const int _RECV_BUFFER_LEN_	= 300;		//报文最大长度

//控制代码
const BYTE STX	= 0x02;
const BYTE ETX	= 0x03;
const BYTE DLE	= 0x10;

//命令代码--请求命令
#define CMD_GETVERSION_OLD				0x08	//查询台湾闸机模块程序版本
#define CMD_GETSTATUS				0x31	//发卡模块轮询
#define CMD_MOTORTEST				0x32	//电机测试
#define CMD_RESET					0x33	//复位
#define CMD_GETVERSION				0x34	//查询发卡模块程序版本
#define CMD_FEEDTICKET				0x20	//将卡发到读写位置
#define CMD_ISSUETICKET				0x21	//出卡到暂存箱或者废票箱
#define CMD_DISPENSETICKET			0x26	//将暂存斗中的车票发给乘客

#define CMD_UPDATE                  0x35    //硬币模块程序更新命令
#define CMD_PROGRAM_START           0x36    //硬币模块编程开始,发送数据长度
#define CMD_SEND_PROGRAM_DATA       0x37    //硬币模块发送编程数据
#define CMD_PROGRAM_END             0x38    //硬币模块编程结束
#define CMD_SWITCH_APP              0x39    //硬币模块编程结束换到应用程序

//模块上发COMState
const BYTE MODULE_STATE_OK		= 0x00;	//命令执行成功
const BYTE MODULE_STATE_FAIL	= 0x01;	//命令执行失败
const BYTE MODULE_STATEBUSY		= 0x02;	//设备忙
const BYTE MODULE_STATE_NAK		= 0x04;	//解包出错请求重发

//DLL上发函数返回值
const int  DLL_RES_OK				= 0;		//执行成功
const int  DLL_RES_PORTUNOPEN		= -4;		//串口未打开
const int  DLL_RES_NAK				= -2;		//解包出错请求重发
const int  DLL_RES_BUSY				= 10;		//设备状态忙
const int  DLL_RES_FAIL				= 103;		//命令执行失败
const int  DLL_RES_TIMEOUT			= -1;		//命令超时(自动重发后，等应答报文仍超时,上发此代码)
const int  DLL_RES_PARAMERROR		= -5;		//参数错误,(超出DLL的参数值范围)
const int  DLL_RES_EXECUTING		= -3;		//有命令正在执行中(例如:第一条命令尚未执行完,又来第二条命令,此时第二条命令上发此应答)
const int  DLL_RES_OTHERWRONG		= 99;		//其他错误

const BYTE RPL_OK = 0;			// 成功应答
const BYTE CMD_REQUEST = 1;		// 请求报文
const BYTE CMD_UNKNOW = 2;		// 未知报文

#define CONFIGFILE  "TIM_Firmware_Update.ini"
#define LOGFILE  "TIM_Firmware_Update.log"