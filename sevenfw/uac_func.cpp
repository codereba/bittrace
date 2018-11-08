#include "common_func.h"
#include "uac_func.h"

/****************************** Module Header ******************************\
* Module Name:	CppUACSelfElevation.cpp
* Project:		CppUACSelfElevation
* Copyright (c) Microsoft Corporation.
* 
* User Account Control (UAC) is a new security component in Windows Vista and 
* newer operating systems. With UAC fully enabled, interactive administrators 
* normally run with least user privileges. This example demonstrates how to 
* check the privilege level of the current process, and how to self-elevate 
* the process by giving explicit consent with the Consent UI. 
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes and Manifest Dependencies
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include "Resource.h"

// Enable Visual Style
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion


#pragma region "Helper Functions for Admin Privileges and Elevation Status"

//
//   FUNCTION: IsUserInAdminGroup()
//
//   PURPOSE: The function checks whether the primary access token of the 
//   process belongs to user account that is a member of the local 
//   Administrators group, even if it currently is not elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsUserInAdminGroup())
//             wprintf (L"User is a member of the Administrators group\n");
//         else
//             wprintf (L"User is not a member of the Administrators group\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsUserInAdminGroup failed w/err %lu\n", dwError);
//     }
//

#pragma comment( lib, "shlwapi.lib" )

typedef enum _TOKEN_ELEVATION_TYPE {
	TokenElevationTypeDefault = 1,
	TokenElevationTypeFull,
	TokenElevationTypeLimited,
} TOKEN_ELEVATION_TYPE, *PTOKEN_ELEVATION_TYPE;

//
// Token Information Classes.

//typedef enum _TOKEN_INFORMATION_CLASS {
//	TokenUser = 1,
//	TokenGroups,
//	TokenPrivileges,
//	TokenOwner,
//	TokenPrimaryGroup,
//	TokenDefaultDacl,
//	TokenSource,
//	TokenType,
//	TokenImpersonationLevel,
//	TokenStatistics,
//	TokenRestrictedSids,
//	TokenSessionId,
//	TokenGroupsAndPrivileges,
//	TokenSessionReference,
//	TokenSandBoxInert,
//	TokenAuditPolicy,
//	TokenOrigin,
//	TokenElevationType,
//	TokenLinkedToken,
//	TokenElevation,
//	TokenHasRestrictions,
//	TokenAccessInformation,
//	TokenVirtualizationAllowed,
//	TokenVirtualizationEnabled,
//	TokenIntegrityLevel,
//	TokenUIAccess,
//	TokenMandatoryPolicy,
//	TokenLogonSid,
//	MaxTokenInfoClass  // MaxTokenInfoClass should always be the last enum
//} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;
#include "pshpack4.h" 
#define TokenElevationType 18
#define TokenLinkedToken 19
#define TokenIntegrityLevel 25

typedef struct _TOKEN_ELEVATION {
	ULONG TokenIsElevated;
} TOKEN_ELEVATION, *PTOKEN_ELEVATION;

typedef struct _TOKEN_MANDATORY_LABEL {
	SID_AND_ATTRIBUTES Label;
} TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;

#include "poppack.h" 

BOOL IsUserInAdminGroup()
{
    BOOL fInAdminGroup = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hTokenToCheck = NULL;
    DWORD cbSize = 0; 
	OSVERSIONINFO osver = { sizeof(osver) };

    // Open the primary access token of the process for query and duplicate.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, 
        &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether system is running Windows Vista or later operating 
    // systems (major version >= 6) because they support linked tokens, but 
    // previous versions (major version < 6) do not.
    
	if( !GetVersionEx( &osver ) )
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    if (osver.dwMajorVersion >= 6)
    {
        // Running Windows Vista or later (major version >= 6). 
        // Determine token type: limited, elevated, or default. 
        TOKEN_ELEVATION_TYPE elevType;
        if (!GetTokenInformation(hToken, ( TOKEN_INFORMATION_CLASS )TokenElevationType, &elevType, 
            sizeof(elevType), &cbSize))
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        // If limited, get the linked elevated token for further check.
        if (TokenElevationTypeLimited == elevType)
        {
            if (!GetTokenInformation(hToken, ( TOKEN_INFORMATION_CLASS )TokenLinkedToken, &hTokenToCheck, 
                sizeof(hTokenToCheck), &cbSize))
            {
                dwError = GetLastError();
                goto Cleanup;
            }
        }
    }
    
    // CheckTokenMembership requires an impersonation token. If we just got a 
    // linked token, it already is an impersonation token.  If we did not get 
    // a linked token, duplicate the original into an impersonation token for 
    // CheckTokenMembership.
    if (!hTokenToCheck)
    {
        if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck))
        {
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Create the SID corresponding to the Administrators group.
    BYTE adminSID[SECURITY_MAX_SID_SIZE];
    cbSize = sizeof(adminSID);
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID,  
        &cbSize))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Check if the token to be checked contains admin SID.
    // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
    // To determine whether a SID is enabled in a token, that is, whether it 
    // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
    if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) 
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (hTokenToCheck)
    {
        CloseHandle(hTokenToCheck);
        hTokenToCheck = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fInAdminGroup;
}


// 
//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as 
//   administrator. In other words, it dictates whether the primary access 
//   token of the process belongs to user account that is a member of the 
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID, 
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 0, 0, 0, 0, 0, 
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}


//
//   FUNCTION: IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current 
//   process. It dictates whether the process is elevated or not. Token 
//   elevation is only available on Windows Vista and newer operating 
//   systems, thus IsProcessElevated throws a C++ exception if it is called 
//   on systems prior to Windows Vista. It is not appropriate to use this 
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if 
//   it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
//   which contains the Win32 error code of the failure. For example, if 
//   IsProcessElevated is called on systems prior to Windows Vista, the error 
//   code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the 
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from 
//   TokenElevation in that, when UAC is turned off, elevation type always 
//   returns TokenElevationTypeDefault even though the process is elevated 
//   (Integrity Level == High). In other words, it is not safe to say if the 
//   process is elevated based on elevation type. Instead, we should use 
//   TokenElevation.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
//

#define TokenElevation 20
BOOL IsProcessElevated()
{
    BOOL fIsElevated = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        dwError = GetLastError();
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get token elevation information failed\n" ) ); 
        goto Cleanup;
    }

    // Retrieve token elevation information.
    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    if (!GetTokenInformation(hToken, ( TOKEN_INFORMATION_CLASS )TokenElevation, &elevation, 
        sizeof(elevation), &dwSize))
    {
        // When the process is run on operating systems prior to Windows 
        // Vista, GetTokenInformation returns FALSE with the 
        // ERROR_INVALID_PARAMETER error code because TokenElevation is 
        // not supported on those operating systems.
        dwError = GetLastError();

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "get token elevation information failed\n" ) ); 
        goto Cleanup;
    }

    fIsElevated = elevation.TokenIsElevated;

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    return fIsElevated;
}


//
//   FUNCTION: GetProcessIntegrityLevel()
//
//   PURPOSE: The function gets the integrity level of the current process. 
//   Integrity level is only available on Windows Vista and newer operating 
//   systems, thus GetProcessIntegrityLevel throws a C++ exception if it is 
//   called on systems prior to Windows Vista.
//
//   RETURN VALUE: Returns the integrity level of the current process. It is 
//   usually one of these values:
//
//     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
//     Means untrusted level. It is used by processes started by the 
//     Anonymous group. Blocks most write access. 
//
//     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
//     Means low integrity level. It is used by Protected Mode Internet 
//     Explorer. Blocks write acess to most objects (such as files and 
//     registry keys) on the system. 
//
//     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
//     Means medium integrity level. It is used by normal applications 
//     being launched while UAC is enabled. 
//
//     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
//     Means high integrity level. It is used by administrative applications 
//     launched through elevation when UAC is enabled, or normal 
//     applications if UAC is disabled and the user is an administrator. 
//
//     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
//     Means system integrity level. It is used by services and other 
//     system-level applications (such as Wininit, Winlogon, Smss, etc.)  
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
//   which contains the Win32 error code of the failure. For example, if 
//   GetProcessIntegrityLevel is called on systems prior to Windows Vista, 
//   the error code will be ERROR_INVALID_PARAMETER.
//
//   EXAMPLE CALL:
//     try 
//     {
//         DWORD dwIntegrityLevel = GetProcessIntegrityLevel();
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"GetProcessIntegrityLevel failed w/err %lu\n", dwError);
//     }
//
DWORD GetProcessIntegrityLevel()
{
    DWORD dwIntegrityLevel = 0;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD cbTokenIL = 0;
    PTOKEN_MANDATORY_LABEL pTokenIL = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Query the size of the token integrity level information. Note that 
    // we expect a FALSE result and the last error ERROR_INSUFFICIENT_BUFFER
    // from GetTokenInformation because we have given it a NULL buffer. On 
    // exit cbTokenIL will tell the size of the integrity level information.
    if (!GetTokenInformation(hToken, ( TOKEN_INFORMATION_CLASS )TokenIntegrityLevel, NULL, 0, &cbTokenIL))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            // When the process is run on operating systems prior to Windows 
            // Vista, GetTokenInformation returns FALSE with the 
            // ERROR_INVALID_PARAMETER error code because TokenElevation 
            // is not supported on those operating systems.
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL *)LocalAlloc(LPTR, cbTokenIL);
    if (pTokenIL == NULL)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token integrity level information.
    if (!GetTokenInformation(hToken, ( TOKEN_INFORMATION_CLASS )TokenIntegrityLevel, pTokenIL, 
        cbTokenIL, &cbTokenIL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g. 
    // S-1-16-0x1000 stands for low integrity level SID). There is one and 
    // only one subauthority.
    dwIntegrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (pTokenIL)
    {
        LocalFree(pTokenIL);
        pTokenIL = NULL;
        cbTokenIL = 0;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return dwIntegrityLevel;
}


#include <windows.h>
#include <tchar.h>
#include <iostream>
using namespace std;

LRESULT get_user_name_from_token( HANDLE dest_token, 
								 LPTSTR user_name, 
								 ULONG max_user_name_len, 
								 LPTSTR domain_name, 
								 ULONG max_domain_name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	DWORD user_name_size = 0; 
	DWORD domain_name_size = 0; 
	PTOKEN_USER user_info = NULL;
	SID_NAME_USE name_use;

	ASSERT( dest_token != NULL ); 

	_ret = GetTokenInformation( dest_token, TokenUser, NULL, 0, &user_name_size );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		if( ret != ERROR_INSUFFICIENT_BUFFER )
		{
			goto _return; 
		}

		ret = ERROR_SUCCESS; 
	}

	user_info = ( PTOKEN_USER )GlobalAlloc( GPTR, user_name_size );
	if( user_info == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_ret = GetTokenInformation( dest_token, TokenUser, user_info, user_name_size, &user_name_size );
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	user_name_size = max_user_name_len * sizeof( TCHAR );
	domain_name_size = max_domain_name_len * sizeof( TCHAR ); 

	_ret = LookupAccountSid( NULL, user_info->User.Sid, 
		user_name, 
		&user_name_size, 
		domain_name, 
		&domain_name_size, 
		&name_use ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}


_return:
	if( user_info != NULL )
	{
		GlobalFree( user_info ); 
	}

	return ret;
}

#include <Wtsapi32.h>
#pragma comment( lib, "Wtsapi32.lib" )

//ÑÓ³Ù¼ÓÔØDll
#pragma comment( lib, "Delayimp.lib" )
#pragma comment( linker, "/DELAYLOAD:\"Wtsapi32.dll\"" )

LRESULT get_logged_on_user_name( LPTSTR user_name, ULONG max_user_name_len )
{
	LRESULT ret = ERROR_SUCCESS;
	TCHAR *szLogName = NULL;
	DWORD dwSize = 0; 

#ifdef DEBUG_NAME_CORRECT
	memcpy( user_name, L"user", sizeof( L"user" ) ); 
	goto _return; 
#endif //DEBUG_NAME_CORRECT

	*user_name = L'\0'; 

	if( WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSUserName, &szLogName, &dwSize))
	{			
		if( wcslen( szLogName ) + 1 > max_user_name_len )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			goto _return; 
		}

		wcscpy( user_name, szLogName ); 
	}
	else
	{	
		ret = GetLastError(); 
	}

_return:
	if( szLogName != NULL )
	{
		WTSFreeMemory( szLogName ); 
	}

	return ret;
}

LRESULT get_current_user_name( LPTSTR user_name, 
							  ULONG max_user_name_len, 
							  LPTSTR domain_name, 
							  ULONG max_domain_name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	//TOKEN_ELEVATION elevated; 
	BOOL fInAdminGroup = FALSE;
	HANDLE hToken = NULL;
	//HANDLE hTokenToCheck = NULL;
	DWORD buf_size = 0;
	DWORD domain_name_size = 0; 
	//OSVERSIONINFO osver; 

	//DBG_BP(); 

	ASSERT( user_name != NULL ); 
	ASSERT( domain_name != NULL ); 

	*domain_name = L'\0'; 
	*user_name = L'\0'; 

	_ret = OpenProcessToken( GetCurrentProcess(), 
		TOKEN_QUERY | TOKEN_DUPLICATE, 
		&hToken ); 

	if( _ret == FALSE )
	{
		ret = GetLastError();
		goto _return; 
	}

#ifdef DEBUG_NAME_CORRECT
	memcpy( user_name, L"administrator", sizeof( L"administrator" ) ); 
	goto _return; 
#endif //DEBUG_NAME_CORRECT

	ret = get_user_name_from_token( hToken, 
		user_name, 
		max_user_name_len, 
		domain_name, 
		max_domain_name_len ); 

#ifdef CHECK_LINK_HANDLE
	osver.dwOSVersionInfoSize = sizeof( osver ); 

	_ret = GetVersionEx( &osver ); 
	if( _ret == FALSE )
	{
		ret = GetLastError();
		goto _return;
	}

	if( osver.dwMajorVersion >= 6 )
	{
		// Running Windows Vista or later (major version >= 6). 
		// Determine token type: limited, elevated, or default. 
		TOKEN_ELEVATION_TYPE elevType;

		_ret = GetTokenInformation( hToken, 
			TokenElevationType, 
			&elevType, 
			sizeof( elevType ), 
			&buf_size ); 

		_ret = GetTokenInformation( hToken, 
			TokenElevation, 
			&elevated, 
			sizeof( elevated ), 
			&buf_size ); 

		if( _ret == FALSE )
		{
			ret = GetLastError();
			goto _return;
		}

		// If limited, get the linked elevated token for further check.
		//if( TokenElevationTypeLimited == elevType )
		if( elevated.TokenIsElevated == TRUE )
		{
			_ret = GetTokenInformation( hToken, 
				TokenLinkedToken, 
				&hTokenToCheck, 
				sizeof( hTokenToCheck ), 
				&buf_size ); 

			if( _ret == FALSE )
			{
				ret = GetLastError();
				// goto _return;
			}
		}
	}

	if( NULL == hTokenToCheck )
	{
		_ret = DuplicateToken( hToken, SecurityIdentification, &hTokenToCheck ); 
		if( _ret == FALSE )
		{
			ret = GetLastError();
			goto _return;
		}
	}

	ret = get_user_name_from_token( hTokenToCheck, 
		user_name, 
		max_user_name_len,  
		domain_name, 
		max_domain_name_len ); 

#endif //CHECK_LINK_HANDLE

_return:
	if( hToken != NULL )
	{
		CloseHandle( hToken );
		hToken = NULL;
	}

	//if( hTokenToCheck != NULL )
	//{
	//    CloseHandle( hTokenToCheck );
	//    hTokenToCheck = NULL; 
	//}

	//if( ERROR_SUCCESS != ret )
	//{
	//    throw dwError;
	//}

	return ret;
}

#include <Shlwapi.h>

LRESULT correct_user_path( LPWSTR file_name, ULONG buf_len )
{
	LRESULT ret; 
	WCHAR cur_user_name[ MAX_PATH ]; 
	ULONG cur_user_name_len; 
	WCHAR logged_on_user_name[ MAX_PATH ]; 
	ULONG logged_on_user_name_len; 
	WCHAR domain_name[ MAX_PATH ]; 
	LPWSTR user_name_begin; 
	LPWSTR file_name_end; 

	ret = get_logged_on_user_name( logged_on_user_name, MAX_PATH ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( *logged_on_user_name == L'\0' ); 
		ret = ERROR_SUCCESS; 
		goto _return; 
	}

	ret = get_current_user_name( cur_user_name, 
		MAX_PATH, 
		domain_name, 
		MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}


	cur_user_name_len = ( ULONG )wcslen( cur_user_name ); 
	logged_on_user_name_len = ( ULONG )wcslen( logged_on_user_name ); 

	if( wcscmp( logged_on_user_name, cur_user_name ) != 0 )
	{
		ULONG name_remain_len; 

		file_name_end = file_name + ( ULONG )wcslen( file_name ); 

		user_name_begin = StrRStrIW( file_name, file_name_end, cur_user_name ); 
		if( user_name_begin == NULL )
		{
			goto _return; 
		}

		name_remain_len = ( user_name_begin - file_name ) 
			+ ( ( file_name_end - user_name_begin ) - ( ULONG )wcslen( cur_user_name ) ); 

		if( name_remain_len + logged_on_user_name_len >= buf_len )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			goto _return; 
		}

		memmove( user_name_begin + logged_on_user_name_len, 
			user_name_begin + cur_user_name_len,  
			( ( file_name_end - user_name_begin ) - wcslen( cur_user_name ) + 1 ) * sizeof( WCHAR ) ); 

		memcpy( user_name_begin, logged_on_user_name, ( logged_on_user_name_len ) * sizeof( WCHAR ) ); 
	}

_return:
	return ret; 
}

LRESULT _get_user_path( LPWSTR file_name, ULONG buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 

	ret = get_user_path( file_name, buf_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = correct_user_path( file_name, buf_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	return ret; 
}

#ifdef UAC_TEST_APP
#pragma endregion


//
//   FUNCTION: ReportError(LPWSTR, DWORD)
//
//   PURPOSE: Display an error dialog for the failure of a certain function.
//
//   PARAMETERS:
//   * pszFunction - the name of the function that failed.
//   * dwError - the Win32 error code. Its default value is the calling 
//   thread's last-error code value.
//
//   NOTE: The failing function must be immediately followed by the call of 
//   ReportError if you do not explicitly specify the dwError parameter of 
//   ReportError. This is to ensure that the calling thread's last-error code 
//   value is not overwritten by any calls of API between the failing 
//   function and ReportError.
//
void ReportError(LPCWSTR pszFunction, DWORD dwError = GetLastError())
{
    wchar_t szMessage[200];
    if (-1 != swprintf_s(szMessage, ARRAYSIZE(szMessage), 
        L"%s failed w/err 0x%08lx", pszFunction, dwError))
    {
        MessageBox(NULL, szMessage, L"Error", MB_ICONERROR);
    }
}


// 
//   FUNCTION: OnInitDialog(HWND, HWND, LPARAM)
//
//   PURPOSE: Process the WM_INITDIALOG message. Check and display the 
//   "run as administrator" status, the elevation information, and the 
//   integrity level of the current process.
//
BOOL OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
{
    // Get and display whether the primary access token of the process 
    // belongs to user account that is a member of the local Administrators 
    // group even if it currently is not elevated (IsUserInAdminGroup).
    HWND hInAdminGroupLabel = GetDlgItem(hWnd, IDC_INADMINGROUP_STATIC);
    try
    {
        BOOL const fInAdminGroup = IsUserInAdminGroup();
        SetWindowText(hInAdminGroupLabel, fInAdminGroup ? L"True" : L"False");
    }
    catch (DWORD dwError)
    {
        SetWindowText(hInAdminGroupLabel, L"N/A");
        ReportError(L"IsUserInAdminGroup", dwError);
    }

    // Get and display whether the process is run as administrator or not 
    // (IsRunAsAdmin).
    HWND hIsRunAsAdminLabel = GetDlgItem(hWnd, IDC_ISRUNASADMIN_STATIC);
    try
    {
        BOOL const fIsRunAsAdmin = IsRunAsAdmin();
        SetWindowText(hIsRunAsAdminLabel, fIsRunAsAdmin ? L"True" : L"False");
    }
    catch (DWORD dwError)
    {
        SetWindowText(hIsRunAsAdminLabel, L"N/A");
        ReportError(L"IsRunAsAdmin", dwError);
    }
    
    // Get and display the process elevation information (IsProcessElevated) 
    // and integrity level (GetProcessIntegrityLevel). The information is not 
    // available on operating systems prior to Windows Vista.

    HWND hIsElevatedLabel = GetDlgItem(hWnd, IDC_ISELEVATED_STATIC);
    HWND hILLabel = GetDlgItem(hWnd, IDC_IL_STATIC);

    OSVERSIONINFO osver = { sizeof(osver) };
    if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
    {
        // Running Windows Vista or later (major version >= 6).

        try
        {
            // Get and display the process elevation information.
            BOOL const fIsElevated = IsProcessElevated();
            SetWindowText(hIsElevatedLabel, fIsElevated ? L"True" : L"False");

            // Update the Self-elevate button to show the UAC shield icon on 
            // the UI if the process is not elevated. The 
            // Button_SetElevationRequiredState macro (declared in Commctrl.h) 
            // is used to show or hide the shield icon in a button. You can 
            // also get the shield directly as an icon by calling 
            // SHGetStockIconInfo with SIID_SHIELD as the parameter.
            HWND hElevateBtn = GetDlgItem(hWnd, IDC_ELEVATE_BN);
            Button_SetElevationRequiredState(hElevateBtn, !fIsElevated);
        }
        catch (DWORD dwError)
        {
            SetWindowText(hIsElevatedLabel, L"N/A");
            ReportError(L"IsProcessElevated", dwError);
        }

        try
        {
            // Get and display the process integrity level.
            DWORD const dwIntegrityLevel = GetProcessIntegrityLevel();
            switch (dwIntegrityLevel)
            {
            case SECURITY_MANDATORY_UNTRUSTED_RID: SetWindowText(hILLabel, L"Untrusted"); break;
            case SECURITY_MANDATORY_LOW_RID: SetWindowText(hILLabel, L"Low"); break;
            case SECURITY_MANDATORY_MEDIUM_RID: SetWindowText(hILLabel, L"Medium"); break;
            case SECURITY_MANDATORY_HIGH_RID: SetWindowText(hILLabel, L"High"); break;
            case SECURITY_MANDATORY_SYSTEM_RID: SetWindowText(hILLabel, L"System"); break;
            default: SetWindowText(hILLabel, L"Unknown"); break;
            }
        }
        catch (DWORD dwError)
        {
            SetWindowText(hILLabel, L"N/A");
            ReportError(L"GetProcessIntegrityLevel", dwError);
        }
    }
    else
    {
        SetWindowText(hIsElevatedLabel, L"N/A");
        SetWindowText(hILLabel, L"N/A");
    }

    return TRUE;
}


//
//   FUNCTION: OnCommand(HWND, int, HWND, UINT)
//
//   PURPOSE: Process the WM_COMMAND message
//
void OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDC_ELEVATE_BN:
        {
            // Check the current process's "run as administrator" status.
            BOOL fIsRunAsAdmin;
            try
            {
                fIsRunAsAdmin = IsRunAsAdmin();
            }
            catch (DWORD dwError)
            {
                ReportError(L"IsRunAsAdmin", dwError);
                break;
            }

            // Elevate the process if it is not run as administrator.
            if (!fIsRunAsAdmin)
            {
                wchar_t szPath[MAX_PATH];
                if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
                {
                    // Launch itself as administrator.
                    SHELLEXECUTEINFO sei = { sizeof(sei) };
                    sei.lpVerb = L"runas";
                    sei.lpFile = szPath;
                    sei.hwnd = hWnd;
                    sei.nShow = SW_NORMAL;

                    if (!ShellExecuteEx(&sei))
                    {
                        DWORD dwError = GetLastError();
                        if (dwError == ERROR_CANCELLED)
                        {
                            // The user refused to allow privileges elevation.
                            // Do nothing ...
                        }
                    }
                    else
                    {
                        EndDialog(hWnd, TRUE);  // Quit itself
                    }
                }
            }
            else
            {
                MessageBox(hWnd, L"The process is running as administrator", L"UAC", MB_OK);
            }
        }
        break;

    case IDOK:
    case IDCANCEL:
        EndDialog(hWnd, 0);
        break;
    }
}


//
//   FUNCTION: OnClose(HWND)
//
//   PURPOSE: Process the WM_CLOSE message
//
void OnClose(HWND hWnd)
{
    EndDialog(hWnd, 0);
}


//
//  FUNCTION: DialogProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main dialog.
//
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        // Handle the WM_INITDIALOG message in OnInitDialog
        HANDLE_MSG (hWnd, WM_INITDIALOG, OnInitDialog);

        // Handle the WM_COMMAND message in OnCommand
        HANDLE_MSG (hWnd, WM_COMMAND, OnCommand);

        // Handle the WM_CLOSE message in OnClose
        HANDLE_MSG (hWnd, WM_CLOSE, OnClose);

    default:
        return FALSE;
    }
    return 0;
}


//
//  FUNCTION: wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
//
//  PURPOSE:  The entry point of the application.
//
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, DialogProc);
}

#endif //UAC_TEST_APP