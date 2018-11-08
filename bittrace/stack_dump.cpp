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

#ifdef _DEBUG_MEM_LEAKS
#undef _DEBUG_MEM_LEAKS
#include "common_func.h"
#endif //_DEBUG_MEM_LEAKS

#include "stack_dump.h"
#include "module_info.h"

#include <strsafe.h>

#define BITTRACE_SYMBOL_HANDLER_ID ( HANDLE )0x8392e8a3

#define DBGHELP_TRANSLATE_TCHAR 1
#include "..\windbg_sdk\inc\dbghelp.h"

#ifndef LIST_OPERA_DEFINED
FORCEINLINE
VOID
InitializeListHead(
				   PLIST_ENTRY ListHead
				   )
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

BOOLEAN
FORCEINLINE
IsListEmpty(
			__in const LIST_ENTRY * ListHead
			)
{
	return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
				__in PLIST_ENTRY Entry
				)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Flink;

	Flink = Entry->Flink;
	Blink = Entry->Blink;
	Blink->Flink = Flink;
	Flink->Blink = Blink;
	return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
			   __inout PLIST_ENTRY ListHead
			   )
{
	PLIST_ENTRY Flink;
	PLIST_ENTRY Entry;

	Entry = ListHead->Flink;
	Flink = Entry->Flink;
	ListHead->Flink = Flink;
	Flink->Blink = ListHead;
	return Entry;
}


FORCEINLINE
PLIST_ENTRY
RemoveTailList(
			   __inout PLIST_ENTRY ListHead
			   )
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Entry;

	Entry = ListHead->Blink;
	Blink = Entry->Blink;
	ListHead->Blink = Blink;
	Blink->Flink = ListHead;
	return Entry;
}


FORCEINLINE
VOID
InsertTailList(
			   __inout PLIST_ENTRY ListHead,
			   __inout __drv_aliasesMem PLIST_ENTRY Entry
			   )
{
	PLIST_ENTRY Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
}


FORCEINLINE 
VOID
InsertHeadList(
			   __inout PLIST_ENTRY ListHead,
			   __inout __drv_aliasesMem PLIST_ENTRY Entry
			   )
{
	PLIST_ENTRY Flink;

	Flink = ListHead->Flink;
	Entry->Flink = Flink;
	Entry->Blink = ListHead;
	Flink->Blink = Entry;
	ListHead->Flink = Entry;
}
#endif //LIST_OPERA_DEFINED


typedef BOOL ( WINAPI *SYMINITIALIZEW )( IN HANDLE   hProcess, 
										  IN PWSTR     UserSearchPath,
										  IN BOOL     fInvadeProcess ); 

typedef DWORD ( WINAPI *SYMGETOPTIONS )( VOID ); 
typedef DWORD ( WINAPI *SYMSETOPTIONS )( __in DWORD SymOptions ); 

typedef BOOL ( WINAPI *SYMCLEANUP )( __in HANDLE hProcess );

typedef BOOL ( WINAPI * SYMGETMODULEINFOW64 )( __in HANDLE hProcess, 
											  __in DWORD64 qwAddr, 
											  __out PIMAGEHLP_MODULEW64 ModuleInfo ); 
typedef BOOL ( WINAPI *SYMGETSYMFROMADDR64 )(
	__in HANDLE hProcess,
	__in DWORD64 qwAddr,
	__out_opt PDWORD64 pdwDisplacement,
	__inout PIMAGEHLP_SYMBOL64  Symbol
	);
typedef BOOL ( WINAPI *SYMUNDNAME64 )( __in PIMAGEHLP_SYMBOL64 sym,            // Symbol to undecorate
									  __out_ecount(UnDecNameLength) PSTR UnDecName,   // Buffer to store undecorated name in
									  __in DWORD UnDecNameLength              // Size of the buffer 
									  ); 

typedef DWORD ( WINAPI * UNDECORATESYMBOLNAME )(
	__in PCSTR name,
	__out_ecount(maxStringLength) PSTR outputString,   
	__in DWORD maxStringLength,
	__in DWORD flags
	); 

typedef DWORD64 ( WINAPI * SYMLOADMODULEEXW )( __in HANDLE hProcess,
											__in_opt HANDLE hFile,
											__in_opt PCWSTR ImageName,
											__in_opt PCWSTR ModuleName,
											__in DWORD64 BaseOfDll,
											__in DWORD DllSize,
											__in_opt PMODLOAD_DATA Data,
											__in_opt DWORD Flags
											); 

typedef BOOL ( WINAPI *SYMSETSEARCHPATHW )( __in HANDLE hProcess, 
										   __in_opt PCWSTR SearchPath ); 

typedef BOOL ( WINAPI *SYMFINDFILEINPATHW )( __in HANDLE hprocess,
											__in_opt PCWSTR SearchPath,
											__in PCWSTR FileName,
											__in_opt PVOID id,
											__in DWORD two,
											__in DWORD three,
											__in DWORD flags,
											__out_ecount(MAX_PATH + 1) PWSTR FoundFile,
											__in_opt PFINDFILEINPATHCALLBACKW callback,
											__in_opt PVOID context
											);

