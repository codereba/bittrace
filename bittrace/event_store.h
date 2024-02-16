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

#ifndef __EVENT_STORE_H__
#define __EVENT_STORE_H__

INLINE ULONG calc_sys_action_record_size( sys_action_type type )
{
	ULONG record_size; 

	record_size = ACTION_RECORD_PARAMETERS_OFFSET; 

	switch( type )
	{
	case EXEC_create:
		record_size += sizeof( exec_create ); 
		break; //�������� ����·���� ��ִ�м�أ� 

	case EXEC_destroy:
		record_size += sizeof( exec_destroy ); 
		break; //�����˳� ����·���� 

	case EXEC_module_load:
		record_size += sizeof( exec_module_load ); 
		break; //ģ����� ģ��·���� 

		//MT_filemon, 
	case FILE_touch:
		record_size += sizeof( file_touch ); 
		break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
	case FILE_open:
		record_size += sizeof( file_open ); 
		break; //���ļ� �ļ�ȫ·�� 
		//case ACCESS_FILE:
		//	record_size += sizeof( file_open ); 
		//	break; 
	case FILE_read:
		record_size += sizeof( _file_read ); 
		break; //��ȡ�ļ� �ļ�ȫ·�� 
	case FILE_write:
		record_size += sizeof( _file_write ); 
		break; //д���ļ� �ļ�ȫ·�� 
		//case MODIFY_FILE:
		//	record_size += sizeof( file_modified ); 
		//	break; 
	case FILE_modified:
		record_size += sizeof( file_modified ); 
		break; //�ļ����޸� �ļ�ȫ·�� 
	case FILE_readdir:
		record_size += sizeof( file_readdir ); 
		break; //����Ŀ¼ Ŀ¼ȫ·�� 
	case FILE_remove:
		record_size += sizeof( file_remove ); 
		break; //ɾ���ļ� �ļ�ȫ·�� 
		//case DELETE_FILE:
		//	record_size += sizeof( file_remove ); 
		//	break; 
	case FILE_rename:
		record_size += sizeof( file_rename ); 
		break; //�������ļ� �ļ�ȫ·�� 
	case FILE_truncate:
		record_size += sizeof( file_truncate ); 
		break; //�ض��ļ� �ļ�ȫ·�� 
	case FILE_mklink:
		record_size += sizeof( file_mklink ); 
		break; //�����ļ�Ӳ���� �ļ�ȫ·�� 
	case FILE_chmod:
		record_size += sizeof( file_chmod ); 
		break; //�����ļ����� �ļ�ȫ·�� 
	case FILE_setsec:
		record_size += sizeof( file_setsec ); 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		//MT_regmon, 
	case REG_openkey:
		record_size += sizeof( reg_openkey ); 
		break; //��ע����� ע�����·��  ��ע�����أ� 
	case REG_mkkey:
		record_size += sizeof( reg_mkkey ); 
		break; //����ע����� ע�����·�� 
		//case MODIFY_KEY:
		//	record_size += sizeof( reg_mkkey ); 
		//	break; 
	case REG_rmkey:
		record_size += sizeof( reg_rmkey ); 
		break; //ɾ��ע����� ע�����·�� 
	case REG_mvkey:
		record_size += sizeof( reg_mvkey ); 
		break; //������ע����� ע�����·�� 
	case REG_getinfo: 
		record_size += sizeof( reg_getinfo ); 
		break; 
	case REG_setinfo: 
		record_size += sizeof( reg_setinfo ); 
		break; 
	case REG_rmval:
		record_size += sizeof( reg_rmval ); 
		break; //ɾ��ע����� ע�����·�� 
	case REG_getval:
		record_size += sizeof( reg_getval ); 
		break; //��ȡע���ֵ ע���ֵ·�� 
	case REG_setval:
		record_size += sizeof( reg_setval ); 
		break; //����ע���ֵ ע���ֵ·�� 
	case REG_loadkey:
		record_size += sizeof( reg_loadkey ); 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_replkey:
		record_size += sizeof( reg_replkey ); 
		break; //�滻ע����� ע�����·�� 
	case REG_rstrkey:
		record_size += sizeof( reg_rstrkey ); 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_setsec:
		record_size += sizeof( reg_setsec ); 
		break; //����ע�������ȫ���� ע�����·�� 

		//MT_procmon, 
	case PROC_exec:
		record_size += sizeof( proc_exec ); 
		break; //�������� Ŀ�����·����  �����̼�أ�
		//case CREATE_PROC:
		//	record_size += sizeof( proc_exec ); 
		//	break; 
	case PROC_open:
		record_size += sizeof( proc_open ); 
		break; //�򿪽��� Ŀ�����·���� 
	case PROC_debug:
		record_size += sizeof( proc_debug ); 
		break; //���Խ��� Ŀ�����·���� 
	case PROC_suspend:
		record_size += sizeof( proc_suspend ); 
		break; //������� Ŀ�����·���� 
	case PROC_resume:
		record_size += sizeof( proc_resume ); 
		break; //�ָ����� Ŀ�����·���� 
	case PROC_exit:
		record_size += sizeof( proc_kill ); 
		break; //�������� Ŀ�����·���� 
		//case TERMINATE_PROC:
		//	record_size += sizeof( proc_kill ); 
		//	break; 
	case PROC_job:
		record_size += sizeof( proc_job ); 
		break; //�����̼��빤���� Ŀ�����·���� 
	case PROC_pgprot:
		record_size += sizeof( proc_pgprot ); 
		break; //������޸��ڴ����� Ŀ�����·���� 
	case PROC_freevm:
		record_size += sizeof( proc_freevm ); 
		break; //������ͷ��ڴ� Ŀ�����·���� 
	case PROC_writevm:
		record_size += sizeof( proc_writevm ); 
		break; //�����д�ڴ� Ŀ�����·���� 
	case PROC_readvm:
		record_size += sizeof( proc_readvm ); 
		break; //����̶��ڴ� Ŀ�����·���� 
	case THRD_remote:
		record_size += sizeof( thrd_remote ); 
		break; //����Զ���߳� Ŀ�����·���� 
	case THRD_setctxt:
		record_size += sizeof( thrd_setctxt ); 
		break; //����������߳������� Ŀ�����·���� 
	case THRD_suspend:
		record_size += sizeof( thrd_suspend ); 
		break; //����̹����߳� Ŀ�����·���� 
	case THRD_resume:
		record_size += sizeof( thrd_resume ); 
		break; //����ָ̻��߳� Ŀ�����·���� 
	case THRD_exit:
		record_size += sizeof( thrd_kill ); 
		break; //����̽����߳� Ŀ�����·���� 
	case THRD_queue_apc:
		record_size += sizeof( thrd_queue_apc ); 
		break; //������Ŷ�APC Ŀ�����·���� 

	case SYS_settime:
		record_size += sizeof( sys_settime ); 
		break; //����ϵͳʱ�� �� 
	case SYS_link_knowndll:
		record_size += sizeof( sys_link_knowndll ); 
		break; //����KnownDlls���� �����ļ��� 
	case SYS_open_physmm:
		record_size += sizeof( sys_open_physmm ); 
		break; //�������ڴ��豸 �� 
		//case ACCESS_MEM:
		//	record_size += sizeof( sys_open_physmm ); 
		//	break; 
	case SYS_read_physmm:
		record_size += sizeof( sys_read_physmm ); 
		break; //�������ڴ� �� 
	case SYS_write_physmm:
		record_size += sizeof( sys_write_physmm ); 
		break; //д�����ڴ� �� 
	case SYS_load_kmod:
		record_size += sizeof( sys_load_kmod ); 
		break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
		//case INSTALL_DRV:
		//	record_size += sizeof( sys_load_kmod ); 
		//	break; 
	case SYS_enumproc:
		record_size += sizeof( sys_enumproc ); 
		break; //ö�ٽ��� �� 
	case SYS_regsrv:
		record_size += sizeof( sys_regsrv ); 
		break; //ע����� �������ȫ·�� 
	case SYS_opendev:
		record_size += sizeof( sys_opendev ); 
		break; //���豸 �豸�� 

		//MT_w32mon
	case W32_postmsg:
		record_size += sizeof( w32_postmsg ); 
		break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
	case W32_sendmsg:
		record_size += sizeof( w32_sendmsg ); 
		break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
	case W32_findwnd:
		record_size += sizeof( w32_findwnd ); 
		break; //���Ҵ��� �� 
	case W32_msghook:
		record_size += sizeof( w32_msghook ); 
		break; //������Ϣ���� �� 
		//case INSTALL_HOOK:
		//	record_size += sizeof( w32_msghook ); 
		//	break; 
	case W32_lib_inject:
		record_size += sizeof( w32_lib_inject ); 
		break; //DLLע�� ע��DLL·���� 

		//MT_netmon
	case NET_create:
		record_size += sizeof( net_create ); 
		break; 
		//case SOCKET_CREATE:
		//	record_size += sizeof( net_create ); 
		//	break; 
	case NET_connect:
		record_size += sizeof( net_connect ); 
		break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
		//case SOCKET_CONNECT:
		//	record_size += sizeof( net_connect ); 
		//	break; 
	case NET_listen:
		record_size += sizeof( net_listen ); 
		break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
		//case SOCKET_LISTEN:
		//	record_size += sizeof( net_listen ); 
		//	break; 
	case NET_send:
		record_size += sizeof( net_send ); 
		break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
		//case SOCKET_SEND:
		//	record_size += sizeof( net_send ); 
		//	break; 
	case NET_recv:
		record_size += sizeof( net_recv ); 
		break; 
		//case SOCKET_RECV:
		//	record_size += sizeof( net_recv ); 
		//	break; 
	case NET_accept:
		record_size += sizeof( net_accept ); 
		break; 
		//case SOCKET_ACCEPT:
		//	record_size += sizeof( net_accept ); 
		//	break; 
	case NET_dns:
		record_size += sizeof( net_dns ); 
		break; //
	case NET_http:
		record_size += sizeof( net_http ); 
		break; //HTTP���� HTTP����·������ʽ������/URL�� 
		//case LOCATE_URL:
		//	record_size += sizeof( ); 
		//	break; 
	//case NET_icmp_send:
	//	record_size += sizeof( ipv4_icmp_action ); 
	//	break; 
	//	//case ICMP_SEND:
	//	//	record_size += sizeof( ); 
	//	//	break; 
	//case NET_icmp_recv:
	//	record_size += sizeof( ipv4_icmp_action ); 
	//	break; 
		//case ICMP_RECV:
		//	record_size += sizeof( ); 
		//	break; 


		//MT_behavior, 
	case BA_extract_hidden:
		record_size += sizeof( ba_extract_hidden ); 
		break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
	case BA_extract_pe:
		record_size += sizeof( ba_extract_pe ); 
		break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
	case BA_self_copy:
		record_size += sizeof( ba_self_copy ); 
		break; //���Ҹ��� ����Ŀ���ļ�·���� 
	case BA_self_delete:
		record_size += sizeof( ba_self_delete ); 
		break; //����ɾ�� ɾ���ļ�·���� 
	case BA_ulterior_exec:
		record_size += sizeof( ba_ulterior_exec ); 
		break; //����ִ�� ��ִ��ӳ��·���� 
	case BA_invade_process:
		record_size += sizeof( ba_invade_process ); 
		break; //���ֽ��� Ŀ�����·���� 
	case BA_infect_pe:
		record_size += sizeof( ba_infect_pe ); 
		break; //��ȾPE�ļ� Ŀ���ļ�·���� 
	case BA_overwrite_pe:
		record_size += sizeof( ba_overwrite_pe ); 
		break; //��дPE�ļ� Ŀ���ļ�·���� 
	case BA_register_autorun:
		record_size += sizeof( ba_register_autorun ); 
		break; //ע���������� �������ļ�·���� 

		//case BA_other:
		//record_size += sizeof( ); 
		//break; 

	default:
		ASSERT( FALSE ); 
		record_size = 0; 
		break; 
	}

	return record_size; 
}

