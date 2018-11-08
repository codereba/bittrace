#include "stdafx.h"
#include <string>
#include <sstream>
#include "../common/common.h"
#include "../inc/public.h"
#include "../CenterStub/centerstub.h"
#include "../inc/socketpublic.h"
#include "../inc/define.h"
#include "../inc/public.h"
#include "../common/SQLite.h"
#include "../Common/sha1.h"
#include "../Common/Md5.h"

#include "util.h"

#include "SecurityTerminal.h"
#include "MainFrm.h"
#include <winsock2.h> 
#include "ShowStatusDlg.h"
#include "ExceptionHandler.h"

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <aclapi.h>
using namespace std;
#define STATUS_SUCCESS                              ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH                ((NTSTATUS)0xC0000004L)
#define OBJ_CASE_INSENSITIVE                        0x00000040L
#define PAGE_READONLY                                        0x02
#define PAGE_READWRITE                                        0x04
#define DEF_KERNEL_BASE                                        0x80400000L
#define        SystemModuleInformation                        11
#define PROT_MEMBASE                                        0x80000000
typedef LONG        NTSTATUS;
typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
DWORD gWinVersion;
typedef struct _STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR  Buffer;
} ANSI_STRING, *PANSI_STRING;
typedef struct _UNICODE_STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;
typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION;
NTSTATUS (WINAPI * _RtlAnsiStringToUnicodeString)
(PUNICODE_STRING  DestinationString,
 IN PANSI_STRING  SourceString,
 IN BOOLEAN);
VOID (WINAPI *_RtlInitAnsiString)
(IN OUT PANSI_STRING  DestinationString,
 IN PCHAR  SourceString);
VOID (WINAPI * _RtlFreeUnicodeString)
(IN PUNICODE_STRING  UnicodeString);
NTSTATUS (WINAPI *_NtOpenSection)
(OUT PHANDLE  SectionHandle,
 IN ACCESS_MASK  DesiredAccess,
 IN POBJECT_ATTRIBUTES  ObjectAttributes);
NTSTATUS (WINAPI *_NtMapViewOfSection)
(IN HANDLE  SectionHandle,
 IN HANDLE  ProcessHandle,
 IN OUT PVOID  *BaseAddress,
 IN ULONG  ZeroBits,
 IN ULONG  CommitSize,
 IN OUT PLARGE_INTEGER  SectionOffset,        /* optional */
 IN OUT PULONG  ViewSize,
 IN SECTION_INHERIT  InheritDisposition,
 IN ULONG  AllocationType,
 IN ULONG  Protect);
NTSTATUS (WINAPI *_NtUnmapViewOfSection)
(IN HANDLE ProcessHandle,
 IN PVOID BaseAddress);
NTSTATUS (WINAPI * _NtQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);
//*******************************************************************************************************
// PE File structure declarations
//
//*******************************************************************************************************
struct PE_Header
{
	unsigned long signature;
	unsigned short machine;
	unsigned short numSections;
	unsigned long timeDateStamp;
	unsigned long pointerToSymbolTable;
	unsigned long numOfSymbols;
	unsigned short sizeOfOptionHeader;
	unsigned short characteristics;
};
struct PE_ExtHeader
{
	unsigned short magic;
	unsigned char majorLinkerVersion;
	unsigned char minorLinkerVersion;
	unsigned long sizeOfCode;
	unsigned long sizeOfInitializedData;
	unsigned long sizeOfUninitializedData;
	unsigned long addressOfEntryPoint;
	unsigned long baseOfCode;
	unsigned long baseOfData;
	unsigned long imageBase;
	unsigned long sectionAlignment;
	unsigned long fileAlignment;
	unsigned short majorOSVersion;
	unsigned short minorOSVersion;
	unsigned short majorImageVersion;
	unsigned short minorImageVersion;
	unsigned short majorSubsystemVersion;
	unsigned short minorSubsystemVersion;
	unsigned long reserved1;
	unsigned long sizeOfImage;
	unsigned long sizeOfHeaders;
	unsigned long checksum;
	unsigned short subsystem;
	unsigned short DLLCharacteristics;
	unsigned long sizeOfStackReserve;
	unsigned long sizeOfStackCommit;
	unsigned long sizeOfHeapReserve;
	unsigned long sizeOfHeapCommit;
	unsigned long loaderFlags;
	unsigned long numberOfRVAAndSizes;
	unsigned long exportTableAddress;
	unsigned long exportTableSize;
	unsigned long importTableAddress;
	unsigned long importTableSize;
	unsigned long resourceTableAddress;
	unsigned long resourceTableSize;
	unsigned long exceptionTableAddress;
	unsigned long exceptionTableSize;
	unsigned long certFilePointer;
	unsigned long certTableSize;
	unsigned long relocationTableAddress;
	unsigned long relocationTableSize;
	unsigned long debugDataAddress;
	unsigned long debugDataSize;
	unsigned long archDataAddress;
	unsigned long archDataSize;
	unsigned long globalPtrAddress;
	unsigned long globalPtrSize;
	unsigned long TLSTableAddress;
	unsigned long TLSTableSize;
	unsigned long loadConfigTableAddress;
	unsigned long loadConfigTableSize;
	unsigned long boundImportTableAddress;
	unsigned long boundImportTableSize;
	unsigned long importAddressTableAddress;
	unsigned long importAddressTableSize;
	unsigned long delayImportDescAddress;
	unsigned long delayImportDescSize;
	unsigned long COMHeaderAddress;
	unsigned long COMHeaderSize;
	unsigned long reserved2;
	unsigned long reserved3;
};

struct SectionHeader
{
	unsigned char sectionName[8];
	unsigned long virtualSize;
	unsigned long virtualAddress;
	unsigned long sizeOfRawData;
	unsigned long pointerToRawData;
	unsigned long pointerToRelocations;
	unsigned long pointerToLineNumbers;
	unsigned short numberOfRelocations;
	unsigned short numberOfLineNumbers;
	unsigned long characteristics;
};
struct MZHeader
{
	unsigned short signature;
	unsigned short partPag;
	unsigned short pageCnt;
	unsigned short reloCnt;
	unsigned short hdrSize;
	unsigned short minMem;
	unsigned short maxMem;
	unsigned short reloSS;
	unsigned short exeSP;
	unsigned short chksum;
	unsigned short exeIP;
	unsigned short reloCS;
	unsigned short tablOff;
	unsigned short overlay;
	unsigned char reserved[32];
	unsigned long offsetToPE;
};

struct ImportDirEntry
{
	DWORD importLookupTable;
	DWORD timeDateStamp;
	DWORD fowarderChain;
	DWORD nameRVA;
	DWORD importAddressTable;
};
#pragma comment(lib,"Ws2_32.lib")


#pragma warning (disable:4995)
#pragma  warning(disable:4996)
using namespace SQLite;

#define APP_ERROR_CODE_BASE                      0x20000000
#define APP_ERROR_CODE_SUCCESS                   APP_ERROR_CODE_BASE+0
#define APP_ERROR_CODE_PERMISSION_DENY           APP_ERROR_CODE_BASE+1
#define APP_ERROR_CODE_NETWORK_INTERRUPT         APP_ERROR_CODE_BASE+2
#define APP_ERROR_CODE_INVALID_PARAMETER         APP_ERROR_CODE_BASE+3
#define APP_ERROR_CODE_PIPE_READ_ERROR           APP_ERROR_CODE_BASE+4
#define APP_ERROR_CODE_PIPE_WRITE_ERROR          APP_ERROR_CODE_BASE+5
#define APP_ERROR_CODE_PIPE_BUSY_ERROR           APP_ERROR_CODE_BASE+6

#define APP_ERROR_CODE_INVLID_SOCKET_ERROR       APP_ERROR_CODE_BASE+7
#define APP_ERROR_CODE_CONNECT_FAILURE_ERROR     APP_ERROR_CODE_BASE+8
#define APP_ERROR_CODE_SEND_ERROR                APP_ERROR_CODE_BASE+9
#define APP_ERROR_CODE_RECEIVE_ERROR             APP_ERROR_CODE_BASE+10



#define APP_ERROR_CODE_READ_BLOB_DATABASE_ERROR  APP_ERROR_CODE_BASE+11
#define APP_ERROR_CODE_WRITE_BLOB_DATABASE_ERROR APP_ERROR_CODE_BASE+12

#define APP_ERROR_CODE_UNKNOWN_ERROR_EXCEPTION   APP_ERROR_CODE_BASE+255


const COLORREF TemaCeleste            = RGB(227,239,255);
const COLORREF TemaPlateado           = RGB(240,241,242);
const COLORREF TemaNegro              = RGB(220,222,224);
const COLORREF TemaAqua               = RGB(195,202,217);

const int Reduc						  = 110;
const COLORREF TemaCelesteR           = RGB(100,130,209);
const COLORREF TemaPlateadoR          = RGB(240 - Reduc,241 - Reduc,242 - Reduc);
const COLORREF TemaNegroR             = RGB(220 - Reduc,222 - Reduc,224 - Reduc);
const COLORREF TemaAquaR              = RGB(195 - Reduc,202 - Reduc,217 - Reduc);

extern CSecurityTerminalApp theApp;
extern StateInfo g_CommData ;


typedef struct tagNetworkThreadParam
{
	PCHAR pbGUIDataBuffer;
	ULONG dwGUIDataBufferLen;
	PBYTE * ppOutBuffer;
	PDWORD pdwOutputLen;
	BOOL   bShowUi;
}NetworkThreadParam,*PNetworkThreadParam;


ULONG g_ulErrorCode = APP_ERROR_CODE_SUCCESS;
void SetMyLastError(ULONG ulErrorCode)
{
	g_ulErrorCode = ulErrorCode;
}
void GetAppLastErrorDesc(TCHAR * tszErrorDesc,int nErrorBufferSize)
{

	switch(g_ulErrorCode)
	{

	case APP_ERROR_CODE_SUCCESS:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("操作成功"));
		break;
	case  APP_ERROR_CODE_PERMISSION_DENY:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("操作权限不够"));
		break;
	case APP_ERROR_CODE_NETWORK_INTERRUPT:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("网络中断"));
		break;
	case  APP_ERROR_CODE_INVALID_PARAMETER:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("无效参数"));
		break;
	case  APP_ERROR_CODE_PIPE_READ_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("管道读操作失败"));
		break;
	case  APP_ERROR_CODE_PIPE_WRITE_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("管道写操作失败"));
		break;
	case  APP_ERROR_CODE_PIPE_BUSY_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("管道忙，暂时无法建立连接"));
		break;
	case APP_ERROR_CODE_INVLID_SOCKET_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("无效套接字错误"));
		break;
	case  APP_ERROR_CODE_CONNECT_FAILURE_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("网络连接失败"));
		break;
	case  APP_ERROR_CODE_SEND_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("网络发送数据过程中发生错误"));
		break;
	case  APP_ERROR_CODE_RECEIVE_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("网络接受数据过程中发生错误"));
		break;
	case APP_ERROR_CODE_UNKNOWN_ERROR_EXCEPTION:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("未知错误异常"));
		break;
	case APP_ERROR_CODE_READ_BLOB_DATABASE_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("读取数据库Blob字段错误"));
		break;
	case APP_ERROR_CODE_WRITE_BLOB_DATABASE_ERROR:
		StringCbCopy(tszErrorDesc,nErrorBufferSize,_T("数据库Blob字段错误"));
		break;
	default:
		StringCbPrintf(tszErrorDesc,nErrorBufferSize,_T("未知错误,错误代码：%u"),g_ulErrorCode);
		break;

	}
}







