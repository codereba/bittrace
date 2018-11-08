
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
#include "common_func.h"
#include "ring0_2_ring3.h"
#include "action_display.h"
#include "_resource.h"

#include "action_ui_support.h"

#define MT_EXEC_DESC L"模块加载"
#define MT_FILE_DESC L"文件"
#define MT_REG_DESC L"注册表"
#define MT_PROC_DESC L"进程/线程"
#define MT_COM_DESC L"COM组件"
#define MT_SYS_DESC L"系统关键"
#define MT_WIN32_DESC L"WIN32"
#define MT_NET_DESC L"网络"
#define MT_BEHAVIOR_DESC L"特殊行为"
#define MT_PERIPHERAL_DESC L"外设"

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
#define EVENT_NAME_OTHER L"Other" //L"其它"
#define EVENT_NAME_EXEC_CREATE L"Process start" //L"进程启动"
#define EVENT_NAME_EXEC_DESTROY L"Process stop" //L"进程退出"
#define EVENT_NAME_MODULE_LOAD L"Module load" //L"模块加载";
#define EVENT_NAME_FILE_TOUCH L"Set file time" //L"修改文件属性" 
#define EVENT_NAME_FILE_OPEN L"Open file" //L"打开文件"
#define EVENT_NAME_FILE_READ L"Read file" //L"读取文件"
#define EVENT_NAME_FILE_WRITE L"Write file" //L"读取文件"
#define EVENT_NAME_FILE_MODIFIED L"File changed" //L"文件被修改"
#define EVENT_NAME_TRAVERSE_DIRECTORY L"Traverse dir" //L"遍历目录"
#define EVENT_NAME_REMOVE_FILE L"Remove file" //L"删除文件"
#define EVENT_NAME_RENAME_FILE L"Rename file" //L"重命名文件"
#define EVENT_NAME_TRUNCATE_FILE L"Truncate file" //L"截断文件"
#define EVENT_NAME_MAKE_LINK L"File hard link" //L"建立文件硬链接"
#define EVENT_NAME_CHANGE_MODE L"Set file attributes" //L"设置文件属性"
#define EVENT_NAME_SET_SECURITY_INFO L"Set file security" //L"设置文件安全属性"
#define EVENT_NAME_GET_INFO L"Get file info" //L"查询文件相关信息"
#define EVENT_NAME_SET_INFO L"Set file info" //L"设置文件相关信息"
#define EVENT_NAME_FILE_CLOSE L"Close file" //L"关闭文件"

#define EVENT_NAME_REG_OPEN_KEY L"Open key" //L"打开注册表键"
#define EVENT_NAME_MAKE_KEY L"Create key" //L"创建注册表键"
#define EVENT_NAME_REMOVE_KEY L"Remove key" //L"删除注册表键"
#define EVENT_NAME_MOVE_KEY L"Rename key" //L"重命名注册表键"
#define EVENT_NAME_GET_KEY_INFO L"Get key info" // L"获取注册表键信息"
#define EVENT_NAME_SET_KEY_INFO L"Set key info" //L"设置注册表键信息"
#define EVENT_NAME_ENUM_KEY_INFO L"Enum sub key" //L"枚举注册表子键"
#define EVENT_NAME_ENUM_KEY_VALUE L"Enum key value" //L"枚举注册表键值"
#define EVENT_NAME_REMOVE_KEY_VALUE L"Remove key value" //L"删除注册表键"
#define EVENT_NAME_GET_KEY_VALUE L"Get key value" //L"获取注册表值"
#define EVENT_NAME_SET_KEY_VALUE L"Set key value" //L"设置注册表值"
#define EVENT_NAME_LOAD_KEY_HIVE L"Load key from hive" //L"挂载注册表Hive文件"
#define EVENT_NAME_REPLACE_KEY L"Replace key" //L"替换注册表键"
#define EVENT_NAME_RESTORE_KEY L"Restore key from hive" //L"导入注册表Hive文件"
#define EVENT_NAME_SET_KEY_SECURITY L"Set key security" //L"设置注册表键安全属性"
#define EVENT_NAME_CLOSE_KEY L"Close key" //L"关闭注册表键"

