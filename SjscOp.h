#pragma once

//#include "Trace.h"
#include "StdAfx.h"
#include "ComDef.h"
#include "windows.h"
#include "config.h"


class CSjscOp
{
public:
	CSjscOp(void);
	~CSjscOp(void);

public:
	HANDLE m_hEvent;
	int m_iRetry;	//��ʱʱ�Զ��ط�����

private:
	HANDLE m_hThread;
	HANDLE	m_hPort;
	BOOL	m_bPortOpen;	// �����Ƿ��
	BYTE	m_nExitFlag;

	BYTE	m_nRecvState;
	BYTE	m_RecvBuf[_RECV_BUFFER_LEN_];
	int		m_nRecvLen;
	BYTE	m_ResponseBuf[_RECV_BUFFER_LEN_];
	int		m_ResponseLen;

	BYTE	m_iSeq;			// ���к�

	//���ڼ�¼ԭʼӦ����,����DLE
	BYTE m_LogRecvBuf[_RECV_BUFFER_LEN_];
	int volatile m_nLogRecvLen;

	CRITICAL_SECTION m_cBuf;
	CRITICAL_SECTION m_cCmd;

private:
	unsigned int InsertDLE(LPBYTE pBuf, DWORD nLen, LPBYTE ptarget);
	BOOL SendCommand(HANDLE hPort, LPBYTE pBuf, DWORD nLen);
	static DWORD _stdcall ThreadFun(LPVOID p);
	BYTE ProcessChar(BYTE Byte);
	BOOL Kx_ReadFile(LPBYTE pBuf, DWORD& ReadLen, DWORD dwTime);
	void ClearRecvBuf();
	void ClearResponseBuf();
	int GetOriData(BYTE data, BOOL& isOriData);

public:
	BOOL Start(Config objconfig, int Retry);
	BOOL Stop(void);

	int OpGetVer(unsigned short *OpmajorVersion, unsigned short *OpminorVersion);	//��ȡģ��汾
	int OpUpdate(BYTE &data);
	int OpProgramStart(int iLength, BYTE &data);
	int OpSendProgramData(BYTE *data, BYTE &data1);
	int OpProgramEnd(BYTE &data);
	int OpProgramSwitchApp(BYTE &data);
};
