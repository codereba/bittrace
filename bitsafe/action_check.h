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

#ifndef __ACTION_CHECK_H__
#define __ACTION_CHECK_H__

/****
record more actions, the limit is max actions count of its policy,
need match n rules of the when one action occur.
****/

/******
action analyze by nfa:
constructing a nfa when one rule added, then just match once 
when one action occur.
******/

/**************************************************************************************
use nfa mode find the balance pointer, if the actions match the base rule, then record 
the action and the result to one rule.
***************************************************************************************/

#include "acl_define.h"
#include "action_type.h"
#include "ref_node.h"

typedef struct _sys_action_link
{
	HANDLE lock; 
	LIST_ENTRY actions; 
} sys_action_link, *psys_action_link; 

//typedef enum _action_reply
//{
//	ALLOW_ACTION, 
//	DENY_ACTION, 
//	MAX_ACTION_REPLY, 
//} action_reply; 

#define DEF_ACTION_REPLY ACTION_ALLOW 
typedef struct _policy_reply
{
	action_reply reply; 
} policy_reply, *ppolicy_reply; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct _sys_action_policy
{
	action_policy policy; 
	policy_reply reply; 
}sys_action_policy, *psys_action_policy; 

#ifdef __cplusplus
}
#endif //__cplusplus

typedef enum _action_source_type 
{
	ACTION_SOURCE_PROCESS, 
	ACTION_SOURCE_SYSTEM, 
} action_source_type, *paction_source_type ; 

struct _policy_holder; 

typedef NTSTATUS ( CALLBACK *compare_policy_callback )( struct _policy_holder *src_pol, 
struct _policy_holder *dst_pol ); 
typedef NTSTATUS ( CALLBACK *find_policy_callback )( struct _policy_holder *pol, PVOID param ); 
typedef NTSTATUS ( CALLBACK *check_policy_callback )( struct _action_source_info *source, struct _policy_holder *pol, PVOID param ); 

/********************************************************************************
group policy nfa �ṹ:
1.ʵ����Լ������������еĲ��ԣ�֮�����еĲ��Զ������ظ���
2.����Լ������������еĲ����飬ͨ�����������������������Ӳ��ԡ�
********************************************************************************/
typedef struct _group_action_policy
{
	action_policy policy; 
}group_action_policy, *pgroup_action_policy; 

typedef struct _policy_holder
{
	ref_obj obj; 

	PVOID lock; 

	group_action_policy policy; 
} policy_holder, *ppolicy_holder; 

/**************************************************************
policy for learning have not action unit( one process now ) name,just learning action.
every action unit for learning checking will be binding with one 
instance of this struct.
***************************************************************/

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