SYMINITIALIZEW sym_initialize_w = NULL; 
SYMGETOPTIONS sym_get_optioins = NULL; 
SYMSETOPTIONS sym_set_optioins = NULL; 
SYMCLEANUP sym_cleanup = NULL; 
SYMGETMODULEINFOW64 sym_get_module_infow64 = NULL; 
SYMGETSYMFROMADDR64 sym_get_sym_from_addr64 = NULL; 
SYMUNDNAME64 sym_und_name64 = NULL; 
UNDECORATESYMBOLNAME undecorate_symbol_name = NULL; 
SYMLOADMODULEEXW sym_load_module_ex_w = NULL; 
SYMFINDFILEINPATHW sym_find_file_in_path_w = NULL; 
SYMSETSEARCHPATHW sym_set_search_path_w = NULL; 

/*************************************************************
对于堆栈，不可以在加入事件时进行同步分析，需要进行异步的分析处理。
可以使用方法如下：
基本模型：
几个或1个工作线程，1个或几个工作队列。

具体实现方法 ：
队列工作单元信息内容 ：
1.需要分析的事件的序列号，需要进行查找，并需要对序号所对应的事件的一对一对应关系进行维护。
2.需要分析的事件本身，需要对事件进行引用计数，不可以在DUMP时，释放此事件。
3.需要分析的事件堆栈数据，这种方式提供的信息更加准确，但不能满足更全面的数据支持需求。同样需要引用计数。
4.需要分析的事件堆栈数据的拷贝，这种方式在实现方式上相对引用计数简单，但需要加入建缓存，拷贝，DUMP完成后的，搜索，设置等更多的步骤。

综合考虑，还是使用第3套方案来实现功能。
*************************************************************/
typedef struct _dump_stack_work_item
{
	LIST_ENTRY entry; 
	PVOID event; 
} dump_stack_work_item, *pdump_stack_work_item; 

typedef struct _dump_stack_works
{
	HANDLE worker; 
	BOOLEAN stop; 
	LIST_ENTRY works; 
	CRITICAL_SECTION lock; 
}dump_stack_works, *pdump_stack_works; 

dump_stack_works dump_stack_work_manager; 

ULONG CALLBACK dump_stack_work_thread( PVOID ); 

LRESULT init_dump_stack_work_manager()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN lock_inited = FALSE; 

	do 
	{
		InitializeListHead( &dump_stack_work_manager.works ); 
		init_cs_lock( dump_stack_work_manager.lock ); 
		lock_inited = TRUE; 

		dump_stack_work_manager.stop = FALSE; 

		dump_stack_work_manager.worker = CreateThread( NULL, 0, dump_stack_work_thread, &dump_stack_work_manager, 0, NULL ); 
		if( dump_stack_work_manager.worker == NULL )
		{
			break; 
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( TRUE == lock_inited )
		{
			uninit_cs_lock( dump_stack_work_manager.lock ); 
		}
	}

	return ret; 
}

LRESULT WINAPI release_event_notfiy_ref( dump_stack_work_item *work_item )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

	}while( FALSE );

	return ret; 
}

LRESULT uninit_dump_stack_work_manager()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	PLIST_ENTRY entry; 
	dump_stack_work_item *work_item; 

	do 
	{
		dump_stack_work_manager.stop = TRUE; 

		lock_cs( dump_stack_work_manager.lock ); 

		WaitForSingleObject( dump_stack_work_manager.worker, INFINITE ); 

		entry = dump_stack_work_manager.works.Flink; 

		for( ; ; )
		{
			if( entry == &dump_stack_work_manager.works )
			{
				break; 
			}

			RemoveEntryList( entry ); 

			work_item = CONTAINING_RECORD( entry, dump_stack_work_item, entry ); 

			_ret = release_event_notfiy_ref( work_item ); 

			//free( work_item ); 
		}

		unlock_cs( dump_stack_work_manager.lock ); 
		uninit_cs_lock( dump_stack_work_manager.lock ); 
	}while( FALSE );

	return ret; 
}

ULONG CALLBACK dump_stack_work_thread( PVOID )
{
	LRESULT ret = ERROR_SUCCESS; 


	return ( ULONG )ret; 
}

