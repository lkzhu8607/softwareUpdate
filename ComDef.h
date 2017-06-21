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
const int _RECV_BUFFER_LEN_	= 300;		//������󳤶�

//���ƴ���
const BYTE STX	= 0x02;
const BYTE ETX	= 0x03;
const BYTE DLE	= 0x10;

//�������--��������
#define CMD_GETVERSION_OLD				0x08	//��ѯ̨��բ��ģ�����汾
#define CMD_GETSTATUS				0x31	//����ģ����ѯ
#define CMD_MOTORTEST				0x32	//�������
#define CMD_RESET					0x33	//��λ
#define CMD_GETVERSION				0x34	//��ѯ����ģ�����汾
#define CMD_FEEDTICKET				0x20	//����������дλ��
#define CMD_ISSUETICKET				0x21	//�������ݴ�����߷�Ʊ��
#define CMD_DISPENSETICKET			0x26	//���ݴ涷�еĳ�Ʊ�����˿�

#define CMD_UPDATE                  0x35    //Ӳ��ģ������������
#define CMD_PROGRAM_START           0x36    //Ӳ��ģ���̿�ʼ,�������ݳ���
#define CMD_SEND_PROGRAM_DATA       0x37    //Ӳ��ģ�鷢�ͱ������
#define CMD_PROGRAM_END             0x38    //Ӳ��ģ���̽���
#define CMD_SWITCH_APP              0x39    //Ӳ��ģ���̽�������Ӧ�ó���

//ģ���Ϸ�COMState
const BYTE MODULE_STATE_OK		= 0x00;	//����ִ�гɹ�
const BYTE MODULE_STATE_FAIL	= 0x01;	//����ִ��ʧ��
const BYTE MODULE_STATEBUSY		= 0x02;	//�豸æ
const BYTE MODULE_STATE_NAK		= 0x04;	//������������ط�

//DLL�Ϸ���������ֵ
const int  DLL_RES_OK				= 0;		//ִ�гɹ�
const int  DLL_RES_PORTUNOPEN		= -4;		//����δ��
const int  DLL_RES_NAK				= -2;		//������������ط�
const int  DLL_RES_BUSY				= 10;		//�豸״̬æ
const int  DLL_RES_FAIL				= 103;		//����ִ��ʧ��
const int  DLL_RES_TIMEOUT			= -1;		//���ʱ(�Զ��ط��󣬵�Ӧ�����Գ�ʱ,�Ϸ��˴���)
const int  DLL_RES_PARAMERROR		= -5;		//��������,(����DLL�Ĳ���ֵ��Χ)
const int  DLL_RES_EXECUTING		= -3;		//����������ִ����(����:��һ��������δִ����,�����ڶ�������,��ʱ�ڶ��������Ϸ���Ӧ��)
const int  DLL_RES_OTHERWRONG		= 99;		//��������

const BYTE RPL_OK = 0;			// �ɹ�Ӧ��
const BYTE CMD_REQUEST = 1;		// ������
const BYTE CMD_UNKNOW = 2;		// δ֪����

#define CONFIGFILE  "TIM_Firmware_Update.ini"
#define LOGFILE  "TIM_Firmware_Update.log"