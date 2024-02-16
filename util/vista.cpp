/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 #ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS

#include "common_func.h"
#include "vista.h"

BOOL WINAPI	IsVistaLater()
{
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);
	::GetVersionEx(&osInfo);

	return osInfo.dwMajorVersion >= 6;
}

BOOL WINAPI	IsOpenUAC()
{
	BOOL bResult = FALSE;

    if( IsVistaLater() )
    {
		LONG	lResult;
		HKEY	hKey;
		DWORD	dwEnableLUA;
		DWORD	dwType = REG_DWORD;
		DWORD	dwSize = sizeof( DWORD );

        lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\"),
			0,
			KEY_READ,
			&hKey
			);
        if( lResult == ERROR_SUCCESS )
        {
            lResult = RegQueryValueEx(hKey,
				_T("EnableLUA"),
				NULL,
				&dwType,
				(BYTE*)&dwEnableLUA,
				&dwSize
				);
			bResult = (lResult == ERROR_SUCCESS) && (0 != dwEnableLUA);

            RegCloseKey(hKey);
        }
    }

    return bResult;
}

BOOL EnablePrivilege(HANDLE hToken, LPCTSTR szPrivName)
{
	TOKEN_PRIVILEGES tkp;
	
	LookupPrivilegeValue(NULL, szPrivName, &tkp.Privileges[0].Luid);//�޸Ľ���Ȩ��
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);//֪ͨϵͳ�޸Ľ���Ȩ��

	return((GetLastError() == ERROR_SUCCESS));
}

typedef BOOL (WINAPI *PFN_ChangeWindowMessageFilter)(UINT uMessage, int nFlag);
BOOL WINAPI	Vista_ChangeWindowMessageFilter(UINT uMessage, int nFlag)
{
	HMODULE hUserMod;
	PFN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter;

	hUserMod = GetModuleHandle(_T("user32.dll"));
	pfnChangeWindowMessageFilter = (PFN_ChangeWindowMessageFilter)GetProcAddress(hUserMod, "ChangeWindowMessageFilter");

	return (pfnChangeWindowMessageFilter != NULL) && pfnChangeWindowMessageFilter(uMessage, nFlag);
}
