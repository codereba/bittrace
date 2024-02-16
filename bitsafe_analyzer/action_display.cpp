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
#include "ring0_2_ring3.h"
#include "action_display.h"
#include "action_logger.h"

#include "volume_name_map.h"

#pragma comment( lib, "ws2_32.lib")

#ifdef LANG_EN
#define CURRENT_PROCESS_THREAD_FORMAT_TEXT L"Current process:Thread[%u]" //L"��������:�߳�[%u]", 
#define PROCESS_THREAD_FORMAT_TEXT L"Process[%u]%s:Thread[%u]" //L"����[%u]%s:�߳�[%u]", L"����[%u]%s:�߳�[%u]", 
#define NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT L"Create socket:(Protocol ID:%u)" //L"�����׽���:(Э��ID:%u)"
#define NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT2 L"Create socket:(%s)" //L"�����׽���:(%s)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT L"Connect:(UDP)" //L"��������:(UDPЭ��)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT2 L"Connect:(TCP)" //L"��������:(UDPЭ��)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT3 L"Connect:(Protocol ID:%u)" //L"��������:(Э��ID:%u)"
#define NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT L"Listen port:(Protocol ID:%u)" //L"�����˿�:(Э��ID:%u)"
#define NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT2 L"Listen port:(%s)" //L"�����˿�:(%s)"
#define NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT L"Send:(Protocol ID:%u)" //L"��������:(Э��ID:%u)"
#define NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT2 L"Send:(%s)" //L"��������:(%s)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT L"Receive:(TCP)" //L"��������:(TCPЭ��)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT2 L"Receive:(UDP)" //L"��������:(UDPЭ��)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT3 L"Receive:(Protocol ID:%u)" //L"��������:(Э��ID:%u)"
#define NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT L"Accept:(Protocol ID:%u)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u" //L"������������:(Э��ID:%u)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u"
#define NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT2 L"Accept:(%s)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u" //L"������������:(%s)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u"
#define NETWORK_ICMP_SEND_OBJECT_PATH_FORMAT_TEXT L"Send icmp:Source:%u.%u.%u.%u Dest:%u.%u.%u.%u" //L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u"
#define NETWORK_ICMP_RECEIVE_OBJECT_PATH_FORMAT_TEXT L"Receive icmp:Source:%u.%u.%u.%u Dest:%u.%u.%u.%u" //L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u"
#else
#define CURRENT_PROCESS_THREAD_FORMAT_TEXT L"��������:�߳�[%u]" 
#define PROCESS_THREAD_FORMAT_TEXT L"����[%u]%s:�߳�[%u]" //L"����[%u]%s:�߳�[%u]" 
#define NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT L"�����׽���:(Э��ID:%u)"
#define NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT2 L"�����׽���:(%s)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT L"��������:(UDPЭ��)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT2 L"��������:(UDPЭ��)"
#define NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT3 L"��������:(Э��ID:%u)"
#define NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT L"�����˿�:(Э��ID:%u)"
#define NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT2 L"�����˿�:(%s)"
#define NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT L"��������:(Э��ID:%u)"
#define NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT2 L"��������:(%s)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT L"��������:(TCPЭ��)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT2 L"��������:(UDPЭ��)"
#define NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT3 L"��������:(Э��ID:%u)"
#define NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT L"������������:(Э��ID:%u)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u"
#define NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT2 L"������������:(%s)%u.%u.%u.%u:%u <- %u.%u.%u.%u:%u"
#define NETWORK_ICMP_SEND_OBJECT_PATH_FORMAT_TEXT L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u"
#define NETWORK_ICMP_RECEIVE_OBJECT_PATH_FORMAT_TEXT L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u"

#endif //LANG_EN

/**************************************************************************
NOTICE:
WINDOWS�ṩ���й���־���ݶ��������ݽṹ�ı��֧�֣�ͨ��ʹ��XML->C�ı�����
��ʵ��XML��ʽ�����ݽṹ���嵽C���Դ����ת����MC.EXE��ֱ��ʹ�����������
����зǳ���ĸ������á�
1.WPP ��Ϣ����֧��
	1.�Զ�������Ϣ������д����Ӧ�Ľṹ��Ϣ��PDB�ļ��С�
	2.���Լ����Զ����ENUM���ͣ�ͬʱ֧���Զ����ַ�����ת����
	3.�Զ�����Ϣ��¼������

2.ETW ��Ϣ����֧��
	1.
**************************************************************************/

/*****************
typedef enum _action_main_type
{
MT_execmon, 
MT_filemon, 
MT_regmon, 
MT_procmon, 
MT_common, 
MT_sysmon, 
MT_w32mon, 
MT_netmon, 
MT_behavior, 
MT_unknown
} action_main_type, *paction_main_type; 
****************/

LRESULT WINAPI _get_action_main_type( sys_action_type type, action_main_type *main_type )
{
	LRESULT ret = ERROR_SUCCESS; 
	action_main_type _main_type = MT_unknown; 

	do 
	{
		switch( type )
		{
		//MT_execmon
		case EXEC_create: //�������� ����·���� ��ִ�м�أ� 
		case EXEC_destroy: //�����˳� ����·���� 
		case EXEC_module_load: //ģ����� ģ��·���� 
			_main_type = MT_execmon; 
			break; 
			//MT_filemon, 
		case FILE_touch: //�����ļ� �ļ�ȫ·�� ���ļ���أ� 
		case FILE_open: //���ļ� �ļ�ȫ·�� 
		case FILE_read: //��ȡ�ļ� �ļ�ȫ·�� 
		case FILE_write: //д���ļ� �ļ�ȫ·�� 
		case FILE_modified: //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir: //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove: //ɾ���ļ� �ļ�ȫ·�� 
		case FILE_rename: //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate: //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink: //�����ļ�Ӳ���� �ļ�ȫ·�� 
		case FILE_chmod: //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec: //�����ļ���ȫ���� �ļ�ȫ·�� 
		case FILE_getinfo:
		case FILE_setinfo: 
		case FILE_close:
			_main_type = MT_filemon; 
			break; 

			//MT_regmon, 
		case REG_openkey: //��ע����� ע�����·��  ��ע�����أ� 
		case REG_mkkey: //����ע����� ע�����·�� 
		case REG_rmkey: //ɾ��ע����� ע�����·�� 
		case REG_mvkey: //������ע����� ע�����·�� 
		case REG_rmval: //ɾ��ע����� ע�����·�� 
		case REG_getinfo:
		case REG_setinfo:
		case REG_enuminfo:
		case REG_enum_val:
		case REG_getval: //��ȡע���ֵ ע���ֵ·�� 
		case REG_setval: //����ע���ֵ ע���ֵ·�� 
		case REG_loadkey: //����ע���Hive�ļ� ע�����·�� 
		case REG_replkey: //�滻ע����� ע�����·�� 
		case REG_rstrkey: //����ע���Hive�ļ� ע�����·�� 
		case REG_setsec: //����ע�������ȫ���� ע�����·�� 
		case REG_closekey: 
			_main_type = MT_regmon; 
			break; 

			//MT_procmon, 
		case PROC_exec: //�������� Ŀ�����·����  �����̼�أ�
		case PROC_open: //�򿪽��� Ŀ�����·���� 
		case PROC_debug: //���Խ��� Ŀ�����·���� 
		case PROC_suspend: //������� Ŀ�����·���� 
		case PROC_resume: //�ָ����� Ŀ�����·���� 
		case PROC_exit: //�������� Ŀ�����·���� 
		case PROC_job: //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot: //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm: //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm: //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm: //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote: //����Զ���߳� Ŀ�����·���� 
		case THRD_create: //�����߳�
		case THRD_setctxt: //����������߳������� Ŀ�����·���� 
		case THRD_suspend: //����̹����߳� Ŀ�����·���� 
		case THRD_resume: //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit: //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc: //������Ŷ�APC Ŀ�����·���� 
			_main_type = MT_procmon; 
			break; 
			//MT_common
		case COM_access: 
			_main_type = MT_common; 
			break; 

		case PORT_read:
		case PORT_write:
		case PORT_urb:
			_main_type = MT_peripheral; 
			break; 

			//MT_sysmon
		case SYS_settime: //����ϵͳʱ�� �� 
		case SYS_link_knowndll: //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm: //�������ڴ��豸 �� 
		case SYS_read_physmm: //�������ڴ� �� 
		case SYS_write_physmm: //д�����ڴ� �� 
		case SYS_load_kmod: //�����ں�ģ�� �ں�ģ��ȫ·�� 
		case SYS_load_mod: //����ģ�� �ں�ģ��ȫ·�� 
		case SYS_unload_mod: //ж��ģ�� �ں�ģ��ȫ·�� 
		case SYS_enumproc: //ö�ٽ��� �� 
		case SYS_regsrv: //ע����� �������ȫ·�� 
		case SYS_opendev: //���豸 �豸�� 
			_main_type = MT_sysmon; 
			break; 
			//MT_w32mon
		case W32_postmsg: //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg: //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd: //���Ҵ��� �� 
		case W32_msghook: //������Ϣ���� �� 
		case W32_lib_inject: //DLLע�� ע��DLL·���� 
			_main_type = MT_w32mon; 
			break; 
			//MT_netmon, 
		case NET_create: 
		case NET_connect: //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
		case NET_listen: //�����˿� ������ַ����ʽ��IP���˿ںţ� 
		case NET_send: //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
		case NET_recv: 
		case NET_accept: 

		case NET_dns:
		case NET_http: //HTTP���� HTTP����·������ʽ������/URL�� 
		case NET_icmp_send: 
		case NET_icmp_recv: 
			_main_type = MT_netmon; 
			break; 
			//MT_behavior, 
		case BA_extract_hidden: //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe: //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy: //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete: //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec: //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process: //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe: //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe: //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun: //ע���������� �������ļ�·���� 
		case BA_other: 
			_main_type = MT_behavior; 
			break; 
		default:
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE );

	*main_type = _main_type; 

	return ret; 
}

LRESULT WINAPI get_access_desc( ACCESS_MASK access, WCHAR *access_desc, ULONG ccb_buf_len, ULONG *ccb_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

	}while( FALSE );

	return ret; 
}

