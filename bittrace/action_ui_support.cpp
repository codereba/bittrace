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
#include "_resource.h"

#include "action_ui_support.h"

#define MT_EXEC_DESC L"ģ�����"
#define MT_FILE_DESC L"�ļ�"
#define MT_REG_DESC L"ע���"
#define MT_PROC_DESC L"����/�߳�"
#define MT_COM_DESC L"COM���"
#define MT_SYS_DESC L"ϵͳ�ؼ�"
#define MT_WIN32_DESC L"WIN32"
#define MT_NET_DESC L"����"
#define MT_BEHAVIOR_DESC L"������Ϊ"
#define MT_PERIPHERAL_DESC L"����"

#define MT_EXEC_IMAGE_FILE_NAME L"exe_event.png"
#define MT_FILE_IMAGE_FILE_NAME L"file_event.png"
#define MT_REG_IMAGE_FILE_NAME L"reg_event.png"
#define MT_PROC_IMAGE_FILE_NAME L"proc_event.png"
#define MT_COM_IMAGE_FILE_NAME L"com_event.png"
#define MT_SYS_IMAGE_FILE_NAME L"sys_event.png"
#define MT_WIN32_IMAGE_FILE_NAME L"w32_event.png"
#define MT_NET_IMAGE_FILE_NAME L"net_event.png"
#define MT_BEHAVIOR_IMAGE_FILE_NAME L"behavior_event.png"
#define MT_PERIPHERAL_IMAGE_FILE_NAME L"peripheral_event.png"

LRESULT WINAPI get_string_width_from_font( )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE );

	return ret; 
}

#ifdef LANG_EN
#define EVENT_NAME_OTHER L"Other" //L"����"
#define EVENT_NAME_EXEC_CREATE L"Process start" //L"��������"
#define EVENT_NAME_EXEC_DESTROY L"Process stop" //L"�����˳�"
#define EVENT_NAME_MODULE_LOAD L"Module load" //L"ģ�����";
#define EVENT_NAME_FILE_TOUCH L"Set file time" //L"�޸��ļ�����" 
#define EVENT_NAME_FILE_OPEN L"Open file" //L"���ļ�"
#define EVENT_NAME_FILE_READ L"Read file" //L"��ȡ�ļ�"
#define EVENT_NAME_FILE_WRITE L"Write file" //L"��ȡ�ļ�"
#define EVENT_NAME_FILE_MODIFIED L"File changed" //L"�ļ����޸�"
#define EVENT_NAME_TRAVERSE_DIRECTORY L"Traverse dir" //L"����Ŀ¼"
#define EVENT_NAME_REMOVE_FILE L"Remove file" //L"ɾ���ļ�"
#define EVENT_NAME_RENAME_FILE L"Rename file" //L"�������ļ�"
#define EVENT_NAME_TRUNCATE_FILE L"Truncate file" //L"�ض��ļ�"
#define EVENT_NAME_MAKE_LINK L"File hard link" //L"�����ļ�Ӳ����"
#define EVENT_NAME_CHANGE_MODE L"Set file attributes" //L"�����ļ�����"
#define EVENT_NAME_SET_SECURITY_INFO L"Set file security" //L"�����ļ���ȫ����"
#define EVENT_NAME_GET_INFO L"Get file info" //L"��ѯ�ļ������Ϣ"
#define EVENT_NAME_SET_INFO L"Set file info" //L"�����ļ������Ϣ"
#define EVENT_NAME_FILE_CLOSE L"Close file" //L"�ر��ļ�"

#define EVENT_NAME_REG_OPEN_KEY L"Open key" //L"��ע�����"
#define EVENT_NAME_MAKE_KEY L"Create key" //L"����ע�����"
#define EVENT_NAME_REMOVE_KEY L"Remove key" //L"ɾ��ע�����"
#define EVENT_NAME_MOVE_KEY L"Rename key" //L"������ע�����"
#define EVENT_NAME_GET_KEY_INFO L"Get key info" // L"��ȡע�������Ϣ"
#define EVENT_NAME_SET_KEY_INFO L"Set key info" //L"����ע�������Ϣ"
#define EVENT_NAME_ENUM_KEY_INFO L"Enum sub key" //L"ö��ע����Ӽ�"
#define EVENT_NAME_ENUM_KEY_VALUE L"Enum key value" //L"ö��ע�����ֵ"
#define EVENT_NAME_REMOVE_KEY_VALUE L"Remove key value" //L"ɾ��ע�����"
#define EVENT_NAME_GET_KEY_VALUE L"Get key value" //L"��ȡע���ֵ"
#define EVENT_NAME_SET_KEY_VALUE L"Set key value" //L"����ע���ֵ"
#define EVENT_NAME_LOAD_KEY_HIVE L"Load key from hive" //L"����ע���Hive�ļ�"
#define EVENT_NAME_REPLACE_KEY L"Replace key" //L"�滻ע�����"
#define EVENT_NAME_RESTORE_KEY L"Restore key from hive" //L"����ע���Hive�ļ�"
#define EVENT_NAME_SET_KEY_SECURITY L"Set key security" //L"����ע�������ȫ����"
#define EVENT_NAME_CLOSE_KEY L"Close key" //L"�ر�ע�����"