#define EVENT_NAME_PROCESS_EXEC L"process execute" //L"创建进程"
#define EVENT_NAME_PROCESS_OPEN L"process open" //L"打开进程"
#define EVENT_NAME_PROCESS_DEBUG L"process debug" //L"调试进程"
#define EVENT_NAME_PROCESS_SUSPEND L"process suspend" //L"挂起进程"
#define EVENT_NAME_PROCESS_RESUME L"process resume" //L"恢复进程"
#define EVENT_NAME_PROCESS_EXIT L"process exit" //L"进程退出"
#define EVENT_NAME_PROCESS_JOB L"process join job" //L"将进程加入工作集"
#define EVENT_NAME_PROCESS_PAGE_PROTECT L"set process page protection" //L"跨进程修改内存属性"
#define EVENT_NAME_PROCESS_FREE_VM L"free process virtual mem" //L"跨进程释放内存"
#define EVENT_NAME_PROCESS_WRITE_VM L"write process virtual mem" //L"跨进程写内存"
#define EVENT_NAME_PROCESS_READ_VM L"read process virtual mem" //L"跨进程读内存"
#define EVENT_NAME_THREAD_REMOTE L"create remote thread" //L"创建远程线程"
#define EVENT_NAME_THREAD_CREATE L"create thread" //L"创建线程"
#define EVENT_NAME_THREAD_SET_CONTEXT L"set thread context" //L"跨进程设置线程上下文"
#define EVENT_NAME_THREAD_SUSPEND L"thread suspend" //L"跨进程挂起线程"
#define EVENT_NAME_THREAD_RESUME L"thread resume" //L"跨进程恢复线程"
#define EVENT_NAME_THREAD_EXIT L"thread exit" //L"线程退出"
#define EVENT_NAME_THREAD_QUEUE_APC L"thread queue apc" //L"跨进程排队APC"
#define EVENT_NAME_LOAD_COM L"load com" //L"加载COM组件"
#define EVENT_NAME_URB_IO L"URB I/O" //L"URB通信"
#define EVENT_NAME_SET_TIME L"set system time" //L"设置系统时间"
#define EVENT_NAME_LINK_KNOWN_DLL L"link known dll" //L"建立KnownDlls链接"
#define EVENT_NAME_OPEN_PHYSICAL_MEM L"open physical mem" //L"打开物理内存设备"
#define EVENT_NAME_READ_PHYSICAL_MEM L"read physical mem" //L"读物理内存"
#define EVENT_NAME_WRITE_PHYSICAL_MEM L"write physical mem" //L"写物理内存"
#define EVENT_NAME_LOAD_KERNLE_MODULE L"load kernel module" //L"加载内核模块"
#define EVENT_NAME_LOAD_MODULE L"load module" //L"加载模块"
#define EVENT_NAME_UNLOAD_MODULE L"unload module" //L"卸载模块"
#define EVENT_NAME_ENUM_PROCESS L"enum process" //L"枚举进程"
#define EVENT_NAME_REG_SERVICE L"register service" //L"注册服务"
#define EVENT_NAME_OPEN_DEVICE L"open device" //L"打开设备"
#define EVENT_NAME_NET_CREATE L"socket create" //L"生成套接字"
#define EVENT_NAME_NET_CONNECT L"socket connect" //L"网络连接"
#define EVENT_NAME_NET_LISTEN L"socket listen" //L"监听端口"
#define EVENT_NAME_NET_SEND L"socket send" //L"发送数据包"
#define EVENT_NAME_NET_RECEIVE L"socket receive" //L"接收数据包"
#define EVENT_NAME_NET_ACCEPT L"socket accept" //L"接收TCP连接"
#define EVENT_NAME_NET_PARSE_DNS L"DNS request" //L"域名解析(DNS)请求"
#define EVENT_NAME_NET_HTTP L"HTTP request" //L"HTTP请求"
#define EVENT_NAME_NET_ICMP_SEND L"ICMP send" //L"发送ICMP包"
#define EVENT_NAME_NET_ICMP_RECEIVE L"ICMP receive" //L"接收ICMP包"

#define EVENT_NAME_OTERH L"Other event" //L"其它" 