BOOL WINAPI ConnectSOC()
{

	char szConfigIni[MAX_PATH]={0};
	GetAppPath(szConfigIni,sizeof(szConfigIni));
	StringCbCatA(szConfigIni,sizeof(szConfigIni),"\\config.ini");
	char szIPAddress[17]={0};
	strcpy(szIPAddress,"192.168.7.141");
	USHORT usPort = 9003;
	BOOL   bConnected = FALSE;

	if (GetFileAttributesA(szConfigIni)!= INVALID_FILE_ATTRIBUTES)
	{
		GetPrivateProfileStringA("NetworkComm:Soc","Ipv4","192.168.7.141",szIPAddress,_countof(szIPAddress),szConfigIni);
		usPort =(USHORT) GetPrivateProfileIntA("NetworkComm:Soc","TransforPort",9003,szConfigIni);

	}
	else
	{
		DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"config.ini文件不存在，无法探测中心服务器的连通性");

		return FALSE;
	}

	SOCKET                           Socket = INVALID_SOCKET;
	SOCKADDR_IN                      SockAddress;


	Socket = socket (AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{

		DbgPrintEx ("%s: invalid socket",EXE_SECURITY_TERMINAL_NAME_A);
		goto Done;
	}

	memset (&SockAddress,0,sizeof (SOCKADDR_IN));	

	SockAddress.sin_family = AF_INET;
	SockAddress.sin_port = htons(usPort);



	SockAddress.sin_addr.s_addr = inet_addr(szIPAddress);//(g_CommData.NetworkCommInformation.LocalIp);

	int nCount = 0;

	if (connect (Socket,(struct sockaddr *)&SockAddress,sizeof (SOCKADDR_IN)) == SOCKET_ERROR)
	{




		const BYTE* byIpAddr = (const BYTE*)&SockAddress.sin_addr.s_addr ;

		DbgPrintW (EXE_SECURITY_TERMINAL_NAME_A,"无法连接到服务器%u.%u.%u.%u:%d",
			byIpAddr[0],
			byIpAddr[1],
			byIpAddr[2],
			byIpAddr[3],
			usPort
			);
		goto Done;
	}
	bConnected = TRUE;
Done:

	if (Socket != INVALID_SOCKET)
	{
		closesocket (Socket);
	}

	return bConnected;

}
DWORD WINAPI SendGuiInfoToServiceThread(__in PVOID pParam)
{
	BOOL bUseCompress = FALSE;
	CHAR szAppPath[MAX_PATH] = {0};
	PCHAR szSendBuffer = NULL;
	GetAppPath(szAppPath,sizeof(szAppPath));
	StringCbCatA(szAppPath,sizeof(szAppPath),"\\config.ini");
	bUseCompress = GetPrivateProfileIntA("NetworkComm:Gui","Compress",0,szAppPath);
	BOOL bDebugSend = GetPrivateProfileIntA("NetworkComm:Local","debug",0,szAppPath);

	char szRemoteFile[MAX_PATH]={0};
	GetAppPath(szRemoteFile,sizeof(szRemoteFile));
	StringCbCatA(szRemoteFile,sizeof(szRemoteFile),"\\remoteman.ini");

	char szIPAddress[17]={0};
	strcpy(szIPAddress,"127.0.0.1");
	USHORT usPort = g_CommData.NetworkCommInformation.LocalCmdPort;

	if (GetFileAttributesA(szRemoteFile)!= INVALID_FILE_ATTRIBUTES)
	{
		GetPrivateProfileStringA("remoteman","ip","127.0.0.1",szIPAddress,_countof(szIPAddress),szRemoteFile);
		usPort =(USHORT) GetPrivateProfileIntA("remoteman","port",9077,szRemoteFile);

	}

	if(usPort == 0)
	{
		usPort =(USHORT) GetPrivateProfileIntA("NetworkComm:Local","CmdPort",9077,szAppPath);
	}
	LPTSTR lpstCmdLine = GetCommandLine();
	BOOL bFind = FALSE;
	if(StrStrI(lpstCmdLine,_T(" /SelfTest")) ||StrStrI(lpstCmdLine,_T(" -SelfTest")))
	{

		int argc = 0;
		LPWSTR* argv = ::CommandLineToArgvW(GetCommandLineW(), &argc);
		for (int i= 0;i<argc;i++)
		{
			if(argv[0] == NULL)
				break;
			if(StrStrIW(argv[0],L"/ip")|| StrStrIW(argv[0],L"-ip"))
			{
				argv++;
				i++;
				StringCbCopyA(szIPAddress,sizeof(szIPAddress),CW2A(argv[0]));
				bFind = TRUE;
			}
			if(StrStrIW(argv[0],L"/port")|| StrStrIW(argv[0],L"-port"))
			{
				argv++;
				i++;
				usPort = _wtoi(argv[0]);
			}
			argv++;
		}
		if(argv[0]!=NULL)
			LocalFree(argv);
	}

	if(!bFind && GetFileAttributesA(szRemoteFile)!= INVALID_FILE_ATTRIBUTES)
	{


		GetPrivateProfileStringA("remoteman","ip","127.0.0.1",szIPAddress,_countof(szIPAddress),szRemoteFile);
		usPort =(USHORT) GetPrivateProfileIntA("remoteman","port",9077,szRemoteFile);


	}
	PNetworkThreadParam pNetworkThreadParam = (PNetworkThreadParam)pParam;
	if(!pNetworkThreadParam)
	{
		SetMyLastError(APP_ERROR_CODE_INVALID_PARAMETER);
		OutputDebugString(_T("SendGuiInfoToServiceThread __out"));
		return FALSE;
	}


	PCHAR pbGUIDataBuffer    = pNetworkThreadParam->pbGUIDataBuffer;

	ULONG dwGUIDataBufferLen = pNetworkThreadParam->dwGUIDataBufferLen;
	PBYTE * ppOutBuffer      = pNetworkThreadParam->ppOutBuffer;
	PDWORD pdwOutputLen      = pNetworkThreadParam->pdwOutputLen;
	BOOL                             bOk = FALSE;
	INFOR_CMD                        InforCmd = {0};

	SOCKET                           Socket = INVALID_SOCKET;
	SOCKADDR_IN                      SockAddress;

	PUCHAR                           RecvBuffer = NULL;

	BYTE*                            pbCompressBuffer = NULL;
	ULONG                              nCompressBufferLen = 0;
	BYTE*                            pbDeCompressBuffer = NULL;
	ULONG                              nDeCompressBufferLen = 0;
	if(!pbGUIDataBuffer || !pdwOutputLen)
	{
		SetMyLastError(APP_ERROR_CODE_INVALID_PARAMETER);
		DbgPrintEx ("%s: invalid parameter",EXE_SECURITY_TERMINAL_NAME_A);
		goto Done;
	}

	Socket = socket (AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		SetMyLastError(APP_ERROR_CODE_INVLID_SOCKET_ERROR);
		DbgPrintEx ("%s: invalid socket",EXE_SECURITY_TERMINAL_NAME_A);
		goto Done;
	}

	memset (&SockAddress,0,sizeof (SOCKADDR_IN));	

	SockAddress.sin_family = AF_INET;
	SockAddress.sin_port = htons(usPort);



	SockAddress.sin_addr.s_addr = inet_addr(szIPAddress);//(g_CommData.NetworkCommInformation.LocalIp);

	if(bDebugSend)
	{
		DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"ip:%s port:%u",szIPAddress,usPort);
	}

	int nCount = 0;

	if (connect (Socket,(struct sockaddr *)&SockAddress,sizeof (SOCKADDR_IN)) == SOCKET_ERROR)
	{



		SetMyLastError(APP_ERROR_CODE_CONNECT_FAILURE_ERROR);
		const BYTE* byIpAddr = (const BYTE*)&SockAddress.sin_addr.s_addr ;

		DbgPrintW (EXE_SECURITY_TERMINAL_NAME_A,"无法连接到服务器%u.%u.%u.%u:%d",
			byIpAddr[0],
			byIpAddr[1],
			byIpAddr[2],
			byIpAddr[3],
			usPort
			);
		goto Done;
	}

	if (bUseCompress)
	{
		nCompressBufferLen = EncodeRle8(pbGUIDataBuffer,dwGUIDataBufferLen,&pbCompressBuffer,nCompressBufferLen);
		DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"compress before dwGUIDataBufferLen:%d nCompressBufferLen:%d",dwGUIDataBufferLen,nCompressBufferLen);

	}
	else
	{
		pbCompressBuffer =   (BYTE*) pbGUIDataBuffer;
		nCompressBufferLen = dwGUIDataBufferLen;


	}

	szSendBuffer = (char*)malloc(sizeof(INFOR_CMD)+nCompressBufferLen);
	if(szSendBuffer)
	{
		PINFOR_CMD pInfoCmd = (PINFOR_CMD)szSendBuffer;
		pInfoCmd->Mode = GUI_INFO_CMD_MODE;
		pInfoCmd->DataLen = nCompressBufferLen;
		memcpy(szSendBuffer+sizeof(INFOR_CMD),pbCompressBuffer,nCompressBufferLen);





		if (send (Socket,(const char*)szSendBuffer,sizeof(INFOR_CMD)+nCompressBufferLen,0) <=0 )
		{
			SetMyLastError(APP_ERROR_CODE_SEND_ERROR);
			DbgPrintW (EXE_SECURITY_TERMINAL_NAME_A," Send data Error");
			goto Done;
		}
		if(ppOutBuffer && pdwOutputLen)
		{

			PCHAR RecvBuffer = (PCHAR) &InforCmd;
			int RecvSize = 0;
			int i = 0;
			int nResult = recv(Socket,(char*)RecvBuffer,sizeof(INFOR_CMD),0);
			while (nResult < (INT)sizeof(INFOR_CMD))
			{
				i = sizeof(INFOR_CMD) - nResult;

				RecvSize = recv (Socket,(PCHAR)&RecvBuffer[nResult],i,0);
				if (RecvSize <= 0)
				{
					break;
				}
				nResult += RecvSize;
			}

			if(nResult ==sizeof(INFOR_CMD))
			{
				if(InforCmd.Mode == 2)
				{
					if( ppOutBuffer != NULL  && (*ppOutBuffer)== NULL)
					{

						*ppOutBuffer = (BYTE*)malloc(InforCmd.DataLen);
					}
					//ZeroMemory(*ppOutBuffer ,InforCmd.DataLen);

					PCHAR pszTemp =  (PCHAR)*ppOutBuffer;
					int nResult = recv(Socket,(PCHAR)pszTemp,InforCmd.DataLen,0);

					while (nResult < (INT)InforCmd.DataLen)
					{
						i = InforCmd.DataLen - nResult;

						RecvSize = recv (Socket,(PCHAR)&pszTemp[nResult],i,0);
						if (RecvSize <= 0)
						{
							break;
						}
						nResult += RecvSize;

					}

					if(nResult != InforCmd.DataLen)
					{
						goto Done;
					}
					*pdwOutputLen = InforCmd.DataLen;

				}
				else
				{
					SetMyLastError(APP_ERROR_CODE_INVALID_PARAMETER);
					DbgPrintW (EXE_SECURITY_TERMINAL_NAME_A," InforCmd.Mode != 2",);


				}
			}	
			else
			{
				SetMyLastError(APP_ERROR_CODE_RECEIVE_ERROR);
				DbgPrintW (EXE_SECURITY_TERMINAL_NAME_A,"返回数据长度要求为:%u 个字节",sizeof(INFOR_CMD));
				goto Done;
			}
		}

		bOk = TRUE;
	}


Done:

	if (bUseCompress)
	{
		if(pbCompressBuffer)
		{
			free(pbCompressBuffer);
			pbCompressBuffer = NULL;
		}

	}

	if(pParam)
	{
		free(pParam);
		pParam = NULL;
	}

	if (Socket != INVALID_SOCKET)
	{
		closesocket (Socket);
	}

	if(bUseCompress)
	{
		if(*ppOutBuffer && *pdwOutputLen)
		{
			//RLE_UnCompress((DWORD* &)*ppOutBuffer,(const   int   &)*pdwOutputLen,   (DWORD*   &)pbDeCompressBuffer,   (int   &)nDeCompressBufferLen); 

			int nReturnLen = DecodeRle8( *ppOutBuffer,
				*pdwOutputLen,
				pbDeCompressBuffer,
				nDeCompressBufferLen );

			if (nReturnLen != nDeCompressBufferLen) {
				DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A," error %d",nReturnLen);
			}

			if(*ppOutBuffer)
			{
				free(*ppOutBuffer);
				*ppOutBuffer = NULL;
			}
			if(*pdwOutputLen)
			{
				*pdwOutputLen = 0;
			}
			*ppOutBuffer = (BYTE*)pbDeCompressBuffer;
			*pdwOutputLen = nDeCompressBufferLen;
			InforCmd.DataLen = nDeCompressBufferLen;
		}
	}

	if(szSendBuffer)
	{
		free(szSendBuffer);
		szSendBuffer = NULL;
	}


	return bOk;
}



BOOL WINAPI RealSendGuiInfoToService(__in PCHAR pbGUIDataBuffer,__in ULONG dwGUIDataBufferLen,__out PBYTE * ppOutBuffer,__out PDWORD pdwOutputLen);



COLORREF DarkenColor(COLORREF col,double factor)
{
	if(factor>0.0&&factor<=1.0){
		BYTE red,green,blue,lightred,lightgreen,lightblue;
		red = GetRValue(col);
		green = GetGValue(col);
		blue = GetBValue(col);
		lightred = (BYTE)(red-(factor*red));
		lightgreen = (BYTE)(green-(factor*green));
		lightblue = (BYTE)(blue-(factor*blue));
		col = RGB(lightred,lightgreen,lightblue);
	}
	return(col);
}