LPCWSTR _get_reg_value_type_desc( ULONG type )
{
	LPCWSTR reg_val_type_desc = L""; 

	switch( type )
	{
	case REG_NONE:
		reg_val_type_desc = L"REG_NONE"; 
		break; 
	case REG_SZ:
		reg_val_type_desc = L"REG_SZ"; 
		break; 
	case REG_EXPAND_SZ:
		reg_val_type_desc = L"REG_EXPAND_SZ"; 
		break; 
	case REG_BINARY:
		reg_val_type_desc = L"REG_BINARY"; 
		break; 
	case REG_DWORD:
		reg_val_type_desc = L"REG_DWORD"; 
		break; 
		//case REG_DWORD_LITTLE_ENDIAN:
		//	reg_val_type_desc = L"REG_DWORD_LITTLE_ENDIAN"; 
		//	break; 
	case REG_DWORD_BIG_ENDIAN:
		reg_val_type_desc = L"REG_DWORD_BIG_ENDIAN"; 
		break; 
	case REG_LINK:
		reg_val_type_desc = L"REG_LINK"; 
		break; 
	case REG_MULTI_SZ:
		reg_val_type_desc = L"REG_MULTI_SZ"; 
		break; 
	case REG_RESOURCE_LIST:
		reg_val_type_desc = L"REG_RESOURCE_LIST"; 
		break; 
	case REG_FULL_RESOURCE_DESCRIPTOR:
		reg_val_type_desc = L"REG_FULL_RESOURCE_DESCRIPTOR"; 
		break; 
	case REG_RESOURCE_REQUIREMENTS_LIST:
		reg_val_type_desc = L"REG_RESOURCE_REQUIREMENTS_LIST"; 
		break; 
	case REG_QWORD:
		reg_val_type_desc = L"REG_QWORD"; 
		break; 
	//case REG_QWORD_LITTLE_ENDIAN:
	//	reg_val_type_desc = L"REG_QWORD_LITTLE_ENDIAN"; 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}

	return reg_val_type_desc; 
}

LRESULT WINAPI get_net_recv_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	TCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	TCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_recv )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->do_net_send.protocol == PROT_TCP 
			&& action->do_net_send.protocol == PROT_UDP )
		{
			ret = ERROR_INVALID_PARAMETER_2; 
			break; 
		}

		ret = ip_addr_2_str( action->do_net_send.local_ip_addr, src_ip_addr, IPV4_ADDR_LEN ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = ip_addr_2_str( action->do_net_send.ip_addr, dest_ip_addr, IPV4_ADDR_LEN ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		fmt = _get_string_by_id( TEXT_SYS_ACTION_RECEIVE_DATA, 
			_T( "������������( %s:%d��%s:%d )" ) ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			action->do_net_send.local_port, 
			dest_ip_addr, 
			action->do_net_send.port ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_accept_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_accept )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		fmt = _T( "��%s:%u����%s:%u �� %s ��������\n" ); 

		ip_addr_2_str( action->do_net_accept.local_ip_addr, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->do_net_accept.ip_addr, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( ( prot_type )action->do_net_accept.protocol ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			action->do_net_accept.local_port, 
			dest_ip_addr, 
			action->do_net_accept.port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_net_send_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	TCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	TCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_send )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->do_net_send.protocol == PROT_TCP 
			&& action->do_net_send.protocol == PROT_UDP )
		{
			ret = ERROR_INVALID_PARAMETER_2; 
			break; 
		}

		ret = ip_addr_2_str( ntohl( action->do_net_send.local_ip_addr ), src_ip_addr, IPV4_ADDR_LEN ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = ip_addr_2_str( ntohl( action->do_net_send.ip_addr ), dest_ip_addr, IPV4_ADDR_LEN ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		fmt = _get_string_by_id( TEXT_SYS_ACTION_SEND_DATA, 
			_T( "������������( %s:%d��%s:%d )" ) ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			action->do_net_send.local_port, 
			dest_ip_addr, 
			action->do_net_send.port ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_create_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	TCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	TCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_create )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = ip_addr_2_str( action->do_net_create.local_ip_addr, src_ip_addr, IPV4_ADDR_LEN ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//IP_ADDR_T local_ip_addr; 
		//PORT_T local_port; 
		//ULONG protocol; //Э�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//IP_ADDR_T ip_addr; 
		//PORT_T port; 

		if( action->do_net_create.protocol == PROT_TCP )
		{
			LPCWSTR port_desc; 

			ret = ip_addr_2_str( action->do_net_create.ip_addr, dest_ip_addr, IPV4_ADDR_LEN ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			port_desc = get_port_desc( ( USHORT )action->do_net_create.port ); 

			fmt = _get_string_by_id( TEXT_SYS_ACTION_CREATE_TCP_SOCKET, 
				_T( "������ %s �׽���( %s:%d��%s:%d%s )" ) ); 

			_ret = _snwprintf( tip, 
				ccb_buf_len, 
				fmt, 
				PROT_TCP_DESC, 
				src_ip_addr, 
				action->do_net_create.local_port, 
				dest_ip_addr, 
				action->do_net_create.port, 
				port_desc );
		}
		else if( action->do_net_create.protocol == PROT_UDP )
		{
			fmt = _get_string_by_id( TEXT_SYS_ACTION_CREATE_UDP_SOCKET, 
				_T( "������ %s �׽���( %s:%d )" ) ); 

			_ret = _snwprintf( tip, 
				ccb_buf_len, 
				fmt, 
				PROT_UDP_DESC, 
				src_ip_addr, 
				action->do_net_create.port );
		}
		else
		{
			_ret = 0; 
		}

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_http_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_http )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG protocol; //Э�� 
		//ULONG cmd; //HTTP���� 
		//ULONG data_len; //�������ݳ��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //URL 
		fmt = _T( "���� %s ����/ͨ�� Э�� %u ���� %u ����� %u \n" ); 

		ASSERT( action->do_net_http.path_name[ action->do_net_http.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_net_http.path_name, 
			action->do_net_http.protocol, 
			action->do_net_http.cmd, 
			action->do_net_http.data_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_extract_hidden_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_extract_hidden )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ͷ��ļ� %s\n"; 


		ASSERT( action->do_ba_extract_hidden.path_name[ action->do_ba_extract_hidden.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_extract_hidden.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_extract_pe_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ASSERT( params_desc != NULL ); 

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_extract_pe )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ͷ�PE�ļ� %s\n"; 

		ASSERT( action->do_ba_extract_pe.path_name[ action->do_ba_extract_pe.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_extract_pe.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_self_copy_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_self_copy )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"���������ļ��� %s\n"; 


		ASSERT( action->do_ba_self_copy.path_name[ action->do_ba_self_copy.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_self_copy.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_self_delete_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_self_delete )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"ͨ�� %s ɾ�������ļ�\n"; 

		ASSERT( action->do_ba_self_delete.path_name[ action->do_ba_self_delete.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_self_delete.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_ulterior_exec_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_ulterior_exec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����ִ�г���%s\n"; 


		ASSERT( action->do_ba_ulterior_exec.path_name[ action->do_ba_ulterior_exec.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt,  
			action->do_ba_ulterior_exec.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_invade_process_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_invade_process )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"ע����� %s (ID:%u)\n"; 

		ASSERT( action->do_ba_invade_process.path_name[ action->do_ba_invade_process.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_invade_process.path_name, 
			action->do_ba_invade_process.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_infect_pe_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		

		
		
		
		
		
		

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_infect_pe )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"��Ⱦ(д�벡������)��ִ��(PE)�ļ� %s\n"; 

		
		
		
		
		
		

		ASSERT( action->do_ba_infect_pe.path_name[ action->do_ba_infect_pe.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_infect_pe.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_overwrite_pe_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		

		
		
		
		
		
		

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_overwrite_pe )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�޸Ŀ�ִ��(PE)�ļ�%s\n"; 

		
		
		
		
		
		

		ASSERT( action->do_ba_overwrite_pe.path_name[ action->do_ba_overwrite_pe.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_overwrite_pe.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_register_autorun_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		

		
		
		
		
		
		

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != BA_register_autorun )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�����Զ����г��� %s ����%u\n"; 

		
		
		
		
		
		

		ASSERT( action->do_ba_register_autorun.path_name[ action->do_ba_register_autorun.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_ba_register_autorun.path_name, 
			action->do_ba_register_autorun.type ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_file_remove_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR path_name; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		
		
		
		
		
		
		

		if( action->type != FILE_remove )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		
		

		fmt = _get_string_by_id( TEXT_SYS_ACTION_DELETE_FILE, 
			_T( "ɾ���������ļ� %s\n" ) ); 

		_name_len = action->do_file_remove.path_len; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_remove.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_remove.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_remove.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_remove.path_len == 0 ); 
		}

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			_MAX_DESC_LEN, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_remove.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param );

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_read_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_read )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		
		

		fmt = _T( "��ȡ�ļ� %s ƫ���� %I64u ���� %u\n"); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_read.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_read.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_read.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_read.path_len == 0 ); 
		}

		_name_len = action->do_file_read.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			action->do_file_read.offset, 
			action->do_file_read.data_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_write_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_write )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		
		

		fmt = _T( "д���ļ� %s ƫ���� %I64u ���� %u\n"); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_write.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_write.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_write.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_write.path_len == 0 ); 
		}

		_name_len = action->do_file_write.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			action->do_file_write.offset, 
			action->do_file_write.data_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_reg_mk_key_desc( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	//WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		

		
		
		
		
		
		

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_mkkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
		break; 

		fmt = _get_string_by_id( TEXT_SYS_ACTION_MODIFY_REGISTRY, 
			_T( "�޸ı�����ע�����/ֵ %s Ȩ�� 0x%0.8x \n" ) ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_mkkey.path_name_ptr; 
		}
		else
		{
			path_name = action->do_reg_mkkey.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_mkkey.path_len ] == L'\0' ); 		
		}
		else
		{
			ASSERT( action->do_reg_mkkey.path_len == 0 ); 
		}

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			( LPWSTR )path_name, 
			action->do_reg_mkkey.access ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_com_access_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != COM_access )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = ERROR_ERRORS_ENCOUNTERED; 
		break; 

		fmt = _get_string_by_id( TEXT_SYS_ACTION_INSTALL_COM, 
			_T( "����COM��� %s," ) ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_readvm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret;  
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_readvm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "ֱ�Ӷ����� %s ����ID %u �ڴ�, ����ַ 0x%0.8x ���� %u\n" ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_readvm.path_name_ptr; 
		}
		else
		{
			path_name = action->do_proc_readvm.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_proc_readvm.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_proc_readvm.path_len == 0 ); 
		}

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			( LPWSTR )path_name, 
			action->do_proc_readvm.target_pid, 
			action->do_proc_readvm.base, 
			action->do_proc_readvm.data_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_exec_desc( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret;  
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_exec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "���ɽ��� %s ����ID %u\n" ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_exec.path_name_ptr; 
		}
		else
		{
			path_name = action->do_proc_exec.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_proc_exec.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_proc_exec.path_len == 0 ); 
		}

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			( LPCWSTR )path_name, 
			action->do_proc_exec.path_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_w32_msg_hook_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != W32_msghook )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		} 
		
		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_w32_msghook.path_name_ptr; 
		}
		else
		{
			path_name = action->do_w32_msghook.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_w32_msghook.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_w32_msghook.path_len == 0 ); 
		}

		fmt = _T( "��װ���Ӻ��� ���� 0x%0.8x ���� %u Ŀ�����ID %u ����ģ���ַ 0x%0.8x ����ģ��·�� %s\n" ); 

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_msghook.hookfunc, 
			action->do_w32_msghook.hooktype, 
			action->do_w32_msghook.target_pid, 
			action->do_w32_msghook.modbase, 
			( LPWSTR )path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_w32_lib_inject_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR lib_path; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != W32_lib_inject )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����� %s (ID:%u) ע�붯̬�� %s\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_w32_lib_inject.path_name_ptr; 
			lib_path = action->do_w32_lib_inject.lib_path_ptr; 

		}
		else
		{
			path_name = action->do_w32_lib_inject.path_name; 
			lib_path = action->do_w32_lib_inject.path_name + action->do_w32_lib_inject.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_w32_lib_inject.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_w32_lib_inject.path_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			action->do_w32_lib_inject.target_pid, 
			lib_path ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_connect_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_connect )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		fmt = _T( "��%s:%u������%s:%u %s�˿�\n" ); 

		ip_addr_2_str( action->do_net_connect.local_ip_addr, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->do_net_connect.ip_addr, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( ( prot_type )action->do_net_connect.protocol ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			action->do_net_connect.local_port, 
			dest_ip_addr, 
			action->do_net_connect.port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_net_listen_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_listen )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "����%s:%u %s�˿�\n" ); 

		ip_addr_2_str( action->do_net_listen.local_ip_addr, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 

		param = get_prot_type_desc( ( prot_type )action->do_net_listen.protocol ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			action->do_net_listen.local_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_exec_create_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	LPCWSTR cmd_line; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != EXEC_create )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _get_string_by_id( TEXT_SYS_ACTION_LOAD_DRIVER, 
			_T( "���ɿ�ִ���ļ� %s," ) ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_exec_create.path_name_ptr; 
			cmd_line = action->do_exec_create.cmd_line_ptr; 
		}
		else
		{
			path_name = action->do_exec_create.path_name; 
			cmd_line = action->do_exec_create.path_name + action->do_exec_create.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_exec_create.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_create.path_len == 0 ); 
		}

		if( cmd_line != NULL )
		{
			ASSERT( cmd_line[ action->do_exec_create.cmd_len] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_create.cmd_len == 0 ); 
		}

		_name_len = action->do_exec_create.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_exec_module_load_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != EXEC_module_load )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _get_string_by_id( TEXT_SYS_ACTION_LOAD_DRIVER, 
			_T( "����DLL %s ������ַ 0x%0.8x %u\n" ) ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_exec_module_load.path_name_ptr; 
		}
		else
		{
			path_name = action->do_exec_module_load.path_name; 
		}

		if( path_name!= NULL )
		{
			ASSERT( path_name[ action->do_exec_module_load.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_module_load.path_len == 0 ); 
		}

		_name_len = action->do_exec_module_load.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_exec_destroy_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR cmd_line; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != EXEC_destroy )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _get_string_by_id( TEXT_SYS_ACTION_LOAD_DRIVER, 
			_T( "�رս��� %s %s\n" ) ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_exec_destroy.path_name_ptr; 
			cmd_line = action->do_exec_destroy.cmd_line_ptr; 
		}
		else
		{
			path_name = action->do_exec_destroy.path_name; 
			cmd_line = action->do_exec_destroy.path_name + action->do_exec_destroy.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_exec_destroy.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_destroy.path_len == 0 ); 
		}

		if( cmd_line != NULL )
		{
			ASSERT( cmd_line[ action->do_exec_destroy.cmd_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_destroy.cmd_len == 0 ); 
		}

		_name_len = action->do_exec_destroy.path_len; 

		if( action->do_exec_destroy.path_name[ action->do_exec_destroy.path_len ] != L'\0' )
		{
			action->do_exec_destroy.path_name[ action->do_exec_destroy.path_len ] = L'\0'; 
		}

		if( cmd_line[ action->do_exec_destroy.cmd_len ] != L'\0' )
		{
			( ( WCHAR* )cmd_line )[ action->do_exec_destroy.cmd_len ] = L'\0'; 
		}

		ret = convert_native_path_to_dos( action->do_exec_destroy.path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			cmd_line ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_touch_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_touch )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�����ļ�%s ����\n"; 

		if( SAVE_INFO_BY_POINTER & flags )
		{
			path_name = action->do_file_touch.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_touch.path_name; 
		}

		ASSERT( path_name[ action->do_file_touch.path_len ] == L'\0' ); 

		_name_len = action->do_file_touch.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_open_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_open )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"���ļ�%s\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_open.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_open.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_open.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( path_name[ action->do_file_open.path_len ] == 0 ); 
		}

		_name_len = action->do_file_open.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_modified_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_modified )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}


		fmt = L"�޸��ļ�%s\n"; //L"�ɹ�" : L"ʧ��"

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_modified.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_modified.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_modified.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_modified.path_len == 0 ); 
		}

		_name_len = action->do_file_modified.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_readdir_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_readdir )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"��Ŀ¼%s\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_readdir.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_readdir.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_readdir.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_readdir.path_len == 0 ); 
		}

		_name_len = action->do_file_readdir.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_rename_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR new_name; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_rename )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�������ļ�%sΪ%s\n"; 


		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_rename.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_rename.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_rename.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_rename.path_len == 0 ); 
		}

		_name_len = action->do_file_rename.path_len; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_rename.path_name_ptr; 
			new_name = action->do_file_rename.new_name_ptr; 
		}
		else
		{
			path_name = action->do_file_rename.path_name; 
			new_name = action->do_file_rename.path_name + action->do_file_rename.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( new_name[ action->do_file_rename.new_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_rename.new_name_len == 0 ); 
		}

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			new_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_truncate_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_truncate )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�ض��ļ�%s������%u\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_truncate.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_truncate.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_truncate.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_truncate.path_len == 0 ); 
		}

		_name_len = action->do_file_truncate.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			action->do_file_truncate.eof ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_mklink_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR link_name; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_mklink )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�����ļ�%sӲ����%s ģʽ%u\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_mklink.path_name_ptr; 
			link_name = action->do_file_mklink.link_name_ptr; 
		}
		else
		{
			path_name = action->do_file_mklink.path_name; 
			link_name = action->do_file_mklink.path_name + action->do_file_mklink.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_mklink.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_mklink.path_len == 0 ); 
		}

		_name_len = action->do_file_mklink.path_len; 

		if( link_name != NULL )
		{
			ASSERT( link_name[ action->do_file_mklink.link_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_mklink.link_name_len == 0 ); 
		}

		ret = convert_native_path_to_dos( action->do_file_mklink.path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			link_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_chmod_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_chmod )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�޸��ļ�%s����Ϊ%u\n"; 

		

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_chmod.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_chmod.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_chmod.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_chmod.path_len == 0 ); 
		}

		_name_len = action->do_file_chmod.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param, 
			action->do_file_chmod.attrib ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_setsec_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_setsec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�����ļ�%s��ȫ����\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_setsec.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_setsec.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_setsec.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_setsec.path_len == 0 ); 
		}

		_name_len = action->do_file_setsec.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_getinfo_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_setsec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"��ѯ�ļ�%s�����Ϣ\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_setsec.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_setsec.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_setsec.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_setsec.path_len == 0 ); 
		}

		_name_len = action->do_file_setsec.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_file_setinfo_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != FILE_setsec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = L"�����ļ�%s�����Ϣ\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_file_setsec.path_name_ptr; 
		}
		else
		{
			path_name = action->do_file_setsec.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_file_setsec.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_setsec.path_len == 0 ); 
		}

		_name_len = action->do_file_setsec.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_reg_open_key_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_openkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"��ע�����%s Ȩ��%u\n"; 

		if( SAVE_INFO_BY_POINTER & flags )
		{
			path_name = action->do_reg_openkey.path_name_ptr; 
		}
		else
		{
			path_name = action->do_reg_openkey.path_name; 
		}

		ASSERT( path_name[ action->do_reg_openkey.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			action->do_reg_openkey.access ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_mkkey_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_mkkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����ע�����%s Ȩ��%u\n"; 

		ASSERT( action->do_reg_mkkey.path_name[ action->do_reg_mkkey.path_len ] == L'\0' ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_mkkey.path_name_ptr; 
		}
		else
		{
			path_name = action->do_reg_mkkey.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_mkkey.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_mkkey.path_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			action->do_reg_mkkey.access ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_rmkey_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_rmkey)
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"ɾ��ע�����%s\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_rmkey.path_name_ptr; 
		}
		else
		{
			path_name = action->do_reg_rmkey.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_rmkey.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_rmkey.path_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_mvkey_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR new_path_name; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_mvkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ƶ�ע����� %s �� %s \n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_mvkey.path_name_ptr; 
			new_path_name = action->do_reg_mvkey.new_key_name_ptr; 

		}
		else
		{
			path_name = action->do_reg_mvkey.path_name; 
			new_path_name = action->do_reg_mvkey.path_name + action->do_reg_mvkey.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_mvkey.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_mvkey.path_len == 0 ); 
		}

		if( new_path_name != NULL )
		{
			ASSERT( new_path_name[ action->do_reg_mvkey.new_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_mvkey.new_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			new_path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_rmval_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_rmval )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"ɾ��ע�����ֵ %s\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_rmval.key_name_ptr; 
		}
		else
		{
			path_name = action->do_reg_rmval.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_rmval.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_rmval.path_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_getval_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR val_name; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_getval )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"��ע�����%s ֵ %s ���� %u ���� %u\n"; 

		//ULONG type; //ע���ֵ���� 
		//ULONG data_len; //���ݳ��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T val_name_len; 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //ע���ֵ·�� 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_getval.key_name_ptr; 
			val_name = action->do_reg_getval.value_name_ptr;  
		}
		else
		{
			path_name = action->do_reg_getval.path_name; 
			val_name = action->do_reg_getval.path_name + action->do_reg_getval.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_getval.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_getval.path_len == 0 ); 
		}

		if( val_name != NULL )
		{
			ASSERT( val_name[ action->do_reg_getval.val_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_getval.val_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			val_name, 
			action->do_reg_getval.type, 
			action->do_reg_getval.info_size ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_setval_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR value_name; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 
	LPCWSTR val_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_setval )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG tid; //�����������߳�ID 
		//ULONG type; //ע���ֵ���� 
		//ULONG data_len; //���ݳ��� 

		fmt = L"�޸�ע�����%s %s ֵ %s ����%u\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_reg_getval.key_name_ptr; 
			val_name = action->do_reg_getval.value_name_ptr;  
		}
		else
		{
			path_name = action->do_reg_getval.path_name; 
			val_name = action->do_reg_getval.path_name + action->do_reg_getval.path_len + 1; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_reg_getval.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_getval.path_len == 0 ); 
		}

		if( val_name != NULL )
		{
			ASSERT( val_name[ action->do_reg_getval.val_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_getval.val_name_len == 0 ); 
		}

		_name_len = action->do_reg_setval.path_len; 

		value_name = action->do_reg_setval.path_name + action->do_reg_setval.path_len + 1; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
			get_reg_value_type_desc( action->do_reg_setval.type ), 
			value_name, 
			action->do_reg_setval.data_len ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_load_key_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR hive_file_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_loadkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����ע�����%s hive�ļ�%s\n"; 

		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//WCHAR path_name[ 1 ]; //ע�����·���� 

		ASSERT( action->do_reg_loadkey.path_name[ action->do_reg_loadkey.path_len ] == L'\0' ); 

		hive_file_name = action->do_reg_loadkey.path_name + action->do_reg_loadkey.path_len + 1; 

		ASSERT( hive_file_name[ action->do_reg_loadkey.hive_name_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_reg_loadkey.path_name, 
			hive_file_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_repl_key_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR old_hive_file_name; 
	LPCWSTR new_hive_file_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_replkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//PATH_SIZE_T new_hive_name_len; 
		//WCHAR path_name[ 1 ]; //ע�����·���� 

		fmt = L"�滻ע�����%s �� %s �� %s HIVE�ļ�\n"; 

		ASSERT( action->do_reg_replkey.path_name[ action->do_reg_replkey.path_len ] == L'\0' ); 

		old_hive_file_name = action->do_reg_replkey.path_name + action->do_reg_replkey.path_len + 1; 
		new_hive_file_name = action->do_reg_replkey.path_name + action->do_reg_replkey.path_len 
			+ action->do_reg_replkey.hive_name_len
			+ 2; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_reg_replkey.path_name, 
			old_hive_file_name, 
			new_hive_file_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_rstr_key_desc( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR hive_file_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != REG_rstrkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"HIVE�ļ� %s\n"; 

		//NTSTATUS result;//������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//WCHAR path_name[ 1 ]; //ע�����·���� 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			hive_file_name = action->do_reg_rstrkey.hive_name_ptr; 
		}
		else
		{
			hive_file_name = action->do_reg_rstrkey.path_name + action->do_reg_rstrkey.path_len + 1; 
		}

		if( hive_file_name != NULL )
		{
			ASSERT( hive_file_name[ action->do_reg_replkey.hive_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_replkey.hive_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			hive_file_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_open_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_open )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG access; //���̴�Ȩ�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"��Ȩ�� %0.8x ��Ŀ����� %u\n"; 

		ASSERT( action->do_proc_open.path_name[ action->do_proc_open.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_open.path_name, 
			action->do_proc_open.access,  
			action->do_proc_open.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_debug_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_debug )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"���Խ���%u\n"; 

		ASSERT( action->do_proc_debug.path_name[ action->do_proc_debug.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_debug.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_suspend_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_suspend )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"�������%u\n"; 

		ASSERT( action->do_proc_suspend.path_name[ action->do_proc_suspend.path_len ] == L'\0' );  

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_suspend.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_resume_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_resume )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ָ�����%s (ID:%u) ִ��\n"; 

		ASSERT( action->do_proc_resume.path_name[ action->do_proc_resume.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_resume.path_name, 
			action->do_proc_resume.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_kill_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_exit )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"��ֹ����%s ID %u �˳�����%u\n"; 

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG exitcode; //�����Ƴ��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		ASSERT( action->do_proc_kill.path_name[ action->do_proc_kill.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_kill.path_name, 
			action->do_proc_kill.target_pid, 
			action->do_proc_kill.exitcode ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_job_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_job )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR pathname[ 1 ]; //Ŀ�����ȫ·��
	
		fmt = L"������̹����� %s ID:%u\n"; 

		ASSERT( action->do_proc_job.path_name[ action->do_proc_job.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_job.path_name, 
			action->do_proc_job.target_pid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_pg_prot_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_pgprot )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//PVOID base; //�޸��ڴ��ַ 
		//ULONG count; //�޸��ڴ泤�� 
		//ACCESS_MASK attrib; //�޸��ڴ����� 
		//ULONG bytes_changed; //�ɹ��޸ĳ��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"�޸Ľ��� %s (ID:%u) �ڴ� 0x%0.8x (����:%u) ������0x%0.8x (�޸Ľ������:%u)\n"; 

		ASSERT( action->do_proc_pgprot.path_name[ action->do_proc_pgprot.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_pgprot.path_name, 
			action->do_proc_pgprot.target_pid, 
			action->do_proc_pgprot.base, 
			action->do_proc_pgprot.count, 
			action->do_proc_pgprot.attrib, 
			action->do_proc_pgprot.bytes_changed ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_free_vm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_freevm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ͷŽ��� %s (ID:%u) �ڴ� 0x%0.8x (����:%u ����:%u) (�ͷŽ������:%u\n)\n"; 

		//ULONG target_pid; //Ŀ�����ID 
		//PVOID base; //�ͷ��ڴ��ַ 
		//ULONG count; //�ͷ��ڴ泤�� 
		//ULONG type; //�ͷ��ڴ����� 
		//ULONG bytes_freed; //�ɹ��ͷų��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 


		ASSERT( action->do_proc_freevm.path_name[ action->do_proc_freevm.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_freevm.path_name, 
			action->do_proc_freevm.target_pid, 
			action->do_proc_freevm.base, 
			action->do_proc_freevm.count, 
			action->do_proc_freevm.type, 
			action->do_proc_freevm.bytes_freed ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_write_vm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_writevm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//PVOID base; //д���ڴ��ַ 
		//ULONG data_len; //��ͼд�볤�� 
		//ULONG bytes_written; //�ɹ�д�볤�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"д�����%s (ID:%u) �ڴ� 0x%0.8x (����:%u) (д��������:%u)\n"; 

		ASSERT( action->do_proc_writevm.path_name[ action->do_proc_writevm.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_writevm.path_name, 
			action->do_proc_writevm.target_pid, 
			action->do_proc_writevm.base, 
			action->do_proc_writevm.data_len, 
			action->do_proc_writevm.bytes_written ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_read_vm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != PROC_readvm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"��ȡ���� %s (ID:%u) �ڴ� 0x%0.8x (����:%u) (��ȡ�������:%u)\n"; 

		//ULONG target_pid; //Ŀ�����ID 
		//PVOID base; //��ȡ�ڴ��ַ 
		//ULONG data_len; //��ͼ��ȡ���� 
		//ULONG bytes_read; //�ɹ���ȡ���� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		ASSERT( action->do_proc_readvm.path_name[ action->do_proc_readvm.path_len ] == L'\0' ); 


		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_readvm.path_name, 
			action->do_proc_readvm.target_pid, 
			action->do_proc_readvm.base, 
			action->do_proc_readvm.data_len, 
			action->do_proc_readvm.bytes_read ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_remote_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_remote )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		//ULONG target_pid; //Ŀ�����ID 
		//ULONG target_tid; //Ŀ���߳�ID 
		//ACCESS_MASK access; //�̴߳���Ȩ�� 
		//BOOLEAN suspended; //�Ƿ����ʽ���� 
		//PVOID start_vaddr; //�߳���ʼ��ַ 
		//PVOID thread_param; //�̲߳��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"��������%s (ID:%u) Զ���߳� (ID:%u) Ȩ��:%u ���� %u ��ʼ��ַ 0x%0.8x ���� 0x%0.8x\n"; 

		ASSERT( action->do_thrd_remote.path_name[ action->do_thrd_remote.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_remote.path_name, 
			action->do_thrd_remote.target_pid, 
			action->do_thrd_remote.target_tid, 
			action->do_thrd_remote.access,
			action->do_thrd_remote.suspended, 
			action->do_thrd_remote.start_vaddr, 
			action->do_thrd_remote.thread_param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_set_ctxt_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_setctxt )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG target_tid; //Ŀ���߳�ID 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"���ý���%s (ID:%u) �߳�:%u ��������\n"; 

		ASSERT( action->do_thrd_setctxt.path_name[ action->do_thrd_setctxt.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_setctxt.path_name, 
			action->do_thrd_setctxt.target_pid, 
			action->do_thrd_setctxt.target_tid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_suspend_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_suspend )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG target_tid; //Ŀ���߳�ID 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"�������%s (ID:%u) �߳�:%u\n"; 

		ASSERT( action->do_thrd_suspend.path_name[ action->do_thrd_suspend.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_suspend.path_name, 
			action->do_thrd_suspend.target_pid, 
			action->do_thrd_suspend.target_tid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_resume_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_resume )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"�ָ�����%s (ID:%u) �߳�:%u ����\n"; 

		ASSERT( action->do_thrd_resume.path_name[ action->do_thrd_resume.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_resume.path_name, 
			action->do_thrd_resume.target_pid, 
			action->do_thrd_resume.target_tid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_kill_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_exit )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG target_tid; //Ŀ���߳�ID 
		//ULONG exitcode; //�߳��˳��� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"��ֹ���� %s (ID:%u) �߳�%u �˳�����:0x%0.8x\n"; 

		ASSERT( action->do_thrd_kill.path_name[ action->do_thrd_kill.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_kill.path_name, 
			action->do_thrd_kill.target_pid, 
			action->do_thrd_kill.target_tid, 
			action->do_thrd_kill.exitcode ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_thread_queue_apc_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != THRD_queue_apc )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//ULONG target_tid; //Ŀ���߳�ID 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·��

		fmt = L"���ӽ��� %s (ID:%u) �߳� %u APC ����\n"; 

		ASSERT( action->do_thrd_queue_apc.path_name[ action->do_thrd_queue_apc.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_queue_apc.path_name, 
			action->do_thrd_queue_apc.target_pid, 
			action->do_thrd_queue_apc.target_tid ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_settime_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_settime )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����ϵͳʱ�� %u(���ʱ��)\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_settime.time_delta ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_link_knowndll_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_link_knowndll )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"����ϵͳ Known Dll(��̬��) %s\n"; 

		ASSERT( action->do_sys_link_knowndll.path_name[ action->do_sys_link_knowndll.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_link_knowndll.path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_open_physmm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_open_physmm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ACCESS_MASK access; //��Ȩ�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 

		fmt = L"ϵͳ�ڴ��豸(PhysicalMemory) Ȩ��:0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_open_physmm.access ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_read_physmm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_read_physmm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG offset; //��ȡƫ�� 
		//ULONG count; //��ȡ���� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 

		fmt = L"ϵͳ�ڴ� ƫ��:%u ����%u\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_read_physmm.offset, 
			action->do_sys_read_physmm.count ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_write_physmm_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_write_physmm )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG offset; //д��ƫ�� 
		//ULONG count; //д�볤�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 

		fmt = L"д��ϵͳ�ڴ��豸(PhysicalMemory) ƫ�� %u ���� %u\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_write_physmm.offset, 
			action->do_sys_write_physmm.count ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}


LRESULT WINAPI get_sys_load_kmod_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *params_desc, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_load_kmod )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _get_string_by_id( TEXT_SYS_ACTION_LOAD_DRIVER, 
			_T( "�������� %s," ) ); 


		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_sys_load_kmod.path_name_ptr; 
		}
		else
		{
			path_name = action->do_sys_load_kmod.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_sys_load_kmod.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_sys_load_kmod.path_len == 0 ); 
		}

		_name_len = action->do_sys_load_kmod.path_len; 

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_sys_enumproc_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_enumproc )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = L"ö�ٽ���\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_regsrv_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR srv_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_regsrv )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ACCESS_MASK access; //���񴴽�/��Ȩ�� 
		//ULONG type; //�������� 
		//ULONG starttype; //������������ 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T srv_name_len; 
		//WCHAR path_name[ 1 ]; //����/�����ļ�ȫ·�� 

		fmt = L"ע�����(����) %s �ļ�·�� %s ���� %u �������� %u Ȩ�� 0x%0.8x\n"; 

		ASSERT( action->do_sys_regsrv.path_name[ action->do_sys_regsrv.path_len ] == L'\0' ); 

		srv_name = action->do_sys_regsrv.path_name + action->do_sys_regsrv.path_len + 1; 

		ASSERT( srv_name[ action->do_sys_regsrv.srv_name_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			srv_name, 
			action->do_sys_regsrv.path_name, 
			action->do_sys_regsrv.type, 
			action->do_sys_regsrv.starttype, 
			action->do_sys_regsrv.access ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_opendev_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SYS_opendev )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG devtype; //�豸���� 
		//ULONG access; //�豸��Ȩ�� 
		//ULONG share; //�豸����Ȩ�� 
		//NTSTATUS result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //�豸·�� 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		fmt = L"���豸 %s ����%u Ȩ�� 0x%0.8x ����Ȩ�� 0x%0.8x\n"; 

		ASSERT( action->do_sys_opendev.path_name[ action->do_sys_opendev.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_opendev.path_name, 
			action->do_sys_opendev.devtype, 
			action->do_sys_opendev.access, 
			action->do_sys_opendev.share ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_w32_postmsg_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != W32_postmsg )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//HWND hwnd; //Ŀ�괰�ھ�� 
		//ULONG msg; //������Ϣ 
		//WPARAM wparam; //��Ϣ����1 
		//LPARAM lparam; //��Ϣ����2 
		//NTSTATUS result; //������ɽ��(BOOL) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"����� %s (ID:%u) ���� 0x%0.8x ������Ϣ %u ����1 0x%0.8x ����2 0x%0.8x\n"; 

		ASSERT( action->do_w32_postmsg.path_name[ action->do_w32_postmsg.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_postmsg.path_name, 
			action->do_w32_postmsg.target_pid, 
			action->do_w32_postmsg.hwnd, 
			action->do_w32_postmsg.msg, 
			action->do_w32_postmsg.wparam, 
			action->do_w32_postmsg.lparam ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sendmsg_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != W32_sendmsg )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //Ŀ�����ID 
		//HWND hwnd; //Ŀ�괰�ھ�� 
		//ULONG msg; //������Ϣ 
		//WPARAM wparam; //��Ϣ����1 
		//LPARAM lparam; //��Ϣ����2 
		//NTSTATUS result; //������ɽ��(LRESULT) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //Ŀ�����ȫ·�� 

		fmt = L"����� %s (ID:%u) ���� 0x%0.8x ������Ϣ %u ����1 0x%0.8x ����2 0x%0.8x\n"; 

		ASSERT( action->do_w32_sendmsg.path_name[ action->do_w32_sendmsg.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_sendmsg.path_name, 
			action->do_w32_sendmsg.target_pid, 
			action->do_w32_sendmsg.hwnd, 
			action->do_w32_sendmsg.msg, 
			action->do_w32_sendmsg.wparam, 
			action->do_w32_sendmsg.lparam ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_w32_findwnd_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR wnd_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != W32_findwnd )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//HWND parent_hwnd; //�����ھ�� 
		//HWND sub_hwnd; //�Ӵ��ھ�� 
		//NTSTATUS result; //������ɽ��(HWND) 
		//PATH_SIZE_T cls_name_len; 
		//PATH_SIZE_T wnd_name_len; 
		//WCHAR cls_name[ 1 ]; //��������

		fmt = L"���Ҹ����� 0x%0.8x �Ӵ��� 0x%0.8x ���� (���� %s ������ %s)\n"; 

		ASSERT( action->do_w32_findwnd.cls_name[ action->do_w32_findwnd.cls_name_len ] == L'\0' ); 

		wnd_name = action->do_w32_findwnd.cls_name + action->do_w32_findwnd.cls_name_len + 1; 

		ASSERT( wnd_name[ action->do_w32_findwnd.wnd_name_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_findwnd.parent_hwnd, 
			action->do_w32_findwnd.sub_hwnd, 
			action->do_w32_findwnd.cls_name, 
			wnd_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE ); 

	return ret; 
}

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
LRESULT WINAPI get_access_file_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != ACCESS_FILE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( "�����ļ� %s," ); 

		_name_len = wcslen( action->file_info.path_name ); 

		ret = convert_native_path_to_dos( action->file_info.path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->do_file_write.path_name, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_modify_file_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *param_descs, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != MODIFY_FILE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( " �޸��ļ�%s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->file_info.path_name_ptr; 
			_name_len = wcslen( path_name ); 
		}
		else
		{
			path_name = action->file_info.path_name; 
			_name_len = wcslen( path_name ); 
		}

		ret = convert_native_path_to_dos( ( LPWSTR )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{

			log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", ctx->proc_name, ret ) ); 

			if( _name_len > MAX_PATH - 1 )
			{
				_name_len = MAX_PATH - 1; 
			}

			memcpy( param, 
				path_name, 
				_name_len << 1 ); 

			param[ _name_len ] = L'\0'; 

			ret = ERROR_SUCCESS; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_delete_file_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPWSTR path_name; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != DELETE_FILE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( " ɾ���ļ�%s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->file_info.path_name_ptr; 
			_name_len = wcslen( path_name ); 
		}
		else
		{
			path_name = action->file_info.path_name; 
			_name_len = wcslen( path_name ); 
		}

		ret = convert_native_path_to_dos( path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_modify_key_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != MODIFY_KEY )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( " �޸�ע�����%s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->reg_info.path_name_ptr; 
		}
		else
		{
			path_name = action->reg_info.path_name; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			path_name ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_create_proc_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != CREATE_PROC )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( "���ɽ���%s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->proc_info.path_name_ptr; 
		}
		else
		{
			path_name = action->proc_info.path_name; 
		}

		_name_len = wcslen( path_name ); 

		ret = convert_native_path_to_dos( ( WCHAR* )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			//log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_terminate_proc_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	LPCWSTR path_name; 
	ULONG _name_len; 
	ULONG name_len; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != TERMINATE_PROC )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( "��ֹ���� %s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->proc_info.path_name_ptr; 
		}
		else
		{
			path_name = action->proc_info.path_name; 
		}

		_name_len = wcslen( path_name ); 

		ret = convert_native_path_to_dos( ( WCHAR* )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			//log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_access_com_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	ULONG _name_len; 
	ULONG name_len; 
	LPCWSTR path_name; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != ACCESS_COM )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		if( param == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		fmt = _T( "����COM���%s\n" ); 

		if( flags & LOAD_DATA_PTR_PARAM )
		{
			path_name = action->file_info.path_name_ptr; 
		}
		else
		{
			path_name = action->file_info.path_name; 
		}

		_name_len = wcslen( path_name ); 

		ret = convert_native_path_to_dos( ( WCHAR* )path_name, 
			_name_len, 
			param, 
			MAX_PATH, 
			&name_len ); 

		if( ret != ERROR_SUCCESS )
		{
			//log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", action->, ret ) ); 
			break; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	if( param != NULL )
	{
		free( param ); 
	}

	return ret; 
}

LRESULT WINAPI get_access_mem_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != ACCESS_MEM )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_install_driver_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_install_hook_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != INSTALL_HOOK )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_create_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_CREATE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u����%s�׽���\n" ); 

		param = get_prot_type_desc( action->socket_info.prot ); 
		ASSERT( param != NULL ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 

		ASSERT( action->socket_info.prot != PROT_TCP ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_connect_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_CONNECT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u������%s:%u %s�˿�\n" ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->socket_info.dest_ip, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( action->socket_info.prot ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port, 
			dest_ip_addr, 
			action->socket_info.dest_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_listen_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR *param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_LISTEN )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u����TCP��������\n" ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 

		ASSERT( action->socket_info.prot != PROT_TCP ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_send_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param = NULL; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_SEND )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u������%s:%u %s ����\n" ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->socket_info.dest_ip, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( action->socket_info.prot ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port, 
			dest_ip_addr, 
			action->socket_info.dest_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_recv_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param; 
	LPCWSTR fmt;  
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_RECV )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u����%s:%u %s ����\n" ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->socket_info.dest_ip, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( action->socket_info.prot ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port, 
			dest_ip_addr, 
			action->socket_info.dest_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_socket_accept_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param; 
	LPCWSTR fmt; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != SOCKET_ACCEPT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = _T( "��%s:%u����%s:%u %s ��������\n" ); 

		ip_addr_2_str( action->socket_info.src_ip, src_ip_addr, ARRAY_SIZE( src_ip_addr ) ); 
		ip_addr_2_str( action->socket_info.dest_ip, dest_ip_addr, ARRAY_SIZE( dest_ip_addr ) ); 

		param = get_prot_type_desc( action->socket_info.prot ); 
		ASSERT( param != NULL ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			action->socket_info.src_port, 
			dest_ip_addr, 
			action->socket_info.dest_port, 
			param ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_locate_url_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR param; 
	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != LOCATE_URL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_icmp_send_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_icmp_send )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		
		ip_addr_2_str( action->icmp_info.src_ip, src_ip_addr, IPV4_ADDR_LEN ); 
		ip_addr_2_str( action->icmp_info.dest_ip, dest_ip_addr, IPV4_ADDR_LEN ); 
		
		fmt = _T( "��%s �� %s ����ICMP���� %u %u\n" ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			src_ip_addr, 
			dest_ip_addr, 
			action->icmp_info.type, 
			action->icmp_info.code ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_icmp_recv_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( param_descs != NULL ); 

		if( param_descs->proc_path == NULL )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_icmp_recv )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ip_addr_2_str( action->icmp_info.src_ip, src_ip_addr, IPV4_ADDR_LEN ); 
		ip_addr_2_str( action->icmp_info.dest_ip, dest_ip_addr, IPV4_ADDR_LEN ); 

		fmt = _T( "[%s] ��%s����ICMP���� %u %u\n" ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			param_descs->proc_path, 
			dest_ip_addr, 
			src_ip_addr, 
			action->icmp_info.type, 
			action->icmp_info.code ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}
#endif //COMPATIBLE_OLD_ACTION_DEFINE

LRESULT _get_icmp_send_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_icmp_send )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ip_addr_2_str( action->do_net_icmp_send.src_ip, src_ip_addr, IPV4_ADDR_LEN ); 
		ip_addr_2_str( action->do_net_icmp_send.dest_ip, dest_ip_addr, IPV4_ADDR_LEN ); 

		fmt = _T( "��%s �� %s ����ICMP���� %u %u\n" ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			src_ip_addr, 
			dest_ip_addr, 
			action->do_net_icmp_send.type, 
			action->do_net_icmp_send.code ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT _get_icmp_recv_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR src_ip_addr[ IPV4_ADDR_LEN ]; 
	WCHAR dest_ip_addr[ IPV4_ADDR_LEN ]; 

	LPCWSTR fmt; 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( action->type != NET_icmp_recv )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ip_addr_2_str( action->do_net_icmp_recv.src_ip, src_ip_addr, IPV4_ADDR_LEN ); 
		ip_addr_2_str( action->do_net_icmp_recv.dest_ip, dest_ip_addr, IPV4_ADDR_LEN ); 

		fmt = _T( "[%s] ��%s����ICMP���� %u %u\n" ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			dest_ip_addr, 
			src_ip_addr, 
			action->do_net_icmp_recv.type, 
			action->do_net_icmp_recv.code ); 

#ifdef _DEBUG
		if( _ret == ccb_buf_len - 1 )
		{
			ASSERT( tip[ ccb_buf_len - 1 ] == L'\0' ); 
		}
#endif //_DEBUG

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( _ret == -1 
			|| ( ULONG )_ret >= ccb_buf_len )
		{
			tip[ ccb_buf_len - 1 ] = L'\0'; 
			ret = ERROR_BUFFER_OVERFLOW; 
		}
	}while( FALSE );

	return ret; 
}

/*********************************************************************************************************************
use the structure compiler to compile the functions for the struct which have same regulations for its component, 
is the effective way to deal with the situation. 
*********************************************************************************************************************/
LRESULT WINAPI get_action_tip( sys_action_record *action, action_context *ctx, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCTSTR fmt = NULL; 

#define MAX_ACTION_DESC_SIZE ( ULONG )( 64 )
	LPCTSTR tmp_fmt; 
#define MAX_PARAMS_NUM ( ULONG )( 16 )
	TCHAR *app_name = NULL; 
	ULONG app_name_len; 
	ULONG _app_name_len; 
	prepared_params_desc prepared_params; 

	ASSERT( action != NULL ); 
	ASSERT( tip != NULL ); 
	ASSERT( ccb_buf_len > 0 ); 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
		tip[ ccb_buf_len - 1 ] = L'\0'; 

		if( PRINT_PROCESS_PATH & flags )
		{
			app_name = ( TCHAR* )malloc( sizeof ( TCHAR ) * MAX_PATH ); 

			if( app_name == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break;  
			}

			_app_name_len = ctx->proc_name_len; 

			ret = convert_native_path_to_dos( ctx->proc_name, 
				_app_name_len, 
				app_name, 
				MAX_PATH, 
				&app_name_len ); 

			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_ERROR, "convert the native path %ws to dos error 0x%0.8x\n", ctx->proc_name, ret ) ); 

				if( _app_name_len > MAX_PATH - 1 )
				{
					_app_name_len = MAX_PATH - 1; 
				}

				memcpy( app_name, 
					ctx->proc_name, 
					_app_name_len << 1 ); 

				app_name[ _app_name_len ] = L'\0'; 

				ret = ERROR_SUCCESS; 
			}

			_ret = _sntprintf( _T( "Ӧ�ó��� %s " ), ccb_buf_len, app_name ); 
			if( _ret <= 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				DBG_BP(); 
				break; 
			}

			if( tip[ ccb_buf_len - 1 ] != L'\0')
			{
				DBG_BP(); 
				ret = ERROR_BUFFER_OVERFLOW; 
				break; 
			}

			tip = tip + _ret; 
			ccb_buf_len = ccb_buf_len - _ret; 

		}

		prepared_params.desc_len = 0; 
		prepared_params.desc = L""; 

		switch( action->type )
		{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		case ACCESS_FILE: 
			{
				ret = get_access_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case MODIFY_FILE: 
			{
				ret = get_modify_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case DELETE_FILE: 
			{
				ret = get_delete_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case MODIFY_KEY: 
			{
				ret = get_modify_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case CREATE_PROC: 
			{
				ret = get_create_proc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case TERMINATE_PROC: 
			{
				ret = get_terminate_proc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ACCESS_COM: 
			{
				ret = get_access_com_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ACCESS_MEM: 
			{
				ret = get_access_mem_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case INSTALL_DRV: 
			{
				ret = get_install_driver_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case INSTALL_HOOK:
			{
				ret = get_install_hook_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_CREATE: 
			{
				ret = get_socket_create_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_CONNECT: 
			{
				ret = get_socket_connect_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_LISTEN: 
			{
				ret = get_socket_listen_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_SEND:
			{
				ret = get_socket_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_RECV: 
			{
				ret = get_socket_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_ACCEPT: 
			{
				ret = get_socket_accept_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case LOCATE_URL: 
			{
				ret = get_locate_url_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ICMP_SEND: 
			{
				ret = get_icmp_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			}
			break; 
		case ICMP_RECV: 
			{
				ret = get_icmp_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		case EXEC_create:
			ret = get_exec_create_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			ret = get_exec_destroy_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			ret = get_exec_module_load_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			ret = get_file_touch_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
		case FILE_open:
			ret = get_file_open_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_read:
			ret = get_file_read_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case FILE_write:
			ret = get_file_write_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_file_write_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			ret = get_file_modified_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:
			ret = get_file_readdir_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			ret = get_file_remove_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_file_remove_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			ret = get_file_rename_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			ret = get_file_truncate_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			ret = get_file_mklink_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			ret = get_file_chmod_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec:
			ret = get_file_setsec_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
		case FILE_getinfo: 
			ret = get_file_getinfo_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case FILE_setinfo: 
			ret = get_file_setinfo_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		//case FILE_close:
		//	ret = get_file_setinfo_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
		//	break; 
			//MT_regmon, 
		case REG_openkey:
			ret = get_reg_open_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			ret = get_reg_mkkey_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_reg_mk_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
			//	break; 
		case REG_rmkey:
			ret = get_reg_rmkey_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			ret = get_reg_mvkey_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������ע����� ע�����·�� 

		case REG_rmval:
			ret = get_reg_rmval_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			ret = get_reg_getval_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			ret = get_reg_setval_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey:
			ret = get_load_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:
			ret = get_repl_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			ret = get_rstr_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���Hive�ļ� ע�����·�� 

			//break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			ret = get_proc_exec_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			ret = get_proc_open_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			ret = get_proc_debug_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			ret = get_proc_suspend_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			ret = get_proc_resume_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			ret = get_proc_kill_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Ŀ�����·���� 

		case PROC_job:
			ret = get_proc_job_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			ret = get_proc_pg_prot_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			ret = get_proc_free_vm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			ret = get_proc_write_vm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			ret = get_proc_read_vm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_proc_readvm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			ret = get_thread_remote_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			ret = get_thread_set_ctxt_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			ret = get_thread_suspend_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			ret = get_thread_resume_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			ret = get_thread_kill_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			ret = get_thread_queue_apc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
			//MT_sysmon
		case SYS_settime:
			ret = get_sys_settime_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			ret = get_sys_link_knowndll_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:
			ret = get_sys_open_physmm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:
			ret = get_sys_read_physmm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ڴ� �� 
		case SYS_write_physmm: 
			ret = get_sys_write_physmm_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //д�����ڴ� �� 
		case SYS_load_kmod:
			ret = get_sys_load_kmod_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
			//	break; 
		case SYS_enumproc:
			ret = get_sys_enumproc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ö�ٽ��� �� 
		case SYS_regsrv:
			ret = get_sys_regsrv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ע����� �������ȫ·�� 
		case SYS_opendev:
			ret = get_sys_opendev_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:
			ret = get_w32_postmsg_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:
			ret = get_sendmsg_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:
			ret = get_w32_findwnd_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Ҵ��� �� 
		case W32_msghook:
			ret = get_w32_msg_hook_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������Ϣ���� �� 

		case W32_lib_inject:
			ret = get_w32_lib_inject_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			ret = get_net_create_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 

		case NET_connect:
			ret = get_net_connect_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			ret = get_net_listen_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			ret = get_net_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			ret = get_net_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			ret = get_net_accept_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//case SOCKET_ACCEPT:
			//	break; 
		case NET_http: 
			ret = get_net_http_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			ret = _get_icmp_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case NET_icmp_recv:
			ret = _get_icmp_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//MT_behavior, 
		case BA_extract_hidden:
			ret = get_ba_extract_hidden_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			ret = get_ba_extract_pe_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:
			ret = get_ba_self_copy_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:
			ret = get_ba_self_delete_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:
			ret = get_ba_ulterior_exec_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:
			ret = get_ba_invade_process_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			ret = get_ba_infect_pe_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			ret = get_ba_overwrite_pe_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			ret = get_ba_register_autorun_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ע���������� �������ļ�·���� 

			//case BA_other:
			//desc = L"BA_other"; 
			//break; 
		case COM_access:
			{
				ret = get_proc_com_access_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
				break; 
			}
		case OTHER_ACTION:
			{
				tmp_fmt = _get_string_by_id( TEXT_SYS_ACTION_PROCTECTED_ACTION, 
					_T( "ִ�б���������" ) ); 

				_ret = _snwprintf( tip, ccb_buf_len, tmp_fmt, 
					app_name );
				break; 
			}
		default:
			{
				ASSERT( FALSE && "unknown action type" ); 
				log_trace( ( MSG_ERROR, "invalid system action type %d \n", action->type ) ); 
			}
		}

		if( tip[ ccb_buf_len - 1 ] != L'\0' )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			ASSERT( FALSE ); 
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	} while ( FALSE );

	if( app_name != NULL )
	{
		free( app_name ); 
	}

	return ret; 
}

/****************************************************************************************************
���ݽṹ��������̵����Ż�������ǹؼ���
****************************************************************************************************/

LRESULT WINAPI get_action_detail( sys_action_record *action, action_context *ctx, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCTSTR fmt = NULL; 

#define MAX_ACTION_DESC_SIZE ( ULONG )( 64 )

	LPCTSTR tmp_fmt; 
#define MAX_PARAMS_NUM ( ULONG )( 16 )
	prepared_params_desc prepared_params; 

	ASSERT( action != NULL ); 
	ASSERT( tip != NULL ); 
	ASSERT( ccb_buf_len > 0 ); 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
		tip[ ccb_buf_len - 1 ] = L'\0'; 

		switch( action->type )
		{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		case ACCESS_FILE: 
			{
				ret = get_access_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case MODIFY_FILE: 
			{
				ret = get_modify_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case DELETE_FILE: 
			{
				ret = get_delete_file_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case MODIFY_KEY: 
			{
				ret = get_modify_key_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case CREATE_PROC: 
			{
				ret = get_create_proc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case TERMINATE_PROC: 
			{
				ret = get_terminate_proc_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ACCESS_COM: 
			{
				ret = get_access_com_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ACCESS_MEM: 
			{
				ret = get_access_mem_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case INSTALL_DRV: 
			{
				ret = get_install_driver_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case INSTALL_HOOK:
			{
				ret = get_install_hook_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_CREATE: 
			{
				ret = get_socket_create_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_CONNECT: 
			{
				ret = get_socket_connect_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_LISTEN: 
			{
				ret = get_socket_listen_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_SEND:
			{
				ret = get_socket_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_RECV: 
			{
				ret = get_socket_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case SOCKET_ACCEPT: 
			{
				ret = get_socket_accept_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case LOCATE_URL: 
			{
				ret = get_locate_url_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
		case ICMP_SEND: 
			{
				ret = get_icmp_send_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			}
			break; 
		case ICMP_RECV: 
			{
				ret = get_icmp_recv_desc( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			}
			break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		case EXEC_create:
			ret = get_exec_create_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			ret = get_exec_destroy_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			ret = get_exec_module_load_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			ret = get_file_touch_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	
		case FILE_open:
			ret = get_file_open_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_read:
			ret = get_file_read_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case FILE_write:
			ret = get_file_write_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_file_write_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			ret = get_file_modified_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:
			ret = get_file_readdir_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			ret = get_file_remove_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_file_remove_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			ret = get_file_rename_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			ret = get_file_truncate_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			ret = get_file_mklink_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			ret = get_file_chmod_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec:
			ret = get_file_setsec_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_getinfo:
			ret = get_file_getinfo_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setinfo:
			ret = get_file_setinfo_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 
		case FILE_close:
			ret = get_file_close_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 

			//MT_regmon, 
		case REG_openkey:
			ret = get_reg_open_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			ret = get_reg_mkkey_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_reg_mk_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
			//	break; 
		case REG_rmkey:
			ret = get_reg_rmkey_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			ret = get_reg_mvkey_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������ע����� ע�����·�� 

		case REG_getinfo:
			ret = get_reg_getinfo_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case REG_setinfo:
			ret = get_reg_setinfo_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case REG_enuminfo:
			ret = get_reg_enum_info_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case REG_enum_val:
			ret = get_reg_enum_value_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case REG_rmval:
			ret = get_reg_rmval_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			ret = get_reg_getval_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			ret = get_reg_setval_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey:
			ret = get_load_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:
			ret = get_repl_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			ret = get_rstr_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���Hive�ļ� ע�����·�� 
		
		case REG_closekey:
			ret = get_close_key_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_setsec:
			ret = get_reg_set_sec_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			ret = get_proc_exec_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			ret = get_proc_open_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			ret = get_proc_debug_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			ret = get_proc_suspend_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			ret = get_proc_resume_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			ret = get_proc_kill_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Ŀ�����·���� 

		case PROC_job:
			ret = get_proc_job_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			ret = get_proc_pg_prot_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			ret = get_proc_free_vm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			ret = get_proc_write_vm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			ret = get_proc_read_vm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			//ret = get_proc_readvm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			ret = get_thread_remote_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			ret = get_thread_set_ctxt_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			ret = get_thread_suspend_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			ret = get_thread_resume_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			ret = get_thread_kill_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			ret = get_thread_queue_apc_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
		//case PORT_read:
		//	ret = get_port_read_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
		//	break; 
		//case PORT_write: 
		//	ret = get_port_write_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
		//	break; 
		//case PORT_urb:
		//	ret = get_port_urb_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
		//	break; 
			//MT_sysmon
		case SYS_settime:
			ret = get_sys_settime_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			ret = get_sys_link_knowndll_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:
			ret = get_sys_open_physmm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:
			ret = get_sys_read_physmm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ڴ� �� 
		case SYS_write_physmm: 
			ret = get_sys_write_physmm_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //д�����ڴ� �� 
		case SYS_load_kmod:
			ret = get_sys_load_kmod_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
			//	break; 
		case SYS_enumproc:
			ret = get_sys_enumproc_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ö�ٽ��� �� 
		case SYS_regsrv:
			ret = get_sys_regsrv_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ע����� �������ȫ·�� 
		case SYS_opendev:
			ret = get_sys_opendev_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:
			ret = get_w32_postmsg_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:
			ret = get_sendmsg_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:
			ret = get_w32_findwnd_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Ҵ��� �� 
		case W32_msghook:
			ret = get_w32_msg_hook_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //������Ϣ���� �� 

		case W32_lib_inject:
			ret = get_w32_lib_inject_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			ret = get_net_create_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 

		case NET_connect:
			ret = get_net_connect_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			ret = get_net_listen_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			ret = get_net_send_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			ret = get_net_recv_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			ret = get_net_accept_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//case SOCKET_ACCEPT:
			//	break; 
		case NET_dns:
			ret = get_net_dns_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case NET_http: 
			ret = get_net_http_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			ret = _get_icmp_send_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
		case NET_icmp_recv:
			ret = _get_icmp_recv_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; 
			//MT_behavior, 
		case BA_extract_hidden:
			ret = get_ba_extract_hidden_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			ret = get_ba_extract_pe_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:
			ret = get_ba_self_copy_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:
			ret = get_ba_self_delete_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:
			ret = get_ba_ulterior_exec_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:
			ret = get_ba_invade_process_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			ret = get_ba_infect_pe_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 

			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			ret = get_ba_overwrite_pe_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			ret = get_ba_register_autorun_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
			break; //ע���������� �������ļ�·���� 

			//case BA_other:
			//desc = L"BA_other"; 
			//break; 
		case COM_access:
			{
				ret = get_proc_com_access_detail( action, ctx, &prepared_params, tip, ccb_buf_len, flags ); 
				break; 
			}
		case OTHER_ACTION:
			{
				tmp_fmt = _get_string_by_id( TEXT_SYS_ACTION_PROCTECTED_ACTION, 
					_T( "ִ�б���������" ) ); 

				_ret = _snwprintf( tip, ccb_buf_len, tmp_fmt );
				break; 
			}
		default:
			{
				ASSERT( FALSE && "unknown action type" ); 

				//fmt = NULL; 
				log_trace( ( MSG_ERROR, "invalid system action type %d \n", action->type ) ); 
			}
		}

		if( tip[ ccb_buf_len - 1 ] != L'\0' )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			ASSERT( FALSE ); 
			tip[ ccb_buf_len - 1 ] = L'\0'; 
		}
	} while ( FALSE );

	return ret; 
}

#if 0
LRESULT WINAPI get_action_param_desc( sys_action_record *action, 
							  ULONG param_index, 
							  LPWSTR text, 
							  ULONG ccb_buf_len, 
							  ULONG *ccb_ret_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}
#endif //0

LRESULT WINAPI get_action_data( sys_action_record *action, LPWSTR data_dump, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != data_dump ); 
		ASSERT( 0 < ccb_buf_len ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_key_data_text( LPWSTR data_dump, ULONG ccb_buf_len, PVOID data, ULONG data_len ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{

		ret = dump_mem_in_buf_w( data_dump, ccb_buf_len, data, data_len, 0 ); 
	} while ( FALSE );

	return ret; 
}

/***************************************************************************************
ʹ��0��������ķ�����������ֱ��ʹ�ýṹ���е��ڴ�ʱ�������п������Ż��˺����������ܡ�
***************************************************************************************/
LRESULT WINAPI get_object_path( sys_action_record *action, 
							   action_context *ctx, 
							   LPWSTR obj_path, 
							   ULONG cc_buf_len, 
							   LPCWSTR *obj_path_ptr, 
							   ULONG flags, 
							   ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPCWSTR _obj_path = L""; 
	ULONG path_len = 0; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( obj_path != NULL ); 
		ASSERT( obj_path_ptr != NULL ); 
		ASSERT( cc_buf_len > 0 ); 
		ASSERT( cc_ret_len != NULL ); 

		switch( action->type )
		{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		case ACCESS_FILE: 
			{
			}
			break; 
		case MODIFY_FILE: 
			{
			}
			break; 
		case DELETE_FILE: 
			{
			}
			break; 
		case MODIFY_KEY: 
			{
			}
			break; 
		case CREATE_PROC: 
			{
			}
			break; 
		case TERMINATE_PROC: 
			{
			}
			break; 
		case ACCESS_COM: 
			{
			}
			break; 
		case ACCESS_MEM: 
			{
			}
			break; 
		case INSTALL_DRV: 
			{
			}
			break; 
		case INSTALL_HOOK:
			{
			}
			break; 
		case SOCKET_CREATE: 
			{
			}
			break; 
		case SOCKET_CONNECT: 
			{
			}
			break; 
		case SOCKET_LISTEN: 
			{
			}
			break; 
		case SOCKET_SEND:
			{
			}
			break; 
		case SOCKET_RECV: 
			{
			}
			break; 
		case SOCKET_ACCEPT: 
			{
			}
			break; 
		case LOCATE_URL: 
			{
			}
			break; 
		case ICMP_SEND: 
			{
			}
			break; 
		case ICMP_RECV: 
			{
			}
			break; 
#endif //COMPATIBLE_OLD_ACTION_DEFINE
		case EXEC_create:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_exec_create.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_exec_create.path_name; 
				}

				path_len = action->do_exec_create.path_len; 
			}
			
			if( _obj_path != NULL )
			{
				ASSERT( _obj_path[ action->do_exec_create.path_len ] == L'\0' ); 
			}
			else
			{
				ASSERT( action->do_exec_create.path_len == 0 ); 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 

			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_exec_destroy.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_exec_destroy.path_name; 
				}
				path_len = action->do_exec_destroy.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_exec_module_load.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_exec_module_load.path_name; 
				}

				path_len = action->do_exec_module_load.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_touch.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_touch.path_name; 
				}

				path_len = action->do_file_touch.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ� �ļ�ȫ·�� ���ļ���أ� 	

		case FILE_open:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_open.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_open.path_name; 
				}

				path_len = action->do_file_open.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 		
			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_read:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_read.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_read.path_name; 
				}

				path_len = action->do_file_read.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 		
			break; 
		case FILE_write:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_write.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_write.path_name; 
				}

				path_len = action->do_file_write.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 	
			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_modified.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_modified.path_name; 
				}

				path_len = action->do_file_modified.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_readdir.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_readdir.path_name; 
				}

				path_len = action->do_file_readdir.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_remove.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_remove.path_name; 
				}

				path_len = action->do_file_remove.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 

			break; //ɾ���ļ� �ļ�ȫ·�� 

		case FILE_rename:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_rename.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_rename.path_name; 
				}

				path_len = action->do_file_rename.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_truncate.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_truncate.path_name; 
				}

				path_len = action->do_file_truncate.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_mklink.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_mklink.path_name; 
				}

				path_len = action->do_file_mklink.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_chmod.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_chmod.path_name; 
				}

				path_len = action->do_file_chmod.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_setsec.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_setsec.path_name; 
				}

				path_len = action->do_file_setsec.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_getinfo:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_getinfo.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_getinfo.path_name; 
				}

				path_len = action->do_file_getinfo.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_setinfo:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_setinfo.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_setinfo.path_name; 
				}

				path_len = action->do_file_setinfo.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_close:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_file_close.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_file_close.path_name; 
				}

				path_len = action->do_file_close.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
			//MT_regmon, 
		case REG_openkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_openkey.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_openkey.path_name; 
				}

				path_len = action->do_reg_openkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_mkkey.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_mkkey.path_name; 
				}

				path_len = action->do_reg_mkkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ע����� ע�����·�� 
			//case MODIFY_KEY
			//	break; 
		case REG_rmkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_rmkey.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_rmkey.path_name; 
				}

				path_len = action->do_reg_rmkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_mvkey.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_mvkey.path_name; 
				}

				path_len = action->do_reg_mvkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //������ע����� ע�����·�� 

		case REG_getinfo:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_getinfo.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_getinfo.path_name; 
				}

				path_len = action->do_reg_getinfo.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
		case REG_setinfo:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_setinfo.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_setinfo.path_name; 
				}

				path_len = action->do_reg_setinfo.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
		case REG_enuminfo:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_enum_info.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_enum_info.path_name; 
				}

				path_len = action->do_reg_enum_info.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
		case REG_enum_val: 
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_enum_val.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_enum_val.path_name; 
				}

				path_len = action->do_reg_enum_val.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
		case REG_rmval:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_rmval.key_name_ptr == NULL ? 
						L"" 
						: action->do_reg_rmval.key_name_ptr );  
				}
				else
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_rmval.path_name );  
				}

				_obj_path = obj_path; 

				path_len = action->do_reg_rmval.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_getval.key_name_ptr == NULL ? 
						L"" 
						: action->do_reg_getval.key_name_ptr );  
				}
				else
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_getval.path_name );  
				}

				_obj_path = obj_path; 


				path_len = action->do_reg_getval.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_setval.key_name_ptr == NULL ? 
						L"" 
						: action->do_reg_setval.key_name_ptr );  
				}
				else
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, L"%s", 
						action->do_reg_setval.path_name );  
				}

				_obj_path = obj_path; 

				path_len = action->do_reg_setval.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey: 
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_loadkey.key_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_loadkey.path_name; 
				}

				path_len = action->do_reg_loadkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_replkey.key_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_replkey.path_name; 
				}

				path_len = action->do_reg_replkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_rstrkey.key_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_rstrkey.path_name; 
				}

				path_len = action->do_reg_rstrkey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ע���Hive�ļ� ע�����·�� 
		case REG_setsec:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_setsec.key_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_setsec.path_name; 
				}

				path_len = action->do_reg_setsec.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ע�������ȫ���� ע�����·�� 

		case REG_closekey: 
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_reg_closekey.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_reg_closekey.path_name; 
				}

				path_len = action->do_reg_closekey.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 
			//MT_procmon, 
		case PROC_exec:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_exec.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_exec.path_name; 
				}

				path_len = action->do_proc_exec.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�������� Ŀ�����·����  �����̼�أ�

		case PROC_open:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_open.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_open.path_name; 
				}

				path_len = action->do_proc_open.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_debug.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_debug.path_name; 
				}

				path_len = action->do_proc_debug.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_suspend.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_suspend.path_name; 
				}

				path_len = action->do_proc_suspend.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_resume.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_resume.path_name; 
				}

				path_len = action->do_proc_resume.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_kill.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_kill.path_name; 
				}

				path_len = action->do_proc_kill.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�������� Ŀ�����·���� 

		case PROC_job:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_job.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_job.path_name; 
				}

				path_len = action->do_proc_job.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_pgprot.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_pgprot.path_name; 
				}

				path_len = action->do_proc_pgprot.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_freevm.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_freevm.path_name; 
				}

				path_len = action->do_proc_freevm.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_writevm.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_writevm.path_name; 
				}

				path_len = action->do_proc_writevm.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_proc_readvm.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_proc_readvm.path_name; 
				}

				path_len = action->do_proc_readvm.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_remote.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, //L"��������:�߳�[%u]", 
						action->do_thrd_remote.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_remote.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_remote.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, //L"����[%u]%s:�߳�[%u]", 
						action->do_thrd_remote.target_pid, 
						path_name, 
						action->do_thrd_remote.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_setctxt.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, //L"��������:�߳�[%u]", 
						action->do_thrd_setctxt.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_setctxt.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_setctxt.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, //L"����[%u]%s:�߳�[%u]", L"����[%u]%s:�߳�[%u]", 
						action->do_thrd_setctxt.target_pid, 
						path_name, 
						action->do_thrd_setctxt.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_suspend.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_suspend.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_suspend.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_suspend.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_suspend.target_pid, 
						path_name, 
						action->do_thrd_suspend.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_resume.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_resume.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_resume.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_resume.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_resume.target_pid, 
						path_name, 
						action->do_thrd_resume.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_exit.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_exit.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_exit.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_exit.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_exit.target_pid, 
						path_name, 
						action->do_thrd_exit.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}	
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			{
				LPCWSTR path_name; 

				if( action->do_thrd_queue_apc.target_pid == ctx->proc_id )
				{
					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						CURRENT_PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_queue_apc.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}
				else 
				{
					if( flags & SAVE_INFO_BY_POINTER )
					{
						path_name = action->do_thrd_queue_apc.path_name_ptr; 
					}
					else
					{
						path_name = action->do_thrd_queue_apc.path_name; 
					}

					hr = StringCchPrintfW( obj_path, 
						cc_buf_len, 
						PROCESS_THREAD_FORMAT_TEXT, 
						action->do_thrd_queue_apc.target_pid, 
						path_name, 
						action->do_thrd_queue_apc.target_tid ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					_obj_path = obj_path; 
				}

				path_len = wcslen( _obj_path ); 
			}	
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
			//case COM_access:
			//	break; 
			//MT_sysmon
		case PORT_read:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_port_read.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_port_read.path_name; 
				}

				path_len = action->do_port_read.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 

		case PORT_write:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_port_write.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_port_write.path_name; 
				}

				path_len = action->do_port_write.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 

		case PORT_urb:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_port_urb.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_port_urb.path_name; 
				}

				path_len = action->do_port_urb.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; 


		case SYS_settime:
			{
				hr = StringCchCopyW( obj_path, 
					cc_buf_len, 
					L"ϵͳʱ��" ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}		

				_obj_path = obj_path; 
			}
			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_sys_link_knowndll.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_sys_link_knowndll.path_name; 
				}

				path_len = action->do_sys_link_knowndll.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:
			{
				hr = StringCchCopyW( obj_path, 
					cc_buf_len, 
					L"ϵͳ�ڴ��豸(PhysicalMemory)" ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}		

				_obj_path = obj_path; 
			}
			break; //�������ڴ��豸 �� 

		case SYS_read_physmm:
			{
				hr = StringCchCopyW( obj_path, 
					cc_buf_len, 
					L"ϵͳ�ڴ��豸(PhysicalMemory)" ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}		

				_obj_path = obj_path; 
			}
			break; //�������ڴ� �� 
		case SYS_write_physmm: 
			{
				hr = StringCchCopyW( obj_path, 
					cc_buf_len, 
					L"ϵͳ�ڴ��豸(PhysicalMemory)" ); 
				
				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}		

				_obj_path = obj_path; 
			}
			break; //д�����ڴ� �� 
		
		case SYS_load_kmod:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_sys_load_kmod.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_sys_load_kmod.path_name; 
				}

				path_len = action->do_sys_load_kmod.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
			//case INSTALL_DRV:
			//	break; 
		case SYS_enumproc:
			*obj_path = L'\0'; 
			_obj_path = obj_path; 
			break; //ö�ٽ��� �� 
		case SYS_regsrv:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_sys_regsrv.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_sys_regsrv.path_name; 
				}

				path_len = action->do_sys_regsrv.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //ע����� �������ȫ·�� 
		case SYS_opendev:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_sys_opendev.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_sys_opendev.path_name; 
				}

				path_len = action->do_sys_opendev.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_w32_postmsg.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_w32_postmsg.path_name; 
				}

				path_len = action->do_w32_postmsg.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_w32_sendmsg.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_w32_sendmsg.path_name; 
				}

				path_len = action->do_w32_sendmsg.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:
			{
				obj_path[ 0 ] = L'\0'; 
				_obj_path = obj_path; 
			}

			break; //���Ҵ��� �� 
		case W32_msghook:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_w32_msghook.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_w32_msghook.path_name; 
				}

				path_len = action->do_w32_msghook.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //������Ϣ���� �� 

		case W32_lib_inject:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_w32_lib_inject.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_w32_lib_inject.path_name; 
				}

				path_len = action->do_w32_lib_inject.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			{
				LPCWSTR prot_desc; 

				LPWSTR remain_text; 
				size_t remain_size; 

				remain_text = obj_path; 
				remain_size = cc_buf_len; 
				
				prot_desc = get_ip_proto_text( action->do_net_create.protocol ); 

				if( *prot_desc == L'\0' )
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT, 
						action->do_net_create.protocol ); 

					ASSERT( SUCCEEDED( hr ) ); 

					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"�����׽���:(Э��ID:%u)%u.%u.%u.%u:%u", 
					//	action->do_net_create.protocol, 
					//	( ( action->do_net_create.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_create.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_create.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_create.local_ip_addr & 0x000000ff ), 
					//	action->do_net_create.local_port );  
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_CREATE_OBJECT_PATH_FORMAT_TEXT2, 
						prot_desc ); 

					ASSERT( SUCCEEDED( hr ) ); 

					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"�����׽���:(%s)%u.%u.%u.%u:%u", 
					//	prot_desc, 
					//	( ( action->do_net_connect.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_connect.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_connect.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_connect.local_ip_addr & 0x000000ff ), 
					//	action->do_net_connect.local_port );  
				}

				if( action->do_net_create.local_ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:*"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_create.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_create.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_create.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_create.local_ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_create.local_port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"*", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u", 
						action->do_net_create.local_port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}				
			break; 

		case NET_connect:
			{
				//LPCWSTR prot_desc; 

				//prot_desc = get_ip_proto_text( action->do_net_connect.protocol ); 
				LPWSTR remain_text; 
				size_t remain_size; 

				remain_text = obj_path; 
				remain_size = cc_buf_len; 

				switch( action->do_net_connect.protocol )
				{
				case PROT_UDP: 
					{
						hr = StringCchCopyExW( remain_text, 
							remain_size, 
							NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT, 
							&remain_text, 
							&remain_size, 
							0 ); 

						ASSERT( SUCCEEDED( hr ) ); 
						//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(UDPЭ��)%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u", 
						//	( ( action->do_net_connect.local_ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_connect.local_ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_connect.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_connect.local_ip_addr & 0x000000ff ), 
						//	action->do_net_connect.local_port, 
						//	//0, 
						//	//0, 
						//	//0, 
						//	//0, 
						//	//0, 
						//	( ( action->do_net_connect.ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_connect.ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_connect.ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_connect.ip_addr & 0x000000ff ), 
						//	action->do_net_connect.port );  
					}
					break; 
				case PROT_TCP:
					{
						hr = StringCchCopyExW( remain_text, 
							remain_size, 
							NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT2, 
							&remain_text, 
							&remain_size, 
							0 ); 

						ASSERT( SUCCEEDED( hr ) );
						//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(TCPЭ��)%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u", 
						//	( ( action->do_net_connect.local_ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_connect.local_ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_connect.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_connect.local_ip_addr & 0x000000ff ), 
						//	action->do_net_connect.local_port, 
						//	( ( action->do_net_connect.ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_connect.ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_connect.ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_connect.ip_addr & 0x000000ff ), 
						//	action->do_net_connect.port ); 
					}

					break; 
				default: 
					{
						hr = StringCchPrintfExW( remain_text, 
							remain_size, 
							&remain_text, 
							&remain_size, 
							0, 
							NETWORK_CONNECT_OBJECT_PATH_FORMAT_TEXT3, 
							action->do_net_connect.protocol ); 

						ASSERT( SUCCEEDED( hr ) );
					}
					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(Э��ID:%u)%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u", 
					//	action->do_net_connect.protocol, 
					//	( ( action->do_net_connect.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_connect.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_connect.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_connect.local_ip_addr & 0x000000ff ), 
					//	action->do_net_connect.local_port, 
					//	( ( action->do_net_connect.ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_connect.ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_connect.ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_connect.ip_addr & 0x000000ff ), 
					//	action->do_net_connect.port ); 

					break; 
				}

				if( action->do_net_connect.local_ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:*"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_connect.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_connect.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_connect.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_connect.local_ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_connect.local_port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"* > ", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u > ", 
						action->do_net_connect.local_port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_connect.ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_connect.ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_connect.ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_connect.ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_connect.ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_connect.port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"*", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u", 
						action->do_net_connect.port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}		
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
			//case SOCKET_CONNECT:
			//	break; 
		case NET_listen:
			{
				LPCWSTR prot_desc; 

				LPWSTR remain_text; 
				size_t remain_size; 

				remain_text = obj_path; 
				remain_size = cc_buf_len; 

				prot_desc = get_ip_proto_text( action->do_net_listen.protocol ); 

				if( *prot_desc == L'\0' )
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT, 
						action->do_net_listen.protocol ); 

					ASSERT( SUCCEEDED( hr ) ); 

					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"�����˿�:(Э��ID:%u)%u.%u.%u.%u:%u", 
					//	action->do_net_listen.protocol, 
					//	( ( action->do_net_listen.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_listen.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_listen.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_listen.local_ip_addr & 0x000000ff ), 
					//	action->do_net_listen.local_port );  
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_LISTEN_OBJECT_PATH_FORMAT_TEXT2,
						prot_desc ); 

					ASSERT( SUCCEEDED( hr ) ); 

					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"�����˿�:(%s)%u.%u.%u.%u:%u", 
					//	prot_desc, 
					//	( ( action->do_net_listen.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_listen.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_listen.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_listen.local_ip_addr & 0x000000ff ), 
					//	action->do_net_listen.local_port );  
				}

				if( action->do_net_listen.ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:*"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_listen.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_listen.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_listen.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_listen.local_ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_listen.local_port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"*", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u", 
						action->do_net_listen.local_port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}
			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_LISTEN:
			//	break; 
		case NET_send:
			{
				LPCWSTR prot_desc; 

				LPWSTR remain_text; 
				size_t remain_size; 

				remain_text = obj_path; 
				remain_size = cc_buf_len; 

				prot_desc = get_ip_proto_text( action->do_net_send.protocol ); 

				if( *prot_desc == L'\0' )
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT, 
						action->do_net_send.protocol ); 

					ASSERT( SUCCEEDED( hr ) ); 


					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(Э��ID:%u)%u.%u.%u.%u:%u > %u.%u.%u.%u:%u", 
					//	action->do_net_send.protocol, 
					//	( ( action->do_net_send.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_send.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_send.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_send.local_ip_addr & 0x000000ff ), 
					//	action->do_net_send.local_port, 
					//	( ( action->do_net_send.ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_send.ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_send.ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_send.ip_addr & 0x000000ff ), 
					//	action->do_net_send.port );  
				}
				else
				{
					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(%s)%u.%u.%u.%u:%u > %u.%u.%u.%u:%u", 
					//	prot_desc, 
					//	( ( action->do_net_send.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_send.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_send.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_send.local_ip_addr & 0x000000ff ), 
					//	action->do_net_send.local_port, 
					//	( ( action->do_net_send.ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_send.ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_send.ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_send.ip_addr & 0x000000ff ), 
					//	action->do_net_send.port );  
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						NETWORK_SEND_OBJECT_PATH_FORMAT_TEXT2, 
						prot_desc ); 

					ASSERT( SUCCEEDED( hr ) ); 
				}

				if( action->do_net_send.local_ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:*"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_send.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_send.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_send.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_send.local_ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_send.local_port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"* > ", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u > ", 
						action->do_net_send.local_port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_send.ip_addr == 0 )
				{
					if( remain_size < 3 ) //L"*:"
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					*remain_text = L'*'; 
					*( remain_text +  1 )= L':'; 
					remain_text += 2; 
					remain_size -= 2; 
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u.%u.%u.%u:", 
						( ( action->do_net_send.ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_send.ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_send.ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_send.ip_addr & 0x000000ff ) ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( action->do_net_send.port == 0 )
				{
					hr = StringCchCopyExW( remain_text, 
						remain_size, 
						L"*", 
						&remain_text, 
						&remain_size, 
						0 ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}
				else
				{
					hr = StringCchPrintfExW( remain_text, 
						remain_size, 
						&remain_text, 
						&remain_size, 
						0, 
						L"%u", 
						action->do_net_send.port ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}	
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
			//case SOCKET_SEND:
			//	break; 
		case NET_recv:
			{
				LPWSTR remain_text; 
				size_t remain_size; 

				//LPCWSTR prot_desc; 
				//prot_desc = get_ip_proto_text( action->do_net_recv.protocol ); 

				remain_text = obj_path; 
				remain_size = cc_buf_len; 

				switch( action->do_net_recv.protocol )
				{
				case PROT_TCP:
					{
						hr = StringCchCopyExW( remain_text, 
							remain_size, 
							NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT, 
							&remain_text, 
							&remain_size, 
							0 ); 
						
						ASSERT( SUCCEEDED( hr ) ); 

						if( action->do_net_recv.ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:*"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_text += 2; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"* < ", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u < ", 
								action->do_net_recv.port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_text += 2; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.local_ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"*", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u", 
								action->do_net_recv.local_port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						//hr = StringCchPrintfExW( remain_text, 
						//	remain_text, 
						//	L"%u.%u.%u.%u:%u < %u.%u.%u.%u:%u", 
						//	( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_recv.ip_addr & 0x000000ff ), 
						//	action->do_net_recv.port, 
						//	( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_recv.local_ip_addr & 0x000000ff ), 
						//	action->do_net_recv.local_port );  
					}
					break; 
				case PROT_UDP:
					{
						hr = StringCchCopyExW( remain_text, 
							remain_size, 
							NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT2, 
							&remain_text, 
							&remain_size, 
							0 ); 

						ASSERT( SUCCEEDED( hr ) ); 

						if( action->do_net_recv.ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:*"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_text += 2; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"* < ", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u < ", 
								action->do_net_recv.port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_text += 2; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.local_ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"*", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u", 
								action->do_net_recv.local_port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(UDPЭ��)%u.%u.%u.%u:%u < %u.%u.%u.%u:%u",  
						//	( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_recv.ip_addr & 0x000000ff ), 
						//	action->do_net_recv.port, 
						//	( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
						//	( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
						//	( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						//	( action->do_net_recv.local_ip_addr & 0x000000ff ), 
						//	action->do_net_recv.local_port );  
					}
					break; 
				default:
					{
						hr = StringCchPrintfExW( remain_text, 
							remain_size, 
							&remain_text, 
							&remain_size, 
							0, 
							NETWORK_RECEIVE_OBJECT_PATH_FORMAT_TEXT3, 
							action->do_net_recv.protocol ); 

						ASSERT( SUCCEEDED( hr ) ); 

						if( action->do_net_recv.ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:*"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"* < ", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u < ", 
								action->do_net_recv.port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_ip_addr == 0 )
						{
							if( remain_size < 3 ) //L"*:"
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}

							*remain_text = L'*'; 
							*( remain_text +  1 )= L':'; 
							remain_size -= 2; 
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u.%u.%u.%u:", 
								( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
								( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
								( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
								( action->do_net_recv.local_ip_addr & 0x000000ff ) ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}

						if( action->do_net_recv.local_port == 0 )
						{
							hr = StringCchCopyExW( remain_text, 
								remain_size, 
								L"*", 
								&remain_text, 
								&remain_size, 
								0 ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
						else
						{
							hr = StringCchPrintfExW( remain_text, 
								remain_size, 
								&remain_text, 
								&remain_size, 
								0, 
								L"%u", 
								action->do_net_recv.local_port ); 

							if( FAILED( hr ) )
							{
								ret = ERROR_BUFFER_TOO_SMALL; 
								break; 
							}
						}
					}
					//hr = StringCchPrintfW( obj_path, cc_buf_len, L"��������:(Э��ID:%u)%u.%u.%u.%u:%u < %u.%u.%u.%u:%u", 
					//	action->do_net_recv.protocol, 
					//	( ( action->do_net_recv.ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_recv.ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_recv.ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_recv.ip_addr & 0x000000ff ), 
					//	action->do_net_recv.port, 
					//	( ( action->do_net_recv.local_ip_addr & 0xff000000 ) >> 24 ), 
					//	( ( action->do_net_recv.local_ip_addr & 0x00ff0000 ) >> 16), 
					//	( ( action->do_net_recv.local_ip_addr & 0x0000ff00 ) >> 8 ), 
					//	( action->do_net_recv.local_ip_addr & 0x000000ff ), 
					//	action->do_net_recv.local_port ); 

					break; 
				}

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				} 

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}
			break; 
			//case SOCKET_RECV:
			//	break; 
		case NET_accept:
			{
				LPCWSTR prot_desc; 
				
				prot_desc = get_ip_proto_text( action->do_net_accept.protocol ); 

				if( *prot_desc == L'\0' )
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT, 
						action->do_net_accept.protocol, 
						( ( action->do_net_accept.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_accept.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_accept.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_accept.local_ip_addr & 0x000000ff ), 
						action->do_net_accept.local_port, 
						( ( action->do_net_accept.ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_accept.ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_accept.ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_accept.ip_addr & 0x000000ff ), 
						action->do_net_accept.port );  
				}
				else
				{
					hr = StringCchPrintfW( obj_path, cc_buf_len, 
						NETWORK_ACCEPT_OBJECT_PATH_FORMAT_TEXT2, 
						prot_desc, 
						( ( action->do_net_accept.local_ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_accept.local_ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_accept.local_ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_accept.local_ip_addr & 0x000000ff ), 
						action->do_net_accept.local_port, 
						( ( action->do_net_accept.ip_addr & 0xff000000 ) >> 24 ), 
						( ( action->do_net_accept.ip_addr & 0x00ff0000 ) >> 16), 
						( ( action->do_net_accept.ip_addr & 0x0000ff00 ) >> 8 ), 
						( action->do_net_accept.ip_addr & 0x000000ff ), 
						action->do_net_accept.port );  
				}

				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				} 

				_obj_path = obj_path; 
				path_len = wcslen( _obj_path ); 
			}
			break; 
			//case SOCKET_ACCEPT:
			//	break; 
		case NET_dns: 
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_net_dns.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_net_dns.path_name; 
				}

				path_len = action->do_net_dns.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			//hr = StringCchCopyW( obj_path, cc_buf_len, action->do_net_http.path_name ); 
			//if( FAILED( hr ) )
			//{
			//	break; 
			//}			
			break; //HTTP���� HTTP����·������ʽ������/URL�� 
		case NET_http: 
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_net_http.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_net_http.path_name; 
				}

				path_len = action->do_net_http.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			//hr = StringCchCopyW( obj_path, cc_buf_len, action->do_net_http.path_name ); 
			//if( FAILED( hr ) )
			//{
			//	break; 
			//}			
			break; //HTTP���� HTTP����·������ʽ������/URL�� 

		case NET_icmp_send:
			hr = StringCchPrintfW( obj_path, cc_buf_len, 
				NETWORK_ICMP_SEND_OBJECT_PATH_FORMAT_TEXT, 
				//L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u", 
				( ( action->do_net_icmp_send.src_ip & 0xff000000 ) >> 24 ), 
				( ( action->do_net_icmp_send.src_ip & 0x00ff0000 ) >> 16), 
				( ( action->do_net_icmp_send.src_ip & 0x0000ff00 ) >> 8 ), 
				( action->do_net_icmp_send.src_ip & 0x000000ff ), 
				( ( action->do_net_icmp_send.dest_ip & 0xff000000 ) >> 24 ), 
				( ( action->do_net_icmp_send.dest_ip & 0x00ff0000 ) >> 16), 
				( ( action->do_net_icmp_send.dest_ip & 0x0000ff00 ) >> 8 ), 
				( action->do_net_icmp_send.dest_ip & 0x000000ff ) );  

			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			} 

			_obj_path = obj_path; 
			path_len = wcslen( _obj_path ); 
			break; 
		case NET_icmp_recv:
			//ULONG src_ip; 
			//ULONG dest_ip; 
			//BYTE type; 
			//BYTE code; 
			hr = StringCchPrintfW( obj_path, cc_buf_len, 
				NETWORK_ICMP_RECEIVE_OBJECT_PATH_FORMAT_TEXT, //L"����icmp:ԴIP:%u.%u.%u.%u Ŀ��IP:%u.%u.%u.%u", 
				( ( action->do_net_icmp_recv.src_ip & 0xff000000 ) >> 24 ), 
				( ( action->do_net_icmp_recv.src_ip & 0x00ff0000 ) >> 16), 
				( ( action->do_net_icmp_recv.src_ip & 0x0000ff00 ) >> 8 ), 
				( action->do_net_icmp_recv.src_ip & 0x000000ff ), 
				( ( action->do_net_icmp_recv.dest_ip & 0xff000000 ) >> 24 ), 
				( ( action->do_net_icmp_recv.dest_ip & 0x00ff0000 ) >> 16), 
				( ( action->do_net_icmp_recv.dest_ip & 0x0000ff00 ) >> 8 ), 
				( action->do_net_icmp_recv.dest_ip & 0x000000ff ) );  

			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			} 

			_obj_path = obj_path; 
			path_len = wcslen( _obj_path ); 
			break; 
			//MT_behavior, 
		case BA_extract_hidden:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_extract_hidden.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_extract_hidden.path_name; 
				}

				path_len = action->do_ba_extract_hidden.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_extract_pe.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_extract_pe.path_name; 
				}

				path_len = action->do_ba_extract_pe.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_self_copy.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_self_copy.path_name; 
				}

				path_len = action->do_ba_self_copy.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_self_delete.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_self_delete.path_name; 
				}

				path_len = action->do_ba_self_delete.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_ulterior_exec.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_ulterior_exec.path_name; 
				}

				path_len = action->do_ba_ulterior_exec.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_invade_process.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_invade_process.path_name; 
				}

				path_len = action->do_ba_invade_process.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_infect_pe.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_infect_pe.path_name; 
				}

				path_len = action->do_ba_infect_pe.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' );
			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_overwrite_pe.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_overwrite_pe.path_name; 
				}

				path_len = action->do_ba_overwrite_pe.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_ba_register_autorun.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_ba_register_autorun.path_name; 
				}

				path_len = action->do_ba_register_autorun.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
			break; //ע���������� �������ļ�·���� 

			//case BA_other:
			//desc = L"BA_other"; 
			//break; 
		case COM_access:
			{
				if( flags & SAVE_INFO_BY_POINTER )
				{
					_obj_path = action->do_com_access.path_name_ptr; 
				}
				else
				{
					_obj_path = action->do_com_access.path_name; 
				}

				path_len = action->do_com_access.path_len; 
			}

			ASSERT( _obj_path[ path_len ] == L'\0' ); 
		case OTHER_ACTION:
			{	
				break; 
			}
		default:
			{
				ASSERT( FALSE && "unknown action type" ); 
				log_trace( ( MSG_ERROR, "invalid system action type %d \n", action->type ) ); 
			}
		}

	}while( FALSE );

	if( ret == ERROR_SUCCESS )
	{
		*obj_path_ptr = _obj_path; 
		*cc_ret_len = path_len; 
	}

	return ret; 
}