#define EVENT_NAME_PROCESS_EXEC L"process execute" //L"��������"
#define EVENT_NAME_PROCESS_OPEN L"process open" //L"�򿪽���"
#define EVENT_NAME_PROCESS_DEBUG L"process debug" //L"���Խ���"
#define EVENT_NAME_PROCESS_SUSPEND L"process suspend" //L"�������"
#define EVENT_NAME_PROCESS_RESUME L"process resume" //L"�ָ�����"
#define EVENT_NAME_PROCESS_EXIT L"process exit" //L"�����˳�"
#define EVENT_NAME_PROCESS_JOB L"process join job" //L"�����̼��빤����"
#define EVENT_NAME_PROCESS_PAGE_PROTECT L"set process page protection" //L"������޸��ڴ�����"
#define EVENT_NAME_PROCESS_FREE_VM L"free process virtual mem" //L"������ͷ��ڴ�"
#define EVENT_NAME_PROCESS_WRITE_VM L"write process virtual mem" //L"�����д�ڴ�"
#define EVENT_NAME_PROCESS_READ_VM L"read process virtual mem" //L"����̶��ڴ�"
#define EVENT_NAME_THREAD_REMOTE L"create remote thread" //L"����Զ���߳�"
#define EVENT_NAME_THREAD_CREATE L"create thread" //L"�����߳�"
#define EVENT_NAME_THREAD_SET_CONTEXT L"set thread context" //L"����������߳�������"
#define EVENT_NAME_THREAD_SUSPEND L"thread suspend" //L"����̹����߳�"
#define EVENT_NAME_THREAD_RESUME L"thread resume" //L"����ָ̻��߳�"
#define EVENT_NAME_THREAD_EXIT L"thread exit" //L"�߳��˳�"
#define EVENT_NAME_THREAD_QUEUE_APC L"thread queue apc" //L"������Ŷ�APC"
#define EVENT_NAME_LOAD_COM L"load com" //L"����COM���"
#define EVENT_NAME_URB_IO L"URB I/O" //L"URBͨ��"
#define EVENT_NAME_SET_TIME L"set system time" //L"����ϵͳʱ��"
#define EVENT_NAME_LINK_KNOWN_DLL L"link known dll" //L"����KnownDlls����"
#define EVENT_NAME_OPEN_PHYSICAL_MEM L"open physical mem" //L"�������ڴ��豸"
#define EVENT_NAME_READ_PHYSICAL_MEM L"read physical mem" //L"�������ڴ�"
#define EVENT_NAME_WRITE_PHYSICAL_MEM L"write physical mem" //L"д�����ڴ�"
#define EVENT_NAME_LOAD_KERNLE_MODULE L"load kernel module" //L"�����ں�ģ��"
#define EVENT_NAME_LOAD_MODULE L"load module" //L"����ģ��"
#define EVENT_NAME_UNLOAD_MODULE L"unload module" //L"ж��ģ��"
#define EVENT_NAME_ENUM_PROCESS L"enum process" //L"ö�ٽ���"
#define EVENT_NAME_REG_SERVICE L"register service" //L"ע�����"
#define EVENT_NAME_OPEN_DEVICE L"open device" //L"���豸"
#define EVENT_NAME_NET_CREATE L"socket create" //L"�����׽���"
#define EVENT_NAME_NET_CONNECT L"socket connect" //L"��������"
#define EVENT_NAME_NET_LISTEN L"socket listen" //L"�����˿�"
#define EVENT_NAME_NET_SEND L"socket send" //L"�������ݰ�"
#define EVENT_NAME_NET_RECEIVE L"socket receive" //L"�������ݰ�"
#define EVENT_NAME_NET_ACCEPT L"socket accept" //L"����TCP����"
#define EVENT_NAME_NET_PARSE_DNS L"DNS request" //L"��������(DNS)����"
#define EVENT_NAME_NET_HTTP L"HTTP request" //L"HTTP����"
#define EVENT_NAME_NET_ICMP_SEND L"ICMP send" //L"����ICMP��"
#define EVENT_NAME_NET_ICMP_RECEIVE L"ICMP receive" //L"����ICMP��"