void DoEvents()
{
	MSG msg; 
	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{ 
		if(msg.message==WM_QUIT) 
			return ; 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
}
BOOL StartRemoteDebug()
{

	BOOL bVerifyOK = FALSE;
	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_VERIFY_REMOTE_SWITCH;

	gui_send_buffer.dwExtraDataSize = 0;
	if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize+8,&ResultBuf,&dwLen,FALSE))
	{
		PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;
		if (Header->QueryResult ==1)
		{

			bVerifyOK = TRUE;
		}

		if(ResultBuf)
		{

			free(ResultBuf);
			ResultBuf = NULL;
		}


	}
	else
	{

		if(ResultBuf)
		{

			free(ResultBuf);
			ResultBuf = NULL;
		}

		/*ShowNotifyInfo(AfxGetMainWnd()->m_hWnd,_T("提示"),tszErrorDesc);*/
		return FALSE;
	}
	return bVerifyOK;
}
BOOL CheckPrivilege(PGUI_COMM_BUFFER pgui_send_buffer)
{


	char szRemoteDebugIni[MAX_PATH] = {0};
	GetAppPath(szRemoteDebugIni,sizeof(szRemoteDebugIni));
	StringCbCatA(szRemoteDebugIni,sizeof(szRemoteDebugIni),"\\remoteman.ini");
	int nRemoteDebug = GetPrivateProfileIntA("remoteman","enable",0,szRemoteDebugIni);

	CHAR                            IniFilePath[MAX_PATH] = {0};
	GetAppPath(IniFilePath,sizeof(IniFilePath));
	StringCbCatA(IniFilePath,sizeof(IniFilePath),"\\Config.ini");
	BOOL bBreak = GetPrivateProfileIntA("Platform","debugbreak",0,IniFilePath);

	BOOL bAllowMaitain = FALSE;
	CSecurityTerminalApp* pApp = (CSecurityTerminalApp*)AfxGetApp();
	if(pApp && pApp->m_bMagic)
	{
		bAllowMaitain = TRUE;
	}

	else {

		if(nRemoteDebug == 1|| bBreak==1)
		{   
			bAllowMaitain = TRUE;
		}
		else
		{
			LPTSTR lpszCmdLine = GetCommandLine();
			if(StrStrI(lpszCmdLine,_T(" /SelfTest")) ||StrStrI(lpszCmdLine,_T(" -SelfTest")))
			{
				bAllowMaitain = TRUE;
			}
		}

	}

	if(!bAllowMaitain)
	{
		switch(pgui_send_buffer->nCmdID)
		{

		case GUI_CMD_ID_GUI_SEND_SIMULATE_DATA:

		case GUI_CMD_ID_GET_STATE_INFO:

		case  GUI_CMD_ID_GET_CENTER_PROCESS_INFO:

		case GUI_CMD_ID_EXPORT_POLICY:

		case GUI_CMD_ID_VERIFY_REMOTE_SK:

		case GUI_CMD_ID_VERIFY_REMOTE_SWITCH:

		case GUI_CMD_ID_RETRIEVE_PLATFORM_INFO:

		case GUI_CMD_ID_GET_FILE_CONTENT:

		case GUI_CMD_ID_GET_DIRECTORY_LIST:

		case GUI_CMD_ID_REMOTE_ENUMERATE_PROCESS:

		case GUI_CMD_ID_REMOTE_EXECUTE_PROCESS:

		case GUI_CMD_ID_REMOTE_QUERY_REIGSTRY_INFO:

		case GUI_CMD_ID_GET_DIRECTORY_LIST_RECURSIVE:

		case GUI_CMD_ID_GET_INSTALL_FILE_INFO:

		case GUI_CMD_ID_UPLOAD_FILE:
		case GUI_CMD_ID_DELETE_REMOTE_FILE:
		case GUI_CMD_ID_GET_INSTALL_PATH:

		case GUI_CMD_ID_GET_OS_INFO:

		case GUI_CMD_ID_LOGON:

		case GUI_CMD_ID_LOGOFF:

		case GUI_CMD_ID_DOWNLOAD_SK:

		case GUI_CMD_ID_GET_PROCESS_CTRL_SYSTEM_COUNT:

		case GUI_CMD_ID_GET_PROCESS_CTRL_SYSTEM_INFORMATION:

		case GUI_CMD_ID_GET_PROCESS_CTRL_PROGRAM_COUNT:

		case GUI_CMD_ID_GET_PROCESS_CTRL_PROGRAM_INFORMATION:

		case GUI_CMD_ID_START_PROCESS_TRACE:

		case GUI_CMD_ID_STOP_PROCESS_TRACE:

		case GUI_CMD_ID_GET_LOCAL_PASSWORD:




		case GUI_CMD_ID_GET_USER_INFO:

		case GUI_CMD_ID_GET_MAC_INFO:

		case GUI_CMD_ID_GET_DAC_INFO:

		case CMD_ID_REGISTRY_LOAD_GROUP:

		case CMD_ID_REGISTRY_LOAD_USERDEFINE:

		case GUI_CMD_ID_GET_OPERATOR_LOG_COUNT:


		case GUI_CMD_ID_GET_OPERATOR_LOG_INFORMATION:

		case GUI_CMD_ID_EXPORT_OPERATOR_LOG_INFORMATION:

		case GUI_CMD_ID_GET_REG_CTRL_LOG_COUNT:

		case GUI_CMD_ID_EXPORT_REG_CTRL_LOG_INFORMATION:

		case GUI_CMD_ID_GET_REG_CTRL_LOG_INFORMATION:

		case GUI_CMD_ID_GET_PROCESS_LOG_COUNT:

		case GUI_CMD_ID_EXPORT_PROCESS_LOG_INFORMATION:

		case GUI_CMD_ID_GET_PROCESS_LOG_INFORMATION:

		case GUI_CMD_ID_GET_FILE_LOG_COUNT:

		case GUI_CMD_ID_GET_FILE_LOG_INFORMATION:

		case GUI_CMD_ID_EXPORT_FILE_LOG_INFORMATION:

		case GUI_CMD_ID_GET_USB_LOG_COUNT:

		case GUI_CMD_ID_EXPORT_USB_LOG_INFORMATION:

		case GUI_CMD_ID_GET_USB_LOG_INFORMATION:

		case GUI_CMD_ID_GET_DEVICE_CTRL_STATE:

		case GUI_CMD_ID_GET_AFFAIR_LOG_COUNT:

		case GUI_CMD_ID_GET_AFFAIR_LOG_INFORMATION:

		case GUI_CMD_ID_EXPORT_AFFAIR_LOG_INFORMATION:
		case  CMD_ID_USB_KEY_GET_LIST:        
		case  CMD_ID_USB_KEY_IS_PRESENT:
		case  CMD_ID_USB_KEY_READ_CONFIG: 
		case CMD_ID_USB_KEY_GET_USB_ID:        

		case CMD_ID_USB_KEY_PLUG_IN:                          
		case  CMD_ID_USB_KEY_PLUG_OUT:    

			{
				bAllowMaitain = TRUE;
				break;
			}
			////下面是禁止项目
		case GUI_CMD_ID_SET_LOCAL_PASSWORD:
		case GUI_CMD_ID_RESET_LOCAL_PASSWORD:
		case GUI_CMD_ID_SET_DRIVER_SWITCH_FUNCTIONS: 
		case GUI_CMD_ID_DELETE_PROCESS_CTRL_PROGRAM:
		case CMD_ID_REGISTRY_ADD_USERDEFINE:
		case CMD_ID_REGISTRY_DEL_USERDEFINE:
		case GUI_CMD_ID_SET_USER_INFO:
		case GUI_CMD_ID_ADD_MAC_INFO:
		case GUI_CMD_ID_DEL_MAC_INFO:
		case GUI_CMD_ID_CHANGE_MAC_GROUP_INFO:
		case GUI_CMD_ID_ADD_DAC_INFO:
		case GUI_CMD_ID_DEL_DAC_INFO:
		case GUI_CMD_ID_CHANGE_DAC_GROUP_INFO:
		case GUI_CMD_ID_SET_DEVICE_CTRL_STATE:
		case CMD_ID_MODIFY_SYSTEM_PASSWORD:
		case CMD_ID_USB_KEY_WRITE_CONFIG:
			{
				bAllowMaitain = FALSE;
				::MessageBoxW(AfxGetMainWnd()->m_hWnd,L"当前终端不具备相应的操作权限，完成相应操作请与管理中心联系",L"提示",MB_ICONERROR|MB_OK);
				break;
			}
		case GUI_CMD_ID_DELETE_PROCESS_CTRL_SYSTEM:
		case GUI_CMD_ID_PROCESS_CTRL_SYSTEM_RESCAN:
		case GUI_CMD_ID_PROCESS_CTRL_PROGRAM_LOCAL_FILE_SCAN:
		case GUI_CMD_ID_PROCESS_CTRL_PROGRAM_LOCAL_DIRECTORY_SCAN:

			{

				bAllowMaitain = FALSE;
				if(g_CommData.TerminalControl.bWhiteListPrivilege == 1)
				{
					bAllowMaitain = TRUE;
				}
				if (!bAllowMaitain)
				{
					::MessageBoxW(AfxGetMainWnd()->m_hWnd,L"当前终端不具备相应的操作权限，完成相应操作请与管理中心联系",L"提示",MB_ICONERROR|MB_OK);
				}
				break;
			}

		default:
			{
				TCHAR tszEvent[256] = {0};
				StringCbPrintf(tszEvent,sizeof(tszEvent),_T("%s:unknown pgui_send_buffer->nCmdID:%08x"),CA2T(__FUNCTION__),pgui_send_buffer->nCmdID);
				WriteEventLog(tszEvent,EVENTLOG_AUDIT_FAILURE,NULL);
				bAllowMaitain = FALSE;
				break;
			}

		}
	}
	return bAllowMaitain;

}
BOOL WINAPI SendGuiInfoToService(__in PCHAR pbGUIDataBuffer,__in ULONG dwGUIDataBufferLen,__out PBYTE * ppOutBuffer,__out PDWORD pdwOutputLen,__in BOOL bShowUi)
{
	BOOL  bResult = FALSE;
	__try
	{
		//PrintBlockEx(pbGUIDataBuffer,dwGUIDataBufferLen,FALSE);
		SetMyLastError(APP_ERROR_CODE_SUCCESS);
		PGUI_COMM_BUFFER pgui_send_buffer = (PGUI_COMM_BUFFER)pbGUIDataBuffer;
		if(!CheckPrivilege(pgui_send_buffer))
		{


			SetMyLastError(APP_ERROR_CODE_PERMISSION_DENY);

			return FALSE;
		}

		//EnterCriticalSection(&theApp.m_csGuiCommunication);
		bResult = RealSendGuiInfoToService(pbGUIDataBuffer,dwGUIDataBufferLen,ppOutBuffer,pdwOutputLen,bShowUi);
		//LeaveCriticalSection(&theApp.m_csGuiCommunication);
		if(pdwOutputLen && bResult)
		{
			//PrintBlockEx((PCHAR)*ppOutBuffer,*pdwOutputLen,FALSE);

		}

	}
	__except(RecordExceptionInfo(GetExceptionInformation()))
	{
		SetMyLastError(APP_ERROR_CODE_PERMISSION_DENY);
		DbgPrint("%s except occur",__FUNCDNAME__);
	}


	return bResult;
}

BOOL WINAPI RealSendGuiInfoToService(__in PCHAR pbGUIDataBuffer,__in ULONG dwGUIDataBufferLen,__out PBYTE * ppOutBuffer,__out PDWORD pdwOutputLen,__in BOOL bShowUi)
{
	DWORD dwRet = 0; 
	DWORD dwThreadID = 0;


	PGUI_COMM_BUFFER pgui_send_buffer = (PGUI_COMM_BUFFER)pbGUIDataBuffer;


	PNetworkThreadParam pNetworkThreadParam = (PNetworkThreadParam)malloc(sizeof(NetworkThreadParam));

	pNetworkThreadParam->pbGUIDataBuffer    = pbGUIDataBuffer;
	pNetworkThreadParam->dwGUIDataBufferLen = dwGUIDataBufferLen;
	pNetworkThreadParam->ppOutBuffer = ppOutBuffer;
	pNetworkThreadParam->pdwOutputLen = pdwOutputLen;
	pNetworkThreadParam->bShowUi = bShowUi;

	HANDLE hThread = NULL;

	if(bShowUi)
	{ 
		CWaitCursor WaitCursor;
		CShowStatusDlg* pDlg = new CShowStatusDlg();
		pDlg->Create(IDD_DLG_SHOW);
		pDlg->ShowWindow(SW_SHOWNORMAL);

		hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SendGuiInfoToServiceThread,pNetworkThreadParam,0,&dwThreadID);

		MSG msg; 
		while (TRUE)  
		{  
			//wait for m_hThread to be over，and wait for  
			//QS_ALLINPUT（Any message is __in the queue）  
			dwRet = MsgWaitForMultipleObjects (1, &hThread,   FALSE, INFINITE, QS_ALLINPUT);  
			switch(dwRet)  
			{  
			case WAIT_OBJECT_0:   
				break; //break the loop  
			case WAIT_OBJECT_0 + 1:  

			case WAIT_TIMEOUT:

				//get the message from Queue  
				//and dispatch it to specific window  
				while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				continue;  

			default:  
				break; // unexpected failure  
			}  
			break;  
		}  
		pDlg->DestroyWindow();
		delete pDlg;


		GetExitCodeThread(hThread,&dwRet);
		CloseHandle(hThread);
	}
	else
	{
		dwRet = SendGuiInfoToServiceThread(pNetworkThreadParam);
	}


	return dwRet;

}


BOOL ShowNotifyInfo(HWND hWnd,const CString& strTitle,const CString& strContent)
{

	CMFCDesktopAlertWnd* pPopup = new CMFCDesktopAlertWnd;

	pPopup->SetAnimationType ((CMFCPopupMenu::ANIMATION_TYPE) 3);
	pPopup->SetAnimationSpeed (0x1e);
	pPopup->SetTransparency ((BYTE)0xc8);
	pPopup->SetSmallCaption (TRUE);
	pPopup->SetAutoCloseTime (3000);


	// Create indirect:
	CMFCDesktopAlertWndInfo params;


	CMainFrame* pMainFrame=  (CMainFrame*)AfxGetMainWnd();
	if(pMainFrame != NULL)
	{
		params.m_hIcon = pMainFrame->m_Icons.ExtractIcon (5);
	}
	else
	{
		params.m_hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_INFORMATION));
	}


	params.m_strText = strTitle;
	params.m_strURL = strContent;
	params.m_nURLCmdID = 101;

	pPopup->Create (CWnd::FromHandle(hWnd), params, NULL, CPoint(-1,-1));


	HICON hIcon = (HICON) ::LoadImage (::AfxGetResourceHandle (), 
		MAKEINTRESOURCE (IDR_MAINFRAME),
		IMAGE_ICON, ::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON), 0);

	pPopup->SetIcon (hIcon, TRUE);
	pPopup->SetWindowText (strContent);

	return TRUE;

}

COLORREF DarColor()
{
	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_OFF_2007_BLUE: return TemaCeleste;
	case ID_VIEW_APPLOOK_OFF_2007_BLACK: return TemaNegro;
	case ID_VIEW_APPLOOK_OFF_2007_SILVER: return TemaPlateado;
	case ID_VIEW_APPLOOK_OFF_2007_AQUA: return TemaAqua;
	}
	return TemaAqua;
}
static COLORREF DarColorR()
{
	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_OFF_2007_BLUE: return TemaCelesteR;
	case ID_VIEW_APPLOOK_OFF_2007_BLACK: return TemaNegroR;
	case ID_VIEW_APPLOOK_OFF_2007_SILVER: return TemaPlateadoR;
	case ID_VIEW_APPLOOK_OFF_2007_AQUA: return TemaAquaR;
	}
}
static COLORREF DarColorFondoGrilla()
{
	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_OFF_2007_BLUE: return RGB(248,248,255);
	default: return RGB(245,245,245);
	}
}
static COLORREF ShiftColor(COLORREF color, int shiftc)
{
	BYTE byRed = GetRValue(color);
	BYTE byGreen = GetGValue(color);
	BYTE byBlue = GetBValue(color);

	byRed	+= min(shiftc, 255);
	byGreen += min(shiftc,255);
	byBlue	+= min(shiftc,255);

	return RGB(byRed, byGreen, byBlue);
}
static COLORREF DarColorSelect()
{
	short shOffset = 30;
	COLORREF color = DarColor();
	BYTE	byRed = 0;
	BYTE	byGreen = 0;
	BYTE	byBlue = 0;

	byRed = GetRValue(color);
	byGreen = GetGValue(color);
	byBlue = GetBValue(color);

	short	shOffsetR = shOffset;
	short	shOffsetG = shOffset;
	short	shOffsetB = shOffset;

	if (shOffset > 0)
	{
		if (byRed + shOffset > 255)		shOffsetR = 255 - byRed;
		if (byGreen + shOffset > 255)	shOffsetG = 255 - byGreen;
		if (byBlue + shOffset > 255)	shOffsetB = 255 - byBlue;

		shOffset = min(min(shOffsetR, shOffsetG), shOffsetB);
	} // if
	else
	{
		if (byRed + shOffset < 0)		shOffsetR = -byRed;
		if (byGreen + shOffset < 0)		shOffsetG = -byGreen;
		if (byBlue + shOffset < 0)		shOffsetB = -byBlue;

		shOffset = max(max(shOffsetR, shOffsetG), shOffsetB);
	} // else

	// Set new color
	color = RGB(byRed + shOffset, byGreen + shOffset, byBlue + shOffset);
	return color;
}
BOOL IsUnicodeChineseSimpleStrng(WCHAR* wszSource)
{
	ULONG ulLen = wcslen(wszSource);
	for (ULONG i =0; i < ulLen; i++)
	{
		if(wszSource[i]<0x4e00 || wszSource[i]> 0x9fff)
			return FALSE;
	}
	return TRUE;
}
BOOL WINAPI CheckPE (IN TCHAR* FileName)
{
	HANDLE                           hFile;

	DWORD                            FileBufferSize = 32 * 1024;

	LARGE_INTEGER                    ByteOffset = {0};
	LARGE_INTEGER                    FileSize = {0};

	PUCHAR                           CheckFileBuffer = NULL;
	LARGE_INTEGER                    CheckFileBufferSize = {0};

	BOOL                             bResult = FALSE;
	PUCHAR                           FileHash = NULL;

	DWORD                            dwErrorCode;
	CHAR                             szErrorDescript[256]={0};

	hFile = CreateFile (FileName,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwErrorCode = GetLastError();

		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));

		DbgPrintEx (" open  %s error,%s ",FileName,szErrorDescript);
		return FALSE;
	}

	if (GetFileSizeEx (hFile,&FileSize) == FALSE)
	{
		dwErrorCode = GetLastError();

		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));


		CloseHandle (hFile);

		DbgPrintEx (" GetFileSizeEx Error %s",szErrorDescript);
		return FALSE;
	}

	if (FileSize.QuadPart == 0)
	{
		CloseHandle (hFile);
		DbgPrintEx ("%s FileSize too small",FileName);
		return FALSE;
	}

