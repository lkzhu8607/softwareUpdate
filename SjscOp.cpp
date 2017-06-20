#include "StdAfx.h"
#include "SjscOp.h"
#include "windows.h"
#include "log.h"
//#include "WinBase.h"
extern CSjscOp theSjscOp;

static int g_iseg;		// DLE标识


CSjscOp::CSjscOp(void)
	: m_iRetry(0)
, m_hEvent(NULL)
, m_hThread(NULL)
, m_hPort(NULL)
, m_nExitFlag(0)
, m_nRecvState(0)
, m_nRecvLen(0)
, m_iSeq(0)
, m_bPortOpen(FALSE)
, m_ResponseLen(0)
, m_nLogRecvLen(0)
{
	InitializeCriticalSection(&m_cBuf);
	InitializeCriticalSection(&m_cCmd);
}

CSjscOp::~CSjscOp(void)
{
	DeleteCriticalSection(&m_cBuf);
	DeleteCriticalSection(&m_cCmd);
}

//发送时在报文中插入DLE字符
unsigned int CSjscOp::InsertDLE(LPBYTE pBuf, DWORD nLen, LPBYTE ptarget)
{	
	unsigned char sdata[_RECV_BUFFER_LEN_];
	int dle_cnt = 0;

	for (DWORD i=0; i<nLen; i++)
	{
		if ( (pBuf[i]==STX)||(pBuf[i]==ETX)||(pBuf[i]==DLE) )
		{
			//插入的DLE字符必须在字符前面插入
			sdata[i+dle_cnt] = DLE;			
			dle_cnt++;
			sdata[i+dle_cnt] = pBuf[i];		
		}
		else
		{
			sdata[i+dle_cnt] = pBuf[i];
		}
	}	

	memcpy(ptarget, sdata, nLen+dle_cnt);
	return (nLen+dle_cnt);
}

BOOL CSjscOp::SendCommand(HANDLE hPort, LPBYTE pBuf, DWORD nLen)
{
/*	char str[2048] = {0};
	char tmp[5] = {0};
	string Str;
	for(unsigned int i=0;i < nLen;i++)
	{
		sprintf_s(tmp, "%02X ", pBuf[i]);
		string tmpStr(tmp);
		Str += tmpStr;
	}
	sprintf_s(str,"%s:%s","发送的命令",Str.c_str());
	gp_objTimLog.logInfo(__FILE__, __LINE__, str);	*/

	//计算校验和
	unsigned char OriData[_RECV_BUFFER_LEN_];
	memcpy(OriData, pBuf, nLen);
	OriData[nLen] = 0; 
	for (DWORD i=0;i<nLen;i++)
	{
		OriData[nLen] ^= OriData[i];
	}

	//插入DLE
	unsigned char localdata[_RECV_BUFFER_LEN_];
	localdata[0] = STX;
	DWORD nNewLen = InsertDLE(OriData, nLen+1, localdata+1) + 2;
	localdata[nNewLen-1] = ETX;

	char str[2048] = {0};
	char tmp[5] = {0};
	string Str;
	for(unsigned int i=0;i < nNewLen;i++)
	{
		sprintf_s(tmp, "%02X ", localdata[i]);
		string tmpStr(tmp);
		Str += tmpStr;
	}
	sprintf_s(str,"%s:%s","发送的命令",Str.c_str());
	gp_objTimLog.logInfo(__FILE__, __LINE__, str);	

	//m_log.Trace(_T("Send: "), localdata, nNewLen);

	//仅用于测试程序,正式程序取消
// #ifdef _TESTAPPSHOW
// 	ProcSendData(localdata, nNewLen);
// #endif

	DWORD iSendLen = 0;
	BOOL nRt = WriteFile(hPort, localdata, nNewLen, &iSendLen, NULL);
	if(!nRt)
	{
		//m_log.Trace(_T("Error: WriteFile failed"));
		nRt = FALSE;
		return nRt;
	}

	if (nNewLen == iSendLen)
	{
		nRt = TRUE;
		ResetEvent(m_hEvent);		
	}
	else
	{
		//m_log.Trace(_T("Error: WriteFile incomplete"));
		printf("Error: WriteFile incomplete");
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: WriteFile incomplete");
		nRt = FALSE;
		return nRt;
	}
	return nRt;
}

BOOL CSjscOp::Stop(void) 
{
	m_nExitFlag = 1;

	if(m_hEvent && m_hEvent!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	if(m_hPort && m_hPort!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPort);
		m_hPort = NULL;
	}

	if(m_hThread && m_hThread!=INVALID_HANDLE_VALUE)
	{
		if(WaitForSingleObject(m_hThread, 1000)!=WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 0xFF);
		}
		m_hThread = NULL;
	}

	m_bPortOpen = FALSE;
	return TRUE;
}

