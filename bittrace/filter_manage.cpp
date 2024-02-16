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
#include "acl_define.h"
#include "action_type_parse.h"
#include "filter_manage.h"
#include "sqlite_supp.h"
#include "action_log_db.h"
#include "action_log_sql.h"
#include "stack_dump.h"
#include "filter_cond_db.h"
#include "action_filter_cond_sql.h"
#include "action_log_sql.h"
#include "action_display.h"
#include "r3_rw_lock.h"

#pragma comment(lib, "Delayimp.lib")
#pragma comment(linker, "/DelayLoad:d3d9.dll")

CRITICAL_SECTION filter_info_lock; 
action_filter_infos filter_infos; 


LRESULT WINAPI r_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		lock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI r_unlock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		lock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_unlock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		unlock_cs( filter_info_lock ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI w_try_lock_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = TryEnterCriticalSection( &filter_info_lock ); 

		if( FALSE == _ret )
		{
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "try enter cs error" ) ); 

			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

/*****************************************************************************
will make more database for caching events which is filtered.so need a function 
set to manage the data structure for caching.
caching database id:
1.timestamp
2.base database information.
3.filtered target database and table.
4.filtered sql

is when remove the temporary database for filtering? 
1.change the filtered condition.( accumulated filtering is best )
*****************************************************************************/

/*************************************************************
filter value type: 
1. value from ring 3. 
2. value from ring 0. 
*************************************************************/

/*************************************************************
time
main type
*************************************************************/
HANDLE WINAPI get_parent_pid( HANDLE pid )
{
	HANDLE parent_pid = INVALID_PROCESS_ID; 

	return parent_pid; 
} 