#ifndef SCAN_WHOLE_FILE
	if (FileSize.QuadPart > FileBufferSize)
	{
		CheckFileBufferSize.QuadPart = FileBufferSize;
	}
	else
#endif
	{
		CheckFileBufferSize.QuadPart = FileSize.QuadPart;
	}


	ByteOffset.QuadPart = 0;

	HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,0,(DWORD)CheckFileBufferSize.QuadPart,NULL);
	if(hFileMapOfView)
	{

		CheckFileBuffer=(unsigned char *)MapViewOfFile(hFileMapOfView,FILE_MAP_READ,0,0,(SIZE_T)CheckFileBufferSize.QuadPart);
		if(CheckFileBuffer)
		{   __try
		{
			if(IsPeFile(CheckFileBuffer,(DWORD)CheckFileBufferSize.QuadPart,&bResult))
			{

			}
			UnmapViewOfFile(CheckFileBuffer);
		}
		__except(GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR ?
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			// Failed to read from the view.
		}


		}

		CloseHandle(hFileMapOfView);
	}
	else
	{
		dwErrorCode = GetLastError();
		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));
		CloseHandle (hFile);
		DbgPrintEx (" open  %s error,%s ",FileName,szErrorDescript);
		return FALSE;
	}
	CloseHandle (hFile);
	return bResult;
}

BOOL IsPeFile(__in PUCHAR pvBuffer,__in ULONG ulBufferLen,__out BOOL* pbIsExe)
{
	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NtHeader;


	DosHeader = (PIMAGE_DOS_HEADER)pvBuffer;
	if (_strnicmp ((PCHAR)(&DosHeader->e_magic),"MZ",2) != 0)
	{
		return FALSE;
	}

	if (DosHeader->e_lfanew < 0 || DosHeader->e_lfanew > (LONG)(ulBufferLen - sizeof (IMAGE_NT_HEADERS)))
	{
		return FALSE;
	}

	NtHeader = (PIMAGE_NT_HEADERS)(pvBuffer + DosHeader->e_lfanew);

	if (_strnicmp ((PCHAR)(&NtHeader->Signature),"PE",2) != 0)
	{
		return FALSE;
	}
	if (!FlagOn (NtHeader->FileHeader.Characteristics,IMAGE_FILE_EXECUTABLE_IMAGE))
	{
		return FALSE;
	}

	if (!FlagOn (NtHeader->FileHeader.Characteristics,IMAGE_FILE_DLL))
	{
		if(pbIsExe)
		{
			*pbIsExe = TRUE;
		}
	}

	return TRUE;

}

ULONG CalcPages(ULONG ulRecordCount,ULONG ulCountPerPage)
{

	if(ulRecordCount ==0)
		return 0;
	else if ( ulRecordCount <= ulCountPerPage )
		return 1;
	else
	{
		UINT uiMod = ulRecordCount % ulCountPerPage;
		if ( uiMod == 0 )
		{
			return ulRecordCount / ulCountPerPage;
		}
		else
		{
			return ( ulRecordCount-uiMod ) / ulCountPerPage + 1;
		}
	}

}
BOOL ClearBlobData()
{
	TCHAR tszDBPathName[MAX_PATH] = {0};

    ExpandEnvironmentStrings(_T("%TEMP%\\CacheData.dat"),tszDBPathName,_countof(tszDBPathName));
	//GetModuleFileName (NULL,tszDBPathName,_countof (tszDBPathName));
	//TCHAR* tszTemp = _tcsrchr (tszDBPathName,_T('\\'));

	//if (tszTemp)
	//{
	//	* tszTemp = NULL;
	//}
	//StringCbCat (tszDBPathName,sizeof (tszDBPathName),_T("\\db\\CacheData.dat"));//数据库路径模块所在目录下面db子目录
	DeleteFile(tszDBPathName);
	return TRUE;
}
BOOL LoadBlobData(__in IN LPTSTR lpstTableName,__in LPTSTR lpstLastModified,__out LPVOID *ppvData,__out ULONG* pulDataLen,__in int role,__in int domain)
{

	TCHAR tszDBPathName[MAX_PATH] = {0};
	BOOL bDBInitSuccess = FALSE;
	Database CacheDb;

    ExpandEnvironmentStrings(_T("%TEMP%\\CacheData.dat"),tszDBPathName,_countof(tszDBPathName));   
	//GetModuleFileName (NULL,tszDBPathName,_countof (tszDBPathName));
	//TCHAR* tszTemp = _tcsrchr (tszDBPathName,_T('\\'));

	//if (tszTemp)
	//{
	//	* tszTemp = NULL;
	//}
	//StringCbCat (tszDBPathName,sizeof (tszDBPathName),_T("\\db\\CacheData.dat"));//数据库路径模块所在目录下面db子目录

	DWORD dwAttribute =  GetFileAttributes (tszDBPathName);
	if (dwAttribute ==INVALID_FILE_ATTRIBUTES)
	{
		bDBInitSuccess = FALSE;
	}
	else
	{
		CacheDb.Open(tszDBPathName);
		bDBInitSuccess = TRUE;
	}
	if(!bDBInitSuccess)
	{

		//DbgPrintW(DLL_POLICY_MANAGE_MOD_NAME_A,"%s 数据库初始化失败","SecurityTerminal");
		return FALSE;
	}
	CString strSQL;
	if(role ==0 || domain ==0)
	{
		strSQL.Format(_T("select cache_data  from %s where table_name=\'%s\' and date_time=\'%s\' ;"), _T("cache_table"),lpstTableName,lpstLastModified);
	}
	else
	{
		strSQL.Format(_T("select cache_data  from %s where table_name=\'%s\' and date_time=\'%s\' and role=\'%u\' and domain=\'%d\';"), _T("cache_table"),lpstTableName,lpstLastModified,role,domain);
	}
	int iSqlErr = CacheDb.ReadBlob(strSQL,(BYTE**)ppvData,(int*)pulDataLen);
	if(iSqlErr == SQLITE_OK)
	{
		if(*pulDataLen ==0 || *ppvData == NULL)
		{
			strSQL.Format(_T("delete *  from %s where table_name=\'%s\' ;"), _T("cache_table"),lpstTableName);
			CacheDb.ExecuteSQL(strSQL);
			return FALSE;
		}
		else
		{

			return TRUE;
		}

	}
	else
	{

		strSQL.Format(_T("delete *  from %s where table_name=\'%s\' ;"), _T("cache_table"),lpstTableName);
		CacheDb.ExecuteSQL(strSQL);

		DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"%s 读取失败，erro：%u",CT2CA(CacheDb.m_strLastError),iSqlErr);

		return FALSE;
	}
	return TRUE;

}
BOOL SaveBlobData(__in LPTSTR lpstTableName,__in LPTSTR lpstLastModified,__in LPVOID pvData,__in ULONG ulDataLen,__in int role,__in int domain)
{
#define CACHE_DB_TABLE_NAME _T("cache_table")

	TCHAR tszDBPathName[MAX_PATH] = {0};
	BOOL bDBInitSuccess = FALSE;
	Database CacheDb;

	//GetModuleFileName (NULL,tszDBPathName,_countof (tszDBPathName));
	//TCHAR* tszTemp = _tcsrchr (tszDBPathName,_T('\\'));

	//if (tszTemp)
	//{
	//	* tszTemp = NULL;
	//}
	//StringCbCat (tszDBPathName,sizeof (tszDBPathName),_T("\\db\\CacheData.dat"));//数据库路径模块所在目录下面db子目录
    ExpandEnvironmentStrings(_T("%TEMP%\\CacheData.dat"),tszDBPathName,_countof(tszDBPathName));   

	DWORD dwAttribute =  GetFileAttributes (tszDBPathName);
	if (dwAttribute ==INVALID_FILE_ATTRIBUTES)
	{
		if (SQLITE_OK==CacheDb.Open (tszDBPathName))
		{
			//if (!CacheDb.IsTableExistd (CACHE_DB_TABLE_NAME))

			CString strSqlStatement;

			strSqlStatement.Format (_T("create table if not exists %s (")
				_T(" table_name text COLLATE NOCASE,")
				_T(" date_time text COLLATE NOCASE,")
				_T(" role text  COLLATE NOCASE,")
				_T(" domain text  COLLATE NOCASE,")
				_T(" cache_data blob")

				_T(" ); "),
				CACHE_DB_TABLE_NAME

				);


			int iErr= CacheDb.ExecuteSQL (strSqlStatement);
			if (iErr==SQLITE_OK)
			{
				bDBInitSuccess = TRUE;
			}
			else
			{
				bDBInitSuccess = FALSE;

				if(CacheDb.m_strLastError.CollateNoCase(_T("table cache_table already exists"))==0)
				{

					bDBInitSuccess = TRUE;
				}
				DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"%s:  error:  %s","SecurityTerminal",CT2CA (CacheDb.m_strLastError))


			}


		}
	}
	else
	{
		CacheDb.Open(tszDBPathName);
		bDBInitSuccess = TRUE;
	}
	if(!bDBInitSuccess)
	{


		//DbgPrintW(DLL_POLICY_MANAGE_MOD_NAME_A,"%s 数据库初始化失败","SecurityTerminal");
		return FALSE;
	}
	CacheDb.BeginTransaction();
	CString strSQL;
	strSQL.Format(_T("insert into %s (table_name,date_time,role,domain,cache_data)  values (\'%s\',\'%s\',\'%d\',\'%d\',?);"),_T("cache_table"),lpstTableName,lpstLastModified,role,domain);
	int iSqlErr = CacheDb.WriteBlob(strSQL,(BYTE*)pvData,ulDataLen);

	if(iSqlErr == SQLITE_OK )
	{

		BOOL bCommit = CacheDb.CommitTransaction();	
		CacheDb.Close();
		return TRUE;
	}
	else
	{	DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"%u 插入失败，erro：%s",iSqlErr,CT2CA(CacheDb.m_strLastError));


	BOOL bCommit = CacheDb.CommitTransaction();
	CacheDb.Close();

	return FALSE;
	}
	return TRUE;

}

BOOL IsWow64Process()    
{       
	typedef BOOL (WINAPI *LPFN_IsWow64Process) (HANDLE, PBOOL);
	LPFN_IsWow64Process fnIsWow64Process;    
	BOOL bIsWow64Process = FALSE;    
	fnIsWow64Process = (LPFN_IsWow64Process)GetProcAddress( GetModuleHandleA("kernel32.dll"),"IsWow64Process");    
	if (NULL != fnIsWow64Process)    
	{    
		fnIsWow64Process(GetCurrentProcess(),&bIsWow64Process); 
	}    
	return bIsWow64Process;    
}

BOOL DisableWow64(LPVOID* dwOldValue)
{
	BOOL bRet = FALSE;
	typedef BOOL (WINAPI *LPFN_WOW64DISABLEWOW64FSREDIRECTION)(LPVOID *);
	LPFN_WOW64DISABLEWOW64FSREDIRECTION gfnDisableWow64 = NULL;
	gfnDisableWow64 = (LPFN_WOW64DISABLEWOW64FSREDIRECTION) GetProcAddress (GetModuleHandleW (L"kernel32"),"Wow64DisableWow64FsRedirection");
	if (gfnDisableWow64 == NULL)
	{
		return bRet;
	}
	else
	{
		if (!gfnDisableWow64(dwOldValue))
		{
			return bRet;
		}
	}
	bRet = TRUE;

	return bRet;
}