LRESULT WINAPI set_default_symbol_path(); 
HMODULE dbg_help_dll = NULL; 
LRESULT load_dbg_help( LPCWSTR file_path )
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( dbg_help_dll == NULL ); 

	do 
	{
		ASSERT( file_path != NULL ); 
		ASSERT( *file_path != L'\0' ); 

		dbg_help_dll = LoadLibraryW( file_path ); 
		if( dbg_help_dll == NULL )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		sym_initialize_w = ( SYMINITIALIZEW )GetProcAddress( dbg_help_dll, "SymInitializeW" ); 
		sym_get_optioins = ( SYMGETOPTIONS )GetProcAddress( dbg_help_dll, "SymGetOptions" ); 
		sym_set_optioins = ( SYMSETOPTIONS )GetProcAddress( dbg_help_dll, "SymSetOptions" ); 
		sym_cleanup = ( SYMCLEANUP )GetProcAddress( dbg_help_dll, "SymCleanup" ); 
		sym_get_module_infow64 = ( SYMGETMODULEINFOW64 )GetProcAddress( dbg_help_dll, "SymGetModuleInfoW64" ); 
		sym_get_sym_from_addr64 = ( SYMGETSYMFROMADDR64 )GetProcAddress( dbg_help_dll, "SymGetSymFromAddr64" ); 
		sym_und_name64 = ( SYMUNDNAME64 )GetProcAddress( dbg_help_dll, "SymUnDName64" ); 
		undecorate_symbol_name = ( UNDECORATESYMBOLNAME )GetProcAddress( dbg_help_dll, "UnDecorateSymbolName" ); 
		sym_load_module_ex_w = ( SYMLOADMODULEEXW )GetProcAddress( dbg_help_dll, "SymLoadModuleExW" ); 
		sym_find_file_in_path_w = ( SYMFINDFILEINPATHW )GetProcAddress( dbg_help_dll, "SymFindFileInPathW" ); 
		sym_set_search_path_w = ( SYMSETSEARCHPATHW )GetProcAddress( dbg_help_dll, "SymSetSearchPathW" ); 
		
		if( sym_initialize_w == NULL 
			|| sym_get_optioins == NULL 
			|| sym_set_optioins == NULL 
			|| sym_cleanup == NULL 
			|| sym_get_module_infow64 == NULL 
			|| sym_get_sym_from_addr64 == NULL 
			|| sym_load_module_ex_w == NULL 
			|| sym_find_file_in_path_w == NULL 
			|| sym_set_search_path_w == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( *event_analyze_help.symbol_path == L'\0' )
		{
			{
				set_default_symbol_path(); 
			}
		}

	}while( FALSE );

	if( ERROR_SUCCESS != ret )
	{
		if( NULL != dbg_help_dll )
		{
			FreeLibrary( dbg_help_dll ); 
			dbg_help_dll = NULL; 
		}
	}

	return ret; 
}

LRESULT unload_dbg_help()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		if( dbg_help_dll == NULL )
		{
			DBG_BP(); 
			ret = ERROR_NOT_READY; 
			break; 
		}

		_ret = FreeLibrary( dbg_help_dll ); 
		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			//break; 
		}

		dbg_help_dll = NULL; 
	} while ( FALSE );

	return ret; 
}

/***************************************************************************************
得到堆栈跟踪信息的效能是最重要的一个影响性能的指标，完全直接利用DBGHELP的速度不可以做到
实时。
可以使用的解决方案为:
1.只记录堆栈的数据记录，不在跟踪中进行翻译，在跟踪停止时，可以手动进行全部跟踪出的事件的
翻译。
2.分析堆栈解释的本质过程，DBGHELP效率低下的原因，如是否由连网操作产生。手动定制解释过程。
占用时间将会很多。
***************************************************************************************/

/***************************************************************************************
分析的核心方法是实时分析与静态分析两种，静态分析可以进行高负载的复杂分析活动。
***************************************************************************************/
LRESULT _get_stack_trace_dump_text( ULONG proc_id, 
								   PLARGE_INTEGER time, 
								   ULONGLONG call_stack[],  
								   ULONG frame_count, 
								   LPWSTR stack_dump, 
								   ULONG ccb_buf_len, 
								   ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#ifdef _DEBUG
		memset( stack_dump, 0xcc, ccb_buf_len < 1024 ? ccb_buf_len : 1024 ); 
#endif //_DEBUG

		ret = get_stack_trace_dump_text_ex( proc_id, 
			time, 
			call_stack, 
			frame_count, 
			event_analyze_help.symbol_path, 
			stack_dump, 
			ccb_buf_len, 
			ccb_ret_len ); 

		if( ret != ERROR_SUCCESS || *stack_dump == L'\0' )
		{}

#ifdef _DEBUG
		{
			INT32 i; 
			INT32 check_size = ( ccb_buf_len < 1024 ? ccb_buf_len : 1024 ); 

			for( i = 0; i < check_size; i ++ )
			{
				if( ( ( BYTE* )stack_dump )[ i ] != 0xcc )
				{
					break; 
				}
			}

			if( i >= check_size )
			{
				DBG_BP(); 
			}
		}
#endif //_DEBUG

	}while( FALSE );

	return ret; 
}

NTSTATUS stack_back_trace( PVOID call_stack[], ULONG frame_count, PULONG frame_count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		if( call_stack == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( frame_count == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		if( frame_count_out == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_3; 
			break; 
		}


	}while( FALSE );

	return ntstatus; 
}
/******************************************************************************************
if process exist, use the process information to enum all module is good method.but if process
exist then must recorded all the modules name and its address region.
1.get ring3 modules of one process:take a tool snapshot to get all the modules of one process.
2.get ring0 modules: retrieve all module information from trace driver. or call the nt function
	QuerySystemInformation to get the system module information.
******************************************************************************************/
/*LRESULT*/ 

LRESULT load_kernel_modules_symbol()
{
	LRESULT ret = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE ); 

	return ret; 
}