ULONG WINAPI get_action_main_type( sys_action_type action_type )
{
	ULONG main_type = 0; 

	switch( action_type )
	{

		//MT_execmon
	case EXEC_create:   //�������� ����·���� ��ִ�м�أ� 
	case EXEC_destroy:  //�����˳� ����·���� 
	case EXEC_module_load:  //ģ����� ģ��·���� 
		main_type = MT_execmon; 
		break; 
		//MT_filemon:  
	case FILE_touch:  //�����ļ� �ļ�ȫ·�� ���ļ���أ� 
	case FILE_open:  //���ļ� �ļ�ȫ·�� 
	case FILE_read:  //��ȡ�ļ� �ļ�ȫ·�� 
	case FILE_write:  //д���ļ� �ļ�ȫ·�� 
	case FILE_modified:  //�ļ����޸� �ļ�ȫ·�� 
	case FILE_readdir:  //����Ŀ¼ Ŀ¼ȫ·�� 
	case FILE_remove:  //ɾ���ļ� �ļ�ȫ·�� 
	case FILE_rename:  //�������ļ� �ļ�ȫ·�� 
	case FILE_truncate:  //�ض��ļ� �ļ�ȫ·�� 
	case FILE_mklink:  //�����ļ�Ӳ���� �ļ�ȫ·�� 
	case FILE_chmod:  //�����ļ����� �ļ�ȫ·�� 
	case FILE_setsec:  //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_getinfo:
	case FILE_setinfo:
	case FILE_close:
		main_type = MT_filemon; 
		break; 
		//MT_regmon:  
	case REG_openkey:  //��ע����� ע�����·��  ��ע�����أ� 
	case REG_mkkey:  //����ע����� ע�����·�� 
	case REG_rmkey:  //ɾ��ע����� ע�����·�� 
	case REG_mvkey:  //������ע����� ע�����·�� 
	case REG_rmval:  //ɾ��ע����� ע�����·�� 
	case REG_getval:  //��ȡע���ֵ ע���ֵ·�� 
	case REG_setval:  //����ע���ֵ ע���ֵ·�� 
	case REG_loadkey:  //����ע���Hive�ļ� ע�����·�� 
	case REG_replkey:  //�滻ע����� ע�����·�� 
	case REG_rstrkey:  //����ע���Hive�ļ� ע�����·�� 
	case REG_setsec:  //����ע�������ȫ���� ע�����·�� 
	case REG_closekey:
		main_type = MT_regmon; 
		break; 

		//MT_procmon:  
	case PROC_exec:  //�������� Ŀ�����·����  �����̼�أ�
	case PROC_open:  //�򿪽��� Ŀ�����·���� 
	case PROC_debug:  //���Խ��� Ŀ�����·���� 
	case PROC_suspend:  //������� Ŀ�����·���� 
	case PROC_resume:  //�ָ����� Ŀ�����·���� 
	case PROC_kill:  
	case PROC_exit:  //�������� Ŀ�����·���� 
	case PROC_job:  //�����̼��빤���� Ŀ�����·���� 
	case PROC_pgprot:  //������޸��ڴ����� Ŀ�����·���� 
	case PROC_freevm:  //������ͷ��ڴ� Ŀ�����·���� 
	case PROC_writevm:  //�����д�ڴ� Ŀ�����·���� 
	case PROC_readvm:  //����̶��ڴ� Ŀ�����·���� 
	case THRD_remote:  //����Զ���߳� Ŀ�����·���� 
	case THRD_create:  //�����߳�
	case THRD_setctxt:  //����������߳������� Ŀ�����·���� 
	case THRD_suspend:  //����̹����߳� Ŀ�����·���� 
	case THRD_resume:  //����ָ̻��߳� Ŀ�����·���� 
	case THRD_kill:  //����̽����߳� Ŀ�����·���� 
	case THRD_exit:  
	case THRD_queue_apc:  //������Ŷ�APC Ŀ�����·���� 
		main_type = MT_procmon; 
		break; 

		//MT_common
	case COM_access:
		main_type = MT_common; 
		break;

	case PORT_read:
	case PORT_write:
	case PORT_urb:
		main_type = MT_peripheral; 
		break; 

		//MT_sysmon
	case SYS_settime:  //����ϵͳʱ�� �� 
	case SYS_link_knowndll:  //����KnownDlls���� �����ļ��� 
	case SYS_open_physmm:  //�������ڴ��豸 �� 
	case SYS_read_physmm:  //�������ڴ� �� 
	case SYS_write_physmm:  //д�����ڴ� �� 
	case SYS_load_kmod:  //�����ں�ģ�� �ں�ģ��ȫ·�� 
	case SYS_load_mod:  //����ģ�� �ں�ģ��ȫ·�� 
	case SYS_unload_mod:  //ж��ģ�� �ں�ģ��ȫ·�� 
	case SYS_enumproc:  //ö�ٽ��� �� 
	case SYS_regsrv:  //ע����� �������ȫ·�� 
	case SYS_opendev:  //���豸 �豸�� 
		main_type = MT_sysmon; 
		break; 

		//MT_w32mon
	case W32_postmsg:  //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
	case W32_sendmsg:  //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
	case W32_findwnd:  //���Ҵ��� �� 
	case W32_msghook:  //������Ϣ���� �� 
	case W32_lib_inject:  //DLLע�� ע��DLL·���� 
		main_type = MT_w32mon; 
		break; 

		//MT_netmon:  
	case NET_create:  
	case NET_connect:  //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
	case NET_listen:  //�����˿� ������ַ����ʽ��IP���˿ںţ� 
	case NET_send:  //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
	case NET_recv:  
	case NET_accept:  

	case NET_dns:  
	case NET_http:  //HTTP���� HTTP����·������ʽ������/URL�� 
	case NET_icmp_send:  
	case NET_icmp_recv:  
		main_type = MT_netmon; 
		break; 

		//MT_behavior:  
	case BA_extract_hidden:  //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
	case BA_extract_pe:  //�ͷ�PE�ļ� �ͷ��ļ�·���� 
	case BA_self_copy:  //���Ҹ��� ����Ŀ���ļ�·���� 
	case BA_self_delete:  //����ɾ�� ɾ���ļ�·���� 
	case BA_ulterior_exec:  //����ִ�� ��ִ��ӳ��·���� 
	case BA_invade_process:  //���ֽ��� Ŀ�����·���� 
	case BA_infect_pe:  //��ȾPE�ļ� Ŀ���ļ�·���� 
	case BA_overwrite_pe:  //��дPE�ļ� Ŀ���ļ�·���� 
	case BA_register_autorun:  //ע���������� �������ļ�·���� 
	case BA_other:  
		main_type = MT_behavior; 
	default:
		main_type = MT_unknown; 
		break; 
	}

	return main_type; 
}

