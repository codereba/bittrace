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

 
 #include "_stdafx.h"
#include "bitsafe_common.h"
#include "resource.h"
#include "common_func.h"
#include "ring0_2_ring3.h"
#include "bitsafe_analyzer.h"
#include "acl_define.h"
#include "action_type_parse.h"
#include "action_check.h"
#include "sqlite3.h"
#include "action_log_db.h"
#include "action_show_dlg.h"

#define INSTALL_SOURCE_FILE_TYPE L"SOURCE_FILE"
#define TEST_ACTION_COUNT 2
sys_action_record all_test_action[ TEST_ACTION_COUNT ] = { 0, SYS_ACTION_END, { 0 } }; 

extern sqlite3 *common_log_db;  

NTSTATUS CALLBACK test_filter_action_log( sys_action_record *action, action_context *ctx, PVOID data, ULONG data_len, PVOID param, ULONG *state_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	return ntstatus; 
}

NTSTATUS test_action_log()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 
	action_context ctx; 
	param_sel_mode all_conds[ MAX_PARAMS_COUNT ]; 
	param_info all_params[ MAX_PARAMS_COUNT ]; 
	ULONG param_count; 
	INT32 i; 

	do 
	{
		//memcpy( action.do_ba_infect_pe.path_name, L"111", sizeof( ) ); 

		ntstatus = _collect_action_context( &ctx ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}


		all_test_action[ 0 ].type = FILE_write; 
		GetModuleFileNameW( NULL, all_test_action[ 0 ].do_file_write.path_name, 
			ARRAY_SIZE( all_test_action[ 0 ].do_file_write.path_name ) ) ; 
		all_test_action[ 0 ].do_file_write.offset = 100; 
		all_test_action[ 0 ].do_file_write.data_len = 1000; 
		//all_test_action[ 0 ].do_file_write.result = STATUS_SUCCESS; 


#define TEST_WRITE_FILE_DATA "asdfasdfasdf"
		ntstatus = input_action_log_to_db( &ctx, 
			&all_test_action[ 0 ], 
			TEST_WRITE_FILE_DATA, 
			sizeof( TEST_WRITE_FILE_DATA ) ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		ntstatus = get_params_from_action( &all_test_action[ 0 ], 
			all_params, 
			ARRAY_SIZE( all_params ), 
			&param_count ); 

		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

		if( param_count + 1 > ARRAY_SIZE( all_params ) )
		{
			break; 
		}

		//all_params[ param_count ].type = DATA_BLOB_TYPE; 
		//all_params[ param_count ].data.uint64_val = 0; 

		//param_count += 1; 

		for( i = 0; ( ULONG )i < param_count; i ++ )
		{
			all_conds[ i ] = SELECT_AND; 
		}

		ret = output_action_log_from_db( &ctx, 
			FILE_write, 
			all_conds, 
			all_params, 
			param_count, 
			0, 
			0, 
			NULL, 
			test_filter_action_log ); 
	}while( FALSE );

	return ret; 
}

/***********************************************************************
行为分析模式: 
初设计模式: 
1.行为归类模式，将行一级级归类，展开。
2.
***********************************************************************/
#include "volume_name_map.h"
#include "filter_dlg.h"

LRESULT show_filter_dlg( ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do
	{
		filter_dlg dlg( this ); 
		POINT cursor_pos; 
		BOOLEAN pos_got = FALSE; 

		if( ( SHOW_WINDOW_CENTER & flags ) == 0 )
		{
			_ret = GetCursorPos( &cursor_pos ); 
			if( TRUE == _ret )
			{
				pos_got = TRUE; 
			}
		}

		dlg.Create( GetHWND(), _T( "filtering" ), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738);
		dlg.SetIcon( IDI_MAIN_ICON ); 
		//dlg.CenterWindow();

		if( TRUE == pos_got )
		{
			SetWindowPos( dlg.GetHWND(), NULL, cursor_pos.x, cursor_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER ); 
		}
		else
		{
			dlg.CenterWindow(); 
		}

		dlg.ShowModal(); 

	}while( FALSE ); 

	return ret; 
}

INT32 APIENTRY wWinMain(HINSTANCE hInstance, 
					   HINSTANCE /*hPrevInstance*/, 
					   LPWSTR lpCmdLine, 
					   INT32 nCmdShow )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LRESULT ret; 
	
	LPWSTR *args; 
	INT32 arg_count; 
	WCHAR action_log_file[ MAX_PATH ]; 

	//sys_action_record action; 

	CPaintManagerUI::SetInstance( hInstance ); 
	CPaintManagerUI::SetResourcePath( CPaintManagerUI::GetInstancePath() );
	//CPaintManagerUI::SetResourceZip( UI_FILE_NAME );
	CPaintManagerUI::SetResourceZipResType( INSTALL_SOURCE_FILE_TYPE, IDR_SETUP_RES_ZIP ); 

	do 
	{
		ret = init_volumes_name_map(); 
		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		args = CommandLineToArgvW( lpCmdLine, &arg_count ); 

		if( arg_count >= 2 )
		{ 
			if( 0 == wcsicmp( args[ 1 ], L"/f" ) )
			{

				break; 
			}

			show_filter_dlg()
		}

		ret = add_app_path_to_file_name( action_log_file, 
			ARRAY_SIZE( action_log_file ), 
			ACTION_LOG_DB_NAME, 
			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		ntstatus = open_action_log_db( action_log_file, 
			( PVOID* )&common_log_db ); 
		if( ntstatus != STATUS_SUCCESS )
		{
			break; 
		}

#define ACTION_ANALYZER_TITLE L"系统行为分析"

		ret = show_action_show_dlg( NULL, ACTION_ANALYZER_TITLE ); 

	}while( FALSE );

	return ( INT32 )ret; 
}