/*******************************************************************************
堆栈回溯的设计:
1.建立系统内核模块区
2.建立应用程序模块区
以上两个区在模块的加载和卸载时,分别进行维护,保证在一个特定的时间中,模块列表是
最新的.

在事件发生时,将事件对应的模块做成列表记录下来.
此方案可以提供在事件时点的,行为模块比较精确记录(事件接收时延为误差可能发生的时间).

第2个方案,不进行模块列表维护,在需要显示事件堆栈回溯时,即时进行模块信息加载.误差较
大.不同时间,结果可能不一样.
*******************************************************************************/

LRESULT WINAPI resolve_symbol( __in HANDLE hProcess, 
//#ifdef _DEBUG_64BIT
					   __in ULONGLONG dwAddress,
//#else
//					   __in UINT_PTR dwAddress,
//#endif //_DEBUG_64BIT

						  DUMP_SYMBOL_INFO &siSymbol )
{
	LRESULT _ret; 
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	CHAR szUndec[SYMBOL_NAME_LEN];
	CHAR szWithOffset[SYMBOL_NAME_LEN];
	LPSTR pszSymbol = NULL;
	IMAGEHLP_MODULEW64 mi; 
	ULONG ccb_ret_len; 

	memset( &siSymbol, 0, sizeof( DUMP_SYMBOL_INFO ) );
	mi.SizeOfStruct = sizeof( IMAGEHLP_MODULEW64 );

	siSymbol.dwAddress = dwAddress;

	if( !sym_get_module_infow64( hProcess, dwAddress, &mi ) ) 

	{
		_ret = GetLastError(); 

		*siSymbol.szModule = L'\0'; 
	}
	else
	{
		LPWSTR pszModule = wcsrchr( mi.ImageName, L'\\' );
		if( pszModule == NULL )
		{
			pszModule = mi.ImageName;
		}
		else
		{
			pszModule ++; 
		}

		hr = StringCbCopyW( siSymbol.szModule, 
			sizeof( siSymbol.szModule ),  
			pszModule ); //_TRUNCATE 
		
		if( FAILED( hr ) )
		{

		}
	}

	__try
	{
		IMAGEHLP_SYMBOL64_PACKAGE sym_pack; 

		memset( &sym_pack.sym, 0, sizeof( sym_pack.sym ) ); 

		sym_pack.sym.SizeOfStruct = sizeof( IMAGEHLP_SYMBOL64 ); 

#ifdef _WIN64
		sym_pack.sym.Address = dwAddress;
#else
		sym_pack.sym.Address = ( DWORD )dwAddress; 
#endif
		sym_pack.sym.MaxNameLength = MAX_SYM_NAME; 

#ifdef _WIN64
		if( sym_get_sym_from_addr64( hProcess, dwAddress, &( siSymbol.dwOffset ), &sym_pack.sym ) ) 
#else
		if( sym_get_sym_from_addr64( hProcess, dwAddress, &( siSymbol.dwOffset ), &sym_pack.sym ) )
#endif
		{
			pszSymbol = sym_pack.sym.Name; 

			if (undecorate_symbol_name(sym_pack.sym.Name, szUndec, sizeof(szUndec)/sizeof(szUndec[0]), 
				UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS))
			{
				pszSymbol = szUndec;
			}
			else if (sym_und_name64(&sym_pack.sym, szUndec, sizeof(szUndec)/sizeof(szUndec[0])))
			{
				pszSymbol = szUndec;
			}
			else
			{
				_ret = GetLastError(); 
			}

			if( siSymbol.dwOffset != 0 )
			{
#if _SECURE_ATL
				sprintf_s( szWithOffset, SYMBOL_NAME_LEN, "%s + %08x", pszSymbol, siSymbol.dwOffset );
#else
				_snprintf( szWithOffset, SYMBOL_NAME_LEN, "%s + %08x", pszSymbol, siSymbol.dwOffset );
#endif

				// ensure null-terminated
				szWithOffset[ SYMBOL_NAME_LEN - 1 ] = '\0'; 

				pszSymbol = szWithOffset;
			}
		}
		else
		{
			pszSymbol = ""; //"<no symbol>";
			_ret = GetLastError(); 
		}
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		_ret = EXCEPTION_ACCESS_VIOLATION; 
		pszSymbol = ""; //"<EX: no symbol>";
		siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
	}

	siSymbol.szSymbol[ ARRAYSIZE( siSymbol.szSymbol ) - 1 ] = L'\0'; 

	ret = mcbs_to_unicode( pszSymbol, 
		siSymbol.szSymbol, 
		ARRAYSIZE( siSymbol.szSymbol ), 
		&ccb_ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		siSymbol.szSymbol[ 0 ] = L'\0'; 
	}

	ASSERT( siSymbol.szSymbol[ ccb_ret_len - 1 ] == L'\0' ); 

	return ret; 
}

#define PDB_FILE_EXTENSION_NAME L".pdb" 

