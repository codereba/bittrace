/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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

#include "common_func.h"
#include "system_event.h"
#include <strsafe.h>

LRESULT read_system_event()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE event_handle;
	EVENTLOGRECORD *ptr;
	BYTE buff[4096];
	DWORD read_len, next_len;
	ptr=(EVENTLOGRECORD *)&buff;

	do 
	{
		event_handle = OpenEventLogW( L"", L"System" ); // System

		while(ReadEventLog(event_handle,EVENTLOG_FORWARDS_READ|EVENTLOG_SEQUENTIAL_READ,
			1,ptr,sizeof(buff),&read_len,&next_len)) 
		{
		}

	}while( FALSE );

	if( event_handle != NULL )
	{
		CloseEventLog( event_handle );
	}

	return ret; 
}

BOOL add_event_source(LPCWSTR csName, DWORD dwCategoryCount)
{

	HKEY         hRegKey = NULL; 

	DWORD   dwError = 0;

	TCHAR     szPath[ MAX_PATH ];



	_stprintf( szPath, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), csName );

	dwError = RegCreateKey( HKEY_LOCAL_MACHINE, szPath, &hRegKey );

	if (dwError != 0)

	{
		return E_FAIL;
	}

	StringCbCopy(szPath,sizeof(szPath),_T("%SystemRoot%\\System32\\eventdll.DLL"));

	dwError = RegSetValueEx( hRegKey, _T("EventMessageFile"), 0, REG_EXPAND_SZ, (PBYTE) szPath, (_tcslen( szPath) + 1) * sizeof TCHAR ); 

	if (dwError == 0)

	{
		DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 

		dwError = RegSetValueEx( hRegKey, _T("TypesSupported"), 0, REG_DWORD, (LPBYTE) &dwTypes, sizeof dwTypes );

		if(dwError == 0 && dwCategoryCount > 0 ) 
		{
			dwError = RegSetValueEx( hRegKey, _T("CategoryMessageFile"), 0, REG_EXPAND_SZ, (PBYTE) szPath, (_tcslen( szPath) + 1) * sizeof TCHAR );

			if (dwError == 0)
				dwError = RegSetValueEx( hRegKey, _T("CategoryCount"), 0, REG_DWORD, (PBYTE) &dwCategoryCount, sizeof dwCategoryCount );
		}
	}

	RegCloseKey( hRegKey );

	return TRUE;
}

BOOL write_event_log(LPCWSTR szEventMsg, WORD wEventType, LPCWSTR szSourceName)  
{  
	HANDLE hEventLog;  
	BOOL bSuccesful;  
	BOOL bReturn = TRUE; 

	TCHAR szEventName[MAX_PATH] = {0};
	if(!szSourceName || !_tcsclen(szSourceName))
	{
		_tcscpy(szEventName,_T("SecurityTerminal"));
	}
	else
	{
		_tcscpy(szEventName,(LPTSTR)szSourceName);
	}

	add_event_source(szSourceName,1024);
	hEventLog = RegisterEventSource(NULL,szEventName);  
	if(NULL == hEventLog)  
	{  
		return false;  
	}  
	bSuccesful = ReportEvent(hEventLog,  
		wEventType,  
		0,  
		1024,  
		NULL,  
		1,  
		0,  
		&szEventMsg,  
		NULL);  

	if(false == bSuccesful)  
	{  
		bReturn = false;  
	}  

	DeregisterEventSource(hEventLog);  

	return bReturn;  

}  
