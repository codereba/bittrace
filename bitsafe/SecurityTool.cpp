/*
 *
 * Copyright 2010 JiJie Shi
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#include "stdafx.h"
#include "GetLoginUser.h"
#include "SecurityTool.h"

#include <Tlhelp32.h>
#include <Wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

//延迟加载Dll
#pragma comment(lib, "Delayimp.lib")
#pragma comment(linker, "/DELAYLOAD:\"Wtsapi32.dll\"")


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSecurityTool::CSecurityTool()
{

}

CSecurityTool::~CSecurityTool()
{

}

//author: yy2better
//email: yy2better@126.com
//得到当前进程用户
BOOL CSecurityTool::GetCurrProcessUser(CString& strName)
{	
	BOOL bRet(TRUE);
	strName = _T("");

	DWORD dwSize = MAX_PATH;
	TCHAR *pszName = new TCHAR[dwSize];
	if (!GetUserName(pszName, &dwSize))
	{
		//if (ERROR_MORE_DATA == GetLastError())	错误码不是这个，MSDN说明有误
		delete[] pszName;
		pszName = new TCHAR[dwSize];
		bRet = GetUserName(pszName, &dwSize);
	}
	
	strName = pszName;
	delete[] pszName;
		
	return bRet;
}


//获取XP登陆用户
BOOL CSecurityTool::GetLogUserXP(CString& strName)
{
	BOOL bRet = FALSE;
	strName = _T("");

	//for xp or above
	TCHAR *szLogName = NULL;
	DWORD dwSize = 0;
	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSUserName, &szLogName, &dwSize))
	{			
		strName = szLogName;
		WTSFreeMemory(szLogName);
		bRet = TRUE;
	}

	return bRet;
}


//获取win2000登陆用户
BOOL CSecurityTool::GetLogUser2K(CString& strName)
{
	BOOL bRet = FALSE;
	HANDLE hSnapshot = NULL;
	strName = _T("");

    __try
	{
		// Get a snapshot of the processes in the system
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == NULL)
		{            
			__leave;
		}

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(pe32);

        // Find the "System" process
        BOOL fProcess = Process32First(hSnapshot, &pe32);
        while (fProcess)
		{
			if (lstrcmpi(pe32.szExeFile, TEXT("explorer.exe")) == 0)
			{	
				TCHAR szUserName[MAX_PATH];
				if (GetProcessUser(pe32.th32ProcessID, szUserName, MAX_PATH))
				{
					bRet = TRUE;
					strName = szUserName;
				}
				
				break;
			}
			fProcess = Process32Next(hSnapshot, &pe32);
		}
        if (!fProcess)
		{			
            __leave;    // Didn't find "System" process
		}
	}
    __finally
	{
		// Cleanup the snapshot
       if (hSnapshot != NULL)
		   CloseHandle(hSnapshot);
    }

	return bRet;	
}

//author: yy2better
//email: yy2better@126.com
//获取进程的用户名
BOOL CSecurityTool::GetProcessUser(DWORD dwProcessID, TCHAR *szUserName, DWORD nNameLen)
{
	BOOL fResult  = FALSE;
    HANDLE hProc  = NULL;
	HANDLE hToken = NULL;
	TOKEN_USER *pTokenUser = NULL;
	
	__try
	{
        // Open the process with PROCESS_QUERY_INFORMATION access
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
        if (hProc == NULL)
		{
			__leave;
		}
        fResult = OpenProcessToken(hProc, TOKEN_QUERY, &hToken);
        if(!fResult)  
		{
			__leave;
		}
		
		DWORD dwNeedLen = 0;		
		fResult = GetTokenInformation(hToken,TokenUser, NULL, 0, &dwNeedLen);
		if (dwNeedLen > 0)
		{
			pTokenUser = (TOKEN_USER*)new BYTE[dwNeedLen];
			fResult = GetTokenInformation(hToken,TokenUser, pTokenUser, dwNeedLen, &dwNeedLen);
			if (!fResult)
			{
				__leave;
			}
		}
		else
		{
			__leave;
		}

		SID_NAME_USE sn;
		TCHAR szDomainName[MAX_PATH];
		DWORD dwDmLen = MAX_PATH;
		fResult = LookupAccountSid(NULL, pTokenUser->User.Sid, szUserName, &nNameLen,
			szDomainName, &dwDmLen, &sn);
	}
	__finally
	{
		if (hProc)
			::CloseHandle(hProc);
		if (hToken)
			::CloseHandle(hToken);
		if (pTokenUser)
			delete[] (char*)pTokenUser;

		return fResult;
	}
}