LRESULT WINAPI get_pe_pdb_file_name( HANDLE symbol_handle, 
							 LPCWSTR file_name, 
							 LPWSTR pdb_file_name, 
							 ULONG cc_buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPCWSTR extension_name; 
	ULONG extension_name_len; 

	do 
	{
		extension_name = wcsrchr( file_name, L'.' ); 
		if( extension_name == NULL )
		{
			extension_name_len = wcslen( file_name ); 
		}
		else
		{
			extension_name_len = ( LPCWSTR )extension_name - ( LPCWSTR )file_name; 
		}

		if( extension_name_len + ARRAYSIZE( PDB_FILE_EXTENSION_NAME ) > cc_buf_len )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		memcpy( pdb_file_name, 
			file_name, 
			extension_name_len << 1 ); 
		memcpy( pdb_file_name + extension_name_len, 
			PDB_FILE_EXTENSION_NAME, 
			sizeof( PDB_FILE_EXTENSION_NAME ) ); 
	
	}while( FALSE );

	return ret; 
}

LRESULT resolve_symbol_ex( HANDLE symbol_handle, 
						  MODULE_INFO *module_info, 
						  //#ifdef _DEBUG_64BIT
						  __in ULONGLONG dwAddress,
						  //#else
						  //					   __in UINT_PTR dwAddress,
						  //#endif //_DEBUG_64BIT
						  DUMP_SYMBOL_INFO &siSymbol )
{
	LRESULT         _ret; 
	LRESULT         ret = ERROR_SUCCESS; 
	HRESULT         hr; 
	CHAR            szUndec[ SYMBOL_NAME_LEN ];
	CHAR            szWithOffset[ SYMBOL_NAME_LEN ];
	LPSTR           pszSymbol = NULL; 
	INT				i = 0; 
	PULONG_PTR		pW32pServiceTable = 0;
	DWORD64			dwload = 0; 
	LPWSTR          pszModule; 
	BOOLEAN         symol_handle_inited = FALSE; 

	ULONG ccb_ret_len; 

	siSymbol.dwAddress = dwAddress;

	pszModule = wcsrchr( module_info->file_name, L'\\' );
	if( pszModule == NULL )
	{
		pszModule = module_info->file_name;
	}
	else
	{
		pszModule ++; 
	}

	hr = StringCbCopyW( siSymbol.szModule, 
		sizeof( siSymbol.szModule ),  
		pszModule ); //_TRUNCATE 

	if( FAILED( hr ) )
	{

	}

	dwload = sym_load_module_ex_w( symbol_handle, 
		NULL, 
		module_info->file_name, 
		NULL, 
		( ULONG64 )module_info->base_addr, 
		module_info->size, 
		NULL, 
		0 ); 

	if( dwload == 0 )
	{
		ret = GetLastError(); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}
	}

	__try
	{
		IMAGEHLP_SYMBOL64_PACKAGE sym_pack; 

		memset( &sym_pack.sym, 0, sizeof( sym_pack.sym ) ); 

		sym_pack.sym.SizeOfStruct = sizeof( IMAGEHLP_SYMBOL64 ); 

#ifdef _WIN64
		sym_pack.sym.Address = dwAddress;
#else
		sym_pack.sym.Address = ( DWORD )dwAddress; 
#endif
		sym_pack.sym.MaxNameLength = MAX_SYM_NAME; 

#ifdef _WIN64
		if( sym_get_sym_from_addr64( symbol_handle, dwAddress, &( siSymbol.dwOffset ), &sym_pack.sym ) ) 
#else
		if( sym_get_sym_from_addr64( symbol_handle, dwAddress, &( siSymbol.dwOffset ), &sym_pack.sym ) )
#endif
		{
			pszSymbol = sym_pack.sym.Name; 

			if( siSymbol.dwOffset != 0 )
			{
#if _SECURE_ATL
				sprintf_s( szWithOffset, SYMBOL_NAME_LEN, "%s + %08x", pszSymbol, siSymbol.dwOffset );
#else
				_snprintf( szWithOffset, SYMBOL_NAME_LEN, "%s + %08x", pszSymbol, siSymbol.dwOffset );
#endif

				// ensure null-terminated
				szWithOffset[ SYMBOL_NAME_LEN - 1 ] = '\0'; 

				pszSymbol = szWithOffset;
			}
		}
		else
		{
			pszSymbol = ""; //"<no symbol>";
			_ret = GetLastError(); 
		}
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
	{
		_ret = EXCEPTION_ACCESS_VIOLATION; 
		pszSymbol = ""; //"<EX: no symbol>";
		siSymbol.dwOffset = dwAddress - ( ULONG_PTR )module_info->base_addr;
	}


	siSymbol.szSymbol[ ARRAYSIZE( siSymbol.szSymbol ) - 1 ] = L'\0'; 

	ret = mcbs_to_unicode( pszSymbol, 
		siSymbol.szSymbol, 
		ARRAYSIZE( siSymbol.szSymbol ), 
		&ccb_ret_len ); 

	if( ret != ERROR_SUCCESS )
	{
		siSymbol.szSymbol[ 0 ] = L'\0'; 
	}

	ASSERT( siSymbol.szSymbol[ ccb_ret_len - 1 ] == L'\0' ); 

	//strncpy_s( siSymbol.szSymbol, _countof( siSymbol.szSymbol ), pszSymbol, _TRUNCATE ); 

_return:
	return ret; 
}