#define EVENT_NAME_OTERH L"Other event" //L"����" 

#else
#define EVENT_NAME_OTHER L"����"
#define EVENT_NAME_EXEC_CREATE L"��������"
#define EVENT_NAME_EXEC_DESTROY L"�����˳�"
#define EVENT_NAME_MODULE_LOAD L"ģ�����";
#define EVENT_NAME_FILE_TOUCH L"�޸��ļ�����" 
#define EVENT_NAME_FILE_OPEN L"���ļ�"
#define EVENT_NAME_FILE_READ L"��ȡ�ļ�"
#define EVENT_NAME_FILE_WRITE L"��ȡ�ļ�"
#define EVENT_NAME_FILE_MODIFIED L"�ļ����޸�"
#define EVENT_NAME_TRAVERSE_DIRECTORY L"����Ŀ¼"
#define EVENT_NAME_REMOVE_FILE L"ɾ���ļ�"
#define EVENT_NAME_RENAME_FILE L"�������ļ�"
#define EVENT_NAME_TRUNCATE_FILE L"�ض��ļ�"
#define EVENT_NAME_MAKE_LINK L"�����ļ�Ӳ����"
#define EVENT_NAME_CHANGE_MODE L"�����ļ�����"
#define EVENT_NAME_SET_SECURITY_INFO L"�����ļ���ȫ����"
#define EVENT_NAME_GET_INFO L"��ѯ�ļ������Ϣ"
#define EVENT_NAME_SET_INFO L"�����ļ������Ϣ"
#define EVENT_NAME_FILE_CLOSE L"�ر��ļ�"

#define EVENT_NAME_REG_OPEN_KEY L"��ע�����"
#define EVENT_NAME_MAKE_KEY L"����ע�����"
#define EVENT_NAME_REMOVE_KEY L"ɾ��ע�����"
#define EVENT_NAME_MOVE_KEY L"������ע�����"
#define EVENT_NAME_GET_KEY_INFO  L"��ȡע�������Ϣ"
#define EVENT_NAME_SET_KEY_INFO L"����ע�������Ϣ"
#define EVENT_NAME_ENUM_KEY_INFO L"ö��ע����Ӽ�"
#define EVENT_NAME_ENUM_KEY_VALUE L"ö��ע�����ֵ"
#define EVENT_NAME_REMOVE_KEY_VALUE L"ɾ��ע�����"
#define EVENT_NAME_GET_KEY_VALUE L"��ȡע���ֵ"
#define EVENT_NAME_SET_KEY_VALUE L"����ע���ֵ"
#define EVENT_NAME_LOAD_KEY_HIVE L"����ע���Hive�ļ�"
#define EVENT_NAME_REPLACE_KEY L"�滻ע�����"
#define EVENT_NAME_RESTORE_KEY L"����ע���Hive�ļ�"
#define EVENT_NAME_SET_KEY_SECURITY L"����ע�������ȫ����"
#define EVENT_NAME_CLOSE_KEY L"�ر�ע�����"