/**********************************************************************************************************************
�ٶ����Ľ��������
������ʹ���ڴ棬ͨ���ڴ���ʵ��ʵʱ��ʾ��־��
�����Ҫ���з������������������ݿ⣬һ���������ܼ��ص���־ȫ�����أ���������ڴ�����������

�ڴ���ط���������ʾ���е���Ϣ��

�༶���淨��
��1�����������ݿ�
��2���������ڴ棬ֱ�Ӽ����������ݿ��е����ݡ�ȫ�������ڴ��С�
��3�������ǹ��˺�����ݣ������ڴ��о������˵���Ϣ��

�ٶȵ���Ҫ�ԣ�


��ʾһ��������������Ϣ�ã����ǽ����������ϸ�ڷֱ���ʾ�ã�

**********************************************************************************************************************/
BOOLEAN WINAPI filter_action_time_value( LARGE_INTEGER *event_time, time_region *duration, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	do 
	{
		switch( flt_mode )
		{
		case FLT_CONTAINS: 
			if( event_time->QuadPart >=
				duration->begin_time.QuadPart 
				&& event_time->QuadPart <= duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EXCLUDES: 
			if( event_time->QuadPart <
				duration->begin_time.QuadPart 
				&& event_time->QuadPart > duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EQUALS: 
			if( event_time->QuadPart >=
				duration->begin_time.QuadPart 
				&& event_time->QuadPart <= duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		case FLT_NOT_EQUALS: 
			if( event_time->QuadPart <
				duration->begin_time.QuadPart 
				&& event_time->QuadPart > duration->end_time.QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		default: 
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return filtered; 
}

BOOLEAN WINAPI _filter_action_time_value( LARGE_INTEGER *event_time, LARGE_INTEGER *dst_time, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	do 
	{
		event_time->QuadPart = event_time->QuadPart - ( event_time->QuadPart % ( 10000000 ) ); 

		switch( flt_mode )
		{
		case FLT_CONTAINS: 
			if( event_time->QuadPart == dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EXCLUDES: 
			if( event_time->QuadPart != dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_EQUALS: 
			if( event_time->QuadPart == dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		case FLT_NOT_EQUALS: 
			if( event_time->QuadPart != dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_GREATER_THAN: 
			if( event_time->QuadPart > dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_LOWER_THAN: 
			if( event_time->QuadPart < dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_START_WITH: 
			if( event_time->QuadPart >= dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 
		case FLT_END_WITH: 
			if( event_time->QuadPart <= dst_time->QuadPart )
			{
				filtered = TRUE; 
			} 
			break; 

		default: 
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	return filtered; 
}

BOOLEAN WINAPI filter_action_ulong_value( ULONG event_type, ULONG policy_type, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 

		if( event_type == ( event_type & policy_type ) )
		{
			filtered = TRUE; 
			break; 
		}

		break; 
	case FLT_EXCLUDES: 
		if( 0 == ( event_type & policy_type ) )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	case FLT_EQUALS: 
		if( event_type == policy_type )
		{
			filtered = TRUE; 
			break; 
		}
		break; 

	case FLT_NOT_EQUALS: 
		if( event_type != policy_type )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

BOOLEAN WINAPI filter_action_ulonglong_value( ULONGLONG source_value, ULONGLONG dest_value, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 

		if( source_value == dest_value )
		{
			filtered = TRUE; 
			break; 
		}

		break; 
	case FLT_EXCLUDES: 
		if( source_value != dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	case FLT_EQUALS: 
		if( source_value == dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 

	case FLT_NOT_EQUALS: 
		if( source_value != dest_value )
		{
			filtered = TRUE; 
			break; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

BOOLEAN WINAPI filter_action_string_value( LPCWSTR source_value, ULONG cc_src_len, LPCWSTR dest_value, ULONG cc_dest_len, action_compare_mode flt_mode )
{
	BOOLEAN filtered = FALSE; 
	INT32 _ret; 
	LPWSTR text_value; 

	switch( flt_mode )
	{
	case FLT_CONTAINS: 
		text_value = wcsstr( ( LPWSTR )source_value, 
			dest_value ); //ctx->proc_name_len  
		if( text_value != NULL )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_EXCLUDES: 
		text_value = wcsstr( ( LPWSTR )source_value, 
			dest_value ); //ctx->proc_name_len  
		if( text_value == NULL )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_EQUALS: 
		_ret = wcsnicmp( source_value, 
			dest_value, 
			cc_dest_len ); 

		if( _ret == 0 )
		{
			filtered = TRUE; 
		}
		break; 
	case FLT_NOT_EQUALS: 
		_ret = wcsnicmp( source_value, 
			dest_value, 
			cc_dest_len ); 

		if( _ret != 0 )
		{
			filtered = TRUE; 
		}
		break; 
	default: 
		ASSERT( FALSE ); 
		break; 
	}

	return filtered; 
}

ULONG WINAPI get_action_io_type( sys_action_type action_type )
{
	ULONG io_type = 0; 
	return io_type; 
}

/***********************************************************************************
�����Զ����2������:
1.���ų��Ĳ��֣������Դﵽ��һ����״̬ʱ����Щ���Խ��ᱻ�ų���
2.�����ų����֣�������й��˲����жϿ��ų������Ƿ����ִ�С�

��ǰ�����ų����ֵĶ���: 
�Ӳ��Լ���ʼ���������������������������¼������͵ļ����������ʱ������ų����ų�����
���ԡ�

��ǰ���ų����ֵĶ���:
���е��ų���ʽ�Ĺ����������ǿ��ų�������
***********************************************************************************/
LRESULT WINAPI filter_action( action_filter_info *filter_info, 
										  sys_action_record *action, 
										  action_context *ctx, 
										  PVOID data, 
										  ULONG data_size )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	LRESULT _ret; 
	BOOLEAN found = FALSE; 

	do 
	{
		switch( filter_info->cond )
		{
		case FLT_PATH:
			{
				WCHAR obj_path[ _MAX_REG_PATH_LEN ]; 
				LPCWSTR _obj_path; 
				ULONG obj_path_len; 
				
				_ret = get_object_path( action, 
					ctx, 
					obj_path, 
					ARRAYSIZE( obj_path ), 
					&_obj_path, 
					0, 
					&obj_path_len ); 
				
				if( _ret != ERROR_SUCCESS )
				{
					break; 
				}

				found = filter_action_string_value( _obj_path, 
					obj_path_len, 
					filter_info->value.text.string_value, 
					filter_info->value.text.cc_string_len, 
					filter_info->compare_mode ); 
				break; 			
			}
			break; 
		case FLT_PROCESS_NAME: 
			{
				LPCWSTR proc_name; 
				ULONG proc_name_len; 

				proc_name = wcsrchr( ctx->proc_name, PATH_DELIM ); 
				if( proc_name == NULL )
				{
					proc_name = ctx->proc_name; 
					proc_name_len = ( ULONG )wcslen( ctx->proc_name ); 

					ASSERT( wcsicmp( proc_name, L"SYSTEM" ) == 0 ); 
				}
				else
				{
					proc_name += 1;  
					proc_name_len = ( ULONG )( ULONG_PTR )( ( ctx->proc_name + ctx->proc_name_len ) - proc_name ); 
				
#ifdef _DEBUG
					if( proc_name[ proc_name_len - 1 ] != L'e' 
						&& proc_name[ proc_name_len - 1 ] != L'E' ) 
					{
						ASSERT( FALSE ); 
					}
#endif //_DEBUG
				}

				found = filter_action_string_value( proc_name, 
					proc_name_len, 
					filter_info->value.text.string_value, 
					filter_info->value.text.cc_string_len, 
					filter_info->compare_mode ); 

				break; 
			}
			break; 
		case FLT_COMMAND: 
			{
				//WCHAR *cmd_line; 

				ret = ERROR_INVALID_PARAMETER; 

				break; 
			}
			break; 
		case FLT_PID: 
			{
				found = filter_action_ulong_value( ctx->proc_id, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_PARENT_PID: 
			{
				ULONG parent_pid; 

				ret = ERROR_NOT_SUPPORTED; 
				break; 
				parent_pid = ( ULONG )( ULONG_PTR )get_parent_pid( ( HANDLE )( ULONG_PTR )ctx->proc_id ); 
				
				found = filter_action_ulong_value( parent_pid, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_TID: 
			{
				ULONG tid; 

				tid = ctx->thread_id; 

				found = filter_action_ulong_value( tid, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_MAIN_TYPE: 
			{
				ULONG main_type; 

				main_type = get_action_main_type( action->type ); 
				found = filter_action_ulong_value( main_type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_ACTION_TYPE: 
			{
				found = filter_action_ulong_value( action->type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_READ_WRITE: 
			{
				ULONG io_type; 
				
				io_type = get_action_io_type( action->type ); 

				found = filter_action_ulong_value( io_type, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_DESCRITION: 
			{
				ret = ERROR_NOT_SUPPORTED; 
				break;
			}
			break; 
		case FLT_DESC_DETAIL: 
			break; 
		case FLT_ACTION_RESULT: 
			{
				found = filter_action_ulong_value( ctx->last_result, 
					filter_info->value.data.ulong_value, 
					filter_info->compare_mode ); 
			}
			break; 
		case FLT_DURATION: 
			{
				//ASSERT( filter_info->value.size >= sizeof( time_region ) ); 

				found = filter_action_time_value( &ctx->time, &filter_info->value.data.duration, filter_info->compare_mode ); 
			}
			break; 
		case FLT_DATE_TIME:  
			{
				found = _filter_action_time_value( &ctx->time, &filter_info->value.data.time_value, filter_info->compare_mode ); 
			}
			break; 
		case FLT_RELATIVE_TIME: 
			//{
			//	found = filter_action_time_value( &ctx->time, &filter_info->value.data.time_value ); 
			//}
			break; 
		case FLT_SESSION:  
			break; 
		case FLT_USER:  
			break; 
		case FLT_AUTH_ID:  
			break; 
		case FLT_ACTION_NO: 
			break; 
		case FLT_CORPORATION:  
			break; 
		case FLT_VERSION: 
			{
				LARGE_INTEGER file_version; 
				
				_ret = get_file_version( ctx->proc_name, &file_version ); 
				if( _ret != ERROR_SUCCESS )
				{
					break; 
				}

				found = filter_action_ulonglong_value( ( ULONGLONG )file_version.QuadPart, filter_info->value.data.ulonglong_value, filter_info->compare_mode ); 
			}
			break; 
		case FLT_VIRTUAL_TECH: 
			break; 
		case FLT_CPU_ARCH: 
			break; 
		default: 
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			break; 
		}

		if( found != TRUE )
		{
			break; 
		}

		//if( filter_info->result != FLT_INCLUDE )
		//{
		ret = ERROR_SUCCESS; 
		//}

	}while( FALSE ); 

	return ret; 
}

/******************************************************************************************
��ǰ�Ĺ��˵ķ��������⣺
1.�����ٶȱȽ�����������˳������ʱ��
���������������ַ�����
1.����ʹ���ڴ滺�棬��PROCMONһ���Ĺ�����ʽ��
2.���Ʒ�����ʱ��
******************************************************************************************/
LRESULT WINAPI init_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	init_cs_lock( filter_info_lock );  

	return ret; 
}

LRESULT WINAPI uninit_filter_infos()
{
	LRESULT ret = ERROR_SUCCESS; 

	uninit_cs_lock( filter_info_lock ); 
	return ret; 
}

#ifdef _DEBUG
ULONG dbg_thread_id = ( ULONG )( ULONG_PTR )INVALID_THREAD_ID - 1; 
ULONG dbg_proc_id = ( ULONG )( ULONG_PTR )INVALID_PROCESS_ID; 
ULONG test_proc_id = ( ULONG )( ULONG_PTR )INVALID_PROCESS_ID; 
#endif //_DEBUG

LRESULT WINAPI filter_action_log( sys_action_record *action, 
								 action_context *ctx,  
								 PVOID data, 
								 ULONG data_size )
{
	LRESULT ret; 
	LRESULT _ret; 
	BOOLEAN lock_held = FALSE; 
	BOOLEAN exclude_secondary_cond = FALSE; 
	ULONG i; 

	do 
	{
		ret = ERROR_SUCCESS; 

		_ret = w_try_lock_filter_infos(); 

		if( ERROR_SUCCESS != _ret )
		{
			break; 
		}

		lock_held = TRUE; 

#ifdef _DEBUG
		if( dbg_thread_id == ctx->thread_id )
		{
			DBG_BP(); 
		}

		if( dbg_proc_id == ctx->proc_id )
		{
			DBG_BP(); 
		}

#endif //_DEBUG

		_ret = ERROR_NOT_FOUND; 

		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->filter_mode != FLT_INCLUDE )
				{
					break; 
				}

				if( exclude_secondary_cond == FALSE 
					&& filter_infos.filter_infos[ i ]->cond != FLT_MAIN_TYPE ) 
				{
					exclude_secondary_cond = TRUE; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

				if( _ret == ERROR_NOT_FOUND )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

			}while( FALSE ); 
		}

		if( FALSE != exclude_secondary_cond 
			|| ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = ERROR_SUCCESS; 
		_ret = ERROR_NOT_FOUND; 
		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->filter_mode != FLT_EXCLUDE )
				{
					break; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

			}while( FALSE );

			if( _ret != ERROR_NOT_FOUND )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

	}while( FALSE );

#ifdef _DEBUG
	if( test_proc_id != ( ULONG )INVALID_PROCESS_ID 
		&& test_proc_id != ctx->proc_id 
		&& ret == ERROR_SUCCESS )
	{
		DBG_BP(); 
	}
#endif //_DEBUG

	if( TRUE == lock_held )
	{
		w_unlock_filter_infos(); 
	}

	return ret; 
}

LRESULT WINAPI filter_log_for_ui( sys_action_record *action, 
								 action_context *ctx,  
								 PVOID data, 
								 ULONG data_size, 
								 filter_ui_info *ui_info )
{
	LRESULT ret; 
	LRESULT _ret; 
	BOOLEAN lock_held = FALSE; 
	ULONG i; 

	do 
	{
		ret = ERROR_INVALID_PARAMETER; 

		if( ui_info == NULL )
		{
			break; 
		}

		_ret = w_try_lock_filter_infos(); 

		if( ERROR_SUCCESS != _ret )
		{
			break; 
		}

		lock_held = TRUE; 

		_ret = ERROR_NOT_FOUND; 

		for( i = 0; i < filter_infos.filter_infos.size(); i ++ )
		{
			do 
			{
				if( filter_infos.filter_infos[ i ]->ui_info.bk_clr == INVALID_UI_FILTERED_BK_COLOR )
				{
					break; 
				}

				_ret = filter_action( filter_infos.filter_infos[ i ], 
					action, 
					ctx, 
					data, 
					data_size ); 

			}while( FALSE );

			if( _ret != ERROR_NOT_FOUND )
			{
				ui_info->bk_clr = filter_infos.filter_infos[ i ]->ui_info.bk_clr; 
				ui_info->cond = filter_infos.filter_infos[ i ]->cond; 

				ret = ERROR_SUCCESS; 
				break; 
			}
		}
		
	}while( FALSE );

	if( TRUE == lock_held )
	{
		w_unlock_filter_infos(); 
	}

	if( ret != ERROR_SUCCESS )
	{
		ui_info->bk_clr = INVALID_UI_FILTERED_BK_COLOR; 
		ui_info->cond = FLT_TYPE_INVALID; 
	}

	return ret; 
}


/**********************************************************************
���˷�ʽ�� 
������������Ӧ��ʽ
1��1: ֱ��ȡ������

�ŵ㣺
�����������Ƚϼ� *��ѯ�ٶȸ���*

ȱ�㣺
���ݿ���Ҫ������У���Ҫ��������ֵ��ռ�ø���Ŀռ䡣*��ѯ�ٶȸ���*


1�Զ�: ��Ҫȡ��������

�ŵ㣺
ռ�ø���Ŀռ䣬������С�

ȱ�㣺
�����������Ƚϸ��ӡ�

��������ֵ��Ӧ��ʽ
1�Ե�һģʽ: ����һ��SQL WHERE ������ʽ��䣬*��ѯ�ٶȿ�*

�ŵ㣺
�����������Ƚϼ�

ȱ�㣺
���ݿ���Ҫ������У���Ҫ��������ֵ��ռ�ø���Ŀռ䡣

1�Զ�ģʽ: �������SQL WHERE ������ʽ��� ��Ҫʹ�� AND��OR��������

�ŵ㣺
ռ�ø���Ŀռ䣬������С�

ȱ�㣺
�����������Ƚϸ��ӣ�*��ѯ�ٶȻ����*��

**********************************************************************/

