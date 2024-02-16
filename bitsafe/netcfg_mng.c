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
 
#include "common_func.h"
#include "netcfg_mng.h"

INT32 netcfg_on = FALSE; 
#define REGSVR_EXE_NAME _T( "regsvr32 " )
#define NETCFGX_MOD_NAME _T( "Netcfgx.dll" )
#define NETMAN_MOD_NAME _T( "Netman.dll" )
#define NETSHELL_MOD_NAME _T( "Netshell.dll" )
#define NETCFGX_OPERA  REGSVR_EXE_NAME NETCFGX_MOD_NAME
#define NETMAN_OPERA REGSVR_EXE_NAME NETMAN_MOD_NAME
#define NETSHELL_OPERA REGSVR_EXE_NAME NETSHELL_MOD_NAME
#define SWITCH_OFF _T( " /u" )
#define SWITCH_ON _T( "" )

#define UNREG_DLL_SEVER_FUNC_NAME "DllUnregisterServer"
#define REG_DLL_SEVER_FUNC_NAME "DllRegisterServer"

typedef LRESULT ( *dll_unreg_func )(); 

HANDLE netcfg_switch_thread = NULL; 

LRESULT switch_netcfg( INT32 is_on )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	//_ret = CoInitialize( NULL ); 
	//if( FAILED( _ret ) )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	ret = com_reg_switch( NETSHELL_MOD_NAME, is_on ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = com_reg_switch( NETMAN_MOD_NAME, is_on ); ; 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	ret = com_reg_switch( NETSHELL_MOD_NAME, is_on ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		goto _return; 
	}

	//CoUninitialize(); 

_return:
	return ret; 
}

DWORD CALLBACK thread_netcfg_switch( PVOID param )
{
	for( ; ; )
	{
		//if( netcfg_on == FALSE )
		//{
		//	//WinExec( NETCFGX_OPERA SWITCH_OFF, SW_HIDE ); 
		//	//WinExec( NETMAN_OPERA SWITCH_OFF, SW_HIDE ); 
		//	//WinExec( NETSHELL_OPERA SWITCH_OFF, SW_HIDE ); 
		//	switch_netcfg( FALSE );  

		//}
		//else 
		//{
		//	//WinExec( NETCFGX_OPERA SWITCH_ON, SW_HIDE ); 
		//	//WinExec( NETMAN_OPERA SWITCH_ON, SW_HIDE ); 
		//	//WinExec( NETSHELL_OPERA SWITCH_ON, SW_HIDE ); 

		//	switch_netcfg( TRUE ); 
		//}

		switch_netcfg( netcfg_on ); 

		Sleep( 20000 ); 
	}

	return 0; 
}

LRESULT CALLBACK com_reg_switch( LPCTSTR dll_name, INT32 is_reg )
{
	LRESULT ret = ERROR_SUCCESS; 
	HMODULE dll_mod = NULL; 
	dll_unreg_func unreg_dll = NULL; 

	ASSERT( dll_name != NULL ); 

	//if( is_reg == TRUE )
	//{
	//	Register( dll_name ); 
	//}
	//else 
	//{
	//	UnRegister( dll_name ); 
	//}

	dll_mod = LoadLibraryEx( dll_name, NULL, 0 ); 
	if( dll_mod == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( is_reg == FALSE )
	{
		unreg_dll = ( dll_unreg_func )GetProcAddress( dll_mod, UNREG_DLL_SEVER_FUNC_NAME ); 
	}
	else 
	{
		unreg_dll = ( dll_unreg_func )GetProcAddress( dll_mod, REG_DLL_SEVER_FUNC_NAME  ); 
	}

	ret = unreg_dll(); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "switch net configure function failed 0x%0.8x\n", ret ) ); 
	}

	FreeLibrary( dll_mod ); 

_return:
	return ret; 
}

LRESULT start_netcfg_manage()
{
	LRESULT ret = ERROR_SUCCESS; 

	netcfg_switch_thread = CreateThread( NULL, 0, thread_netcfg_switch, NULL, 0, NULL ); 
	if( netcfg_switch_thread == NULL )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "manage the net configure function failed\n" ) ); 
	}

_return:
	return ret; 
}

LRESULT start_net_cfg()
{
	LRESULT ret = ERROR_SUCCESS; 

	netcfg_on = TRUE; 
	ret = switch_netcfg( netcfg_on ); 

	return ret; 
}

LRESULT stop_net_cfg()
{
	LRESULT ret = ERROR_SUCCESS; 
	netcfg_on = FALSE; 
	ret = switch_netcfg( netcfg_on ); 
	return ret; 
}