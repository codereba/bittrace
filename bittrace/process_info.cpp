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

#include "stdafx.h"
#include "process_info.h"
#include "shellapi.h"

#define SYSTEM_ROOT L"\\SYSTEMROOT\\"

typedef struct _SYMBOL_NAME
{
	LPCWSTR name; 
	ULONG cc_name_len; 
	WCHAR dest_name[ MAX_PATH ]; 
	ULONG cc_dest_name_len; 
} SYMBOL_NAME, *PSYMBOL_NAME; 

SYMBOL_NAME symbols[] = { { SYSTEM_ROOT, CONST_STR_LEN( SYSTEM_ROOT ), { 0 }, 0 } }; 

LRESULT WINAPI init_symbol_mapping()
{
	LRESULT ret = ERROR_SUCCESS; 
	UINT32 _ret; 

	do 
	{
		_ret = GetWindowsDirectoryW( symbols[ 0 ].dest_name, ARRAYSIZE( symbols[ 0 ].dest_name ) ); 

		if( _ret >= ARRAYSIZE( symbols[ 0 ].dest_name ) || _ret == 0 )
		{
			*symbols[ 0 ].dest_name = L'\0'; 
			symbols[ 0 ].cc_dest_name_len = 0; 

			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		symbols[ 0 ].cc_dest_name_len = _ret; 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI convert_symbolic_file_path( LPCWSTR file_path, 
										  ULONG cc_path_len, 
										  LPWSTR path_out, 
										  ULONG cc_buf_len, 
										  ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR remain_text; 
	ULONG remain_len; 

	do 
	{
		ASSERT( file_path != NULL ); 
		ASSERT( cc_path_len > 0 ); 
		ASSERT( path_out != NULL ); 
		ASSERT( cc_buf_len > 0 ); 
		ASSERT( cc_ret_len != NULL ); 

		*cc_ret_len = 0; 

		if( 0 != wcsnicmp( file_path, SYSTEM_ROOT, CONST_STR_LEN( SYSTEM_ROOT) ) ) 
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		remain_text = file_path + CONST_STR_LEN( SYSTEM_ROOT ); 
		ASSERT( cc_buf_len > ( remain_text - file_path ) ); 

		remain_len = cc_path_len - ( ULONG )( ULONG_PTR )( remain_text - file_path ); 

		if( cc_buf_len <= remain_len + symbols[ 0 ].cc_dest_name_len + 1 )
		{
			*cc_ret_len = remain_len + symbols[ 0 ].cc_dest_name_len + 1; 

			ret = ERROR_BUFFER_TOO_SMALL; 
		}

		path_out[ symbols[ 0 ].cc_dest_name_len ] = L'\\'; 
		memcpy( path_out + symbols[ 0 ].cc_dest_name_len + 1, remain_text, ( remain_len << 1 ) ); 

		memcpy( path_out, symbols[ 0 ].dest_name, ( symbols[ 0 ].cc_dest_name_len << 1 ) ); 

		*cc_ret_len = remain_len + symbols[ 0 ].cc_dest_name_len; 
	}while( FALSE );

	return ret; 
}

#define TRACE_ERR( _x_ )
Process::Process() : m_hProcessModule( 0 ), 
					m_ahmProcess( NULL ), 
					m_dwProcId( ( ULONG )INVALID_PROCESS_ID ), 
					m_dwParentProcId( ( ULONG )INVALID_PROCESS_ID ), 
					SessionId( ( ULONG )-1 )
{
    ZeroMemory( &m_stPrcEntry32, sizeof( m_stPrcEntry32 ));
    m_stPrcEntry32.dwSize = sizeof( m_stPrcEntry32 );
}

Process::~Process()
{
    Clear();
}

LRESULT Process::GetMainModuleFullName()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG path_len; 

	do 
	{
		if( NULL == m_ahmProcess )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		DWORD dwNeeded = 0;
		_ret = ::EnumProcessModules( m_ahmProcess, &m_hProcessModule,  sizeof( m_hProcessModule ), &dwNeeded );

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		::GetModuleBaseName( m_ahmProcess, m_hProcessModule, m_csBaseName.GetBufferSetLength( MAX_PATH ), MAX_PATH );
		m_csBaseName.ReleaseBuffer();

		::GetModuleFileNameEx( m_ahmProcess, m_hProcessModule, m_csFullPath.GetBufferSetLength( MAX_PATH ), MAX_PATH );
		m_csFullPath.ReleaseBuffer(); 

		ret = convert_symbolic_file_path( m_csFullPath.GetBuffer(), 
			m_csFullPath.GetLength(), 
			m_csFullPath.GetBufferSetLength( MAX_PATH ), 
			MAX_PATH, 
			&path_len ); 

		if( ret != ERROR_SUCCESS && ret != ERROR_NOT_FOUND )
		{
			DBG_BP(); 
		}

		m_csFullPath.ReleaseBuffer(); 

	}while( FALSE );

	return ret; 
}

LRESULT Process::SetProcessId( ULONG ProcessId )
{
	LRESULT Ret = ERROR_SUCCESS; 
	HANDLE ProcessHandle; 

	m_dwProcId = ProcessId; 

	ProcessHandle = ::OpenProcess( PROC_SAFE_ACCESS, FALSE, ProcessId );
	if( NULL == ProcessHandle )
	{
		ProcessHandle = ::OpenProcess( PROC_READ_ONLY_ACCESS, TRUE, ProcessId );
	}

	if( ProcessHandle == NULL )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	m_ahmProcess = ProcessHandle; 	

_return:
	return Ret; 
}

//LRESULT WINAPI get_parent_process_id( ULONG process_id, ULONG *parent_process_id )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	LONG                      status;  
//	DWORD                     dwParentPID = (DWORD)-1;  
//	HANDLE                    hProcess;  
//	PROCESS_BASIC_INFORMATION pbi;  
//	PROCNTQSIP NtQueryInformationProcess;  
//
//	do 
//	{
//		*parent_process_id = 0; 
//		// ntdll!NtQueryInformationProcess (NT specific!)  
//		//  
//		// The function copies the process information of the  
//		// specified type into a buffer  
//		//  
//		// NTSYSAPI  
//		// NTSTATUS  
//		// NTAPI  
//		// NtQueryInformationProcess(  
//		//    IN HANDLE ProcessHandle,              // handle to process  
//		//    IN PROCESSINFOCLASS InformationClass, // information type  
//		//    OUT PVOID ProcessInformation,         // pointer to buffer  
//		//    IN ULONG ProcessInformationLength,    // buffer size in bytes  
//		//    OUT PULONG ReturnLength OPTIONAL      // pointer to a 32-bit  
//		//                                          // variable that receives  
//		//                                          // the number of bytes  
//		//                                          // written to the buffer   
//		// );  
//
//		NtQueryInformationProcess = ( PROCNTQSIP )GetProcAddress( GetModuleHandleW( L"ntdll" ), 
//			"NtQueryInformationProcess" ); 
//
//		if (!NtQueryInformationProcess)  
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		// Get process handle  
//		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwId);  
//		if (!hProcess)  
//		{
//			ret = GetLastError(); 
//			break; 
//		}
//
//		// Retrieve information  
//		status = NtQueryInformationProcess( hProcess,  
//			ProcessBasicInformation,  
//			(PVOID)&pbi,  
//			sizeof(PROCESS_BASIC_INFORMATION),  
//			NULL  
//			);  
//
//		// Copy parent Id on success  
//		if  (!status)  
//			dwParentPID = pbi.InheritedFromUniqueProcessId;  
//
//
//		*parent_process_id = dwParentPID; 
//
//	}while( FALSE ); 
//
//	if( NULL != hProcess )
//	{
//		CloseHandle( hProcess );  
//	}
//
//	return ret; 
//}

LRESULT Process::GetAllInfo()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = ExtractGDIInfo();
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		if( PRT_INVALID == ExtractProcessType( m_csFullPath.GetBuffer(), &m_csProcessType ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

		_ret = ExtractProcessIOCounters();
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = ExtractProcessTimes();
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		_ret = ExtractProcessPriority( m_csPriority );
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		 _ret = ExtractProcessCommandLine( m_ahmProcess, m_csProcessCommandLine ); 

		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		ExtractProcessMemoryCounters(); 

		_ret = ExtractProcessOwner( m_ahmProcess, m_csUserName ); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
		}

		m_fviFileVersion.SetFilePath( m_csFullPath.GetBuffer( ) ); 

		_ret = m_fviFileVersion.GetInfo(); 
		if( _ret == false )
		{
			ret = _ret; 
		}

		_ret = ProcessIdToSessionId( m_dwProcId, &SessionId ); 
		if( _ret == FALSE )
		{
			ret = _ret; 
		}
	}while( FALSE );

	return ret; 
}

bool Process::SetProcessEntry32( const PROCESSENTRY32& stpePrcEntry32_i )
{
    // Copy information
    m_stPrcEntry32 = stpePrcEntry32_i;

    Clear();

    // Open this process
    m_ahmProcess = ::OpenProcess( PROC_FULL_ACCESS, FALSE, stpePrcEntry32_i.th32ProcessID );
    if( !m_ahmProcess )
    {
        // Make a last ditch attempt to open this process
        m_ahmProcess = ::OpenProcess( PROC_SAFE_ACCESS, TRUE, stpePrcEntry32_i.th32ProcessID );
        if( !m_ahmProcess )
        {
            return false;
        }// End if
    }// End if

    // Get process module
    DWORD dwNeeded = 0;
    ::EnumProcessModules( m_ahmProcess, &m_hProcessModule,  sizeof( m_hProcessModule ), &dwNeeded );

    // Did we find the module
    if( !dwNeeded )
    {
        return false;
    }

    // Get base name for module
    ::GetModuleBaseName( m_ahmProcess, m_hProcessModule, m_csBaseName.GetBufferSetLength( MAX_PATH ), MAX_PATH );
    m_csBaseName.ReleaseBuffer();

    // Get full path
    ::GetModuleFileNameEx( m_ahmProcess, m_hProcessModule, m_csFullPath.GetBufferSetLength( MAX_PATH ), MAX_PATH );
    m_csFullPath.ReleaseBuffer();

    // We are here since all went well
    return true;
}// End set


/** 
 * 
 * Adds a kid to this process
 * 
 * @param       pKid_i - Kid
 * @return      bool   - Returns execution status
 * @exception   Nil
 * @see         Nil
 * @since       1.0
 */
bool Process::AddKid( Process*& pKid_i )
{
    // Verify kid
    if( !pKid_i )
    {
        return false;
    }

    // Add to tail
    //m_lstKids.AddTail( pKid_i );
    return true;
}


/** 
 * 
 * Enumerate all modules
 * 
 * @param       Nil
 * @return      bool - Returns execution status.
 * @exception   Nil
 * @see         Nil
 * @since       1.0
 */

#pragma pack( push )
#pragma pack( 4 )
typedef struct _LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} LANGANDCODEPAGE, *PLANGANDCODEPAGE; 
#pragma pack( pop )