#define EVENT_NAME_PROCESS_EXEC L"��������"
#define EVENT_NAME_PROCESS_OPEN L"�򿪽���"
#define EVENT_NAME_PROCESS_DEBUG L"���Խ���"
#define EVENT_NAME_PROCESS_SUSPEND L"�������"
#define EVENT_NAME_PROCESS_RESUME L"�ָ�����"
#define EVENT_NAME_PROCESS_EXIT L"�����˳�"
#define EVENT_NAME_PROCESS_JOB L"�����̼��빤����"
#define EVENT_NAME_PROCESS_PAGE_PROTECT L"������޸��ڴ�����"
#define EVENT_NAME_PROCESS_FREE_VM L"������ͷ��ڴ�"
#define EVENT_NAME_PROCESS_WRITE_VM L"�����д�ڴ�"
#define EVENT_NAME_PROCESS_READ_VM L"����̶��ڴ�"
#define EVENT_NAME_THREAD_REMOTE L"����Զ���߳�"
#define EVENT_NAME_THREAD_CREATE L"�����߳�"
#define EVENT_NAME_THREAD_SET_CONTEXT L"����������߳�������"
#define EVENT_NAME_THREAD_SUSPEND L"����̹����߳�"
#define EVENT_NAME_THREAD_RESUME L"����ָ̻��߳�"
#define EVENT_NAME_THREAD_EXIT L"�߳��˳�"
#define EVENT_NAME_THREAD_QUEUE_APC L"������Ŷ�APC"
#define EVENT_NAME_LOAD_COM L"����COM���"
#define EVENT_NAME_URB_IO L"URBͨ��"
#define EVENT_NAME_SET_TIME L"����ϵͳʱ��"
#define EVENT_NAME_LINK_KNOWN_DLL L"����KnownDlls����"
#define EVENT_NAME_OPEN_PHYSICAL_MEM L"�������ڴ��豸"
#define EVENT_NAME_READ_PHYSICAL_MEM L"�������ڴ�"
#define EVENT_NAME_WRITE_PHYSICAL_MEM L"д�����ڴ�"
#define EVENT_NAME_LOAD_KERNLE_MODULE L"�����ں�ģ��"
#define EVENT_NAME_LOAD_MODULE L"����ģ��"
#define EVENT_NAME_UNLOAD_MODULE L"ж��ģ��"
#define EVENT_NAME_ENUM_PROCESS L"ö�ٽ���"
#define EVENT_NAME_REG_SERVICE L"ע�����"
#define EVENT_NAME_OPEN_DEVICE L"���豸"
#define EVENT_NAME_NET_CREATE L"�����׽���"
#define EVENT_NAME_NET_CONNECT L"��������"
#define EVENT_NAME_NET_LISTEN L"�����˿�"
#define EVENT_NAME_NET_SEND L"�������ݰ�"
#define EVENT_NAME_NET_RECEIVE L"�������ݰ�"
#define EVENT_NAME_NET_ACCEPT L"����TCP����"
#define EVENT_NAME_NET_PARSE_DNS L"��������(DNS)����"
#define EVENT_NAME_NET_HTTP L"HTTP����"
#define EVENT_NAME_NET_ICMP_SEND L"����ICMP��"
#define EVENT_NAME_NET_ICMP_RECEIVE L"����ICMP��"