BOOL Wow64Revert(LPVOID dwOldValue)
{
	BOOL bRet = FALSE;
	typedef BOOL (WINAPI *LPFN_WOW64REVERTWOW64FSREDIRECTIIO)(LPVOID);
	LPFN_WOW64REVERTWOW64FSREDIRECTIIO gfnWow64Revert = NULL;
	gfnWow64Revert = (LPFN_WOW64REVERTWOW64FSREDIRECTIIO) GetProcAddress (GetModuleHandleW (L"kernel32"),"Wow64RevertWow64FsRedirection");
	if (gfnWow64Revert == NULL)
	{
		return bRet;
	}
	else
	{
		if(!gfnWow64Revert(dwOldValue))
		{
			return bRet;
		}
	}
	bRet = TRUE;

	return bRet;
}
BOOL GetRemoteData(TCHAR* tszRemoteFileName,PVOID * ppRemoteData,PULONG pulRemoteDataLen)
{
	PBYTE pvRemoteDataMemory = NULL;
	ULONG ulRemoteDataMemoryTotal = 0;


	BOOL bResult = FALSE;
	BOOL bEnd = FALSE;
	BOOL bCompress = FALSE;
	Read_File_Request read_file_request = {0};
	PRead_File_Response pReadFileResponse = NULL;

	HANDLE hRemoteSrc = INVALID_HANDLE_VALUE;
	HANDLE hLocalDst = INVALID_HANDLE_VALUE;

	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_GET_FILE_CONTENT;
	StringCbCopyA(gui_send_buffer.ReadFileRequest.szFileName,sizeof(gui_send_buffer.ReadFileRequest.szFileName),CT2A(tszRemoteFileName));
	gui_send_buffer.ReadFileRequest.dwReadPos = 0;
	gui_send_buffer.ReadFileRequest.dwReadRequest = 1024;

	do 
	{
		gui_send_buffer.dwExtraDataSize = sizeof(Read_File_Request);
		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;


			pReadFileResponse = (PRead_File_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

			if (Header->QueryResult>=sizeof(Read_File_Response))
			{
				if(pReadFileResponse->bCompressFile)
				{
					bCompress = TRUE;
				}
				if (pReadFileResponse->dwErrorCode)
				{
					DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"Read RemoteFile:%s,error:%u",pReadFileResponse->szFileName,pReadFileResponse->dwErrorCode);
					bEnd = TRUE;
				}
				else
				{
					if (pReadFileResponse->dwReadResponse)
					{
						if(pvRemoteDataMemory == NULL)
						{   
							pvRemoteDataMemory = (PBYTE)malloc(pReadFileResponse->dwReadResponse);
						}
						else
						{
							pvRemoteDataMemory = (PBYTE)realloc(pvRemoteDataMemory,pReadFileResponse->dwReadResponse+ulRemoteDataMemoryTotal);
						}

						memcpy((pvRemoteDataMemory+ulRemoteDataMemoryTotal),pReadFileResponse->pbReadBuffer,(DWORD)pReadFileResponse->dwReadResponse);

						ulRemoteDataMemoryTotal += pReadFileResponse->dwReadResponse;





					}
					if(pReadFileResponse->dwReadResponse < pReadFileResponse->dwReadRequest)
					{
						bEnd = TRUE;
					}

				}
			}
			else
			{
				bEnd = TRUE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}

		if(bEnd)
		{
			bResult = TRUE;
			if (bCompress)
			{
				char szCabPath[MAX_PATH] = {0};
				GetTempPathA(sizeof(szCabPath),szCabPath);
				char szCabFileName[MAX_PATH] = {0};
				GetTempFileNameA(szCabPath,"_",1,szCabFileName);
				StringCbCatA(szCabFileName,sizeof(szCabFileName),".cab");
				DeleteFileA(szCabFileName);

				HANDLE hFile = CreateFileA (szCabFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{

					DWORD dwWritten = 0;
					WriteFile(hFile,pvRemoteDataMemory,ulRemoteDataMemoryTotal,&dwWritten,NULL);
					CloseHandle(hFile);

					char szFileName[MAX_PATH] = {0};
					GetTempFileNameA(szCabPath,"_",1,szFileName);

					CHAR szCmdLine[1024] ={0};
					StringCbPrintfA(szCmdLine,sizeof(szCmdLine),"expand.exe \"%s\" \"%s\"",szCabFileName,szFileName);
					WinExec(szCmdLine,SW_HIDE);

					Sleep(500);
					HANDLE hFileRead = CreateFileA (szFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
					if (hFileRead != INVALID_HANDLE_VALUE)
					{
						if(pvRemoteDataMemory)
						{
							free(pvRemoteDataMemory);
							pvRemoteDataMemory = NULL;
							ulRemoteDataMemoryTotal = 0;
						}
						DWORD dwFileSize = GetFileSize(hFileRead,NULL);
						pvRemoteDataMemory = (PBYTE) malloc(dwFileSize);
						ulRemoteDataMemoryTotal = dwFileSize;


						DWORD dwRead = 0;
						ReadFile(hFileRead,pvRemoteDataMemory,ulRemoteDataMemoryTotal,&dwRead,NULL);
						CloseHandle(hFileRead);
						DeleteFileA(szCabFileName);
						DeleteFileA(szFileName);
					}



				}
			}
			break;
		}
		gui_send_buffer.ReadFileRequest.dwReadPos += gui_send_buffer.ReadFileRequest.dwReadRequest;
	} while (TRUE);

	if (bResult)
	{
		if(ppRemoteData)
		{
			*ppRemoteData = pvRemoteDataMemory;
		}
		if(pulRemoteDataLen)
		{
			*pulRemoteDataLen = ulRemoteDataMemoryTotal;
		}
	}
	return bResult ;
}
BOOL ExportRemoteFile(TCHAR * tszLocalFileName,TCHAR* tszRemoteFileName)
{

	if(!theApp.m_bRemoteDebug)
	{
		return TRUE;
	}
	BOOL bResult = FALSE;
	BOOL bEnd = FALSE;
	BOOL bCompress = FALSE;
	Read_File_Request read_file_request = {0};
	PRead_File_Response pReadFileResponse = NULL;

	HANDLE hRemoteSrc = INVALID_HANDLE_VALUE;
	HANDLE hLocalDst = INVALID_HANDLE_VALUE;

	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_GET_FILE_CONTENT;
	StringCbCopyA(gui_send_buffer.ReadFileRequest.szFileName,sizeof(gui_send_buffer.ReadFileRequest.szFileName),CT2A(tszRemoteFileName));
	gui_send_buffer.ReadFileRequest.dwReadPos = 0;
	gui_send_buffer.ReadFileRequest.dwReadRequest = 1024;
	do 
	{
		gui_send_buffer.dwExtraDataSize = sizeof(Read_File_Request);
		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;


			pReadFileResponse = (PRead_File_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

			if (Header->QueryResult>=sizeof(Read_File_Response))
			{

				if (pReadFileResponse->dwErrorCode)
				{
					DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"Read RemoteFile:%s,error:%u",pReadFileResponse->szFileName,pReadFileResponse->dwErrorCode);
					bEnd = TRUE;
				}
				else
				{
					if(pReadFileResponse->bCompressFile)
					{
						bCompress = TRUE;
					}
					if (pReadFileResponse->dwReadResponse)
					{
						HANDLE hFile = CreateFile (tszLocalFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
						if (hFile == INVALID_HANDLE_VALUE)
						{

							DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"write LocalFile:%s,error:%u",CT2A(tszLocalFileName),GetLastError());

						}
						else
						{
							SetFilePointer (hFile,0,NULL,FILE_END);
							DWORD dwWritten = 0;
							WriteFile (hFile,pReadFileResponse->pbReadBuffer,(DWORD)pReadFileResponse->dwReadResponse,&dwWritten,NULL);


							CloseHandle(hFile);
						}
					}
					if(pReadFileResponse->dwReadResponse < pReadFileResponse->dwReadRequest)
					{
						bEnd = TRUE;
					}

				}
			}
			else
			{
				bEnd = TRUE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}

		if(bEnd)
		{
			bResult = TRUE;
			if( bCompress)
			{
				CHAR szCmdLine[1024] ={0};
				StringCbPrintfA(szCmdLine,sizeof(szCmdLine),"expand.exe \"%s\" \"%s\"",pReadFileResponse->szFileName,gui_send_buffer.ReadFileRequest.szFileName);
				WinExec(szCmdLine,SW_HIDE);
				Sleep(500);
			}
			break;
		}
		gui_send_buffer.ReadFileRequest.dwReadPos += gui_send_buffer.ReadFileRequest.dwReadRequest;
	} while (TRUE);

	return bResult ;

}

BOOL RemoteExecCmd(TCHAR* tszCmdLine,TCHAR* tszLocalFileName)
{
	BOOL bResult = FALSE;
	BOOL bEnd = FALSE;
	BOOL bCompress = FALSE;
	CHAR szCabFileName[MAX_PATH] = {0};

	ZeroMemory(szCabFileName,sizeof(szCabFileName));
	PExec_Process_Information_Response pExecProcessInformation = NULL;

	Read_File_Request read_file_request = {0};
	PRead_File_Response pReadFileResponse = NULL;

	HANDLE hRemoteSrc = INVALID_HANDLE_VALUE;
	HANDLE hLocalDst = INVALID_HANDLE_VALUE;

	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_REMOTE_EXECUTE_PROCESS;
	StringCbCopyA(gui_send_buffer.ExecuteProcessInformation.szProcessName,sizeof(gui_send_buffer.ExecuteProcessInformation.szProcessName),CT2A(tszCmdLine));

	if(GetFileAttributes(tszLocalFileName) != INVALID_FILE_ATTRIBUTES)
	{
		DeleteFile(tszLocalFileName);
	}
	gui_send_buffer.dwExtraDataSize = sizeof(Exec_Process_Information_Request);
	if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
	{

		PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;
		if (Header->QueryResult>sizeof (GUI_QUERY_HEADER))
		{
			pExecProcessInformation = (PExec_Process_Information_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));


			if (pExecProcessInformation->dwErrorCode)
			{
				DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"exec error RemoteFile:%s,error:%u",pExecProcessInformation->szExecuteResultFileName,pExecProcessInformation->dwErrorCode);

			}
			else
			{
				PBYTE           ResultBuf2       =  NULL;
				DWORD           dwLen2           =  0;

				char szRemoteFileName[MAX_PATH] = {0};
				StringCbCopyA(szRemoteFileName,sizeof(szRemoteFileName),pExecProcessInformation->szExecuteResultFileName);


				gui_send_buffer.nCmdID = GUI_CMD_ID_GET_FILE_CONTENT;
				StringCbCopyA(gui_send_buffer.ReadFileRequest.szFileName,sizeof(gui_send_buffer.ReadFileRequest.szFileName),szRemoteFileName);
				gui_send_buffer.ReadFileRequest.dwReadPos = 0;
				gui_send_buffer.ReadFileRequest.dwReadRequest = 1024;

				do 
				{               
					gui_send_buffer.dwExtraDataSize = sizeof(Read_File_Request);
					if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf2,&dwLen2))
					{
						PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf2;
						pReadFileResponse = (PRead_File_Response)(ResultBuf2 + sizeof(GUI_QUERY_HEADER));
						if (Header->QueryResult>=sizeof(Read_File_Response))
						{
							if (pReadFileResponse->dwErrorCode)
							{
								DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"Read RemoteFile:%s,error:%u",pReadFileResponse->szFileName,pReadFileResponse->dwErrorCode);
								bEnd = TRUE;
								free(ResultBuf2);
								ResultBuf2 = NULL;
							}
							else
							{
								if(pReadFileResponse->bCompressFile)
								{
									char szCabPath[MAX_PATH] = {0};
									GetTempPathA(sizeof(szCabPath),szCabPath);

									GetTempFileNameA(szCabPath,"_",1,szCabFileName);
									StringCbCatA(szCabFileName,sizeof(szCabFileName),".cab");
									DeleteFileA(szCabFileName);

									bCompress = TRUE;
								}
								if (pReadFileResponse->dwReadResponse)
								{
									HANDLE hFile = CreateFileA (!bCompress?CT2A(tszLocalFileName):szCabFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
									if (hFile == INVALID_HANDLE_VALUE)
									{

										DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"write LocalFile:%s,error:%u",CT2A(tszLocalFileName),GetLastError());

									}
									else
									{
										SetFilePointer (hFile,0,NULL,FILE_END);
										DWORD dwWritten = 0;
										WriteFile (hFile,pReadFileResponse->pbReadBuffer,(DWORD)pReadFileResponse->dwReadResponse,&dwWritten,NULL);


										CloseHandle(hFile);
									}
								}
								if(pReadFileResponse->dwReadResponse < pReadFileResponse->dwReadRequest)
								{
									bEnd = TRUE;
								}

								free(ResultBuf2);
								ResultBuf2 = NULL;

							}  
							if(bEnd)
							{
								bResult = TRUE;
								if(bCompress)
								{
									CHAR szCmdLine[1024] ={0};
									StringCbPrintfA(szCmdLine,sizeof(szCmdLine),"expand.exe \"%s\" \"%s\"",szCabFileName,CT2A( tszLocalFileName));
									WinExec(szCmdLine,SW_HIDE);
									Sleep(500);
								}
								break;
							}
						}   
						else
						{
							free(ResultBuf2);
							ResultBuf2 = NULL;
						}
						gui_send_buffer.ReadFileRequest.dwReadPos += gui_send_buffer.ReadFileRequest.dwReadRequest;
					}
					else
					{
						break;
					}
				} while (TRUE);

				DeleteRemoteFile(CA2T(pExecProcessInformation->szExecuteResultFileName));


			}
		}
		free(ResultBuf);
		ResultBuf = NULL;
	}
	return bResult ;
}