//r3_action_notify
typedef struct _event_log_compressed
{
	ULONG size; 
	ULONG data_size; 
	ULARGE_INTEGER id; 
	ULONG frame_count; 
	sys_action_info action; 
	BYTE append[ 1 ]; 
	/*************************
	the data that's saved in the append: 
	1.append data in action record (can compressed)
	2.stack frame (can compressed)
	3.data
	4.the context of the *can compress* too, but process it that is more complex.
	*************************/

	/****************************************************
	2 steps to resolve the store size problem:
	1.correct the size that's allocated in the kernel.
	2.compress the size that's save in ring 3.
	****************************************************/

} event_log_compressed, *pevent_log_compressed; 

#define EVENT_SEARCH_RESULT 0x00000001

typedef struct _r3_action_notify_buf
{
	ULONG status; 
	//LPWSTR stack_dump; 
	r3_action_notify action; 
} r3_action_notify_buf, *pr3_action_notify_buf; 

ULONG WINAPI calc_correct_record_append_size( r3_action_notify *event ); 

ULONG WINAPI calc_correct_all_append_size( r3_action_notify *event ); 

LRESULT WINAPI compress_event_log( r3_action_notify *action, r3_action_notify_buf **event_out ); 

LRESULT WINAPI compress_event_log_ex( r3_action_notify *action, event_log_compressed ** event_out ); 

#endif //__EVENT_STORE_H__