BOOL CSjscOp::Start(Config objconfig, int Retry)
{
	//m_log.Init(_T("Sjsclog"));
	//m_log.Trace(SJSC_API_VERSION);

	m_iRetry = Retry+1;	//+1,用于do-while循环处理

	TCHAR szCom[20];
 	_stprintf_s(szCom, 20, _T("\\\\.\\COM%d"), objconfig.Port);

	if(m_hPort==INVALID_HANDLE_VALUE||m_hPort==NULL)
	{
		//同步串口通信
		m_hPort = CreateFile (szCom, 
			GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

		if(m_hPort == INVALID_HANDLE_VALUE)
		{
			//m_log.Trace(_T("Error: Open port Error"));
			m_hPort = NULL;

			Stop();			
			char str[1024] = {0};
			sprintf_s(str,"Fail to open COM%d",objconfig.Port);
			gp_objTimLog.logInfo(__FILE__, __LINE__, str);
			//printf("Fail to open COM%d",objconfig.Port);
			return DLL_RES_PORTUNOPEN;
		}

		DCB ComDcb;
		COMMTIMEOUTS CommTimeOut;
		GetCommState(m_hPort, &ComDcb);

		//ComDcb.BaudRate = CBR_57600;
		ComDcb.BaudRate = objconfig.Baudrate;//波特率
		ComDcb.fBinary = true;
		if(objconfig.Parity == 0){
			ComDcb.fParity = NOPARITY;
		}
		else if(objconfig.Parity == 1){
			ComDcb.fParity = ODDPARITY;
		}
		else if(objconfig.Parity == 2){
			ComDcb.fParity = EVENPARITY;
		}
		else if(objconfig.Parity == 3){
			ComDcb.fParity = MARKPARITY;
		}
		else if(objconfig.Parity == 4){
			ComDcb.fParity = SPACEPARITY;
		}
		//ComDcb.fParity = NOPARITY;
		ComDcb.fOutxCtsFlow = false;
		ComDcb.fOutxDsrFlow = false;
		ComDcb.fDtrControl = DTR_CONTROL_DISABLE;
		ComDcb.fDsrSensitivity = false;		
		ComDcb.fTXContinueOnXoff = false; //
		ComDcb.fOutX = false;
		ComDcb.fInX = false;
		ComDcb.fErrorChar = false;
		ComDcb.fNull = false; 
		ComDcb.fRtsControl = RTS_CONTROL_DISABLE; 
		ComDcb.fAbortOnError = false;	//
		ComDcb.ByteSize = 8;
		//ComDcb.ByteSize = objconfig.Databits;//数据位
		//ComDcb.Parity = 0;
		ComDcb.Parity = objconfig.Parity;//校验位
		if(objconfig.StopBits == 1){//停止位
			ComDcb.StopBits = ONESTOPBIT;
		}
		else if(objconfig.StopBits == 2){
			ComDcb.StopBits = ONE5STOPBITS;
		}
		else if(objconfig.StopBits == 3){
			ComDcb.StopBits = TWOSTOPBITS;
		}
		//ComDcb.StopBits = ONESTOPBIT;	

		if ( !SetCommState(m_hPort, &ComDcb))
		{
			Stop();
			return DLL_RES_PORTUNOPEN;
		}

		CommTimeOut.ReadIntervalTimeout = 0;
		CommTimeOut.ReadTotalTimeoutMultiplier = 0;
		CommTimeOut.ReadTotalTimeoutConstant = 10;
		CommTimeOut.WriteTotalTimeoutMultiplier = 0;
		CommTimeOut.WriteTotalTimeoutConstant = 0;
		SetCommTimeouts(m_hPort, &CommTimeOut);
	}

	if(m_hEvent==NULL||m_hEvent==INVALID_HANDLE_VALUE)
	{
		m_hEvent=CreateEvent( 
			NULL,         // no security attributes
			TRUE,         // manual-reset event
			FALSE,         // initial state is signaled
			_T("TIM_Update_Com")  // object name
			); 

		if (m_hEvent == NULL) 
		{ 
			//m_log.Trace(_T("Error: CreateEvent Failed"));
			//printf("Error: CreateEvent Failed");
			gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: CreateEvent Failed");

			CloseHandle(m_hPort);
			m_hPort = NULL;

			Stop();
			return DLL_RES_PORTUNOPEN;
		}
	}

	//开启线程,主要是接收数据用
	m_nExitFlag=0;
	if(m_hThread==NULL || m_hThread==INVALID_HANDLE_VALUE)
	{
		DWORD dwThreadId(0); 
		m_hThread = CreateThread( 
			NULL,                        // default security attributes 
			0,                           // use default stack size  
			CSjscOp::ThreadFun,			 // thread function 
			this,                        // argument to thread function 
			0,                           // use default creation flags 
			&dwThreadId);                // returns the thread identifier 

		// Check the return value for success. 
		if (m_hThread == NULL) 
		{
			//m_log.Trace(_T("Error: CreateThread Failed"));
			//printf("Error: CreateEvent Failed");
			gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: CreateThread Failed");

			CloseHandle(m_hPort);
			m_hPort = NULL;

			Stop();
			return DLL_RES_PORTUNOPEN;
		}
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Succ: CreateThread Succ");
	}
//	gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: CreateThread Failed");
	m_bPortOpen = TRUE;
	return TRUE;
}

DWORD __stdcall CSjscOp::ThreadFun(LPVOID p)
{
	CSjscOp* pThis = (CSjscOp*)p;
	DWORD  dwBytesRead;
	BYTE pRecv[128];
	BYTE nRt;

	while(!pThis->m_nExitFlag)
	{
		dwBytesRead = 0;
		if(::ReadFile(pThis->m_hPort, pRecv, 1, &dwBytesRead, NULL) && dwBytesRead>0)
		{			
			nRt = pThis->ProcessChar(pRecv[0]); //每次读一个Byte

			if(nRt == RPL_OK)
			{				
				SetEvent(pThis->m_hEvent); //收到应答
			}
		}
		else
		{
			Sleep(1);
		}
	}
	//pThis->m_log.Trace(_T("Exit ThreadFun"));
	//printf("Exit ThreadFun");
	gp_objTimLog.logInfo(__FILE__, __LINE__, "Exit ThreadFun");

	return 0;
}

BOOL CSjscOp::Kx_ReadFile(LPBYTE pBuf, DWORD& ReadLen, DWORD dwTime = 400)
{
	DWORD nWRt = WaitForSingleObject(m_hEvent, dwTime);

	if(WAIT_OBJECT_0 == nWRt)
	{
		EnterCriticalSection(&m_cBuf);
		memcpy(pBuf, m_ResponseBuf, _RECV_BUFFER_LEN_);
		ReadLen = m_ResponseLen;
		ClearResponseBuf();
		LeaveCriticalSection(&m_cBuf);

		char str[2048] = {0};
		char tmp[5] = {0};
		string Str;
		for(unsigned int i=0;i < ReadLen;i++)
		{
			sprintf_s(tmp, "%02X ", pBuf[i]);
			string tmpStr(tmp);
			Str += tmpStr;
		}
		sprintf_s(str,"%s:%s","接收的应答数据",Str.c_str());
		gp_objTimLog.logInfo(__FILE__, __LINE__, str);	

		ResetEvent(m_hEvent);
		return TRUE;
	}
	else if(nWRt == WAIT_TIMEOUT)
	{
		//m_log.Trace(_T("Error: Timeout"));
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: Timeout");
	}
	else
	{
		//m_log.Trace(_T("Error: Other"));
		gp_objTimLog.logInfo(__FILE__, __LINE__, "Error: Other");
	}

	return FALSE;
}

void CSjscOp::ClearRecvBuf()
{
	m_nRecvState = STATE_IDLE;
	m_nRecvLen = 0;
	memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
	g_iseg = 0;

	m_nLogRecvLen = 0;
	memset(m_LogRecvBuf, 0, sizeof(m_LogRecvBuf));
}

void CSjscOp::ClearResponseBuf()
{
	memset(m_ResponseBuf, 0, _RECV_BUFFER_LEN_);
	m_ResponseLen = 0;
}

int CSjscOp::GetOriData(BYTE data, BOOL& isOriData)
{
	if (g_iseg == 0)
	{
		isOriData = TRUE;
		if ( data == DLE )
		{
			g_iseg = 1;
			return -1;
		}
		else
		{
			return data;
		}
	}
	else
	{
		g_iseg = 0;
		isOriData = FALSE;
		return data;
	}
}

BYTE CSjscOp::ProcessChar(BYTE Byte)
{
	BYTE ret = 0xFF;
	BOOL isOriData;
	int tmp;

	switch (m_nRecvState)
	{
	case STATE_IDLE:
		{
			tmp = GetOriData(Byte,isOriData);
			if(tmp == STX && isOriData)
			{
				//记下日志
				if (m_nLogRecvLen != 0)
				{
					//m_log.Trace(_T("Error 1,Recv: "), m_LogRecvBuf, m_nLogRecvLen);
					//gp_objTimLog.logError(__FILE__,__LINE__,);
				}

				//开始新报文
				ClearRecvBuf();

				m_nRecvState = STATE_RECVDATA;

				m_RecvBuf[m_nRecvLen++] = Byte;
				g_iseg = 0;

				m_LogRecvBuf[m_nLogRecvLen++] = Byte;
			}
			else
			{
				//记下日志
				//m_log.Trace(_T("Error 2,Recv: "), &Byte, 1);
			}
			return ret;
		}		
	case STATE_RECVDATA:
		{
			tmp = GetOriData(Byte,isOriData);
			if(tmp == STX && isOriData)
			{
				//记下日志
				if (m_nLogRecvLen != 0)
				{
					//m_log.Trace(_T("Error 3,Recv: "), m_LogRecvBuf, m_nLogRecvLen);
				}

				//开始新报文
				ClearRecvBuf();

				m_nRecvState = STATE_RECVDATA;		

				m_RecvBuf[m_nRecvLen++] = Byte;
				g_iseg = 0;

				m_LogRecvBuf[m_nLogRecvLen++] = Byte;
			}
			else if (tmp == ETX && isOriData)
			{
				//报文结束
				m_nRecvState = STATE_IDLE;
				m_RecvBuf[m_nRecvLen++] = Byte;

				m_LogRecvBuf[m_nLogRecvLen++] = Byte;

				//m_log.Trace(_T("Recv: "), m_LogRecvBuf, m_nLogRecvLen);

				//检查长度
				if ( m_nRecvLen > 4 && m_RecvBuf[1] == m_nRecvLen-4)
				{
					//BCC校验验证	
					BYTE tch = 0;
					for(int i=1; i<(int)(m_nRecvLen-2); i++) 
					{
						tch ^= m_RecvBuf[i];
					}
					if (tch == m_RecvBuf[m_nRecvLen-2])
					{
						//EnterCriticalSection(&m_cBuf);
						m_ResponseLen = m_nRecvLen;
						memcpy(m_ResponseBuf, m_RecvBuf, _RECV_BUFFER_LEN_);
						//LeaveCriticalSection(&m_cBuf);
						ret = RPL_OK;
					}
					else
					{
						//m_log.Trace(_T("Error: BCC"));

					}
				}
				else
				{
					//m_log.Trace(_T("Error: Wrong length"));
				}

				//开始新报文
				ClearRecvBuf();
			}
			else if ( tmp != -1)
			{
				//非DLE
				m_RecvBuf[m_nRecvLen++] = tmp;

				m_LogRecvBuf[m_nLogRecvLen++] = tmp;
			}
			else if (tmp == -1)
			{
				m_LogRecvBuf[m_nLogRecvLen++] = DLE;
			}
			return ret;
		}		
	}
	return ret;
}


int CSjscOp::OpGetVer(unsigned short *OpmajorVersion, unsigned short *OpminorVersion)
{
	//{0x02,0x02,0x34,0x00,0x00,0x03};	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 3;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);	

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_GETVERSION;
	szBuf[2] = m_iSeq;

	*OpmajorVersion = 0;
	*OpminorVersion = 0;

	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen))
			{
				//if ( (szRecv[2]==CMD_GETVERSION) && (szRecv[3]==m_iSeq) )
				if ( (szRecv[2]==CMD_GETVERSION))
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						*OpmajorVersion = szRecv[5];
						*OpminorVersion = szRecv[6];
						ret = DLL_RES_OK;
					}
					else if (szRecv[4]==MODULE_STATE_FAIL) 
					{
						ret = DLL_RES_FAIL;					
					}
					else if (szRecv[4]==MODULE_STATEBUSY) 
					{
						ret = DLL_RES_BUSY;					
					}
					else if (szRecv[4]==MODULE_STATE_NAK) 
					{
						ret = DLL_RES_NAK;					
					}
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}				
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}

