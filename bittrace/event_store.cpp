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
#include "event_store.h"

/**********************************************************************************
procmon���ٹ��ܵķ�����
1.������־��1/3��PROFILING�¼�������¼��Ƕ�ʱ��ϵͳ�����е��̵߳���ʱ�������CONTEXT
SWITCH���и��ٷ���������ֵ��ʲô����

bittrace��procmon�Ĺؼ����
1.ע������Ĺر��¼�PROCMON�ǳ��࣬���е�ԭ����ʲô��(��ʹ��CALLBACKӦ�ã���ô���Ϊʲô
��ô��)
**********************************************************************************/
ULONG WINAPI calc_correct_record_append_size( r3_action_notify *event )
{
	ULONG record_append_size = 0; 
	sys_action_record *_action; 
	
	_action = &event->action.action; 
		
	switch( _action->type )
	{
	case EXEC_create:
		record_append_size += ( ( event->action.action.do_exec_create.path_len + 1 ) << 1 ); 
		break; //�������� ����·���� ��ִ�м�أ� 

	case EXEC_destroy:
		record_append_size += ( ( event->action.action.do_exec_destroy.path_len + 1 ) << 1 ); 
		break; //�����˳� ����·���� 

	case EXEC_module_load:
		record_append_size += ( ( event->action.action.do_exec_module_load.path_len + 1 ) << 1 ); 
		break; //ģ����� ģ��·���� 

		//MT_filemon, 
	case FILE_touch:
		record_append_size += ( ( event->action.action.do_file_touch.path_len + 1 ) << 1 ); 
		break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
	case FILE_open:
		record_append_size += ( ( event->action.action.do_file_open.path_len + 1 ) << 1 ); 
		break; //���ļ� �ļ�ȫ·�� 
		//case ACCESS_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_read:
		record_append_size += ( ( event->action.action.do_file_read.path_len + 1 ) << 1 ); 
		break; //��ȡ�ļ� �ļ�ȫ·�� 
	case FILE_write:
		record_append_size += ( ( event->action.action.do_file_write.path_len + 1 ) << 1 ); 
		break; //д���ļ� �ļ�ȫ·�� 
		//case MODIFY_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_modified:
		record_append_size += ( ( event->action.action.do_file_modified.path_len + 1 ) << 1 ); 
		break; //�ļ����޸� �ļ�ȫ·�� 
	case FILE_readdir:
		record_append_size += ( ( event->action.action.do_file_readdir.path_len + 1 ) << 1 ); 
		break; //����Ŀ¼ Ŀ¼ȫ·�� 
	case FILE_remove:
		record_append_size += ( ( event->action.action.do_file_remove.path_len + 1 ) << 1 ); 
		break; //ɾ���ļ� �ļ�ȫ·�� 
		//case DELETE_FILE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case FILE_rename:
		record_append_size += ( ( event->action.action.do_file_rename.path_len + 1 ) << 1 ); 
		break; //�������ļ� �ļ�ȫ·�� 
	case FILE_truncate:
		record_append_size += ( ( event->action.action.do_file_truncate.path_len + 1 ) << 1 ); 
		break; //�ض��ļ� �ļ�ȫ·�� 
	case FILE_mklink:
		record_append_size += ( ( event->action.action.do_file_mklink.path_len + 1 ) << 1 ); 
		break; //�����ļ�Ӳ���� �ļ�ȫ·�� 
	case FILE_chmod:
		record_append_size += ( ( event->action.action.do_file_chmod.path_len + 1 ) << 1 ); 
		break; //�����ļ����� �ļ�ȫ·�� 
	case FILE_setsec:
		record_append_size += ( ( event->action.action.do_file_setsec.path_len + 1 ) << 1 ); 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 

	case FILE_getinfo:
		record_append_size += ( ( event->action.action.do_file_getinfo.path_len + 1 ) << 1 ); 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 

	case FILE_setinfo:
		record_append_size += ( ( event->action.action.do_file_setinfo.path_len + 1 ) << 1 ); 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		//MT_regmon, 
	case REG_openkey:
		record_append_size += ( ( event->action.action.do_reg_openkey.path_len + 1 ) << 1 ); 
		break; //��ע����� ע�����·��  ��ע�����أ� 
	case REG_mkkey:
		record_append_size += ( ( event->action.action.do_reg_mkkey.path_len + 1 ) << 1 ); 
		break; //����ע����� ע�����·�� 
		//case MODIFY_KEY:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case REG_rmkey:
		record_append_size += ( ( event->action.action.do_reg_rmkey.path_len + 1 ) << 1 ); 
		break; //ɾ��ע����� ע�����·�� 
	case REG_mvkey:
		record_append_size += ( ( event->action.action.do_reg_mvkey.path_len + 1 ) << 1 ); 
		break; //������ע����� ע�����·�� 
	case REG_rmval:
		record_append_size += ( ( event->action.action.do_reg_rmval.path_len + 1 
			+ event->action.action.do_reg_rmval.val_len + 1 ) << 1 ); 
		break; //ɾ��ע����� ע�����·�� 
	case REG_getval:
		record_append_size += ( ( event->action.action.do_reg_getval.path_len + 1 
			+ event->action.action.do_reg_getval.val_name_len + 1 ) << 1 ) 
			+ event->action.action.do_reg_getval.info_size; 
		break; //��ȡע���ֵ ע���ֵ·�� 
	case REG_setval:
		record_append_size += ( ( event->action.action.do_reg_getval.path_len + 1 
			+ event->action.action.do_reg_getval.val_name_len + 1 ) << 1 ) 
			+ event->action.action.do_reg_getval.info_size; 
		break; //����ע���ֵ ע���ֵ·�� 
	case REG_loadkey:
		record_append_size += ( ( event->action.action.do_reg_loadkey.path_len + 1 ) << 1 ); 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_replkey:
		record_append_size += ( ( event->action.action.do_reg_replkey.path_len + 1 ) << 1 ); 
		break; //�滻ע����� ע�����·�� 
	case REG_rstrkey:
		record_append_size += ( ( event->action.action.do_reg_rstrkey.path_len + 1 ) << 1 ); 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_setsec:
		record_append_size += ( ( event->action.action.do_reg_setsec.path_len + 1 ) << 1 ); 
		break; //����ע�������ȫ���� ע�����·�� 

		//MT_procmon, 
	case PROC_exec:
		record_append_size += ( ( event->action.action.do_proc_exec.path_len + 1 ) << 1 ); 
		break; //�������� Ŀ�����·����  �����̼�أ�
		//case CREATE_PROC:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case PROC_open:
		record_append_size += ( ( event->action.action.do_proc_open.path_len + 1 ) << 1 ); 
		break; //�򿪽��� Ŀ�����·���� 
	case PROC_debug:
		record_append_size += ( ( event->action.action.do_proc_debug.path_len + 1 ) << 1 ); 
		break; //���Խ��� Ŀ�����·���� 
	case PROC_suspend:
		record_append_size += ( ( event->action.action.do_proc_suspend.path_len + 1 ) << 1 ); 
		break; //������� Ŀ�����·���� 
	case PROC_resume:
		record_append_size += ( ( event->action.action.do_proc_resume.path_len + 1 ) << 1 ); 
		break; //�ָ����� Ŀ�����·���� 
	case PROC_exit:
		record_append_size += ( ( event->action.action.do_proc_exit.path_len + 1 ) << 1 ); 
		break; //�������� Ŀ�����·���� 
		//case TERMINATE_PROC:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case PROC_job:
		record_append_size += ( ( event->action.action.do_proc_job.path_len + 1 ) << 1 ); 
		break; //�����̼��빤���� Ŀ�����·���� 
	case PROC_pgprot:
		record_append_size += ( ( event->action.action.do_proc_pgprot.path_len + 1 ) << 1 ); 
		break; //������޸��ڴ����� Ŀ�����·���� 
	case PROC_freevm:
		record_append_size += ( ( event->action.action.do_proc_freevm.path_len + 1 ) << 1 ); 
		break; //������ͷ��ڴ� Ŀ�����·���� 
	case PROC_writevm:
		record_append_size += ( ( event->action.action.do_proc_writevm.path_len + 1 ) << 1 ); 
		break; //�����д�ڴ� Ŀ�����·���� 
	case PROC_readvm:
		record_append_size += ( ( event->action.action.do_proc_readvm.path_len + 1 ) << 1 ); 
		break; //����̶��ڴ� Ŀ�����·���� 
	case THRD_remote:
		record_append_size += ( ( event->action.action.do_thrd_remote.path_len + 1 ) << 1 ); 
		break; //����Զ���߳� Ŀ�����·���� 
	case THRD_setctxt:
		record_append_size += ( ( event->action.action.do_thrd_setctxt.path_len + 1 ) << 1 ); 
		break; //����������߳������� Ŀ�����·���� 
	case THRD_suspend:
		record_append_size += ( ( event->action.action.do_thrd_suspend.path_len + 1 ) << 1 ); 
		break; //����̹����߳� Ŀ�����·���� 
	case THRD_resume:
		record_append_size += ( ( event->action.action.do_thrd_resume.path_len + 1 ) << 1 ); 
		break; //����ָ̻��߳� Ŀ�����·���� 
	case THRD_exit:
		record_append_size += ( ( event->action.action.do_thrd_exit.path_len + 1 ) << 1 ); 
		break; //����̽����߳� Ŀ�����·���� 
	case THRD_queue_apc:
		record_append_size += ( ( event->action.action.do_thrd_queue_apc.path_len + 1 ) << 1 ); 
		break; //������Ŷ�APC Ŀ�����·���� 

		//MT_common
		//case COM_access:
		//	record_append_size += ( ( event->action.action.do_com_access.path_len + 1 ) << 1 ); 
		//	break; 
		//case ACCESS_COM:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 

		//MT_sysmon
	case SYS_settime:
		break; //����ϵͳʱ�� �� 
	case SYS_link_knowndll:
		record_append_size += ( ( event->action.action.do_sys_link_knowndll.path_len + 1 ) << 1 ); 
		break; //����KnownDlls���� �����ļ��� 
	case SYS_open_physmm:
		break; //�������ڴ��豸 �� 
		//case ACCESS_MEM:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case SYS_read_physmm:
		break; //�������ڴ� �� 
	case SYS_write_physmm:
		break; //д�����ڴ� �� 
	case SYS_load_kmod:
		record_append_size += ( ( event->action.action.do_sys_load_kmod.path_len + 1 ) << 1 ); 
		break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
		//case INSTALL_DRV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case SYS_enumproc:
		break; //ö�ٽ��� �� 
	case SYS_regsrv:
		record_append_size += ( ( event->action.action.do_sys_regsrv.path_len + 1 ) << 1 ); 
		break; //ע����� �������ȫ·�� 
	case SYS_opendev:
		record_append_size += ( ( event->action.action.do_sys_opendev.path_len + 1 ) << 1 ); 
		break; //���豸 �豸�� 

		//MT_w32mon
	case W32_postmsg:
		record_append_size += ( ( event->action.action.do_w32_postmsg.path_len + 1 ) << 1 ); 
		break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
	case W32_sendmsg:
		record_append_size += ( ( event->action.action.do_w32_sendmsg.path_len + 1 ) << 1 ); 
		break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
	case W32_findwnd:
		record_append_size = 0; 
		break; //���Ҵ��� �� 
	case W32_msghook:
		record_append_size += ( ( event->action.action.do_w32_msghook.path_len + 1 ) << 1 ); 
		break; //������Ϣ���� �� 
		//case INSTALL_HOOK:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case W32_lib_inject:
		record_append_size += ( ( event->action.action.do_w32_lib_inject.path_len + 1 ) << 1 ); 
		break; //DLLע�� ע��DLL·���� 

		//MT_netmon
	case NET_create:
		break; 
		//case SOCKET_CREATE:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_connect:
		break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
		//case SOCKET_CONNECT:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_listen:
		break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
		//case SOCKET_LISTEN:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_send:
		break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
		//case SOCKET_SEND:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_recv:
		break; 
		//case SOCKET_RECV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_accept:
		break; 
		//case SOCKET_ACCEPT:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break;
	case NET_http:
		break; //HTTP���� HTTP����·������ʽ������/URL�� 
		//case LOCATE_URL:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	case NET_icmp_send:
		record_append_size += ( ( event->action.action.do_net_icmp_send.path_len + 1 ) << 1 ); 
		break; 
		//case ICMP_SEND:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 
	case NET_icmp_recv:
		record_append_size += ( ( event->action.action.do_net_icmp_recv.path_len + 1 ) << 1 ); 
		break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		//case ICMP_RECV:
		//	record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//	break; 


		//MT_behavior, 
	case BA_extract_hidden:
		record_append_size += ( ( event->action.action.do_ba_extract_hidden.path_len + 1 ) << 1 ); 
		break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
	case BA_extract_pe:
		record_append_size += ( ( event->action.action.do_ba_extract_pe.path_len + 1 ) << 1 ); 
		break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
	case BA_self_copy:
		record_append_size += ( ( event->action.action.do_ba_self_copy.path_len + 1 ) << 1 ); 
		break; //���Ҹ��� ����Ŀ���ļ�·���� 
	case BA_self_delete:
		record_append_size += ( ( event->action.action.do_ba_self_delete.path_len + 1 ) << 1 ); 
		break; //����ɾ�� ɾ���ļ�·���� 
	case BA_ulterior_exec:
		record_append_size += ( ( event->action.action.do_ba_ulterior_exec.path_len + 1 ) << 1 ); 
		break; //����ִ�� ��ִ��ӳ��·���� 
	case BA_invade_process:
		record_append_size += ( ( event->action.action.do_ba_invade_process.path_len + 1 ) << 1 ); 
		break; //���ֽ��� Ŀ�����·���� 
	case BA_infect_pe:
		record_append_size += ( ( event->action.action.do_ba_infect_pe.path_len + 1 ) << 1 ); 
		break; //��ȾPE�ļ� Ŀ���ļ�·���� 
	case BA_overwrite_pe:
		record_append_size += ( ( event->action.action.do_ba_overwrite_pe.path_len + 1 ) << 1 ); 
		break; //��дPE�ļ� Ŀ���ļ�·���� 
	case BA_register_autorun:
		record_append_size += ( ( event->action.action.do_ba_register_autorun.path_len + 1 ) << 1 ); 
		break; //ע���������� �������ļ�·���� 

		//case BA_other:
		//record_append_size += ( ( event->action.action..path_len + 1 ) << 1 ); 
		//break; 

	default:
		ASSERT( FALSE ); 
		record_append_size = 0; 
		break; 
	}

	return record_append_size; 
}