#else
#define EVENT_NAME_OTHER L"其它"
#define EVENT_NAME_EXEC_CREATE L"进程启动"
#define EVENT_NAME_EXEC_DESTROY L"进程退出"
#define EVENT_NAME_MODULE_LOAD L"模块加载";
#define EVENT_NAME_FILE_TOUCH L"修改文件属性" 
#define EVENT_NAME_FILE_OPEN L"打开文件"
#define EVENT_NAME_FILE_READ L"读取文件"
#define EVENT_NAME_FILE_WRITE L"读取文件"
#define EVENT_NAME_FILE_MODIFIED L"文件被修改"
#define EVENT_NAME_TRAVERSE_DIRECTORY L"遍历目录"
#define EVENT_NAME_REMOVE_FILE L"删除文件"
#define EVENT_NAME_RENAME_FILE L"重命名文件"
#define EVENT_NAME_TRUNCATE_FILE L"截断文件"
#define EVENT_NAME_MAKE_LINK L"建立文件硬链接"
#define EVENT_NAME_CHANGE_MODE L"设置文件属性"
#define EVENT_NAME_SET_SECURITY_INFO L"设置文件安全属性"
#define EVENT_NAME_GET_INFO L"查询文件相关信息"
#define EVENT_NAME_SET_INFO L"设置文件相关信息"
#define EVENT_NAME_FILE_CLOSE L"关闭文件"

#define EVENT_NAME_REG_OPEN_KEY L"打开注册表键"
#define EVENT_NAME_MAKE_KEY L"创建注册表键"
#define EVENT_NAME_REMOVE_KEY L"删除注册表键"
#define EVENT_NAME_MOVE_KEY L"重命名注册表键"
#define EVENT_NAME_GET_KEY_INFO  L"获取注册表键信息"
#define EVENT_NAME_SET_KEY_INFO L"设置注册表键信息"
#define EVENT_NAME_ENUM_KEY_INFO L"枚举注册表子键"
#define EVENT_NAME_ENUM_KEY_VALUE L"枚举注册表键值"
#define EVENT_NAME_REMOVE_KEY_VALUE L"删除注册表键"
#define EVENT_NAME_GET_KEY_VALUE L"获取注册表值"
#define EVENT_NAME_SET_KEY_VALUE L"设置注册表值"
#define EVENT_NAME_LOAD_KEY_HIVE L"挂载注册表Hive文件"
#define EVENT_NAME_REPLACE_KEY L"替换注册表键"
#define EVENT_NAME_RESTORE_KEY L"导入注册表Hive文件"
#define EVENT_NAME_SET_KEY_SECURITY L"设置注册表键安全属性"
#define EVENT_NAME_CLOSE_KEY L"关闭注册表键"

#define EVENT_NAME_PROCESS_EXEC L"创建进程"
#define EVENT_NAME_PROCESS_OPEN L"打开进程"
#define EVENT_NAME_PROCESS_DEBUG L"调试进程"
#define EVENT_NAME_PROCESS_SUSPEND L"挂起进程"
#define EVENT_NAME_PROCESS_RESUME L"恢复进程"
#define EVENT_NAME_PROCESS_EXIT L"进程退出"
#define EVENT_NAME_PROCESS_JOB L"将进程加入工作集"
#define EVENT_NAME_PROCESS_PAGE_PROTECT L"跨进程修改内存属性"
#define EVENT_NAME_PROCESS_FREE_VM L"跨进程释放内存"
#define EVENT_NAME_PROCESS_WRITE_VM L"跨进程写内存"
#define EVENT_NAME_PROCESS_READ_VM L"跨进程读内存"
#define EVENT_NAME_THREAD_REMOTE L"创建远程线程"
#define EVENT_NAME_THREAD_CREATE L"创建线程"
#define EVENT_NAME_THREAD_SET_CONTEXT L"跨进程设置线程上下文"
#define EVENT_NAME_THREAD_SUSPEND L"跨进程挂起线程"
#define EVENT_NAME_THREAD_RESUME L"跨进程恢复线程"
#define EVENT_NAME_THREAD_EXIT L"线程退出"
#define EVENT_NAME_THREAD_QUEUE_APC L"跨进程排队APC"
#define EVENT_NAME_LOAD_COM L"加载COM组件"
#define EVENT_NAME_URB_IO L"URB通信"
#define EVENT_NAME_SET_TIME L"设置系统时间"
#define EVENT_NAME_LINK_KNOWN_DLL L"建立KnownDlls链接"
#define EVENT_NAME_OPEN_PHYSICAL_MEM L"打开物理内存设备"
#define EVENT_NAME_READ_PHYSICAL_MEM L"读物理内存"
#define EVENT_NAME_WRITE_PHYSICAL_MEM L"写物理内存"
#define EVENT_NAME_LOAD_KERNLE_MODULE L"加载内核模块"
#define EVENT_NAME_LOAD_MODULE L"加载模块"
#define EVENT_NAME_UNLOAD_MODULE L"卸载模块"
#define EVENT_NAME_ENUM_PROCESS L"枚举进程"
#define EVENT_NAME_REG_SERVICE L"注册服务"
#define EVENT_NAME_OPEN_DEVICE L"打开设备"
#define EVENT_NAME_NET_CREATE L"生成套接字"
#define EVENT_NAME_NET_CONNECT L"网络连接"
#define EVENT_NAME_NET_LISTEN L"监听端口"
#define EVENT_NAME_NET_SEND L"发送数据包"
#define EVENT_NAME_NET_RECEIVE L"接收数据包"
#define EVENT_NAME_NET_ACCEPT L"接收TCP连接"
#define EVENT_NAME_NET_PARSE_DNS L"域名解析(DNS)请求"
#define EVENT_NAME_NET_HTTP L"HTTP请求"
#define EVENT_NAME_NET_ICMP_SEND L"发送ICMP包"
#define EVENT_NAME_NET_ICMP_RECEIVE L"接收ICMP包"