//硬币模块程序更新
int CSjscOp::OpUpdate(BYTE &data)
{	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 4;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open\n");
		gp_objTimLog.logError(__FILE__,__LINE__,"Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_UPDATE;
	szBuf[2] = m_iSeq;
	szBuf[3] = 0x01;
	
	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen, 10000))
			{
				//if (szRecv[2]== CMD_UPDATE && szRecv[3]==m_iSeq)
				if (szRecv[2]== CMD_UPDATE)
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						ret = DLL_RES_OK;
					}
					else 
					{
						ret = szRecv[4];
					}					
					data = szRecv[5];
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}

//硬币模块程序更新开始命令
int CSjscOp::OpProgramStart(int iLength, BYTE &data)
{	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 8;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open\n");
		gp_objTimLog.logError(__FILE__,__LINE__,"Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_PROGRAM_START;
	szBuf[2] = m_iSeq;
	szBuf[3] = 0x01;
	//低字节在前
	szBuf[4] = (BYTE)((iLength << 24) & 0xFF000000);
	szBuf[5] = (BYTE)((iLength << 16) & 0xFF000000);
	szBuf[6] = (BYTE)((iLength << 8) & 0xFF0000000);
	szBuf[7] = (BYTE)(iLength & 0xFF000000);

	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen, 3200))
			{
				//if (szRecv[2]== CMD_PROGRAM_START && szRecv[3]==m_iSeq)
				if (szRecv[2]== CMD_PROGRAM_START)
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						ret = DLL_RES_OK;
					}
					else 
					{
						ret = szRecv[4];
					}					
					data = szRecv[5];
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}

