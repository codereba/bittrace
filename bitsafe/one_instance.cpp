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
#define NO_UILIB_SUPP
#include "bitsafe_common.h"
#include "one_instance.h"

#define RECT_HEIGHT( _rc_ ) _abs( ( _rc_ ).bottom - ( _rc_ ).top )
#define RECT_WIDTH( _rc_ ) _abs( ( _rc_ ).right - ( _rc_ ).left )

HWND find_inst_main_wnd( INT32 is_update )
{
	HWND wnd_find = NULL; 
	LPCTSTR prop_name = NULL; 

	if( is_update == TRUE )
	{
		prop_name = BITSAFE_UPDATE_INST_PROP_NAME; 
	}
	else
	{
		prop_name = BITSAFE_INST_PROP_NAME; 
	}

	wnd_find = ::GetWindow( ::GetDesktopWindow(), GW_CHILD );

	while( ::IsWindow( wnd_find ) )
	{
		if( ::GetProp( wnd_find, prop_name ) )
		{    
			goto _return; 
		}

		wnd_find = ::GetWindow( wnd_find, GW_HWNDNEXT );
	}  

	wnd_find = NULL; 

_return:
	return wnd_find; 
}

LRESULT kill_app_instance( INT32 is_update )
{
	LRESULT ret = ERROR_SUCCESS; 
	HWND wnd_find; 
	LRESULT _ret; 
	DWORD thread_id; 
	DWORD process_id; 
	HANDLE process = NULL; 
	DWORD wait_ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	wnd_find = find_inst_main_wnd( is_update ); 

	if( wnd_find == NULL )
	{
		ret = ERROR_MAIN_WND_PROC_NOT_FOUND; 
		goto _return; 
	}

	thread_id = GetWindowThreadProcessId( wnd_find, &process_id ); 

	process = OpenProcess( SYNCHRONIZE, FALSE, process_id ); 

	if( process == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_ret = SendMessage( wnd_find, WM_CLOSE, 0, 0 ); 
	
	wait_ret = WaitForSingleObject( process, 1000 ); 
	if( wait_ret == WAIT_OBJECT_0 
		|| wait_ret == WAIT_ABANDONED )
	{
		goto _return; 
	}

	if( wait_ret == WAIT_FAILED )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process failed \n", GetLastError() ) );  
	}

	_ret = PostThreadMessage( thread_id, WM_QUIT, 0, 0 ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	wait_ret = WaitForSingleObject( process, 1000 ); 
	if( wait_ret == WAIT_OBJECT_0 
		|| wait_ret == WAIT_ABANDONED )
	{
		goto _return; 
	}

	if( wait_ret == WAIT_FAILED )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait process failed \n", GetLastError() ) );  
	}

	TerminateProcess( process, 0 ); 

_return:

	if( process != NULL )
	{
		CloseHandle( process ); 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT check_app_instance( ULONG flags )
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 
	INT32 _ret; 
	HWND wnd_find; 
	LPCTSTR sem_name; 
	HANDLE test_sem; 

	if( flags == UPDATE_INSTANCE )
	{
		sem_name = BITSAFE_UPDATE_INST_TIP; 
	}
	else if( flags == DAEMON_INSTANCE )
	{
		sem_name = BITSAFE_DAEMON_INST_TIP; 
	}
	else
	{
		sem_name = BITSAFE_INST_TIP; 
	}

	test_sem = CreateSemaphore( NULL, 1, 1, sem_name ); 

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		RECT wnd_rc; 
		CloseHandle( test_sem );  

		if( flags == DAEMON_INSTANCE )
		{
			ret = ERROR_SUCCESS; 
			goto _return; 
		}

		wnd_find = find_inst_main_wnd( !!( flags & UPDATE_INSTANCE ) ); 
		if( wnd_find == NULL )
		{
			ret = ERROR_NOT_FOUND; 
			goto _return; 
		}

		_ret = GetWindowRect( wnd_find, &wnd_rc ); 

		if( _ret == FALSE 
			|| ( 0 == RECT_WIDTH( wnd_rc ) )
			|| ( 0 == RECT_HEIGHT( wnd_rc ) )
			|| FALSE == IsWindowVisible( wnd_find ) )
		{
			::ShowWindow( wnd_find, SW_RESTORE );    
		}

		::SetForegroundWindow( wnd_find );
		::SetForegroundWindow( ::GetLastActivePopup( wnd_find ) ); 

		ret = ERROR_SUCCESS; 
	}

_return:
	if( ret == ERROR_NOT_FOUND )
	{
		log_trace( ( MSG_ERROR, "the application isntance semaphore is created, but the window tag is not still.\n" ) ); 
		ret = ERROR_SUCCESS; 
	}
	return ret;
}

LRESULT set_app_flag( HWND wnd, INT32 is_update )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR prop_name; 

	if( is_update == TRUE )
	{
		prop_name = BITSAFE_UPDATE_INST_PROP_NAME; 
	}
	else
	{
		prop_name = BITSAFE_INST_PROP_NAME; 
	}

	ret = SetProp( wnd, prop_name, ( HANDLE )1 ); 

	ret = convert_bool_to_ret( ret ); 

	return ret; 
}

LRESULT remove_app_flag( HWND wnd, INT32 is_update )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR prop_name; 

	if( is_update == TRUE )
	{
		prop_name = BITSAFE_UPDATE_INST_PROP_NAME; 
	}
	else
	{
		prop_name = BITSAFE_INST_PROP_NAME; 
	}

	RemoveProp( wnd, prop_name ); 

	return ret; 
}