#define EVENT_NAME_OTERH L"����" 
#endif //LANG_EN
LPCWSTR WINAPI get_event_name( sys_action_type type )
{
	LPCWSTR event_name = EVENT_NAME_OTHER; 

	switch( type )
	{
	case EXEC_create: //�������� ����·���� ��ִ�м�أ�
		event_name = EVENT_NAME_EXEC_CREATE; 
		break; 

	case EXEC_destroy: //�����˳� ����·���� 
			event_name = EVENT_NAME_EXEC_DESTROY; 
			break; 

	case EXEC_module_load: 
		event_name = EVENT_NAME_MODULE_LOAD; 
		break; //ģ����� ģ��·���� 

		//MT_filemon: break 
	case FILE_touch: 
		event_name = EVENT_NAME_FILE_TOUCH; 
		break;  //�����ļ� �ļ�ȫ·�� ���ļ���أ� 
	case FILE_open: event_name = EVENT_NAME_FILE_OPEN; break;  //���ļ� �ļ�ȫ·�� 
	case FILE_read: event_name = EVENT_NAME_FILE_READ; break;  //��ȡ�ļ� �ļ�ȫ·�� 
	case FILE_write: event_name = EVENT_NAME_FILE_WRITE;  break;  //д���ļ� �ļ�ȫ·�� 
	case FILE_modified: event_name = EVENT_NAME_FILE_MODIFIED;  break;  //�ļ����޸� �ļ�ȫ·�� 
	case FILE_readdir: event_name = EVENT_NAME_TRAVERSE_DIRECTORY;  break;  //����Ŀ¼ Ŀ¼ȫ·�� 
	case FILE_remove: event_name = EVENT_NAME_REMOVE_FILE;  break;  //ɾ���ļ� �ļ�ȫ·�� 
	case FILE_rename: event_name = EVENT_NAME_RENAME_FILE;  break;  //�������ļ� �ļ�ȫ·�� 
	case FILE_truncate: event_name = EVENT_NAME_TRUNCATE_FILE;  break;  //�ض��ļ� �ļ�ȫ·�� 
	case FILE_mklink: event_name = EVENT_NAME_MAKE_LINK;  break;  //�����ļ�Ӳ���� �ļ�ȫ·�� 
	case FILE_chmod: event_name = EVENT_NAME_CHANGE_MODE;  break;  //�����ļ����� �ļ�ȫ·�� 
	case FILE_setsec: event_name = EVENT_NAME_SET_SECURITY_INFO;  break;  //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_getinfo: event_name = EVENT_NAME_GET_INFO;  break;  //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_setinfo: event_name = EVENT_NAME_SET_INFO;  break;  //�����ļ���ȫ���� �ļ�ȫ·�� 
	case FILE_close: event_name = EVENT_NAME_FILE_CLOSE;  break;  //�����ļ���ȫ���� �ļ�ȫ·�� 

		//MT_regmon: event_name = L"";  break;  
	case REG_openkey: event_name = EVENT_NAME_REG_OPEN_KEY;  break;  //��ע����� ע�����·��  ��ע�����أ� 
	case REG_mkkey: event_name = EVENT_NAME_MAKE_KEY;  break;  //����ע����� ע�����·�� 
	case REG_rmkey: event_name = EVENT_NAME_REMOVE_KEY;  break;  //ɾ��ע����� ע�����·�� 
	case REG_mvkey: event_name = EVENT_NAME_MOVE_KEY;  break;  //������ע����� ע�����·�� 
	case REG_getinfo: event_name = EVENT_NAME_GET_KEY_INFO;  break;  
	case REG_setinfo: event_name = EVENT_NAME_SET_KEY_INFO;  break;  
	case REG_enuminfo: event_name = EVENT_NAME_ENUM_KEY_INFO;  break;  
	case REG_enum_val: event_name = EVENT_NAME_ENUM_KEY_VALUE;  break;  
	case REG_rmval: event_name = EVENT_NAME_REMOVE_KEY_VALUE;  break;  //ɾ��ע����� ע�����·�� 
	case REG_getval: event_name = EVENT_NAME_GET_KEY_VALUE;  break;  //��ȡע���ֵ ע���ֵ·�� 
	case REG_setval: event_name = EVENT_NAME_SET_KEY_VALUE;  break;  //����ע���ֵ ע���ֵ·�� 
	case REG_loadkey: event_name = EVENT_NAME_LOAD_KEY_HIVE;  break;  //����ע���Hive�ļ� ע�����·�� 
	case REG_replkey: event_name = EVENT_NAME_REPLACE_KEY;  break;  //�滻ע����� ע�����·�� 
	case REG_rstrkey: event_name = EVENT_NAME_RESTORE_KEY;  break;  //����ע���Hive�ļ� ע�����·�� 
	case REG_setsec: event_name = EVENT_NAME_SET_KEY_SECURITY;  break;  //����ע�������ȫ���� ע�����·�� 
	case REG_closekey: event_name = EVENT_NAME_CLOSE_KEY;  break;  //����ע�������ȫ���� ע�����·�� 

		//MT_procmon: event_name = L"";  break;  
	case PROC_exec: event_name = EVENT_NAME_PROCESS_EXEC;  break;  //�������� Ŀ�����·����  �����̼�أ�
	case PROC_open: event_name = EVENT_NAME_PROCESS_OPEN;  break;  //�򿪽��� Ŀ�����·���� 
	case PROC_debug: event_name = EVENT_NAME_PROCESS_DEBUG;  break;  //���Խ��� Ŀ�����·���� 
	case PROC_suspend: event_name = EVENT_NAME_PROCESS_SUSPEND;  break;  //������� Ŀ�����·���� 
	case PROC_resume: event_name = EVENT_NAME_PROCESS_RESUME;  break;  //�ָ����� Ŀ�����·���� 
	case PROC_exit: event_name = EVENT_NAME_PROCESS_EXIT;  break;  //�������� Ŀ�����·���� 
	case PROC_job: event_name = EVENT_NAME_PROCESS_JOB;  break;  //�����̼��빤���� Ŀ�����·���� 
	case PROC_pgprot: event_name = EVENT_NAME_PROCESS_PAGE_PROTECT;  break;  //������޸��ڴ����� Ŀ�����·���� 
	case PROC_freevm: event_name = EVENT_NAME_PROCESS_FREE_VM;  break;  //������ͷ��ڴ� Ŀ�����·���� 
	case PROC_writevm: event_name = EVENT_NAME_PROCESS_WRITE_VM;  break;  //�����д�ڴ� Ŀ�����·���� 
	case PROC_readvm: event_name = EVENT_NAME_PROCESS_READ_VM;  break;  //����̶��ڴ� Ŀ�����·���� 
	case THRD_remote: event_name = EVENT_NAME_THREAD_REMOTE;  break;  //����Զ���߳� Ŀ�����·���� 
	case THRD_create: event_name = EVENT_NAME_THREAD_CREATE;  break;  //�����߳�
	case THRD_setctxt: event_name = EVENT_NAME_THREAD_SET_CONTEXT;  break;  //����������߳������� Ŀ�����·���� 
	case THRD_suspend: event_name = EVENT_NAME_THREAD_SUSPEND;  break;  //����̹����߳� Ŀ�����·���� 
	case THRD_resume: event_name = EVENT_NAME_THREAD_RESUME;  break;  //����ָ̻��߳� Ŀ�����·���� 
	case THRD_exit: event_name = EVENT_NAME_THREAD_EXIT;  break;  //����̽����߳� Ŀ�����·���� 
	//case THRD_exit: event_name = L"����̽����߳�";  break;  //����̽����߳� Ŀ�����·���� 
	case THRD_queue_apc: event_name = EVENT_NAME_THREAD_QUEUE_APC;  break;  //������Ŷ�APC Ŀ�����·���� 

		//MT_common
	case COM_access: event_name = EVENT_NAME_LOAD_COM;  break;  
	//case PORT_read: event_name = L"��ȡ����"; break; 
	//case PORT_write: event_name = L"д�봮��"; break; 
	case PORT_urb: event_name = EVENT_NAME_URB_IO; break; 

		//MT_sysmon
	case SYS_settime: event_name = EVENT_NAME_SET_TIME;  break;  //����ϵͳʱ�� �� 
	case SYS_link_knowndll: event_name = EVENT_NAME_LINK_KNOWN_DLL;  break;  //����KnownDlls���� �����ļ��� 
	case SYS_open_physmm: event_name = EVENT_NAME_OPEN_PHYSICAL_MEM;  break;  //�������ڴ��豸 �� 
	case SYS_read_physmm: event_name = EVENT_NAME_READ_PHYSICAL_MEM;  break;  //�������ڴ� �� 
	case SYS_write_physmm: event_name = EVENT_NAME_WRITE_PHYSICAL_MEM;  break;  //д�����ڴ� �� 
	case SYS_load_kmod: event_name = EVENT_NAME_LOAD_KERNLE_MODULE;  break;  //�����ں�ģ�� �ں�ģ��ȫ·�� 
	case SYS_load_mod: event_name = EVENT_NAME_LOAD_MODULE;  break;  //����ģ�� �ں�ģ��ȫ·�� 
	case SYS_unload_mod: event_name = EVENT_NAME_UNLOAD_MODULE;  break;  //ж��ģ�� �ں�ģ��ȫ·�� 
	case SYS_enumproc: event_name = EVENT_NAME_ENUM_PROCESS;  break;  //ö�ٽ��� �� 
	case SYS_regsrv: event_name = EVENT_NAME_REG_SERVICE;  break;  //ע����� �������ȫ·�� 
	case SYS_opendev: event_name = EVENT_NAME_OPEN_DEVICE;  break;  //���豸 �豸�� 

		//MT_w32mon
	case W32_postmsg: event_name = L"���ʹ�����Ϣ(Post)";  break;  //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
	case W32_sendmsg: event_name = L"���ʹ�����Ϣ(Send)";  break;  //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
	case W32_findwnd: event_name = L"���Ҵ���";  break;  //���Ҵ��� �� 
	case W32_msghook: event_name = L"������Ϣ����";  break;  //������Ϣ���� �� 
	case W32_lib_inject: event_name = L"DLLע��";  break;  //DLLע�� ע��DLL·���� 

		//MT_netmon: event_name = L"";  break;  
	case NET_create: event_name = EVENT_NAME_NET_CREATE;  break;  
	case NET_connect: event_name = EVENT_NAME_NET_CONNECT;  break;  //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
	case NET_listen: event_name = EVENT_NAME_NET_LISTEN;  break;  //�����˿� ������ַ����ʽ��IP���˿ںţ� 
	case NET_send: event_name = EVENT_NAME_NET_SEND;  break;  //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
	case NET_recv: event_name = EVENT_NAME_NET_RECEIVE;  break;  
	case NET_accept: event_name = EVENT_NAME_NET_ACCEPT;  break;  
	case NET_dns: event_name = EVENT_NAME_NET_PARSE_DNS;  break;  
	case NET_http: event_name = EVENT_NAME_NET_HTTP;  break;  //HTTP���� HTTP����·������ʽ������/URL�� 
	case NET_icmp_send: event_name = EVENT_NAME_NET_ICMP_SEND;  break;  
	case NET_icmp_recv: event_name = EVENT_NAME_NET_ICMP_RECEIVE;  break;  

		//MT_behavior: event_name = L"";  break;  
	case BA_extract_hidden: event_name = L"�ͷ������ļ�";  break;  //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
	case BA_extract_pe: event_name = L"�ͷ�PE�ļ�";  break;  //�ͷ�PE�ļ� �ͷ��ļ�·���� 
	case BA_self_copy: event_name = L"���Ҹ���";  break;  //���Ҹ��� ����Ŀ���ļ�·���� 
	case BA_self_delete: event_name = L"����ɾ��";  break;  //����ɾ�� ɾ���ļ�·���� 
	case BA_ulterior_exec: event_name = L"����ִ��";  break;  //����ִ�� ��ִ��ӳ��·���� 
	case BA_invade_process: event_name = L"���ֽ���";  break;  //���ֽ��� Ŀ�����·���� 
	case BA_infect_pe: event_name = L"��ȾPE�ļ�";  break;  //��ȾPE�ļ� Ŀ���ļ�·���� 
	case BA_overwrite_pe: event_name = L"��дPE�ļ�";  break;  //��дPE�ļ� Ŀ���ļ�·���� 
	case BA_register_autorun: event_name = L"ע����������";  break;  //ע���������� �������ļ�·���� 
	case BA_other: event_name = EVENT_NAME_OTERH;  break;  
	default:
		event_name = L"";  
		break; 
	}

	return event_name; 
}