CRITICAL_SECTION sym_op_lock = { 0 }; 
LRESULT WINAPI init_sym_context()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOLEAN module_info_inited = FALSE; 
	BOOLEAN dbg_help_loaded = FALSE; 
	WCHAR dbg_help_dll_path[ MAX_PATH ]; 

	do 
	{
		init_cs_lock( sym_op_lock ); 
		ret = init_module_infos(); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		module_info_inited = TRUE; 

		if( *event_analyze_help.dbg_help_path != L'\0' )
		{
			ret = load_dbg_help( event_analyze_help.dbg_help_path ); 
		}
		else
		{
			ret = add_app_path_to_file_name( dbg_help_dll_path, 
				ARRAY_SIZE( dbg_help_dll_path ), 
#ifdef _WIN64
				L"dbghelp64.dll", 
#else
				L"dbghelp32.dll", 
#endif //_WIN64	

				FALSE ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = load_dbg_help( dbg_help_dll_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		dbg_help_loaded = TRUE; 
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( module_info_inited == TRUE )
		{
			uninit_module_infos(); 
		}

		uninit_cs_lock( sym_op_lock ); 
	}

	return ret; 
}

LRESULT WINAPI uninit_sym_context()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		uninit_cs_lock( sym_op_lock ); 

		ret = uninit_module_infos(); 

	}while( FALSE );

	return ret; 
}

#define JUST_TEST
#ifdef JUST_TEST
#pragma comment(linker,"/export:MessageBoxA=kernel32.dll.MessageBoxA")
#endif //JUST_TEST
NTSTATUS WINAPI _dump_stack( ULONG proc_id, PVOID call_stack[], ULONG frame_count )
{
	return dump_stack( proc_id, call_stack, frame_count, event_analyze_help.dbg_help_path ); 
}

NTSTATUS WINAPI dump_stack( ULONG proc_id, PVOID call_stack[], ULONG frame_count, LPCWSTR sym_path )
{
	NTSTATUS ntstatus = ERROR_SUCCESS; 
	ULONG _ret; 
	INT32 __ret; 
	HANDLE symbol_handle = NULL; 
	//ULONG proc_id; 
	DWORD sym_option; 
	DUMP_SYMBOL_INFO info; 
	LPWSTR module_name; 
	LPWSTR sym_name; 
	BOOLEAN lock_held = FALSE; 
	BOOLEAN sym_inited = FALSE; 
	INT32 i; 
	//WCHAR path[ MAX_PATH ]; 

	do
	{
		if( call_stack == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}
		
		if( frame_count == 0 )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}

		symbol_handle = BITTRACE_SYMBOL_HANDLER_ID; 

#define SYM_OPTION ( SYMOPT_CASE_INSENSITIVE | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_AUTO_PUBLICS ) 

		lock_cs( sym_op_lock ); 
		lock_held = TRUE; 

		sym_option = sym_get_optioins(); 
		sym_option |= ( SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES ); 
		sym_option &= ~SYMOPT_UNDNAME; 
		sym_set_optioins( sym_option ); 

		ASSERT( sym_initialize_w != NULL ); 

		__ret = sym_initialize_w( symbol_handle, ( PWSTR )sym_path, FALSE ); 

		if( __ret == FALSE )
		{
			_ret = GetLastError(); 

			log_trace( ( MSG_ERROR, "initialize the symbol function error %u\n", _ret ) ); 
			break; 
		}

		sym_inited = TRUE;  

		for( i = 0; ( ULONG )i < frame_count; i ++ )
		{
			__ret = resolve_symbol( symbol_handle, ( UINT_PTR )call_stack[ i ], info); 

			module_name = info.szModule; 
			sym_name = info.szSymbol; 

			log_trace( ( MSG_INFO, "%s:%s\n", module_name, sym_name ) ); 
		}

	}while( FALSE ); 

	if( sym_inited == TRUE )
	{
		ASSERT( symbol_handle != NULL ); 
		sym_cleanup( symbol_handle ); 
	}

	if( lock_held == TRUE )
	{
		unlock_cs( sym_op_lock ); 
	}

	return ntstatus; 
}