INLINE LPCWSTR get_action_desc( sys_action_type type )
{
	LPCWSTR	desc = L""; 
	switch( type )
	{
	case OTHER_ACTION: 
		desc = L"OTHER_ACTION"; 
		break; 
	case EXEC_create:
		desc = L"EXEC_create"; 
		break; //�������� ����·���� ��ִ�м�أ� 

	case EXEC_destroy:
		desc = L"EXEC_destroy"; 
		break; //�����˳� ����·���� 

	case EXEC_module_load:
		desc = L"EXEC_module_load"; 
		break; //ģ����� ģ��·���� 

		//MT_filemon, 
	case FILE_touch:
		desc = L"FILE_touch"; 
		break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
	case FILE_open:
		desc = L"FILE_open"; 
		break; //���ļ� �ļ�ȫ·�� 
		//case ACCESS_FILE:
		//	desc = L"ACCESS_FILE"; 
		//	break; 
	case FILE_read:
		desc = L"FILE_read"; 
		break; //��ȡ�ļ� �ļ�ȫ·�� 
	case FILE_write:
		desc = L"FILE_write"; 
		break; //д���ļ� �ļ�ȫ·�� 
		//case MODIFY_FILE:
		//	desc = L"MODIFY_FILE"; 
		//	break; 
	case FILE_modified:
		desc = L"FILE_modified"; 
		break; //�ļ����޸� �ļ�ȫ·�� 
	case FILE_readdir:
		desc = L"FILE_readdir"; 
		break; //����Ŀ¼ Ŀ¼ȫ·�� 
	case FILE_remove:
		desc = L"FILE_remove"; 
		break; //ɾ���ļ� �ļ�ȫ·�� 
		//case DELETE_FILE:
		//	desc = L"DELETE_FILE"; 
		//	break; 
	case FILE_rename:
		desc = L"FILE_rename"; 
		break; //�������ļ� �ļ�ȫ·�� 
	case FILE_truncate:
		desc = L"FILE_truncate"; 
		break; //�ض��ļ� �ļ�ȫ·�� 
	case FILE_mklink:
		desc = L"FILE_mklink"; 
		break; //�����ļ�Ӳ���� �ļ�ȫ·�� 
	case FILE_chmod:
		desc = L"FILE_chmod"; 
		break; //�����ļ����� �ļ�ȫ·�� 
	case FILE_setsec:
		desc = L"FILE_setsec"; 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_getinfo:
		desc = L"FILE_getinfo"; 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_setinfo:
		desc = L"FILE_setinfo"; 
		break; //�����ļ���ȫ���� �ļ�ȫ·�� 
		//MT_regmon, 
	case REG_openkey:
		desc = L"REG_openkey"; 
		break; //��ע����� ע�����·��  ��ע�����أ� 
	case REG_mkkey:
		desc = L"REG_mkkey"; 
		break; //����ע����� ע�����·�� 
		//case MODIFY_KEY:
		//desc = L"MODIFY_KEY"; 
		//	break; 
	case REG_rmkey:
		desc = L"REG_rmkey"; 
		break; //ɾ��ע����� ע�����·�� 
	case REG_mvkey:
		desc = L"REG_mvkey"; 
		break; //������ע����� ע�����·�� 
	case REG_getinfo: 
		desc = L"REG_getinfo"; 
		break; 
	case REG_setinfo: 
		desc = L"REG_setinfo"; 
		break; 
	case REG_rmval:
		desc = L"REG_rmval"; 
		break; //ɾ��ע����� ע�����·�� 
	case REG_getval:
		desc = L"REG_getval"; 
		break; //��ȡע���ֵ ע���ֵ·�� 
	case REG_setval:
		desc = L"REG_setval"; 
		break; //����ע���ֵ ע���ֵ·�� 
	case REG_loadkey:
		desc = L"REG_loadkey"; 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_replkey:
		desc = L"REG_replkey"; 
		break; //�滻ע����� ע�����·�� 
	case REG_rstrkey:
		desc = L"REG_rstrkey"; 
		break; //����ע���Hive�ļ� ע�����·�� 
	case REG_setsec:
		desc = L"REG_setsec"; 
		break; //����ע�������ȫ���� ע�����·�� 
		//MT_procmon, 
	case PROC_exec:
		desc = L"PROC_exec"; 
		break; //�������� Ŀ�����·����  �����̼�أ�
		//case CREATE_PROC:
		//desc = L"CREATE_PROC"; 
		//	break; 
	case PROC_open:
		desc = L"PROC_open"; 
		break; //�򿪽��� Ŀ�����·���� 
	case PROC_debug:
		desc = L"PROC_debug"; 
		break; //���Խ��� Ŀ�����·���� 
	case PROC_suspend:
		desc = L"PROC_suspend"; 
		break; //������� Ŀ�����·���� 
	case PROC_resume:
		desc = L"PROC_resume"; 
		break; //�ָ����� Ŀ�����·���� 
	case PROC_exit:
		desc = L"PROC_kill"; 
		break; //�������� Ŀ�����·���� 
		//case TERMINATE_PROC:
		//desc = L"TERMINATE_PROC"; 
		//	break; 
	case PROC_job:
		desc = L"PROC_job"; 
		break; //�����̼��빤���� Ŀ�����·���� 
	case PROC_pgprot:
		desc = L"PROC_pgprot"; 
		break; //������޸��ڴ����� Ŀ�����·���� 
	case PROC_freevm:
		desc = L"PROC_freevm"; 
		break; //������ͷ��ڴ� Ŀ�����·���� 
	case PROC_writevm:
		desc = L"PROC_writevm"; 
		break; //�����д�ڴ� Ŀ�����·���� 
	case PROC_readvm:
		desc = L"PROC_readvm"; 
		break; //����̶��ڴ� Ŀ�����·���� 
	case THRD_remote:
		desc = L"THRD_remote"; 
		break; //����Զ���߳� Ŀ�����·���� 
	case THRD_setctxt:
		desc = L"THRD_setctxt"; 
		break; //����������߳������� Ŀ�����·���� 
	case THRD_suspend:
		desc = L"THRD_suspend"; 
		break; //����̹����߳� Ŀ�����·���� 
	case THRD_resume:
		desc = L"THRD_resume"; 
		break; //����ָ̻��߳� Ŀ�����·���� 
	case THRD_exit:
		desc = L"THRD_kill"; 
		break; //����̽����߳� Ŀ�����·���� 
	case THRD_queue_apc:
		desc = L"THRD_queue_apc"; 
		break; //������Ŷ�APC Ŀ�����·���� 

		//MT_common
	case COM_access:
		desc = L"COM_access"; 
		break; 
	
	case PORT_read: 
		desc = L"PORT_read"; 
		break; 

	case PORT_write: 
		desc = L"PORT_write"; 
		break; 
	case PORT_urb:
		desc = L"PORT_urb"; 
		//case ACCESS_COM:
		//desc = L"ACCESS_COM"; 
		//	break; 

		//MT_sysmon
	case SYS_settime:
		desc = L"SYS_settime"; 
		break; //����ϵͳʱ�� �� 
	case SYS_link_knowndll:
		desc = L"SYS_link_knowndll"; 
		break; //����KnownDlls���� �����ļ��� 
	case SYS_open_physmm:
		desc = L"SYS_open_physmm"; 
		break; //�������ڴ��豸 �� 
		//case ACCESS_MEM:
		//desc = L"ACCESS_MEM"; 
		//	break; 
	case SYS_read_physmm:
		desc = L"SYS_read_physmm"; 
		break; //�������ڴ� �� 
	case SYS_write_physmm:
		desc = L"SYS_write_physmm"; 
		break; //д�����ڴ� �� 
	case SYS_load_kmod:
		desc = L"SYS_load_kmod"; 
		break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
		//case INSTALL_DRV:
		//desc = L"INSTALL_DRV"; 
		//	break; 
	case SYS_enumproc:
		desc = L"SYS_enumproc"; 
		break; //ö�ٽ��� �� 
	case SYS_regsrv:
		desc = L"SYS_regsrv"; 
		break; //ע����� �������ȫ·�� 
	case SYS_opendev:
		desc = L"SYS_opendev"; 
		break; //���豸 �豸�� 

		//MT_w32mon
	case W32_postmsg:
		desc = L"W32_postmsg"; 
		break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
	case W32_sendmsg:
		desc = L"W32_sendmsg"; 
		break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
	case W32_findwnd:
		desc = L"W32_findwnd"; 
		break; //���Ҵ��� �� 
	case W32_msghook:
		desc = L"W32_msghook"; 
		break; //������Ϣ���� �� 
		//case INSTALL_HOOK:
		//desc = L"INSTALL_HOOK"; 
		//	break; 
	case W32_lib_inject:
		desc = L"W32_lib_inject"; 
		break; //DLLע�� ע��DLL·���� 

		//MT_netmon
	case NET_create:
		desc = L"NET_create"; 
		break; 
	case NET_connect:
		desc = L"NET_connect"; 
		break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
	case NET_listen:
		desc = L"NET_listen"; 
		break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
	case NET_send:
		desc = L"NET_send"; 
		break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
	case NET_recv:
		desc = L"NET_recv"; 
		break; 
	case NET_accept:
		desc = L"NET_accept"; 
		break; 
	case NET_dns:
		desc = L"NET_dns"; 
		break; 

	case NET_http:
		desc = L"NET_http"; 
		break; //HTTP���� HTTP����·������ʽ������/URL�� 
	case NET_icmp_send:
		desc = L"NET_icmp_send"; 
		break; 
		//case ICMP_SEND:
		//desc = L"ICMP_SEND"; 
		//	break; 
	case NET_icmp_recv:
		desc = L"NET_icmp_recv"; 
		break; 

		//MT_behavior, 
	case BA_extract_hidden:
		desc = L"BA_extract_hidden"; 
		break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
	case BA_extract_pe:
		desc = L"BA_extract_pe"; 
		break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
	case BA_self_copy:
		desc = L"BA_self_copy"; 
		break; //���Ҹ��� ����Ŀ���ļ�·���� 
	case BA_self_delete:
		desc = L"BA_self_delete"; 
		break; //����ɾ�� ɾ���ļ�·���� 
	case BA_ulterior_exec:
		desc = L"BA_ulterior_exec"; 
		break; //����ִ�� ��ִ��ӳ��·���� 
	case BA_invade_process:
		desc = L"BA_invade_process"; 
		break; //���ֽ��� Ŀ�����·���� 
	case BA_infect_pe:
		desc = L"BA_infect_pe"; 
		break; //��ȾPE�ļ� Ŀ���ļ�·���� 
	case BA_overwrite_pe:
		desc = L"BA_overwrite_pe"; 
		break; //��дPE�ļ� Ŀ���ļ�·���� 
	case BA_register_autorun:
		desc = L"BA_register_autorun"; 
		break; //ע���������� �������ļ�·���� 
	default:
		ASSERT( FALSE ); 
		break; 
	}

	return desc; 
}

NTSTATUS init_action_check_context(); 
NTSTATUS uninit_action_check_context(); 

#ifdef TEST_IN_RING3
INLINE NTSTATUS _collect_action_context( action_context *info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	ULONG name_len; 

	ASSERT( info != NULL ); 

	info->proc_id = 0; 
	*info->proc_name = '\0'; 
	info->thread_id = 0; 

	do 
	{
		info->proc_id = GetCurrentProcessId(); 
		name_len = GetModuleFileNameW( NULL, ( WCHAR* )info->proc_name, sizeof( info->proc_name ) >> 1 ); 
		if( name_len == 0 )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			break; 
		}

		info->proc_name_len = name_len >> 1; 

		if( info->proc_name[ info->proc_name_len - 1 ] != L'\0' )
		{
			info->proc_name[ info->proc_name_len - 1 ] = L'\0'; 
			info->proc_name_len -= 1; 
		}

		info->thread_id = ( ULONG )GetCurrentThreadId(); 
	}while( FALSE );

	return ntstatus; 
}
#endif //TEST_IN_RING3


NTSTATUS check_sys_action( r3_action_notify *action, 
						  action_reply *resp ); 

NTSTATUS load_action_policies(); 


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__ACTION_CHECK_H__