LRESULT WINAPI get_action_main_type_desc( action_main_type type, LPWSTR *type_desc, UI_PARAM *ui_param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( type_desc != NULL ); 
		ASSERT( ui_param != NULL ); 

		switch( type )
		{
		case MT_execmon: 
			*type_desc = MT_EXEC_DESC; 
			*ui_param = MT_EXEC_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_EXE; 
			break; 
		case MT_filemon:
			*type_desc = MT_FILE_DESC; 
			*ui_param = MT_FILE_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_FILE; 
			break; 

		case MT_regmon: 
			*type_desc = MT_REG_DESC; 
			*ui_param = MT_REG_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_REG; 
			break; 

		case MT_procmon: 
			*type_desc = MT_PROC_DESC; 
			*ui_param = MT_PROC_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_PROC; 
			break; 

		case MT_common: 
			*type_desc = MT_COM_DESC; 
			*ui_param = MT_COM_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_COM; 
			break; 

		case MT_sysmon: 
			*type_desc = MT_SYS_DESC; 
			*ui_param = MT_SYS_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_SYS; 
			break; 

		case MT_w32mon: 
			*type_desc = MT_WIN32_DESC; 
			*ui_param = MT_WIN32_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_W32; 
			break; 

		case MT_netmon: 
			*type_desc = MT_NET_DESC; 
			*ui_param = MT_NET_IMAGE_FILE_NAME; 
				//( UI_PARAM )IDI_ICON_NET; 
			break; 

		case MT_behavior: 
			*type_desc = MT_BEHAVIOR_DESC; 
			*ui_param = MT_BEHAVIOR_IMAGE_FILE_NAME; 
			//( UI_PARAM )IDI_ICON_BEHAVIOR; 
			break; 

		case MT_peripheral: 
			*type_desc = MT_PERIPHERAL_DESC; 
			*ui_param = MT_PERIPHERAL_IMAGE_FILE_NAME; 
			//( UI_PARAM )IDI_ICON_BEHAVIOR; 
			break; 

		default:
			DBG_BP(); 
			ret = ERROR_INVALID_PARAMETER; 
			*type_desc = L"unknown"; 
			*ui_param = ( UI_PARAM )"other_event.png"; 
			break; 
		}
	}while( FALSE );
	
	return ret; 
}