BOOL UploadRemoteFile(TCHAR* tszLocalFile)
{
	BOOL bResult = FALSE;
	BOOL bEnd = FALSE;
	Upload_File_Request Upload_file_request = {0};
	PUpload_File_Response pUploadFileResponse = NULL;
	TCHAR tszTempFile[MAX_PATH] = {0};
	StringCbCopy(tszTempFile,sizeof(tszTempFile),tszLocalFile);
	TCHAR* tszTemp = _tcsrchr(tszTempFile,_T('\\'));
	if(tszTemp)
	{
		* tszTemp = 0;
		tszTemp++;

	}
	else
	{
		tszTemp = tszTempFile;
	}

	HANDLE hLocalDst = INVALID_HANDLE_VALUE;

	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_UPLOAD_FILE;

	StringCbCopyA(gui_send_buffer.UploadFileRequest.szFileName,sizeof(gui_send_buffer.UploadFileRequest.szFileName),CT2A(tszTemp));
	gui_send_buffer.UploadFileRequest.dwWritePos = 0;
	gui_send_buffer.UploadFileRequest.dwWriteRequest =  sizeof(gui_send_buffer.UploadFileRequest.pbWriteBuffer);
	HANDLE hFile = CreateFile (tszLocalFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{

		DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"read LocalFile:%s,error:%u",CT2A(tszLocalFile),GetLastError());
		return FALSE;

	}

	do 
	{

		SetFilePointer (hFile,gui_send_buffer.UploadFileRequest.dwWritePos,NULL,FILE_BEGIN);
		DWORD dwRead = 0;
		gui_send_buffer.UploadFileRequest.dwWriteRequest = sizeof(gui_send_buffer.UploadFileRequest.pbWriteBuffer);
		ReadFile(hFile,gui_send_buffer.UploadFileRequest.pbWriteBuffer,gui_send_buffer.UploadFileRequest.dwWriteRequest,&dwRead,NULL);
		if(dwRead < gui_send_buffer.UploadFileRequest.dwWriteRequest)
		{
			gui_send_buffer.UploadFileRequest.dwWriteRequest = dwRead;
		}
		CloseHandle(hFile);
		gui_send_buffer.dwExtraDataSize = sizeof(Upload_File_Request);

		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;


			pUploadFileResponse = (PUpload_File_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

			if (Header->QueryResult>=sizeof(Upload_File_Response))
			{

				if (pUploadFileResponse->dwErrorCode)
				{
					DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"Upload RemoteFile:%s,error:%u",pUploadFileResponse->szFileName,pUploadFileResponse->dwErrorCode);
					bEnd = TRUE;
				}
				else
				{

					if(dwRead <  sizeof(gui_send_buffer.UploadFileRequest.pbWriteBuffer))
					{
						bEnd = TRUE;
					}

				}
			}
			else
			{
				bEnd = TRUE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}

		if(bEnd)
		{
			bResult = TRUE;
			break;
		}
		gui_send_buffer.UploadFileRequest.dwWritePos += gui_send_buffer.UploadFileRequest.dwWriteRequest;
	} while (TRUE);
	return TRUE;
}
BOOL DeleteRemoteFile(TCHAR* tszRemoteFile)
{
	BOOL bResult = FALSE;

	Delete_Remote_File_Request Delete_Remote_File_Request = {0};
	PDelete_Remote_File_Response pDeleteFileResponse = NULL;



	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_DELETE_REMOTE_FILE;

	StringCbCopyA(gui_send_buffer.DeleteRemoteFileRequest.szFileName,sizeof(gui_send_buffer.DeleteRemoteFileRequest.szFileName),CT2A(tszRemoteFile));

	do 
	{
		gui_send_buffer.dwExtraDataSize = sizeof(Delete_Remote_File_Request);

		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;


			pDeleteFileResponse = (PDelete_Remote_File_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

			if (Header->QueryResult>=sizeof(Delete_Remote_File_Response))
			{

				if (pDeleteFileResponse->dwErrorCode)
				{
					DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"Delete RemoteFile:%s,error:%u",Delete_Remote_File_Request.szFileName,pDeleteFileResponse->dwErrorCode);

				}
				else
				{

					bResult = TRUE;

				}
			}
			else
			{
				bResult = FALSE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}



	} while (FALSE);
	return bResult;
}
BOOL GetRemoteInstallPath(TCHAR * szInstallPath,ULONG ulInstallPathLen)
{
	BOOL bResult = FALSE; 
	PGet_Install_Path_Response pGetRemoteInstallPath = NULL;
	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_GET_INSTALL_PATH;
	do 
	{

		gui_send_buffer.dwExtraDataSize = sizeof(Get_Install_Path_Request);

		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;
			if (Header->QueryResult==sizeof(Get_Install_Path_Response))
			{
				pGetRemoteInstallPath = (PGet_Install_Path_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

				if(pGetRemoteInstallPath)
				{
					StringCbCopy(szInstallPath,ulInstallPathLen,CA2T(pGetRemoteInstallPath->szInstallPath));
					bResult = TRUE;
				}
			}
			else
			{
				bResult = FALSE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}

	} while (FALSE);
	return bResult;

}
BOOL GetWindowVersion(TCHAR* str, int bufferSize,OSVERSIONINFOEXA &osvi)
{
	BOOL bResult = FALSE; 
	PGet_OS_Version_Info_Response pGetOsVersionInfo = NULL;
	GUI_COMM_BUFFER gui_send_buffer = {0};
	DWORD           dwOutputLen     =  0;
	PBYTE           ResultBuf       =  NULL;
	DWORD           dwLen           =  0;
	ZeroMemory(&gui_send_buffer,sizeof(gui_send_buffer));

	gui_send_buffer.nCmdID = GUI_CMD_ID_GET_OS_INFO;
	do 
	{
		gui_send_buffer.dwExtraDataSize = sizeof(Get_OS_Version_Info_Request);

		if(SendGuiInfoToService((PCHAR)&gui_send_buffer,gui_send_buffer.dwExtraDataSize +8,&ResultBuf,&dwLen))
		{

			PGUI_QUERY_HEADER Header = (PGUI_QUERY_HEADER)ResultBuf;
			if (Header->QueryResult==sizeof(Get_OS_Version_Info_Response))
			{
				pGetOsVersionInfo = (PGet_OS_Version_Info_Response)(ResultBuf + sizeof(GUI_QUERY_HEADER));

				if(pGetOsVersionInfo)
				{
					StringCbCopy(str,bufferSize,CA2T(pGetOsVersionInfo->szOsDecription));
					memcpy(&osvi,&pGetOsVersionInfo->osvi,sizeof(OSVERSIONINFOEXA));
					bResult = TRUE;
				}
			}
			else
			{
				bResult = FALSE;
			}
			free(ResultBuf);
			ResultBuf = NULL;
		}

	} while (FALSE);
	return bResult;
}


BOOL WINAPI GetFileSizeHash (IN PCHAR FileName, OUT PCHAR szFileHash,IN ULONG ulFileHashLen,OUT ULONG64* pul64FileSize)
{
	HANDLE                           hFile;

	DWORD                            FileBufferSize = 32 * 1024;

	LARGE_INTEGER                    ByteOffset = {0};
	LARGE_INTEGER                    FileSize = {0};

	PUCHAR                           CheckFileBuffer = NULL;
	LARGE_INTEGER                    CheckFileBufferSize = {0};



	sha1_context                     Sha1Context = {0};
	PUCHAR                           FileHash = NULL;

	DWORD                            dwErrorCode;
	CHAR                             szErrorDescript[256]={0};


	if (szFileHash == NULL)
	{
		DbgPrintEx (" invalid parameter %08x",ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	memset (szFileHash,0,ulFileHashLen);


	hFile = CreateFileA (FileName,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwErrorCode = GetLastError();

		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));
		DbgPrintEx ("open  %s error,%d,%s ",FileName,dwErrorCode,szErrorDescript);
		return FALSE;
	}



	if (GetFileSizeEx (hFile,&FileSize) == FALSE)
	{
		dwErrorCode = GetLastError();

		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));


		CloseHandle (hFile);

		DbgPrintEx ("GetFileSizeEx Error %s",szErrorDescript);
		return FALSE;
	}

	if (FileSize.QuadPart == 0)
	{
		CloseHandle (hFile);
		DbgPrintEx ("%s FileSize too small",FileName);
		return FALSE;
	}

	if(pul64FileSize)
	{
		*pul64FileSize =  FileSize.QuadPart;
	}

#ifndef SCAN_WHOLE_FILE
	if (FileSize.QuadPart > FileBufferSize)
	{
		CheckFileBufferSize.QuadPart = FileBufferSize;
	}
	else