#define EVENT_NAME_OTERH L"其它" 
#endif //LANG_EN
LPCWSTR WINAPI get_event_name( sys_action_type type )
{
	LPCWSTR event_name = EVENT_NAME_OTHER; 

	switch( type )
	{
	case EXEC_create: //进程启动 进程路径名 （执行监控）
		event_name = EVENT_NAME_EXEC_CREATE; 
		break; 

	case EXEC_destroy: //进程退出 进程路径名 
			event_name = EVENT_NAME_EXEC_DESTROY; 
			break; 

	case EXEC_module_load: 
		event_name = EVENT_NAME_MODULE_LOAD; 
		break; //模块加载 模块路径名 

		//MT_filemon: break 
	case FILE_touch: 
		event_name = EVENT_NAME_FILE_TOUCH; 
		break;  //创建文件 文件全路径 （文件监控） 
	case FILE_open: event_name = EVENT_NAME_FILE_OPEN; break;  //打开文件 文件全路径 
	case FILE_read: event_name = EVENT_NAME_FILE_READ; break;  //读取文件 文件全路径 
	case FILE_write: event_name = EVENT_NAME_FILE_WRITE;  break;  //写入文件 文件全路径 
	case FILE_modified: event_name = EVENT_NAME_FILE_MODIFIED;  break;  //文件被修改 文件全路径 
	case FILE_readdir: event_name = EVENT_NAME_TRAVERSE_DIRECTORY;  break;  //遍历目录 目录全路径 
	case FILE_remove: event_name = EVENT_NAME_REMOVE_FILE;  break;  //删除文件 文件全路径 
	case FILE_rename: event_name = EVENT_NAME_RENAME_FILE;  break;  //重命名文件 文件全路径 
	case FILE_truncate: event_name = EVENT_NAME_TRUNCATE_FILE;  break;  //截断文件 文件全路径 
	case FILE_mklink: event_name = EVENT_NAME_MAKE_LINK;  break;  //建立文件硬链接 文件全路径 
	case FILE_chmod: event_name = EVENT_NAME_CHANGE_MODE;  break;  //设置文件属性 文件全路径 
	case FILE_setsec: event_name = EVENT_NAME_SET_SECURITY_INFO;  break;  //设置文件安全属性 文件全路径 
	case FILE_getinfo: event_name = EVENT_NAME_GET_INFO;  break;  //设置文件安全属性 文件全路径 
	case FILE_setinfo: event_name = EVENT_NAME_SET_INFO;  break;  //设置文件安全属性 文件全路径 
	case FILE_close: event_name = EVENT_NAME_FILE_CLOSE;  break;  //设置文件安全属性 文件全路径 

		//MT_regmon: event_name = L"";  break;  
	case REG_openkey: event_name = EVENT_NAME_REG_OPEN_KEY;  break;  //打开注册表键 注册表键路径  （注册表监控） 
	case REG_mkkey: event_name = EVENT_NAME_MAKE_KEY;  break;  //创建注册表键 注册表键路径 
	case REG_rmkey: event_name = EVENT_NAME_REMOVE_KEY;  break;  //删除注册表键 注册表键路径 
	case REG_mvkey: event_name = EVENT_NAME_MOVE_KEY;  break;  //重命名注册表键 注册表键路径 
	case REG_getinfo: event_name = EVENT_NAME_GET_KEY_INFO;  break;  
	case REG_setinfo: event_name = EVENT_NAME_SET_KEY_INFO;  break;  
	case REG_enuminfo: event_name = EVENT_NAME_ENUM_KEY_INFO;  break;  
	case REG_enum_val: event_name = EVENT_NAME_ENUM_KEY_VALUE;  break;  
	case REG_rmval: event_name = EVENT_NAME_REMOVE_KEY_VALUE;  break;  //删除注册表键 注册表键路径 
	case REG_getval: event_name = EVENT_NAME_GET_KEY_VALUE;  break;  //获取注册表值 注册表值路径 
	case REG_setval: event_name = EVENT_NAME_SET_KEY_VALUE;  break;  //设置注册表值 注册表值路径 
	case REG_loadkey: event_name = EVENT_NAME_LOAD_KEY_HIVE;  break;  //挂载注册表Hive文件 注册表键路径 
	case REG_replkey: event_name = EVENT_NAME_REPLACE_KEY;  break;  //替换注册表键 注册表键路径 
	case REG_rstrkey: event_name = EVENT_NAME_RESTORE_KEY;  break;  //导入注册表Hive文件 注册表键路径 
	case REG_setsec: event_name = EVENT_NAME_SET_KEY_SECURITY;  break;  //设置注册表键安全属性 注册表键路径 
	case REG_closekey: event_name = EVENT_NAME_CLOSE_KEY;  break;  //设置注册表键安全属性 注册表键路径 

		//MT_procmon: event_name = L"";  break;  
	case PROC_exec: event_name = EVENT_NAME_PROCESS_EXEC;  break;  //创建进程 目标进程路径名  （进程监控）
	case PROC_open: event_name = EVENT_NAME_PROCESS_OPEN;  break;  //打开进程 目标进程路径名 
	case PROC_debug: event_name = EVENT_NAME_PROCESS_DEBUG;  break;  //调试进程 目标进程路径名 
	case PROC_suspend: event_name = EVENT_NAME_PROCESS_SUSPEND;  break;  //挂起进程 目标进程路径名 
	case PROC_resume: event_name = EVENT_NAME_PROCESS_RESUME;  break;  //恢复进程 目标进程路径名 
	case PROC_exit: event_name = EVENT_NAME_PROCESS_EXIT;  break;  //结束进程 目标进程路径名 
	case PROC_job: event_name = EVENT_NAME_PROCESS_JOB;  break;  //将进程加入工作集 目标进程路径名 
	case PROC_pgprot: event_name = EVENT_NAME_PROCESS_PAGE_PROTECT;  break;  //跨进程修改内存属性 目标进程路径名 
	case PROC_freevm: event_name = EVENT_NAME_PROCESS_FREE_VM;  break;  //跨进程释放内存 目标进程路径名 
	case PROC_writevm: event_name = EVENT_NAME_PROCESS_WRITE_VM;  break;  //跨进程写内存 目标进程路径名 
	case PROC_readvm: event_name = EVENT_NAME_PROCESS_READ_VM;  break;  //跨进程读内存 目标进程路径名 
	case THRD_remote: event_name = EVENT_NAME_THREAD_REMOTE;  break;  //创建远程线程 目标进程路径名 
	case THRD_create: event_name = EVENT_NAME_THREAD_CREATE;  break;  //创建线程
	case THRD_setctxt: event_name = EVENT_NAME_THREAD_SET_CONTEXT;  break;  //跨进程设置线程上下文 目标进程路径名 
	case THRD_suspend: event_name = EVENT_NAME_THREAD_SUSPEND;  break;  //跨进程挂起线程 目标进程路径名 
	case THRD_resume: event_name = EVENT_NAME_THREAD_RESUME;  break;  //跨进程恢复线程 目标进程路径名 
	case THRD_exit: event_name = EVENT_NAME_THREAD_EXIT;  break;  //跨进程结束线程 目标进程路径名 
	//case THRD_exit: event_name = L"跨进程结束线程";  break;  //跨进程结束线程 目标进程路径名 
	case THRD_queue_apc: event_name = EVENT_NAME_THREAD_QUEUE_APC;  break;  //跨进程排队APC 目标进程路径名 

		//MT_common
	case COM_access: event_name = EVENT_NAME_LOAD_COM;  break;  
	//case PORT_read: event_name = L"读取串口"; break; 
	//case PORT_write: event_name = L"写入串口"; break; 
	case PORT_urb: event_name = EVENT_NAME_URB_IO; break; 

		//MT_sysmon
	case SYS_settime: event_name = EVENT_NAME_SET_TIME;  break;  //设置系统时间 无 
	case SYS_link_knowndll: event_name = EVENT_NAME_LINK_KNOWN_DLL;  break;  //建立KnownDlls链接 链接文件名 
	case SYS_open_physmm: event_name = EVENT_NAME_OPEN_PHYSICAL_MEM;  break;  //打开物理内存设备 无 
	case SYS_read_physmm: event_name = EVENT_NAME_READ_PHYSICAL_MEM;  break;  //读物理内存 无 
	case SYS_write_physmm: event_name = EVENT_NAME_WRITE_PHYSICAL_MEM;  break;  //写物理内存 无 
	case SYS_load_kmod: event_name = EVENT_NAME_LOAD_KERNLE_MODULE;  break;  //加载内核模块 内核模块全路径 
	case SYS_load_mod: event_name = EVENT_NAME_LOAD_MODULE;  break;  //加载模块 内核模块全路径 
	case SYS_unload_mod: event_name = EVENT_NAME_UNLOAD_MODULE;  break;  //卸载模块 内核模块全路径 
	case SYS_enumproc: event_name = EVENT_NAME_ENUM_PROCESS;  break;  //枚举进程 无 
	case SYS_regsrv: event_name = EVENT_NAME_REG_SERVICE;  break;  //注册服务 服务进程全路径 
	case SYS_opendev: event_name = EVENT_NAME_OPEN_DEVICE;  break;  //打开设备 设备名 

		//MT_w32mon
	case W32_postmsg: event_name = L"发送窗口消息(Post)";  break;  //发送窗口消息（Post） 目标进程路径名 
	case W32_sendmsg: event_name = L"发送窗口消息(Send)";  break;  //发送窗口消息（Send） 目标进程路径名 
	case W32_findwnd: event_name = L"查找窗口";  break;  //查找窗口 无 
	case W32_msghook: event_name = L"设置消息钩子";  break;  //设置消息钩子 无 
	case W32_lib_inject: event_name = L"DLL注入";  break;  //DLL注入 注入DLL路径名 

		//MT_netmon: event_name = L"";  break;  
	case NET_create: event_name = EVENT_NAME_NET_CREATE;  break;  
	case NET_connect: event_name = EVENT_NAME_NET_CONNECT;  break;  //网络连接 远程地址（格式：IP：端口号） （网络监控） 
	case NET_listen: event_name = EVENT_NAME_NET_LISTEN;  break;  //监听端口 本机地址（格式：IP：端口号） 
	case NET_send: event_name = EVENT_NAME_NET_SEND;  break;  //发送数据包 远程地址（格式：IP：端口号） 
	case NET_recv: event_name = EVENT_NAME_NET_RECEIVE;  break;  
	case NET_accept: event_name = EVENT_NAME_NET_ACCEPT;  break;  
	case NET_dns: event_name = EVENT_NAME_NET_PARSE_DNS;  break;  
	case NET_http: event_name = EVENT_NAME_NET_HTTP;  break;  //HTTP请求 HTTP请求路径（格式：域名/URL） 
	case NET_icmp_send: event_name = EVENT_NAME_NET_ICMP_SEND;  break;  
	case NET_icmp_recv: event_name = EVENT_NAME_NET_ICMP_RECEIVE;  break;  

		//MT_behavior: event_name = L"";  break;  
	case BA_extract_hidden: event_name = L"释放隐藏文件";  break;  //释放隐藏文件 释放文件路径名 （行为监控） 
	case BA_extract_pe: event_name = L"释放PE文件";  break;  //释放PE文件 释放文件路径名 
	case BA_self_copy: event_name = L"自我复制";  break;  //自我复制 复制目标文件路径名 
	case BA_self_delete: event_name = L"自我删除";  break;  //自我删除 删除文件路径名 
	case BA_ulterior_exec: event_name = L"隐秘执行";  break;  //隐秘执行 被执行映像路径名 
	case BA_invade_process: event_name = L"入侵进程";  break;  //入侵进程 目标进程路径名 
	case BA_infect_pe: event_name = L"感染PE文件";  break;  //感染PE文件 目标文件路径名 
	case BA_overwrite_pe: event_name = L"覆写PE文件";  break;  //覆写PE文件 目标文件路径名 
	case BA_register_autorun: event_name = L"注册自启动项";  break;  //注册自启动项 自启动文件路径名 
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