//程序更新发送程序数据命令
int CSjscOp::OpSendProgramData(BYTE *data,BYTE &data1)
{	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 68;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open\n");
		gp_objTimLog.logError(__FILE__,__LINE__,"Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_SEND_PROGRAM_DATA;
	szBuf[2] = m_iSeq;
	szBuf[3] = 0x01;
	memcpy((szBuf+4),data,64);

	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen, 5000))
			{
				//if (szRecv[2]== CMD_SEND_PROGRAM_DATA && szRecv[3]==m_iSeq)
				if (szRecv[2]== CMD_SEND_PROGRAM_DATA)
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						ret = DLL_RES_OK;
					}
					else 
					{
						ret = szRecv[4];
					}		
					data1 = szRecv[5];
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}

//硬币模块程序更新结束命令
int CSjscOp::OpProgramEnd(BYTE &data)
{	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 4;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open\n");
		gp_objTimLog.logError(__FILE__,__LINE__,"Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_PROGRAM_END;
	szBuf[2] = m_iSeq;
	szBuf[3] = 0x01;

	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen, 3200))
			{
				//if (szRecv[2]== CMD_PROGRAM_END && szRecv[3]==m_iSeq)
				if (szRecv[2]== CMD_PROGRAM_END)
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						ret = DLL_RES_OK;
					}
					else 
					{
						ret = szRecv[4];
					}			
					data = szRecv[5];
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}