#define MAX_FILE_DESC 512
#define MAX_FILE_VERSION 64
#define MAX_FILE_VENDOR 64
#define MAX_FILE_VENDOR_VERSION  64
#define MAX_FILE_COPY_RIGHT 128
#define MAX_FILE_LANGUAGE 64
#define MAX_SIGN_SUBJECT_NAME 260
#define MAX_FILE_SIGN_ALGRITHM 128

#define MAX_FILE_TIME_TEXT 32

#define MAX_STRING_FILE_INFO 50
#define MAX_STRING_FILE_VERSION_TEXT 260

INT32 WINAPI CopyFileVersionString( PVOID FileVersionInfo, 
								   LPCWSTR StringName, 
								   LPWSTR StringBuffer, 
								   ULONG ccBufferLength, 
								   ULONG *ReturnLength )
{
	INT32 Ret = 0; 
	INT32 _Ret; 
	LPWSTR Text; 
	UINT TextSize; 

	_Ret = VerQueryValue( FileVersionInfo, 
		StringName, 
		( LPVOID* )&Text, 
		&TextSize ); 

	if( _Ret == FALSE 
		|| TextSize == 0 )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	if( TextSize >= ccBufferLength )
	{
		memcpy( StringBuffer, 
			Text, 
			( ( ccBufferLength - 1 ) << 1 ) ); 

		StringBuffer[ ccBufferLength - 1 ] = L'\0'; 

		*ReturnLength = ccBufferLength - 1; 

		Ret = ERROR_BUFFER_TOO_SMALL; 
	}
	else
	{
		memcpy( StringBuffer, 
			Text, 
			( TextSize << 1 ) ); 

		if( StringBuffer[ TextSize - 1 ] != L'\0' )
		{
			if( TextSize + 1 <= ccBufferLength )
			{
				StringBuffer[ TextSize ] = L'\0'; 
				*ReturnLength = TextSize; 
			}
			else
			{
				StringBuffer[ TextSize - 1 ] = L'\0'; 
				Ret = ERROR_BUFFER_TOO_SMALL; 

				*ReturnLength = TextSize - 1; 
			}
		}
		else
		{
			*ReturnLength = TextSize; 
		}
	}

_return:
	return Ret; 
}