ULONG WINAPI calc_correct_all_append_size( r3_action_notify *event )
{
	ULONG append_size; 

	do 
	{
		append_size = 0; 
		append_size += event->data_size; 
		append_size += event->action.action.size; 

		ASSERT( event->frame_count >= 0 
			&& event->frame_count <= ARRAYSIZE( event->stack_frame ) ); 
		
		append_size += event->frame_count * sizeof( ULONGLONG ); 
		
	}while( FALSE );

	return append_size; 
}

LRESULT WINAPI compress_event_log_ex( r3_action_notify *action, event_log_compressed ** event_out )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_event_buf_other_part( r3_action_notify_buf *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		action->status = 0; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI compress_event_log( r3_action_notify *action, r3_action_notify_buf **event_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG record_size; 
	ULONG event_size; 
	r3_action_notify_buf *_action = NULL; 
	PVOID src_data; 
	PVOID dest_data; 

	do 
	{
		ASSERT( event_out != NULL ); 
		ASSERT( action != NULL ); 

		*event_out = NULL; 

#ifdef _DEBUG
		if( STATUS_SUCCESS != check_r3_action_notify_output_valid( action ) )
		{
			DBG_BP(); 
		}
#endif //_DEBUG

		record_size = action->action.action.size; 
		event_size = record_size; 
		
		event_size += ACTION_RECORD_OFFEST; 

		if( action->data_inited == FALSE )
		{
		}
		else
		{
			event_size += action->data_size; 
		}

		event_size += FIELD_OFFSET( r3_action_notify_buf, action ); 

		_action = ( r3_action_notify_buf* )malloc( event_size ); 
		if( _action == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( &_action->action, action, ACTION_RECORD_OFFEST + record_size ); 

		if( action->data_size > 0 )
		{
			src_data = get_action_output_data( action ); 
			dest_data = ( PVOID )( ( BYTE* )&_action->action.action.action + record_size );  

			memcpy( dest_data, src_data, action->data_size ); 
		}

		_action->action.action.action.size = record_size; 
		_action->action.size = ACTION_RECORD_OFFEST + record_size + action->data_size; 

		ret = init_event_buf_other_part( _action ); 
		if( ret != ERROR_SUCCESS )
		{

		}
		*event_out = _action; 

	}while( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		free( _action ); 
		ASSERT( *event_out == NULL ); 
	}

	return ret; 
}