//std::basic_string<char,std::char_traits<char>,std::allocator<char>>::compare();
//硬币程序更新完成切换到应用模式命令
int CSjscOp::OpProgramSwitchApp(BYTE &data)
{	
	int ret = DLL_RES_OTHERWRONG;
	int iretry = 0;
	const int nLen = 4;
	BYTE szBuf[nLen];
	memset(szBuf, 0, nLen);

	if ( !m_bPortOpen)
	{
		//m_log.Trace(_T("Error: COM not open"));
		printf("Error: COM not open\n");
		gp_objTimLog.logError(__FILE__,__LINE__,"Error: COM not open");
		return DLL_RES_PORTUNOPEN;
	}

	//EnterCriticalSection(&m_cCmd);
	if (m_iSeq>255 || m_iSeq<1)
	{
		m_iSeq = 1;
	}
	
	szBuf[0] = nLen-1;
	szBuf[1] = CMD_SWITCH_APP;
	szBuf[2] = m_iSeq;
	szBuf[3] = 0x01;
	do 
	{
		if ( SendCommand(m_hPort, szBuf, nLen) )
		{
			BYTE szRecv[_RECV_BUFFER_LEN_];
			memset(szRecv, 0, sizeof(szRecv));
			DWORD nRecvLen = 0;

			if (Kx_ReadFile(szRecv, nRecvLen, 3200))
			{
				//if (szRecv[2]== CMD_SWITCH_APP && szRecv[3]==m_iSeq)
				if (szRecv[2]== CMD_SWITCH_APP)
				{
					if (szRecv[4]==MODULE_STATE_OK)
					{
						ret = DLL_RES_OK;
					}
					else 
					{
						ret = szRecv[4];
					}					
					data = szRecv[5];
				}
				else
				{
					//m_log.Trace(_T("Error: Cmd or Seq No."));
					printf("Error: Cmd or Seq No.");
					gp_objTimLog.logError(__FILE__, __LINE__, "Error: Cmd or Seq No.");
				}
				break;			
			}
			else
			{
				ret = DLL_RES_TIMEOUT;
			}
		}

		iretry++;
	} while (iretry < m_iRetry);

	m_iSeq++;
	//LeaveCriticalSection(&m_cCmd);
	return ret;
}