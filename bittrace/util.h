#ifndef _UTIL_H
#define _UTIL_H

BOOL IsUnicodeChineseSimpleStrng(WCHAR* wszSource);
BOOL WINAPI SendGuiInfoToService(__in PCHAR pbGUIDataBuffer,__in ULONG dwGUIDataBufferLen,__out PBYTE * ppOutBuffer,__out PDWORD pdwOutputLen,__in BOOL bShowUi = FALSE);
BOOL WINAPI RealSendGuiInfoToService(__in PCHAR pbGUIDataBuffer,__in ULONG dwGUIDataBufferLen,__out PBYTE * ppOutBuffer,__out PDWORD pdwOutputLen,__in BOOL bShowUi  = TRUE);
BOOL ShowNotifyInfo(HWND hWnd,const CString& strTitle,const CString& strContent);
BOOL WINAPI CheckPE (IN TCHAR*  FileName);
BOOL IsPeFile(__in PUCHAR pvBuffer,__in ULONG ulBufferLen,__out BOOL* pbIsExe);
COLORREF DarColor();
BOOL StartRemoteDebug();
ULONG CalcPages(ULONG ulRecordCount,ULONG ulCountPerPage);
COLORREF DarkenColor(COLORREF col,double factor);

BOOL SaveBlobData(__in LPTSTR lpstTableName,__in LPTSTR lpsttLastModified,__in LPVOID pvData,__in ULONG ulDataLen,__in int role=0,__in int domain=0);
BOOL LoadBlobData(__in IN LPTSTR lpstTableName,__in LPTSTR lpsttLastModified,__out LPVOID *ppvData,__out ULONG* pulDataLen,__in int role=0,__in int domain=0);
BOOL ClearBlobData();
void GetAppLastErrorDesc(TCHAR * szErrorDesc,int nErrorBufferSize);
BOOL RemoteExecCmd(TCHAR* tszCmdLine,TCHAR* szResultFile);
DWORD __stdcall PipeService(VOID);
BOOL IsWow64Process();
BOOL DisableWow64(LPVOID* dwOldValue);
BOOL Wow64Revert(LPVOID dwOldValue);
BOOL WINAPI ConnectSOC();
BOOL ExportRemoteFile(TCHAR * tszLocalFileName,TCHAR* tszRemoteFileName);
BOOL GetRemoteData(TCHAR* tszRemoteFileName,PVOID * ppRemoteData,PULONG pulRemoteDataLen);
BOOL UploadRemoteFile(TCHAR* tszLocalFile);
BOOL DeleteRemoteFile(TCHAR* tszRemoteFile);
BOOL GetRemoteInstallPath(TCHAR * szInstallPath,ULONG ulInstallPathLen);
BOOL GetWindowVersion(TCHAR* str, int bufferSize,OSVERSIONINFOEXA &osvi);
typedef struct tagUserDefineString
{
	PVOID pbStartBuffer;
	ULONG  ulBufferLen;
}UserDefineString,*PUserDefineString;
BOOL WINAPI GetFileSizeHash (IN PCHAR FileName, OUT PCHAR szFileHash,IN ULONG ulFileHashLen,OUT ULONG64* pul64FileSize);
#if 0
int RestoreSSDT();
#endif
#endif