#endif
	{
		CheckFileBufferSize.QuadPart = FileSize.QuadPart;
	}


	sha1_starts (&Sha1Context); 

	ByteOffset.QuadPart = 0;


	//HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,0,CheckFileBufferSize.QuadPart,NULL);
	HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,CheckFileBufferSize.HighPart,CheckFileBufferSize.LowPart,NULL);
	if(hFileMapOfView)
	{
		__try
		{
			CheckFileBuffer=(unsigned char *)MapViewOfFile(hFileMapOfView,FILE_MAP_READ,0,0,(SIZE_T)CheckFileBufferSize.QuadPart);
			if(CheckFileBuffer)
			{
				__try
				{




					sha1_update (&Sha1Context,CheckFileBuffer,(size_t)CheckFileBufferSize.QuadPart);


					BYTE FileHash[HASH_SIZE+1]={0};
					ZeroMemory(FileHash,sizeof(FileHash));
					//memset (CheckFileBuffer,0,(size_t)CheckFileBufferSize.QuadPart);
					sha1_finish (&Sha1Context,(PUCHAR)FileHash); 

					if (CommonHexDumpToBuffer (FileHash,HASH_SIZE / 2,szFileHash,ulFileHashLen) == FALSE)
					{
						UnmapViewOfFile(CheckFileBuffer);
						CloseHandle(hFileMapOfView);
						CloseHandle (hFile);

						return FALSE;
					}

				}
				__except(GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR ?
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
				{
					UnmapViewOfFile(CheckFileBuffer);
					CloseHandle(hFileMapOfView);
					CloseHandle (hFile);

					return FALSE;
				}

				UnmapViewOfFile(CheckFileBuffer);
			}
			else
			{
				CloseHandle(hFileMapOfView);
				CloseHandle (hFile);
				return FALSE;
			}

			CloseHandle(hFileMapOfView);
		}
		__except(GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR ?
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{

		}


	}
	else
	{
		dwErrorCode = GetLastError();
		GetErrorDesc(dwErrorCode,szErrorDescript,sizeof(szErrorDescript));
		CloseHandle (hFile);
		DbgPrintEx ("open  %s error,%s ",FileName,szErrorDescript);
		return FALSE;
	}

	CloseHandle (hFile);
	return TRUE;
}


#if 0
DWORD myStrlenA(char *ptr)
{
	DWORD len = 0;
	while(*ptr)
	{
		len++;
		ptr++;
	}
	return len;
}

BOOL myStrcmpA(char *str1, char *str2)
{
	while(*str1 && *str2)
	{
		if(*str1 == *str2)
		{
			str1++;
			str2++;
		}
		else
		{
			return FALSE;
		}
	}
	if(*str1 && !*str2)
	{
		return FALSE;
	}
	else if(*str2 && !*str1)
	{
		return FALSE;
	}
	return TRUE;       
}
//*******************************************************************************************************
// Fills the various structures with info of a PE image.  The PE image is located at modulePos.
//
//*******************************************************************************************************
bool RetrievePEInfo(char *modulePos, MZHeader *outMZ, PE_Header *outPE, PE_ExtHeader *outpeXH,
				SectionHeader **outSecHdr)
{
	// read MZ Header
	MZHeader *mzH;
	mzH = (MZHeader *)modulePos;
	if(mzH->signature != 0x5a4d)                // MZ
	{
		DbgPrintEx("File does not have MZ header\n");
		return false;
	}
	// read PE Header
	PE_Header *peH;
	peH = (PE_Header *)(modulePos + mzH->offsetToPE);
	if(peH->sizeOfOptionHeader != sizeof(PE_ExtHeader))
	{
		DbgPrintEx("Unexpected option header size.\n");

		return false;
	}
	// read PE Ext Header
	PE_ExtHeader *peXH;
	peXH = (PE_ExtHeader *)((char *)peH + sizeof(PE_Header));
	// read the sections
	SectionHeader *secHdr = (SectionHeader *)((char *)peXH + sizeof(PE_ExtHeader));
	*outMZ = *mzH;
	*outPE = *peH;
	*outpeXH = *peXH;
	*outSecHdr = secHdr;
	return true;
}

//*******************************************************************************************************
// Returns the total size required to load a PE image into memory
//
//*******************************************************************************************************
int calcTotalImageSize(MZHeader *inMZ, PE_Header *inPE, PE_ExtHeader *inpeXH,
					   SectionHeader *inSecHdr)
{
	int result = 0;
	int alignment = inpeXH->sectionAlignment;
	if(inpeXH->sizeOfHeaders % alignment == 0)
		result += inpeXH->sizeOfHeaders;
	else
	{
		int val = inpeXH->sizeOfHeaders / alignment;
		val++;
		result += (val * alignment);
	}
	for(int i = 0; i < inPE->numSections; i++)
	{
		if(inSecHdr[i].virtualSize)
		{
			if(inSecHdr[i].virtualSize % alignment == 0)
				result += inSecHdr[i].virtualSize;
			else
			{
				int val = inSecHdr[i].virtualSize / alignment;
				val++;
				result += (val * alignment);
			}
		}
	}
	return result;
}

//*******************************************************************************************************
// Returns the aligned size of a section
//
//*******************************************************************************************************
unsigned long getAlignedSize(unsigned long curSize, unsigned long alignment)
{       
	if(curSize % alignment == 0)
		return curSize;
	else
	{
		int val = curSize / alignment;
		val++;
		return (val * alignment);
	}
}
//*******************************************************************************************************
// Copy a PE image from exePtr to ptrLoc with proper memory alignment of all sections
//
//*******************************************************************************************************
bool loadPE(char *exePtr, MZHeader *inMZ, PE_Header *inPE, PE_ExtHeader *inpeXH,
			SectionHeader *inSecHdr, LPVOID ptrLoc)
{
	char *outPtr = (char *)ptrLoc;

	memcpy(outPtr, exePtr, inpeXH->sizeOfHeaders);
	outPtr += getAlignedSize(inpeXH->sizeOfHeaders, inpeXH->sectionAlignment);
	for(int i = 0; i < inPE->numSections; i++)
	{
		if(inSecHdr[i].sizeOfRawData > 0)
		{
			unsigned long toRead = inSecHdr[i].sizeOfRawData;
			if(toRead > inSecHdr[i].virtualSize)
				toRead = inSecHdr[i].virtualSize;
			memcpy(outPtr, exePtr + inSecHdr[i].pointerToRawData, toRead);
			outPtr += getAlignedSize(inSecHdr[i].virtualSize, inpeXH->sectionAlignment);
		}
	}
	return true;
}

//*******************************************************************************************************
// Loads the DLL into memory and align it
//
//*******************************************************************************************************
LPVOID LoadNtoskrnlDLL(char *dllName)
{
	char moduleFilename[MAX_PATH + 1];
	LPVOID ptrLoc = NULL;
	MZHeader mzH2;
	PE_Header peH2;
	PE_ExtHeader peXH2;
	SectionHeader *secHdr2;
	GetSystemDirectoryA(moduleFilename, MAX_PATH);
	if((myStrlenA(moduleFilename) + myStrlenA(dllName)) >= MAX_PATH)
		return NULL;
	strcat(moduleFilename, dllName);
	// load this EXE into memory because we need its original Import Hint Table
	HANDLE fp;
	fp = CreateFileA(moduleFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(fp != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION fileInfo;
		GetFileInformationByHandle(fp, &fileInfo);
		DWORD fileSize = fileInfo.nFileSizeLow;
		//DbgPrintEx("Size = %d\n", fileSize);
		if(fileSize)
		{
			LPVOID exePtr = HeapAlloc(GetProcessHeap(), 0, fileSize);
			if(exePtr)
			{
				DWORD read;
				if(ReadFile(fp, exePtr, fileSize, &read, NULL) && read == fileSize)
				{                                       
					if(RetrievePEInfo((char *)exePtr, &mzH2, &peH2, &peXH2, &secHdr2))
					{
						int imageSize = calcTotalImageSize(&mzH2, &peH2, &peXH2, secHdr2);                                               
						//ptrLoc = VirtualAlloc(NULL, imageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
						ptrLoc = HeapAlloc(GetProcessHeap(), 0, imageSize);
						if(ptrLoc)
						{                                                       
							loadPE((char *)exePtr, &mzH2, &peH2, &peXH2, secHdr2, ptrLoc);
						}
					}
				}
				HeapFree(GetProcessHeap(), 0, exePtr);
			}
		}
		CloseHandle(fp);
	}
	return ptrLoc;
}

DWORD RetrieveNativeApiExportAddress(DWORD hModule, char *apiName)
{       
	if(!hModule || !apiName)
		return 0;
	char *ptr = (char *)hModule;
	ptr += 0x3c;                // offset 0x3c contains offset to PE header

	ptr = (char *)(*(DWORD *)ptr) + hModule + 0x78;                // offset 78h into PE header contains addr of export table
	ptr = (char *)(*(DWORD *)ptr) + hModule;                        // ptr now points to export directory table
	// offset 24 into the export directory table == number of entries in the Export Name Pointer Table
	// table
	DWORD numEntries = *(DWORD *)(ptr + 24);
	//DbgPrintEx("NumEntries = %d\n", numEntries);
	DWORD *ExportNamePointerTable = (DWORD *)(*(DWORD *)(ptr + 32) + hModule);  // offset 32 into export directory contains offset to Export Name Pointer Table       

	DWORD ordinalBase = *((DWORD *)(ptr + 16));
	//DbgPrintEx("OrdinalBase is %d\n", ordinalBase);

	WORD *ExportOrdinalTable = (WORD *)((*(DWORD *)(ptr + 36)) + hModule);        // offset 36 into export directory contains offset to Ordinal Table
	DWORD *ExportAddrTable = (DWORD *)((*(DWORD *)(ptr + 28)) + hModule); // offset 28 into export directory contains offset to Export Addr Table
	for(DWORD i = 0; i < numEntries; i++)
	{               
		char *exportName = (char *)(ExportNamePointerTable[i] + hModule);
		if(myStrcmpA(exportName, apiName) == TRUE)
		{                       
			WORD ordinal = ExportOrdinalTable[i];
			//DbgPrintEx("%s (i = %d) Ordinal = %d at %X\n", exportName, i, ordinal, ExportAddrTable[ordinal]);
			return (DWORD)(ExportAddrTable[ordinal]);
		}               
	}
	return 0;
}
//*******************************************************************************************************
// -- END PE File support functions --
//
//*******************************************************************************************************

//*********************************************************************************************
// Builds a table of native API names using the export table of ntdll.dll
//
//*********************************************************************************************
BOOL buildNativeAPITable(DWORD hModule, char *nativeAPINames[], DWORD numNames)
{
	if(!hModule)
		return FALSE;
	char *ptr = (char *)hModule;
	ptr += 0x3c;                // offset 0x3c contains offset to PE header

	ptr = (char *)(*(DWORD *)ptr) + hModule + 0x78;                // offset 78h into PE header contains addr of export table
	ptr = (char *)(*(DWORD *)ptr) + hModule;                        // ptr now points to export directory table

	// offset 24 into the export directory table == number of entries in the Name Pointer Table
	// table
	DWORD numEntries = *(DWORD *)(ptr + 24);       

	DWORD *ExportNamePointerTable = (DWORD *)(*(DWORD *)(ptr + 32) + hModule);  // offset 32 into export directory contains offset to Export Name Pointer Table       
	DWORD ordinalBase = *((DWORD *)(ptr + 16));
	WORD *ExportOrdinalTable = (WORD *)((*(DWORD *)(ptr + 36)) + hModule);        // offset 36 into export directory contains offset to Ordinal Table
	DWORD *ExportAddrTable = (DWORD *)((*(DWORD *)(ptr + 28)) + hModule); // offset 28 into export directory contains offset to Export Addr Table

	for(DWORD i = 0; i < numEntries; i++)
	{               
		// i now contains the index of the API in the Ordinal Table
		// ptr points to Export directory table
		WORD ordinalValue = ExportOrdinalTable[i];               
		DWORD apiAddr = (DWORD)ExportAddrTable[ordinalValue] + hModule;
		char *exportName = (char *)(ExportNamePointerTable[i] + hModule);

		// Win2K
		if(gWinVersion == 0 &&
			*((unsigned char *)apiAddr) == 0xB8 &&
			*((unsigned char *)apiAddr + 9) == 0xCD &&
			*((unsigned char *)apiAddr + 10) == 0x2E)
		{
			DWORD serviceNum = *(DWORD *)((char *)apiAddr + 1);
			if(serviceNum < numNames)
			{
				nativeAPINames[serviceNum] = exportName;
			}
			//DbgPrintEx("%X - %s\n", serviceNum, exportName);
		}
		// WinXP
		else if(gWinVersion == 1 &&
			*((unsigned char *)apiAddr) == 0xB8 &&
			*((unsigned char *)apiAddr + 5) == 0xBA &&
			*((unsigned char *)apiAddr + 6) == 0x00 &&
			*((unsigned char *)apiAddr + 7) == 0x03 &&
			*((unsigned char *)apiAddr + 8) == 0xFE &&
			*((unsigned char *)apiAddr + 9) == 0x7F)
		{
			DWORD serviceNum = *(DWORD *)((char *)apiAddr + 1);
			if(serviceNum < numNames)
			{
				nativeAPINames[serviceNum] = exportName;
			}
			//DbgPrintEx("%X - %s\n", serviceNum, exportName);
		}
	}
	return TRUE;
}

//*******************************************************************************************************
// Gets address of native API's that we'll be using
//
//*******************************************************************************************************
BOOL GetNativeAPIsAddress(void)
{
	HMODULE hntdll;
	hntdll = GetModuleHandleA("ntdll.dll");

	*(FARPROC *)&_RtlAnsiStringToUnicodeString =
		GetProcAddress(hntdll, "RtlAnsiStringToUnicodeString");
	*(FARPROC *)&_RtlInitAnsiString =
		GetProcAddress(hntdll, "RtlInitAnsiString");
	*(FARPROC *)&_RtlFreeUnicodeString =
		GetProcAddress(hntdll, "RtlFreeUnicodeString");
	*(FARPROC *)&_NtOpenSection =
		GetProcAddress(hntdll, "NtOpenSection");
	*(FARPROC *)&_NtMapViewOfSection =
		GetProcAddress(hntdll, "NtMapViewOfSection");
	*(FARPROC *)&_NtUnmapViewOfSection =
		GetProcAddress(hntdll, "NtUnmapViewOfSection");
	*(FARPROC *)&_NtQuerySystemInformation =
		GetProcAddress(hntdll, "ZwQuerySystemInformation");
	if(_RtlAnsiStringToUnicodeString && _RtlInitAnsiString && _RtlFreeUnicodeString &&
		_NtOpenSection && _NtMapViewOfSection && _NtUnmapViewOfSection && _NtQuerySystemInformation)
	{
		return TRUE;
	}
	return FALSE;
}

//*******************************************************************************************************
// Obtain a handle to /device/physicalmemory
//
//*******************************************************************************************************
HANDLE OpenPhysicalMemory()
{
	HANDLE hPhyMem;
	OBJECT_ATTRIBUTES oAttr;
	ANSI_STRING aStr;

	_RtlInitAnsiString(&aStr, "\\device\\physicalmemory");

	UNICODE_STRING uStr;
	if(_RtlAnsiStringToUnicodeString(&uStr, &aStr, TRUE) != STATUS_SUCCESS)
	{               
		return INVALID_HANDLE_VALUE;       
	}
	oAttr.Length = sizeof(OBJECT_ATTRIBUTES);
	oAttr.RootDirectory = NULL;
	oAttr.Attributes = OBJ_CASE_INSENSITIVE;
	oAttr.ObjectName = &uStr;
	oAttr.SecurityDescriptor = NULL;
	oAttr.SecurityQualityOfService = NULL;
	if(_NtOpenSection(&hPhyMem, SECTION_MAP_READ | SECTION_MAP_WRITE, &oAttr ) != STATUS_SUCCESS)
	{               
		return INVALID_HANDLE_VALUE;
	}
	return hPhyMem;
}

//*******************************************************************************************************
// Map in a section of physical memory into this process's virtual address space.
//
//*******************************************************************************************************
BOOL MapPhysicalMemory(HANDLE hPhyMem, DWORD *phyAddr, DWORD *length, PVOID *virtualAddr)
{
	NTSTATUS                        ntStatus;
	PHYSICAL_ADDRESS        viewBase;
	*virtualAddr = 0;
	viewBase.QuadPart = (ULONGLONG) (*phyAddr);
	ntStatus = _NtMapViewOfSection(hPhyMem, (HANDLE)-1, virtualAddr, 0,
		*length, &viewBase, length,
		ViewShare, 0, PAGE_READWRITE );
	if(ntStatus != STATUS_SUCCESS)
	{
		DbgPrintEx("Failed to map physical memory view of length %X at %X!", *length, *phyAddr);
		return FALSE;                                       
	}
	*phyAddr = viewBase.LowPart;
	return TRUE;
}

//*******************************************************************************************************
// Unmap section of physical memory
//
//*******************************************************************************************************
void UnmapPhysicalMemory(DWORD virtualAddr)
{
	NTSTATUS status;
	status = _NtUnmapViewOfSection((HANDLE)-1, (PVOID)virtualAddr);
	if(status != STATUS_SUCCESS)
	{
		DbgPrintEx("Unmapping view failed!\n");
	}
}

//*******************************************************************************************************
// Assign SECTION_MAP_WRITE assess of /device/physicalmemory to current user.
//
//*******************************************************************************************************
BOOL OpenPhysicalMemoryAndSetAcl(void)
{
	HANDLE hPhyMem;
	OBJECT_ATTRIBUTES oAttr;
	BOOL result = FALSE;
	ANSI_STRING aStr;

	_RtlInitAnsiString(&aStr, "\\device\\physicalmemory");

	UNICODE_STRING uStr;
	if(_RtlAnsiStringToUnicodeString(&uStr, &aStr, TRUE) != STATUS_SUCCESS)
	{               
		return FALSE;
	}
	oAttr.Length = sizeof(OBJECT_ATTRIBUTES);
	oAttr.RootDirectory = NULL;
	oAttr.Attributes = OBJ_CASE_INSENSITIVE;
	oAttr.ObjectName = &uStr;
	oAttr.SecurityDescriptor = NULL;
	oAttr.SecurityQualityOfService = NULL;
	if(_NtOpenSection(&hPhyMem, READ_CONTROL | WRITE_DAC, &oAttr ) != STATUS_SUCCESS)
	{               
		return FALSE;
	}
	else
	{
		PACL dacl;
		PSECURITY_DESCRIPTOR sd;

		if(GetSecurityInfo(hPhyMem, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL,
			&dacl, NULL, &sd) == ERROR_SUCCESS)
		{
			EXPLICIT_ACCESSA ea;
			char userName[MAX_PATH];
			DWORD userNameSize = MAX_PATH-1;
			GetUserNameA(userName, &userNameSize);
			ea.grfAccessPermissions = SECTION_MAP_WRITE;
			ea.grfAccessMode = GRANT_ACCESS;
			ea.grfInheritance = NO_INHERITANCE;
			ea.Trustee.pMultipleTrustee = NULL;
			ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
			ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
			ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
			ea.Trustee.ptstrName = userName;
			PACL newDacl;
			if(SetEntriesInAclA(1, &ea, dacl, &newDacl) == ERROR_SUCCESS)
			{
				if(SetSecurityInfo(hPhyMem, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL,
					newDacl, NULL) == ERROR_SUCCESS)
				{               
					result = TRUE;
				}
				LocalFree(newDacl);
			}
		}
	}
	return result;       
}

//*******************************************************************************************************
// Gets the kernel base address
//
//*******************************************************************************************************
DWORD RetrieveKernelBaseAddress(void)
{
	HANDLE hHeap = GetProcessHeap();

	NTSTATUS Status;
	ULONG cbBuffer = 0x8000;
	PVOID pBuffer = NULL;
	DWORD retVal = DEF_KERNEL_BASE;
	do
	{
		pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
		if (pBuffer == NULL)
			return DEF_KERNEL_BASE;
		Status = _NtQuerySystemInformation(SystemModuleInformation,
			pBuffer, cbBuffer, NULL);
		if(Status == STATUS_INFO_LENGTH_MISMATCH)
		{
			HeapFree(hHeap, 0, pBuffer);
			cbBuffer *= 2;
		}
		else if(Status != STATUS_SUCCESS)
		{
			HeapFree(hHeap, 0, pBuffer);
			return DEF_KERNEL_BASE;
		}
	}
	while (Status == STATUS_INFO_LENGTH_MISMATCH);
	DWORD numEntries = *((DWORD *)pBuffer);
	SYSTEM_MODULE_INFORMATION *smi = (SYSTEM_MODULE_INFORMATION *)((char *)pBuffer + sizeof(DWORD));
	for(DWORD i = 0; i < numEntries; i++)
	{
		if(strcmpi(smi->ImageName, "ntoskrnl.exe"))
		{
			//DbgPrintEx("%.8X - %s\n", smi->Base, smi->ImageName);
			retVal = (DWORD)(smi->Base);
			break;
		}
		smi++;
	}
	HeapFree(hHeap, 0, pBuffer);
	return retVal;
}


typedef struct {
    DWORD    dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION    smi;
} MODULES, *PMODULES;
#define    SystemModuleInformation    11
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) == STATUS_SUCCESS)
#define RVATOVA(base,offset) ((PVOID)((DWORD)(base)+(DWORD)(offset)))
typedef struct {
    WORD    offset:12;
    WORD    type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;
#define ibaseDD *(PDWORD)&ibase
DWORD GetHeaders(PCHAR ibase,
                 PIMAGE_FILE_HEADER *pfh,
                 PIMAGE_OPTIONAL_HEADER *poh,
                 PIMAGE_SECTION_HEADER *psh)

{
    PIMAGE_DOS_HEADER mzhead=(PIMAGE_DOS_HEADER)ibase;

    if    ((mzhead->e_magic!=IMAGE_DOS_SIGNATURE) ||        
        (ibaseDD[mzhead->e_lfanew]!=IMAGE_NT_SIGNATURE))
        return FALSE;

    *pfh=(PIMAGE_FILE_HEADER)&ibase[mzhead->e_lfanew];
    if (((PIMAGE_NT_HEADERS)*pfh)->Signature!=IMAGE_NT_SIGNATURE)
        return FALSE;
    *pfh=(PIMAGE_FILE_HEADER)((PBYTE)*pfh+sizeof(IMAGE_NT_SIGNATURE));

    *poh=(PIMAGE_OPTIONAL_HEADER)((PBYTE)*pfh+sizeof(IMAGE_FILE_HEADER));
    if ((*poh)->Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return FALSE;

    *psh=(PIMAGE_SECTION_HEADER)((PBYTE)*poh+sizeof(IMAGE_OPTIONAL_HEADER));
    if(!*pfh ||!*pfh||!*poh )
        return FALSE;

 /*   if(!IsBadReadPtr(*pfh,sizeof(IMAGE_FILE_HEADER)) ||
        !IsBadReadPtr(*poh,sizeof(IMAGE_OPTIONAL_HEADER))||
        !IsBadReadPtr(*psh,sizeof(IMAGE_SECTION_HEADER)) )
        return FALSE;*/

    return TRUE;
}
DWORD FindKiServiceTable(HMODULE hModule,DWORD dwKSDT)
{
    PIMAGE_FILE_HEADER    pfh;
    PIMAGE_OPTIONAL_HEADER    poh;
    PIMAGE_SECTION_HEADER    psh;
    PIMAGE_BASE_RELOCATION    pbr;
    PIMAGE_FIXUP_ENTRY    pfe;    

    DWORD    dwFixups=0,i,dwPointerRva,dwPointsToRva,dwKiServiceTable;
    BOOL    bFirstChunk;

    if(!GetHeaders((PCHAR)hModule,&pfh,&poh,&psh))
        return FALSE;

    // loop thru relocs to speed up the search
    if ((poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) &&
        (!((pfh->Characteristics)&IMAGE_FILE_RELOCS_STRIPPED))) {

            pbr=(PIMAGE_BASE_RELOCATION)RVATOVA(poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,hModule);
            if(!pbr/* || !IsBadReadPtr(pbr,sizeof(IMAGE_BASE_RELOCATION))*/)
                return FALSE;
            bFirstChunk=TRUE;
            // 1st IMAGE_BASE_RELOCATION.VirtualAddress of ntoskrnl is 0
            while (bFirstChunk || pbr->VirtualAddress) {
                bFirstChunk=FALSE;

                pfe=(PIMAGE_FIXUP_ENTRY)((DWORD)pbr+sizeof(IMAGE_BASE_RELOCATION));
              /*  if( !IsBadReadPtr(pfe,sizeof(IMAGE_FIXUP_ENTRY)))
                    return FALSE;*/
                for (i=0;i<(pbr->SizeOfBlock-sizeof(IMAGE_BASE_RELOCATION))>>1;i++,pfe++) {
                    if (pfe->type==IMAGE_REL_BASED_HIGHLOW) {
                        dwFixups++;
                        dwPointerRva=pbr->VirtualAddress+pfe->offset;
                        // DONT_RESOLVE_DLL_REFERENCES flag means relocs aren't fixed
                        dwPointsToRva=*(PDWORD)((DWORD)hModule+dwPointerRva)-(DWORD)poh->ImageBase;

                        // does this reloc point to KeServiceDescriptorTable.Base?
                        if (dwPointsToRva==dwKSDT) {
                            // check for mov [mem32],imm32. we are trying to find
                            // "mov ds:_KeServiceDescriptorTable.Base, offset _KiServiceTable"
                            // from the KiInitSystem.
                            if (*(PWORD)((DWORD)hModule+dwPointerRva-2)==0x05c7) {
                                // should check for a reloc presence on KiServiceTable here
                                // but forget it
                                dwKiServiceTable=*(PDWORD)((DWORD)hModule+dwPointerRva+4)-poh->ImageBase;
                                return dwKiServiceTable;
                            }
                        }

                    } else
                        if (pfe->type!=IMAGE_REL_BASED_ABSOLUTE)
                            // should never get here
                            DbgPrint("\trelo type %d found at .%X\n",pfe->type,pbr->VirtualAddress+pfe->offset);
                }
                *(PDWORD)&pbr+=pbr->SizeOfBlock;
            }
    }    

    if (!dwFixups)
        // should never happen - nt, 2k, xp kernels have relocation data
        OutputDebugStringA("No fixups!\n");
    return 0;
}
//*******************************************************************************************************
// Program entry point
// No commandline arguments required.
//
//*******************************************************************************************************
int RestoreSSDT()
{

    HMODULE    hKernel;
    DWORD    dwKSDT;                // rva of KeServiceDescriptorTable
    DWORD    dwKiServiceTable;    // rva of KiServiceTable
    PMODULES    pModules=(PMODULES)&pModules;
    DWORD    dwNeededSize,rc;
    DWORD    dwKernelBase,dwServices=0;
    PCHAR    pKernelName;
 

	MZHeader mzH2;
	PE_Header peH2;
	PE_ExtHeader peXH2;
	SectionHeader *secHdr2;

	OSVERSIONINFO ov;
	ov.dwOSVersionInfoSize = sizeof(ov);
	GetVersionEx(&ov);
	if(ov.dwMajorVersion != 5)
	{
		DbgPrintEx("Sorry, this version supports only Win2K and WinXP.\n");
		return 1;
	}
	if(ov.dwMinorVersion != 0 && ov.dwMinorVersion != 1)
	{
		DbgPrintEx("Sorry, this version supports only Win2K and WinXP.\n");
		return 1;
	}

    HANDLE hToken = NULL;

    LUID luid ={0};

    TOKEN_PRIVILEGES tkp ={0};

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken) ) {

        if (LookupPrivilegeValueA(0, "SeDebugPrivilege", &luid)) {

            tkp.Privileges[0].Luid.LowPart = luid.LowPart;

            tkp.Privileges[0].Luid.HighPart = luid.HighPart;

            tkp.PrivilegeCount = 1;

            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0x10, NULL, 0);

        }

    }

	gWinVersion = ov.dwMinorVersion;
	if(!GetNativeAPIsAddress())
	{
		DbgPrintEx("Failed to get addresses of Native APIs!\n");
		return 1;
	}
	OpenPhysicalMemoryAndSetAcl();
	HANDLE hPhyMem = OpenPhysicalMemory();
	if(hPhyMem == INVALID_HANDLE_VALUE)
		OpenPhysicalMemoryAndSetAcl();
	hPhyMem = OpenPhysicalMemory();
	if(hPhyMem == INVALID_HANDLE_VALUE)
	{
		cout << GetLastError() << endl;
		DbgPrintEx("Could not open physical memory device!\nMake sure you are running as Administrator.\n");
		return 1;
	}

    // get system modules - ntoskrnl is always first there
    rc=_NtQuerySystemInformation(SystemModuleInformation,pModules,4,&dwNeededSize);
    if (rc==STATUS_INFO_LENGTH_MISMATCH) {
        pModules=(PMODULES)GlobalAlloc(GPTR,dwNeededSize);
        rc=_NtQuerySystemInformation(SystemModuleInformation,pModules,dwNeededSize,NULL);
    } else {
strange:
        printf("strange _NtQuerySystemInformation()!\n");
        return 1;
    }
    if (!NT_SUCCESS(rc)) goto strange;
    // imagebase
    dwKernelBase=(DWORD)pModules->smi.Base;
    // filename - it may be renamed in the boot.ini
    pKernelName=pModules->smi.ModuleNameOffset+pModules->smi.ImageName;

    // map ntoskrnl - hopefully it has relocs
    hKernel=LoadLibraryExA(pKernelName,0,DONT_RESOLVE_DLL_REFERENCES);
    if (!hKernel) {
        printf("Failed to load! LastError=%i\n",GetLastError());
        return 1;        
    }

    GlobalFree(pModules);

    // our own export walker is useless here - we have GetProcAddress :)    
    if (!(dwKSDT=(DWORD)GetProcAddress(hKernel,"KeServiceDescriptorTable"))) {
        printf("Can't find KeServiceDescriptorTable\n");
        return 1;
    }

    // get KeServiceDescriptorTable rva
    dwKSDT-=(DWORD)hKernel;    
    // find KiServiceTable
    if (!(dwKiServiceTable=FindKiServiceTable(hKernel,dwKSDT))) {
        printf("Can't find KiServiceTable...\n");
        return 1;
    }

	PVOID exeAddr = LoadNtoskrnlDLL("\\ntoskrnl.exe");
	if(!exeAddr)
	{
		DbgPrintEx("Failed to load ntoskrnl.exe!\n");
		return 1;
	}
	DWORD sdtAddr = RetrieveNativeApiExportAddress((DWORD)exeAddr, "KeServiceDescriptorTable");
	if(!sdtAddr)
	{
		DbgPrintEx("Failed to get address of KeServiceDescriptorTable!\n");
		return 1;
	}

	if(!RetrievePEInfo((char *)exeAddr, &mzH2, &peH2, &peXH2, &secHdr2))
	{
		DbgPrintEx("Failed to get PE header of ntoskrnl.exe!\n");
		return 1;
	}
	DWORD kernelPhyBase = RetrieveKernelBaseAddress() - PROT_MEMBASE;
	DWORD kernelOffset = kernelPhyBase - peXH2.imageBase;
	DbgPrintEx("KeServiceDescriptorTable\t\t%X\n", sdtAddr + kernelPhyBase + PROT_MEMBASE);
	unsigned char *ptr = NULL;
	DWORD pAddr = sdtAddr + kernelPhyBase;
	DWORD wantedAddr = pAddr;
	DWORD len = 0x2000;
	// map in page containing KeServiceDecriptorTable
	if(MapPhysicalMemory(hPhyMem, &pAddr, &len, (LPVOID *)&ptr))
	{
		DWORD start = wantedAddr - pAddr;
		DWORD serviceTableAddr, sdtCount;
		DWORD wantedBytes = len - start;
		if(wantedBytes >= 4)
		{
			serviceTableAddr = *((DWORD *)(&ptr[start]));
			DbgPrintEx("KeServiceDecriptorTable.ServiceTable\t%X\n", serviceTableAddr);
			if(wantedBytes >= 12)
			{
				sdtCount = *(((DWORD *)(&ptr[start])) + 2);
				DbgPrintEx("KeServiceDescriptorTable.ServiceLimit\t%d\n", sdtCount);
			}
		}
		else
		{
			DbgPrintEx("Sorry, an unexpected situation occurred!\n");
			return 1;
		}
		UnmapPhysicalMemory((DWORD)ptr);
		DbgPrintEx("\n");
		if(sdtCount >= 300)
		{
			DbgPrintEx("Sorry, an unexpected error occurred! SDT Count > 300???\n");
			return 1;
		}
		pAddr = serviceTableAddr - PROT_MEMBASE;
		wantedAddr = pAddr;
		ptr = NULL;
		len = 0x2000;
		if(MapPhysicalMemory(hPhyMem, &pAddr, &len, (LPVOID *)&ptr))
		{
			start = wantedAddr - pAddr;
			DWORD numEntries = (len - start) >> 2;
			if(numEntries >= sdtCount)
			{
				char **nativeApiNames = NULL;
				nativeApiNames = (char **)malloc(sizeof(char *) * sdtCount);
				if(!nativeApiNames)
				{
					DbgPrintEx("Failed to allocate memory for Native API name table.\n");
					return 1;
				}
				memset(nativeApiNames, 0, sizeof(char *) * sdtCount);
				PVOID ntdll = LoadNtoskrnlDLL("\\ntdll.dll");
				if(!ntdll)
				{
					DbgPrintEx("Failed to load ntdll.dll!\n");
					return 1;
				}
				buildNativeAPITable((DWORD)ntdll, nativeApiNames, sdtCount);
				DWORD *serviceTable = (DWORD *)(&ptr[start]);
				DWORD *fileServiceTable = (DWORD *)((DWORD)exeAddr + wantedAddr - kernelOffset - peXH2.imageBase);
				if(!IsBadReadPtr(fileServiceTable, sizeof(DWORD)) &&
					!IsBadReadPtr(&fileServiceTable[sdtCount-1], sizeof(DWORD)))
				{
					DWORD hookCount = 0;
					for(DWORD i = 0; i < sdtCount; i++)
					{                                                       
						if((serviceTable[i] - PROT_MEMBASE - kernelOffset) != fileServiceTable[i])
						{
							DbgPrintEx("%-25s %3X --[hooked by unknown at %X]--\n",
								(nativeApiNames[i] ? nativeApiNames[i] : "Unknown API"),
								i, serviceTable[i]);
							hookCount++;
						}

					}
					DbgPrintEx("\nNumber of Service Table entries hooked = %u\n", hookCount);

					if(hookCount)
					{
						DbgPrintEx("\nWARNING:  THIS IS EXPERIMENTAL CODE.  FIXING THE SDT MAY HAVE GRAVE\n"
							"CONSEQUENCES, SUCH AS SYSTEM CRASH, DATA LOSS OR SYSTEM CORRUPTION.\n"
							"PROCEED AT YOUR OWN RISK.  YOU HAVE BEEN WARNED.\n");
						DbgPrintEx("\nFix SDT Entries (Y/N)? : ");

						char inputReply[10];
						memset(inputReply, 0, sizeof(inputReply));
						fgets(inputReply, sizeof(inputReply) - 1, stdin);
						DbgPrintEx("\n");

						if(stricmp(inputReply, "y\n") == 0)
						{
							for(DWORD i = 0; i < sdtCount; i++)
							{
								if((serviceTable[i] - PROT_MEMBASE - kernelOffset) != fileServiceTable[i])
								{
									serviceTable[i] = fileServiceTable[i] + PROT_MEMBASE + kernelOffset;
									DbgPrintEx("[+] Patched SDT entry %.2X to %.8X\n", i,
										fileServiceTable[i] + PROT_MEMBASE + kernelOffset);
								}
							}
						}
						else
							DbgPrintEx("[-] SDT Entries NOT fixed.\n");
					}
				}
				else
				{
					DbgPrintEx("It's likely that the SDT service table has been relocated.\n"
						"This POC code cannot support patching of relocated SDT service table.\n");
				}
			}
			UnmapPhysicalMemory((DWORD)ptr);
		}
	}
	return 0;
}
#endif