LRESULT Process::_GetVersionInfo()
{
	LRESULT Ret = ERROR_SUCCESS; 
	INT32 _Ret; 
	HRESULT hr;

	ULONG ccStringLength; 
	PVOID FileVersionInfo = NULL; 
	ULONG FileVersionInfoSize; 

	LPWSTR FileVersion = NULL; 
	LPWSTR FileCopyRight = NULL; 
	LPWSTR FileVendorVersion = NULL; 
	LPWSTR FileDesc = NULL; 
	LPWSTR FileVendor = NULL; 
	LPWSTR FileLanguge = NULL; 
	LPWSTR StringFileInfo = NULL; 
	LPWSTR StringVersionName = NULL; 
	LPWSTR ErrorDesc = NULL; 

	if( TRUE == m_csFullPath.IsEmpty() )
	{
		Ret = ERROR_NOT_READY; 
		goto _return; 
	}

	FileVersionInfoSize = GetFileVersionInfoSizeW( m_csFullPath.GetBuffer(), NULL ); 
	if( FileVersionInfoSize == 0 )
	{
		log_trace( ( MSG_FATAL_ERROR, "get the size of the information of the file version error" ) );  
		goto _return; 
	}

	FileVersionInfo = malloc( FileVersionInfoSize ); 
	if( NULL == FileVersionInfo )
	{
		log_trace( ( MSG_FATAL_ERROR, "allocate the buffer for the information of the file version error" ) );  

		Ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_Ret = GetFileVersionInfoW( m_csFullPath.GetBuffer(), 0, FileVersionInfoSize, FileVersionInfo ); 
	if( FALSE == _Ret )
	{
		log_trace( ( MSG_FATAL_ERROR, "get the information of the file version error" ) );  

		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	do
	{
		INT32 i; 
		PLANGANDCODEPAGE Translation; 
		UINT32 TranslationSize;

		FileVersion = ( LPWSTR )malloc( MAX_FILE_VERSION << 1 );
		if( FileVersion == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		FileCopyRight = ( LPWSTR )malloc( MAX_FILE_COPY_RIGHT << 1 );
		if( FileCopyRight == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		FileVendorVersion = ( LPWSTR )malloc( MAX_FILE_VENDOR_VERSION << 1 );
		if( FileVendorVersion == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		FileDesc = ( LPWSTR )malloc( MAX_FILE_DESC << 1 );
		if( FileDesc == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		FileVendor = ( LPWSTR )malloc( MAX_FILE_VENDOR << 1 );
		if( FileVendor == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		FileLanguge = ( LPWSTR )malloc( MAX_FILE_LANGUAGE << 1 );
		if( FileLanguge == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		StringFileInfo = ( LPWSTR )malloc( MAX_STRING_FILE_INFO << 1 );
		if( StringFileInfo == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		StringVersionName = ( LPWSTR )malloc( MAX_STRING_FILE_VERSION_TEXT << 1 );
		if( StringVersionName == NULL )
		{
			Ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		try
		{
			_Ret = VerQueryValue( FileVersionInfo, 
				TEXT("\\VarFileInfo\\Translation"),
				( LPVOID* )&Translation,
				&TranslationSize ); 

			if( _Ret == FALSE )
			{
				log_trace( ( MSG_FATAL_ERROR, "get the language information error" ) ); 
				Ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			// Read the file description for each language and code page.

			for( i = 0; ( ULONG )i < ( TranslationSize / sizeof( LANGANDCODEPAGE ) ); i++ ) 
			{
				do 
				{
					hr = StringCchPrintfW( StringFileInfo, 
						MAX_STRING_FILE_INFO, 
						L"\\StringFileInfo\\%04x%04x\\",
						Translation[ i ].wLanguage,
						Translation[ i ].wCodePage ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					//Comments
					//	InternalName 
					//	ProductName 
					//	CompanyName 
					//	LegalCopyright 
					//	ProductVersion 
					//	FileDescription 
					//	LegalTrademarks 
					//	PrivateBuild 
					//	FileVersion 
					//	OriginalFilename 
					//	SpecialBuild 

					hr = StringCchCopyW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						StringFileInfo ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					hr = StringCchCatW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						L"FileVersion" ); 
					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					FileVersion[ 0 ] = L'\0'; 
					_Ret = CopyFileVersionString( FileVersionInfo, 
						StringVersionName, 
						FileVersion, 
						MAX_FILE_VERSION, 
						&ccStringLength ); 

					if( _Ret != 0 )
					{
						Ret = _Ret; 
					}
					else
					{
					}

					hr = StringCchCopyW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						StringFileInfo ); 

					if( FAILED( hr ) )
					{
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					hr = StringCchCatW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						L"LegalCopyright" ); 

					if( FAILED( hr ) )
					{
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					FileCopyRight[ 0 ] = L'\0'; 
					_Ret = CopyFileVersionString( FileVersionInfo, 
						StringVersionName, 
						FileCopyRight, 
						MAX_FILE_COPY_RIGHT, 
						&ccStringLength ); 

					if( _Ret != 0 )
					{
						Ret = _Ret; 
					}
					else
					{
					}

					hr = StringCchCopyW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						StringFileInfo ); 

					if( FAILED( hr ) )
					{
						Ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					hr = StringCchCatW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						L"ProductVersion" ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					FileVendorVersion[ 0 ] = L'\0'; 
					_Ret = CopyFileVersionString( FileVersionInfo, 
						StringVersionName, 
						FileVendorVersion, 
						MAX_FILE_VENDOR_VERSION, 
						&ccStringLength ); 

					if( _Ret != 0 )
					{
						Ret = _Ret; 
					}
					else
					{
					}

					hr = StringCchCopyW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						StringFileInfo ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					hr = StringCchCatW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						L"FileDescription" ); 

					FileDesc[ 0 ] = L'\0'; 
					_Ret = CopyFileVersionString( FileVersionInfo, 
						StringVersionName, 
						FileDesc, 
						MAX_FILE_DESC, 
						&ccStringLength ); 

					if( _Ret != 0 )
					{
						Ret = _Ret; 
					}
					else
					{
					}

					hr = StringCchCopyW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						StringFileInfo ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					hr = StringCchCatW( StringVersionName, 
						MAX_STRING_FILE_VERSION_TEXT, 
						L"CompanyName" ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

					FileVendor[ 0 ] = L'\0'; 

					_Ret = CopyFileVersionString( FileVersionInfo, 
						StringVersionName, 
						FileVendor, 
						MAX_FILE_VENDOR, 
						&ccStringLength ); 

					if( _Ret != 0 )
					{
						Ret = _Ret; 
					}
					else
					{
					}

					hr = StringCchPrintfW( FileLanguge, 
						MAX_FILE_LANGUAGE , 
						L"%u", 
						Translation[ i ].wLanguage ); 

					if( FAILED( hr ) )
					{
						log_trace( ( MSG_FATAL_ERROR, "get the resource path error" ) ); 
						Ret = ERROR_ERRORS_ENCOUNTERED; 

						break; 
					}

				} while ( FALSE ); 

				break; 
			} 
		}
		catch( ... )
		{
			OutputDebugStringW( L"exception happen when get file version information\n" ); 
		}

	}while( FALSE ); 


_return: 

	if( NULL != FileVersionInfo )
	{
		free( FileVersionInfo ); 
	}

	if( NULL != FileVersion )
	{
		free( FileVersion ); 
	}

	if( NULL != FileCopyRight )
	{
		free( FileCopyRight ); 

	}

	if( NULL != FileVendorVersion )
	{
		free( FileVendorVersion ); 

	}

	if( NULL != FileDesc )
	{
		free( FileDesc ); 

	}

	if( NULL != FileVendor )
	{
		free( FileVendor ); 

	}

	if( NULL != FileLanguge )
	{
		free( FileLanguge ); 

	}

	if( NULL != StringFileInfo )
	{
		free( StringFileInfo ); 

	}

	if( NULL != StringVersionName )
	{
		free( StringVersionName ); 

	}

	if( NULL != ErrorDesc )
	{
		free( ErrorDesc ); 
	}

	return Ret; 
}

LRESULT Process::EnumModules()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE hSnapModules = INVALID_HANDLE_VALUE; 

	do 
	{
		if( !m_ahmProcess )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		hSnapModules = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, m_stPrcEntry32.th32ProcessID );
		if( hSnapModules == INVALID_HANDLE_VALUE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		MODULEENTRY32 stmeModuleEntry32 = { 0 };
		stmeModuleEntry32.dwSize = sizeof( stmeModuleEntry32 );

		if( !Module32First( hSnapModules, &stmeModuleEntry32 ))
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		do 
		{
			Module* pModule = new Module;
			if( !pModule )
			{
				continue;
			}

			pModule->SetModuleEntry32( stmeModuleEntry32 );

			m_ProcessModules.push_back( pModule );
		}while( Module32Next( hSnapModules, &stmeModuleEntry32 ));

	}while( FALSE );

	if( hSnapModules != NULL )
	{
		CloseHandle( hSnapModules ); 
	}

	return ret;
}



void Process::Clear()
{
	std::vector<Module*>::iterator it; 
    for( it = m_ProcessModules.begin(); it != m_ProcessModules.end(); it ++ )
    {
        Module* pModule = *it; 
		ASSERT( pModule != NULL ); 

        delete pModule;
        //pModule = NULL;
    }

    m_ProcessModules.clear();

    if( NULL != m_ahmProcess )
	{
		CloseHandle( m_ahmProcess ); 
	}

    m_hProcessModule = 0;

    ::ZeroMemory( &m_stPrcEntry32, sizeof( m_stPrcEntry32 ));
    m_stPrcEntry32.dwSize = sizeof( m_stPrcEntry32 );

    m_csBaseName.Empty();
    m_csFullPath.Empty();
}


PROCESS_TYPE_e Process::ExtractProcessType( CString* pcsProcessType_o) const
{
    return ExtractProcessType( GetFullPath(), pcsProcessType_o );
}


PROCESS_TYPE_e Process::ExtractProcessType( LPCTSTR lpctszProcessPath_i, CString* pcsProcessType_o )
{
    if( !lpctszProcessPath_i ||            
        !lpctszProcessPath_i[0] 
	/*||    
        !PathIsExe( lpctszProcessPath_i )*/) 
    {
        return PRT_INVALID;
    }

    const DWORD dwRetVal = ( const ULONG )SHGetFileInfo( lpctszProcessPath_i, 
                                          FILE_ATTRIBUTE_NORMAL, 
                                          0, 
                                          0, 
                                          SHGFI_EXETYPE );
    if( !dwRetVal )
    {
        return PRT_INVALID;
    }

    
   /************************************************************************
       LOWORD = NE or PE and HIWORD = 3.0, 3.5, or 4.0  Windows application 
       LOWORD = MZ and HIWORD = 0  MS-DOS .exe, .com, or .bat file 
       LOWORD = PE and HIWORD = 0  Win32 console application 
    ************************************************************************/

    const WORD wLowWord =  LOWORD( dwRetVal );
    const WORD wHiWord = HIWORD( dwRetVal );
    const WORD wPEWord = MAKEWORD( 'P', 'E' );
    const WORD wMZWord = MAKEWORD( 'M', 'Z' );
    const WORD wNEWord = MAKEWORD( 'N', 'E' );

    if( wLowWord == wPEWord || wLowWord == wNEWord )
    {
        if( wHiWord == 0 )
        {
            if( pcsProcessType_o )
            {
                *pcsProcessType_o = _T( "Win32 Console application" );
            }
            return PRT_WIN32_CONSOLE;
        }
        else
        {
            if( *pcsProcessType_o )
            {
                *pcsProcessType_o = _T( "Win32 Windows application" );
            }

            return PRT_WIN32_WINDOWS;
        }
    }
    else if( wLowWord == wMZWord && wHiWord == 0 )
    {
        if( *pcsProcessType_o )
        {
            *pcsProcessType_o = _T( "MS-DOS application" );
        }

        return PRT_MSDOS;
    }

    return PRT_INVALID;
}

HICON Process::ExtractAssociatedProcessIcon()
{
    if( GetFullPath().IsEmpty() )
    {
        return 0;
    }

    SHFILEINFO shFileInfo = { 0 };

    ASSERT( SHGetFileInfo( GetFullPath(), 
                           FILE_ATTRIBUTE_NORMAL, 
                           &shFileInfo, 
                           sizeof( shFileInfo ), 
                           SHGFI_SMALLICON | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES ));
    return NULL;
}

LRESULT Process::Kill( const UINT uExitCode_i ) const
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( TerminateProcess( m_ahmProcess, uExitCode_i ) == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT Process::ExtractGDIInfo()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( NULL == m_ahmProcess )
		{
			ret = ERROR_NOT_READY; 
		}

		m_dwGDIObjs = GetGuiResources( m_ahmProcess, GR_GDIOBJECTS );
		m_dwUserObjs = GetGuiResources( m_ahmProcess, GR_USEROBJECTS );

		ret = GetLastError(); 
	}while( FALSE );

	return ret; 
}


LRESULT Process::ExtractProcessIOCounters()
{
    return m_ProcIoCounters.GetProcessIoCounters( m_ahmProcess );
}


LRESULT Process::ExtractProcessTimes()
{
    return m_ProcTimes.GetProcessTimes( m_ahmProcess );
}

LRESULT Process::ExtractProcessPriority( CString& csPriority_o ) const
{
	LRESULT ret = ERROR_SUCCESS; 

    const DWORD dwPriorityClass = GetPriorityClass( m_ahmProcess );
    switch( dwPriorityClass )
    {
        case NORMAL_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: Normal" );
            break;
        case ABOVE_NORMAL_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: Above normal" );
            break;
        case BELOW_NORMAL_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: Below normal" );
            break;
        case HIGH_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: High" );
            break;
        case REALTIME_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: Realtime" );
            break;
        case IDLE_PRIORITY_CLASS:
            csPriority_o = _T( "Priority: Idle" );
            break;
        default:
            ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
    }

    return ret; 
}


const PROCESS_MEMORY_COUNTERS& Process::ExtractProcessMemoryCounters()
{
    GetProcessMemoryInfo( m_ahmProcess, &m_stpmcMemCounters, sizeof( m_stpmcMemCounters ));
    return m_stpmcMemCounters;
}


void Process::ParseOutFilePath( CString& csFileName_io ) const
{
   
    const int nIndex = csFileName_io.Find( _T( ':' ));
    if( nIndex == -1 )
    {
        return;
    }

    csFileName_io = csFileName_io.Mid( nIndex - 1 );
}

LRESULT Process::ExtractProcessOwner( HANDLE hProcess_i, 
                                   CString& csOwner_o )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HANDLE hProcessToken = NULL;

	do 
	{

		if ( !::OpenProcessToken( hProcess_i, TOKEN_READ, &hProcessToken ) || !hProcessToken )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
		DWORD dwProcessTokenInfoAllocSize = 0;
		_ret = ::GetTokenInformation( hProcessToken, TokenUser, NULL, 0, &dwProcessTokenInfoAllocSize );


		if( ::GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		PTOKEN_USER pUserToken = ( PTOKEN_USER )( new BYTE[dwProcessTokenInfoAllocSize] );
		if (pUserToken == NULL)
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
		if( !::GetTokenInformation( hProcessToken, TokenUser, pUserToken, dwProcessTokenInfoAllocSize, &dwProcessTokenInfoAllocSize ) ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		{
			SID_NAME_USE   snuSIDNameUse;
			TCHAR          szUser[MAX_PATH] = { 0 };
			DWORD          dwUserNameLength = MAX_PATH;
			TCHAR          szDomain[MAX_PATH] = { 0 };
			DWORD          dwDomainNameLength = MAX_PATH;

			if ( !::LookupAccountSid( NULL, 
				pUserToken->User.Sid, 
				szUser, 
				&dwUserNameLength, 
				szDomain, 
				&dwDomainNameLength, 
				&snuSIDNameUse ) )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
			csOwner_o = _T("\\\\");
			csOwner_o += szDomain;
			csOwner_o += _T("\\");
			csOwner_o += szUser;
		}

	}while( FALSE );

    return ret;
}

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation,
	ProcessQuotaLimits,
	ProcessIoCounters,
	ProcessVmCounters,
	ProcessTimes,
	ProcessBasePriority,
	ProcessRaisePriority,
	ProcessDebugPort,
	ProcessExceptionPort,
	ProcessAccessToken,
	ProcessLdtInformation,
	ProcessLdtSize,
	ProcessDefaultHardErrorMode,
	ProcessIoPortHandlers,         
	ProcessPooledUsageAndLimits,
	ProcessWorkingSetWatch,
	ProcessUserModeIOPL,
	ProcessEnableAlignmentFaultFixup,
	ProcessPriorityClass,
	ProcessWx86Information,
	ProcessHandleCount,
	ProcessAffinityMask,
	ProcessPriorityBoost,
	ProcessDeviceMap,
	ProcessSessionInformation,
	ProcessForegroundInformation,
	ProcessWow64Information,
	ProcessImageFileName,
	ProcessLUIDDeviceMapsEnabled,
	ProcessBreakOnTermination,
	ProcessDebugObjectHandle,
	ProcessDebugFlags,
	ProcessHandleTracing,
	ProcessIoPriority,
	ProcessExecuteFlags,
	ProcessTlsInformation,
	ProcessCookie,
	ProcessImageInformation,
	ProcessCycleTime,
	ProcessPagePriority,
	ProcessInstrumentationCallback,
	ProcessThreadStackAllocation,
	ProcessWorkingSetWatchEx,
	ProcessImageFileNameWin32,
	ProcessImageFileMapping,
	ProcessAffinityUpdateMode,
	ProcessMemoryAllocationMode,
	ProcessGroupInformation,
	ProcessTokenVirtualizationEnabled,
	ProcessConsoleHostProcess,
	ProcessWindowInformation,
	MaxProcessInfoClass          
} PROCESSINFOCLASS;

extern "C" NTSTATUS
WINAPI
ZwQueryInformationProcess (
						   __in HANDLE ProcessHandle,
						   __in PROCESSINFOCLASS ProcessInformationClass,
						   __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
						   __in ULONG ProcessInformationLength,
						   __out_opt PULONG ReturnLength
						   ); 

LRESULT Process::ExtractProcessCommandLine( HANDLE hProcess, CString& csCommandLine_o )
{
	LRESULT ret = ERROR_SUCCESS; 
	NTSTATUS ntstatus; 
	ULONG iReturn = 1;
	SIZE_T dwSize = 0;
	PROCESS_BASIC_INFORMATION basic_info = { 0 };

	do 
	{
		ASSERT( hProcess != NULL ); 
		if( hProcess == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ntstatus = ZwQueryInformationProcess( hProcess, ProcessBasicInformation, &basic_info, sizeof( basic_info ), &iReturn );
		if( ntstatus != STATUS_SUCCESS && 
			ntstatus != STATUS_INFO_LENGTH_MISMATCH )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( iReturn < sizeof( basic_info ) )
		{
			ASSERT( FALSE ); 
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		if( basic_info.PebBaseAddress == NULL )
		{
			ntstatus = STATUS_NOT_FOUND; 
			break; 
		}

		m_dwParentProcId = basic_info.InheritedFromUniqueProcessId;  

		{
			PEB Peb = { 0 };
			if (!::ReadProcessMemory(hProcess, basic_info.PebBaseAddress, &Peb, sizeof(Peb), &dwSize))
			{
				TRACE_ERR( "ReadProcessMemory failed, process environment block extraction failed" );
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			RTL_USER_PROCESS_PARAMETERS Block = { 0 };
			if( !::ReadProcessMemory( hProcess, ( LPVOID )Peb.ProcessParameters, &Block, FIELD_OFFSET( RTL_USER_PROCESS_PARAMETERS, Environment ), &dwSize ) )
			{
				TRACE_ERR( "ReadProcessMemory failed, block extraction failed" );

				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			wchar_t wszCmdLine[ MAX_PATH+1] = { 0 };
			if (!::ReadProcessMemory(hProcess, 
				(LPVOID)Block.CommandLine.Buffer, 
				wszCmdLine, 
				( Block.CommandLine.Length > sizeof( WCHAR ) * MAX_PATH ) ? 
				sizeof( WCHAR ) * MAX_PATH : Block.CommandLine.Length, 
				&dwSize ) )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
			csCommandLine_o = wszCmdLine;
		}  
	}while( FALSE );

	return ret;
}

bool Process::ChangeProcessPriority( const HANDLE& hProcess_i, const UINT uPriority_i )
{
    return SetPriorityClass( hProcess_i, uPriority_i ) != 0;
}