/***************************************************************************************
有两种符号解析的方式: 
1.一种直接使用系统自带dbghelp.dll
2.一种使用自编的dbghelp: 
	可以使用reactos的dbghelp进行源码编译。
3.不对reactos的dbghelp进行尝试的定顁，对性能的回不效果不大。

符号加载的问题:
1.如果只用模块名+偏移地址不可以直接系统DBGHELP解析符号信息。
***************************************************************************************/
LRESULT WINAPI get_stack_trace_dump_text( ULONG proc_id, 
								  ULONGLONG call_stack[],  							  
								  ULONG frame_count, 
								  LPWSTR sym_path, 
								  LPWSTR stack_dump, 
								  ULONG ccb_buf_len, 
								  ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HRESULT hr; 
	ULONG outputed = 0;  
	ULONG __ret; 
	HANDLE proc_handle = NULL; 
	DUMP_SYMBOL_INFO info; 
	LPWSTR module_name; 
	LPWSTR sym_name; 
	BOOLEAN lock_held = FALSE; 
	BOOLEAN sym_inited = FALSE; 
	INT32 i; 
	size_t ccb_remain_len; 

	do 
	{
		ASSERT( call_stack != NULL );  
		ASSERT( stack_dump != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 

		if( ccb_ret_len != NULL )
		{
			*ccb_ret_len = 0; 
		}

		if( call_stack == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( frame_count == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*stack_dump = L'\0'; 

		proc_handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, proc_id ); 
		if( proc_handle == NULL )
		{
			_ret = GetLastError(); 

			log_trace( ( MSG_ERROR, "open process error %u\n", _ret ) ); 

			proc_handle = ( HANDLE )call_stack; 
		}
		lock_cs( sym_op_lock ); 
		lock_held = TRUE; 

		{
			DWORD sym_option; 

			sym_option = SYM_OPTION; 
			sym_set_optioins( sym_option ); 
		}

		__ret = sym_initialize_w( proc_handle, ( PWSTR )sym_path, TRUE ); 

		if( __ret == FALSE )
		{
			_ret = GetLastError(); 

			log_trace( ( MSG_ERROR, "initialize the symbol function error %u\n", _ret ) ); 
			
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		sym_inited = TRUE; 

		for( i = 0; ( ULONG )i < frame_count; i ++ )
		{
			do 
			{
				do 
				{
					__ret = resolve_symbol( proc_handle, ( UINT_PTR )call_stack[ i ], info); 

					if( __ret != ERROR_SUCCESS  )
					{
						break; 
					}

#if 0
					__ret = format_sql_string_value( info.szModule, ARRAYSIZE( info.szModule ), &cc_value_len ); 
					if( __ret != ERROR_SUCCESS )
					{
						break; 
					}

					__ret = format_sql_string_value( info.szSymbol, ARRAYSIZE( info.szSymbol ), &cc_value_len ); 
					if( __ret != ERROR_SUCCESS )
					{
						break; 
					}
#endif //0
				}while( FALSE );

				module_name = info.szModule; 
				sym_name = info.szSymbol; 

				if( *module_name == L'\0' )
				{
					module_name = L"<unknown>"; 
				}

				if( ccb_buf_len < outputed + 1 )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}

				if( *sym_name == L'\0' )
				{
					hr = StringCchPrintfExW( stack_dump + outputed, 
						ccb_buf_len - outputed - 1, 
						NULL, 
						&ccb_remain_len, 
						0, 
						L"%u %s:%p\r\n", 
						i, 
						module_name, 
						call_stack[ i ] ); 
				}
				else
				{
					hr = StringCchPrintfExW( stack_dump + outputed, 
						ccb_buf_len - outputed - 1, 
						NULL, 
						&ccb_remain_len, 
						0, 
						L"%u %s:%s\r\n", 
						i, 
						module_name, 
						sym_name ); 
				}

				if( FAILED( hr ) )
				{
					if( STRSAFE_E_INSUFFICIENT_BUFFER == hr )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						stack_dump[ ccb_buf_len - 1 ] = L'\0'; 
						break; 
					}
					else
					{
						DBG_BP(); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}
				}
				else
				{
					ASSERT( ccb_remain_len < ( ccb_buf_len - outputed ) ); 
					outputed = ccb_buf_len - ( ULONG )ccb_remain_len - 1; 
				}
			}while( FALSE ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
	}while( FALSE ); 

	if( sym_inited == TRUE )
	{
		ASSERT( proc_handle != NULL ); 
		sym_cleanup( proc_handle ); 
	}

	if( lock_held == TRUE )
	{
		unlock_cs( sym_op_lock ); 
	}

	if( proc_handle != NULL 
		&& proc_handle != call_stack )
	{
		CloseHandle( proc_handle ); 
	}

	return ret; 
}

LRESULT WINAPI get_stack_trace_dump_text_ex( ULONG proc_id, 
										 PLARGE_INTEGER time, 
										 ULONGLONG call_stack[],  							  
										 ULONG frame_count, 
										 LPWSTR sym_path, 
										 LPWSTR stack_dump, 
										 ULONG ccb_buf_len, 
										 ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HRESULT hr; 
	ULONG outputed = 0;  
	ULONG __ret; 
	HANDLE symbol_handle = NULL; 
	//WCHAR *remain_buf; 
	DUMP_SYMBOL_INFO info; 
	LPWSTR module_name; 
	LPWSTR sym_name; 
	BOOLEAN lock_held = FALSE; 
	BOOLEAN sym_inited = FALSE; 
	INT32 i; 
	size_t ccb_remain_len; 
	//ULONG cc_value_len; 
	MODULE_INFO_TABLE *proc_modules = NULL; 
	MODULE_INFO_TABLE *kern_modules = NULL; 
	MODULE_INFO_ITERATOR it; 

	MODULE_INFO *_module_info; 
	MODULE_INFO *module_info; 

	do 
	{
		ASSERT( call_stack != NULL ); 
		ASSERT( stack_dump != NULL ); 
		ASSERT( ccb_buf_len > 0 ); 

		if( ccb_ret_len != NULL )
		{
			*ccb_ret_len = 0; 
		}

		if( call_stack == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( frame_count == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( time == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		*stack_dump = L'\0'; 

		symbol_handle = BITTRACE_SYMBOL_HANDLER_ID; 

		lock_cs( sym_op_lock ); 
		lock_held = TRUE; 

		sym_set_optioins( SYM_OPTION ); 

		__ret = sym_initialize_w( symbol_handle, ( PWSTR )sym_path, FALSE ); 

		if( __ret == FALSE )
		{
			_ret = GetLastError(); 

			log_trace( ( MSG_ERROR, "initialize the symbol function error %u\n", _ret ) ); 

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		sym_inited = TRUE; 

		kern_modules = get_kernel_modules(); 

		_ret = get_proc_module_info( proc_id, time, &proc_modules ); 
		if( _ret != ERROR_SUCCESS 
			&& _ret != ERROR_NOT_FOUND ) 
		{
			break; 
		}

#define EVENTMON_INNER_FRAME_COUNT 1
		for( i = EVENTMON_INNER_FRAME_COUNT; ( ULONG )i < frame_count; i ++ )
		{
			module_info = NULL; 

			do 
			{
				do 
				{
					if( proc_modules == NULL )
					{
						break; 
					}

					for( it = proc_modules->modules.begin(); it != proc_modules->modules.end(); it ++ )
					{
						_module_info = ( *it ); 

						if( _module_info->base_addr <= ( PVOID )( UINT_PTR )call_stack[ i ] 
						&& ( PVOID )( UINT_PTR )call_stack[ i ] <= ( PBYTE )_module_info->base_addr + _module_info->size )
						{
							if( ( _module_info->duration.end_time.QuadPart == 0 
								|| _module_info->duration.end_time.QuadPart >= time->QuadPart )
								&& _module_info->duration.start_time.QuadPart <= time->QuadPart )
							{
								module_info = _module_info; 
								break; 
							}
						}
					}
				}while( FALSE ); 

				do 
				{
					if( module_info != NULL )
					{
						break; 
					}

					for( it = kern_modules->modules.begin(); it != kern_modules->modules.end(); it ++ )
					{
						_module_info = ( *it ); 

						if( _module_info->base_addr <= ( PVOID )( UINT_PTR )call_stack[ i ] 
						&& ( PVOID )( UINT_PTR )call_stack[ i ] <= ( PBYTE )_module_info->base_addr + _module_info->size )
						{
							if( ( _module_info->duration.end_time.QuadPart == 0 
								|| _module_info->duration.end_time.QuadPart >= time->QuadPart )
								&& _module_info->duration.start_time.QuadPart <= time->QuadPart ) 
							{
								module_info = _module_info; 
								break; 
							}
						}
					}
				}while( FALSE ); 

				*info.szModule = L'\0'; 
				*info.szSymbol = L'\0'; 

				do 
				{
					if( module_info == NULL )
					{
						info.dwOffset = ( ULONG )call_stack[ i ]; 
						break; 
					}

					info.dwOffset = ( ULONG )( ULONG_PTR )( ( BYTE* )call_stack[ i ] - ( BYTE* )module_info->base_addr ); 

					__ret = resolve_symbol_ex( symbol_handle, 
						module_info, 
						call_stack[ i ], 
						info ); 

					if( __ret != ERROR_SUCCESS )
					{
						//DBG_BP(); 
						break; 
					}
				}while( FALSE );

				module_name = info.szModule; 
				sym_name = info.szSymbol; 

				if( *module_name == L'\0' )
				{
					module_name = L"<unknown>"; 
				}

				if( ccb_buf_len < outputed + 1 )
				{
					//hr = MEM_E_INVALID_SIZE; 
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}

				if( *sym_name == L'\0' )
				{
					hr = StringCchPrintfExW( stack_dump + outputed, 
						ccb_buf_len - outputed - 1, 
						NULL, 
						&ccb_remain_len, 
						0, 
						L"%u %s:%0.8x\r\n", 
						i, 
						module_name, 
						info.dwOffset ); 
				}
				else
				{
					hr = StringCchPrintfExW( stack_dump + outputed, 
						ccb_buf_len - outputed - 1, 
						NULL, 
						&ccb_remain_len, 
						0, 
						L"%u %s:%s\r\n", 
						i, 
						module_name, 
						sym_name ); 
				}

				if( FAILED( hr ) )
				{
					if( STRSAFE_E_INSUFFICIENT_BUFFER == hr )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						stack_dump[ ccb_buf_len - 1 ] = L'\0'; 
						break; 
					}
					else
					{
						DBG_BP(); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}
				}
				else
				{
					ASSERT( ccb_remain_len < ( ccb_buf_len - outputed ) ); 
					outputed = ccb_buf_len - ( ULONG )ccb_remain_len - 1; 
				}

				if( ret != ERROR_SUCCESS )
				{ 
					break; 
				}
			}while( FALSE );
		}

	}while( FALSE ); 

	if( sym_inited == TRUE )
	{
		ASSERT( symbol_handle != NULL ); 
		sym_cleanup( symbol_handle ); 
	}

	if( NULL != proc_modules )
	{
		release_proc_module_info( proc_modules ); 
	}

	if( lock_held == TRUE )
	{
		unlock_cs( sym_op_lock ); 
	}

	return ret; 
}

LRESULT WINAPI load_module_symbol( LPCWSTR file_name )
{
    LRESULT ret = ERROR_SUCCESS; 
    return ret; 
}