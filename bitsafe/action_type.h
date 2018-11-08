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

#ifndef __ACTION_TYPE_H__
#define __ACTION_TYPE_H__

#include "action_type_common.h"

/*************************************************************************
PVOID类型与ULONG，ULONGLONG类型关系：
1.PVOID类型的长度是由于编译器编译时定的，但运行时必须有相应
WINDOWS系统位数与硬件平台的支持。
2.所以当驱动与应用编译的位数是一样的，这时对内存和ULONGLONG的处理最高效的。
3.而当驱动与应用的编译的位数是不一样的，就需要一种相应的约定好的转换方式，
或者都定义为兼容性最好的ULONGLONG型。
*************************************************************************/

typedef ULONGLONG POINTER_TYPE; 
typedef enum _action_main_type
{
	MT_execmon = 0x00000001, 
	MT_filemon = 0x00000002, 
	MT_regmon = 0x00000004, 
	MT_procmon = 0x00000008, 
	MT_sysmon = 0x00000010, 
	MT_w32mon = 0x00000020, 
	MT_netmon = 0x00000040, 
	MT_behavior = 0x00000080, 
	MT_common = 0x00000100, 
	MT_peripheral = 0x00000200, 
	MT_unknown = 0x00000400, 
} action_main_type, *paction_main_type; 

typedef enum _sys_action_type
{
	//MT_execmon
	EXEC_create,  //进程启动 进程路径名 （执行监控） 
	EXEC_destroy, //进程退出 进程路径名 
	EXEC_module_load, //模块加载 模块路径名 
	
	//MT_filemon, 
	FILE_touch, //创建文件 文件全路径 （文件监控） 
	FILE_open, //打开文件 文件全路径 
	FILE_read, //读取文件 文件全路径 
	FILE_write, //写入文件 文件全路径 
	FILE_modified, //文件被修改 文件全路径 
	FILE_readdir, //遍历目录 目录全路径 
	FILE_remove, //删除文件 文件全路径 
	FILE_rename, //重命名文件 文件全路径 
	FILE_truncate, //截断文件 文件全路径 
	FILE_mklink, //建立文件硬链接 文件全路径 
	FILE_chmod, //设置文件属性 文件全路径 
	FILE_setsec, //设置文件安全属性 文件全路径 
	FILE_getinfo, 
	FILE_setinfo, 
	FILE_close, 
	
	//MT_regmon, 
	REG_openkey, //打开注册表键 注册表键路径  （注册表监控） 
	REG_mkkey, //创建注册表键 注册表键路径 
	REG_rmkey, //删除注册表键 注册表键路径 
	REG_mvkey, //重命名注册表键 注册表键路径 
	REG_getinfo, 
	REG_setinfo, 
	REG_enuminfo, 
	REG_enum_val, 
	REG_rmval, //删除注册表键 注册表键路径 
	REG_getval, //获取注册表值 注册表值路径 
	REG_setval, //设置注册表值 注册表值路径 
	REG_loadkey, //挂载注册表Hive文件 注册表键路径 
	REG_replkey, //替换注册表键 注册表键路径 
	REG_rstrkey, //导入注册表Hive文件 注册表键路径 
	REG_setsec, //设置注册表键安全属性 注册表键路径 
	REG_closekey, 
	
	//MT_procmon, 
	PROC_exec, //创建进程 目标进程路径名  （进程监控）
	PROC_open, //打开进程 目标进程路径名 
	PROC_debug, //调试进程 目标进程路径名 
	PROC_suspend, //挂起进程 目标进程路径名 
	PROC_resume, //恢复进程 目标进程路径名 
	PROC_kill, 
	PROC_exit, //结束进程 目标进程路径名 
	PROC_job, //将进程加入工作集 目标进程路径名 
	PROC_pgprot, //跨进程修改内存属性 目标进程路径名 
	PROC_freevm, //跨进程释放内存 目标进程路径名 
	PROC_writevm, //跨进程写内存 目标进程路径名 
	PROC_readvm, //跨进程读内存 目标进程路径名 
	THRD_remote, //创建远程线程 目标进程路径名 
	THRD_create, //创建线程
	THRD_setctxt, //跨进程设置线程上下文 目标进程路径名 
	THRD_suspend, //跨进程挂起线程 目标进程路径名 
	THRD_resume, //跨进程恢复线程 目标进程路径名 
	THRD_kill, //跨进程结束线程 目标进程路径名 
	THRD_exit, 
	THRD_queue_apc, //跨进程排队APC 目标进程路径名 

	//MT_common
	COM_access, 
	PORT_read, 
	PORT_write, 
	PORT_urb, 

	//MT_sysmon
	SYS_settime, //设置系统时间 无 
	SYS_link_knowndll, //建立KnownDlls链接 链接文件名 
	SYS_open_physmm, //打开物理内存设备 无 
	SYS_read_physmm, //读物理内存 无 
	SYS_write_physmm, //写物理内存 无 
	SYS_load_kmod, //加载内核模块 内核模块全路径 
	SYS_load_mod, //加载模块 内核模块全路径 
	SYS_unload_mod, //卸载模块 内核模块全路径 
	SYS_enumproc, //枚举进程 无 
	SYS_regsrv, //注册服务 服务进程全路径 
	SYS_opendev, //打开设备 设备名 

	//MT_w32mon
	W32_postmsg, //发送窗口消息（Post） 目标进程路径名 
	W32_sendmsg, //发送窗口消息（Send） 目标进程路径名 
	W32_findwnd, //查找窗口 无 
	W32_msghook, //设置消息钩子 无 
	W32_lib_inject, //DLL注入 注入DLL路径名 
	
	//MT_netmon, 
	NET_create, 
	NET_connect, //网络连接 远程地址（格式：IP：端口号） （网络监控） 
	NET_listen, //监听端口 本机地址（格式：IP：端口号） 
	NET_send, //发送数据包 远程地址（格式：IP：端口号） 
	NET_recv, 
	NET_accept, 
	
	NET_dns, 
	NET_http, //HTTP请求 HTTP请求路径（格式：域名/URL） 
	NET_icmp_send, 
	NET_icmp_recv, 
	
	//MT_behavior, 
	BA_extract_hidden, //释放隐藏文件 释放文件路径名 （行为监控） 
	BA_extract_pe, //释放PE文件 释放文件路径名 
	BA_self_copy, //自我复制 复制目标文件路径名 
	BA_self_delete, //自我删除 删除文件路径名 
	BA_ulterior_exec, //隐秘执行 被执行映像路径名 
	BA_invade_process, //入侵进程 目标进程路径名 
	BA_infect_pe, //感染PE文件 目标文件路径名 
	BA_overwrite_pe, //覆写PE文件 目标文件路径名 
	BA_register_autorun, //注册自启动项 自启动文件路径名 
	BA_other, 
	OTHER_ACTION = BA_other, 
	MAX_SYS_ACTION_TYPE, 
	INVALID_SYS_ACTION_TYPE, 
	SYS_ACTION_NONE = INVALID_SYS_ACTION_TYPE, 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
	SYS_ACTION_BEGIN = ACCESS_FILE, 
#else
	SYS_ACTION_BEGIN = EXEC_create, 
#endif //COMPATIBLE_OLD_ACTION_DEFINE

	SYS_ACTION_END = BA_other, 

	//CHECK_NAME_ACTION_BEGIN = OTHER_ACTION, 
	//CHECK_NAME_ACTION_END = LOCATE_URL, 

	ICMP_ACTION_BEGIN = NET_icmp_send, 
	ICMP_ACTION_END = NET_icmp_recv, 

	SOCKET_ACTION_BEGIN = NET_create, 
	SOCKET_ACTION_END = NET_accept, 

	NETWORK_ACTION_BEGIN = NET_create, 
	NETWORK_ACTION_END = NET_icmp_recv, 
} sys_action_type, *psys_action_type; 

#define is_valid_file_action_type( type ) ( type >= FILE_touch && type <= FILE_setsec )
#define is_valid_action_type( type ) ( type >= SYS_ACTION_BEGIN && type <= SYS_ACTION_END )

#define is_socket_action( type ) ( type >= SOCKET_ACTION_BEGIN && type <= SOCKET_ACTION_END )

#define is_icmp_action( type ) ( type >= ICMP_ACTION_BEGIN && type <= ICMP_ACTION_END )
#define is_network_action( type ) ( type >= NETWORK_ACTION_BEGIN && type <= NETWORK_ACTION_END )

#define is_name_action( type ) ( FALSE == is_network_action( type ) )
#define is_common_action( type ) ( FALSE == is_network_action( type ) )

#pragma pack( push )
#pragma pack( 1 )

/*********************************************************************************
action description structures
*********************************************************************************/

typedef USHORT PATH_SIZE_T; 

//动作 路径过滤器 参数 MT_execmon 
typedef struct _exec_create //(执行监控) 
{
	ULONG pid; // 进程ID 
	ULONG parent_pid; // 父进程ID 
	POINTER_TYPE image_base; //process virtual address space // 主映像基址 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T cmd_len; 
	union 
	{
		WCHAR path_name[ 1 ]; //进程全路径 
		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *cmd_line_ptr; 
		};
	};
	//WCHAR cmdline[ 0 ]; //follow the path name// 进程命令行 
} exec_create, *pexec_create; 

typedef struct _exec_destroy
{
	ULONG pid; 
	ULONG parent_pid; 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T cmd_len; 
	
	union
	{
		WCHAR path_name[ 1 ]; //进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *cmd_line_ptr; 
		};
	};
	//WCHAR cmd_line[ 0 ]; //follow the path name //进程命令行 
} exec_destroy, *pexec_destroy; 

#define LOAD_RING3_MODULE 0x00000002
#define LOAD_RING0_MODULE 0x00000001

typedef struct exec_module_load 
{
	//ULONG pid; //进程ID 

	ULONG64 base; //模块基址 
	ULONG64 size; // 模块大小 
	ULONG time_stamp; 
	ULONG flags; 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
}exec_module_load, *pexec_module_load; 

typedef struct _file_open 
{
	ULONG access; //文件访问权限 
	ULONG alloc_size; //文件初始长度 
	ULONG attrib; //文件属性 
	ULONG share; //文件共享属性 
	ULONG disposition; //文件打开/创建选项 
	ULONG options; //文件打开/创建选项 
	//ULONG result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	}; 
	//WCHAR path_name[ 1 ]; //文件全路径 
} file_open, *pfile_open; 

typedef file_open file_touch; 

typedef struct _file_read
{
	ULONGLONG offset; //文件内偏移 
	ULONG data_len; //数据长度 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} _file_read, *pfile_read; 

typedef _file_read _file_write; 

typedef enum _PERIPHERAL_PORT_TYPE
{
	PORT_COM_TYPE, 
	PORT_PIPE_TYPE, 
	PORT_MAILSLOT_TYPE, 
	PORT_USB_TYPE, 
	MAX_PORT_TYPE, 
} PERIPHERAL_PORT_TYPE, *PPERIPHERAL_PORT_TYPE; 

typedef struct _port_read
{
	PATH_SIZE_T path_len; 
	BYTE port_type; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} port_read, *pport_read; 

typedef port_read port_write; 
typedef port_read port_urb; 

typedef struct _file_modified
{

	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} file_modified, *pfile_modified; 

typedef struct _file_readdir
{
	ULONG filter; //文件遍历过滤条件 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} file_readdir, *pfile_readdir; 


typedef file_modified file_remove, *pfile_remove; 

typedef struct _file_rename
{

	BOOLEAN replace_existing; //是否覆盖已存在文件 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T new_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *new_name_ptr; 
		};
	};
} file_rename, *pfile_rename; 

typedef struct _file_truncate
{ 

	ULONGLONG eof; //截断后的文件长度  
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} file_truncate, *pfile_truncate; 

typedef struct _file_mklink
{
	ULONG flags; //like replace existing; //是否覆盖已存在文件  
	PATH_SIZE_T path_len; 
	PATH_SIZE_T link_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *link_name_ptr; 
		};
	};
} file_mklink, *pfile_mklink; 

typedef struct _file_chmod
{
	ULONG attrib; //文件属性 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} file_chmod, *pfile_chmod; 


typedef file_modified file_setsec, *pfile_setsec; 

typedef struct _file_getinfo
{
	//NTSTATUS result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 
	ULONG info_class; 
	ULONG info_size; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		}; 
	}; 
} file_getinfo, *pfile_getinfo; 

typedef file_getinfo file_setinfo; 

typedef struct _file_close
{
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	}; 
} file_close, *pfile_close; 

typedef struct _reg_openkey
{ 
	ULONG access; //注册表打开/创建权限 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} reg_openkey, *preg_openkey; 

typedef struct _reg_closekey
{ 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} reg_closekey, *preg_closekey; 

typedef struct _reg_mkkey 
{
	ULONG access; //注册表打开/创建权限  
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表键路径

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} reg_mkkey, *preg_mkkey; 

typedef struct _reg_rmkey
{
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表键路径

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} reg_rmkey, *preg_rmkey; 

typedef struct _reg_mvkey
{ 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T new_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表键路径

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *new_key_name_ptr; 
		};
	};
} reg_mvkey, *preg_mvkey; 

typedef struct _reg_rmval
{
	PATH_SIZE_T path_len; 
	PATH_SIZE_T val_len; 

	union
	{
		WCHAR path_name[ 1 ]; //注册表键路径

		struct 
		{
			WCHAR *key_name_ptr; 
			WCHAR *val_name_ptr; 
		};
	};

} reg_rmval, *preg_rmval; 

#define INVALID_KEY_INFORMATION_CLASS ( ULONG )( -1 ) 

typedef struct _reg_getinfo
{
	PATH_SIZE_T path_len; 
	ULONG info_class; 
	ULONG info_size; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		}; 
	}; 
} reg_getinfo, *preg_getinfo; 

typedef reg_getinfo reg_setinfo; 

typedef struct _reg_enum_val
{
	ULONG index; 
	PATH_SIZE_T path_len; 
	ULONG info_class; 
	ULONG info_size; 

	union
	{
		WCHAR path_name[ 1 ]; //文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		}; 
	}; 
} reg_enum_val, *preg_enum_val; 

typedef reg_enum_val reg_enuminfo, *preg_enuminfo; 

typedef struct _reg_setval
{
	ULONG type; //注册表值类型 
	ULONG data_len; //数据长度 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T val_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *key_name_ptr; 
			WCHAR *value_name_ptr; 
		};
	};
} reg_setval, *preg_setval; 

typedef struct _reg_getval
{
	ULONG type; //注册表值类型 
	ULONG info_size; //数据长度 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T val_name_len; 

	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *key_name_ptr; 
			WCHAR *value_name_ptr; 
		};
	};
} reg_getval, *preg_getval; 

typedef struct reg_loadkey
{
	PATH_SIZE_T path_len; 
	PATH_SIZE_T hive_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *key_name_ptr; 
			WCHAR *hive_name_ptr; 
		};
	};
} reg_loadkey, *preg_loadkey; 

typedef struct _reg_replkey
{
	PATH_SIZE_T path_len; 
	PATH_SIZE_T hive_name_len; 
	PATH_SIZE_T new_hive_name_len; 

	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *key_name_ptr; 
			WCHAR *hive_name_ptr; 
			WCHAR *new_hive_name_ptr; 
		};
	};
} reg_replkey, *preg_replkey; 

typedef reg_loadkey reg_rstrkey; 

typedef struct _reg_setsec
{
	PATH_SIZE_T path_len; 
	//WCHAR path_name[ 1 ]; //注册表键路径名 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *key_name_ptr; 
		};
	};
} reg_setsec, *preg_setsec; 


//_MT_procmon (进程监控)  动作 路径过滤器 参数 
typedef struct _proc_exec
{
	ULONG target_pid; //目标进程ID 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_exec, *pproc_exec;  

typedef struct _proc_open
{
	ULONG target_pid; //目标进程ID 
	ULONG access; //进程打开权限 
	PATH_SIZE_T path_len; 
	
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};

} proc_open, *pproc_open; 

typedef struct _com_access
{
	ULONG target_pid; //目标进程ID 
	ULONG access; //进程打开权限 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} com_access, *pcom_access; 

typedef struct _proc_debug
{
	ULONG target_pid; //目标进程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_debug, *pproc_debug; 

typedef struct _proc_suspend
{
	ULONG target_pid; //目标进程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_suspend, *pproc_suspend; 

typedef proc_suspend proc_resume, *pproc_resume; 

typedef struct _proc_kill
{
	ULONG target_pid; //目标进程ID 
	ULONG exitcode; //进程推出码 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_kill, *pproc_kill; 

typedef proc_kill proc_exit, *pproc_exit; 

typedef struct _proc_job
{ 
	ULONG target_pid; //目标进程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_job, *pproc_job; 

typedef struct _proc_pgprot 
{ 
	ULONG target_pid; //目标进程ID 
	POINTER_TYPE base; //修改内存基址 
	ULONG count; //修改内存长度 
	ACCESS_MASK attrib; //修改内存属性 
	ULONG bytes_changed; //成功修改长度 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_pgprot, *pproc_pgprot; 

typedef struct _proc_freevm 
{ 
	ULONG target_pid; //目标进程ID 
	POINTER_TYPE base; //释放内存基址 
	ULONG count; //释放内存长度 
	ULONG type; //释放内存类型 
	ULONG bytes_freed; //成功释放长度 
	//NTSTATUS result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_freevm, *pproc_freevm; 

typedef struct _proc_writevm 
{ 
	ULONG target_pid; //目标进程ID 
	POINTER_TYPE base; //写入内存基址 
	ULONG data_len; //试图写入长度 
	ULONG bytes_written; //成功写入长度 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_writevm, *pproc_writevm; 

typedef struct _proc_readvm 
{ 
	ULONG target_pid; //目标进程ID 
	POINTER_TYPE base; //读取内存基址 
	ULONG data_len; //试图读取长度 
	ULONG bytes_read; //成功读取长度 
	NTSTATUS result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} proc_readvm, *pproc_readvm; 

typedef struct _thrd_remote 
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	ACCESS_MASK access; //线程创建权限 
	BOOLEAN suspended; //是否挂起方式创建 
	POINTER_TYPE start_vaddr; //线程起始地址 
	POINTER_TYPE thread_param; //线程参数 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_remote, *pthrd_remote; 

typedef struct _thrd_create
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	ACCESS_MASK access; //线程创建权限 
	BOOLEAN suspended; //是否挂起方式创建 
	POINTER_TYPE start_vaddr; //线程起始地址 
	POINTER_TYPE thread_param; //线程参数 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_create, *pthrd_create; 

typedef struct _thrd_setctxt 
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_setctxt, *pthrd_setctxt; 

typedef struct _thrd_suspend 
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};

} thrd_suspend, *pthrd_suspend; 

typedef struct _thrd_resume 
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_resume, *pthrd_resume; 

typedef struct _thrd_kill 
{ 

	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	ULONG exitcode; //线程退出码 
	PATH_SIZE_T path_len; 
	
	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_kill, *pthrd_kill; 

typedef thrd_kill thrd_exit, *pthrd_exit; 

typedef struct _thrd_queue_apc 
{ 
	ULONG target_pid; //目标进程ID 
	ULONG target_tid; //目标线程ID 
	POINTER_TYPE apc_routine; 
	POINTER_TYPE arg1; 
	POINTER_TYPE arg2; 
	POINTER_TYPE arg3; 
	//do some memory dump for the above arguments ?

	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} thrd_queue_apc, *pthrd_queue_apc; 

typedef struct _sys_settime 
{ 
	ULONG time_delta; //设置时间与动作发起时间差 
} sys_settime, *psys_settime; 

typedef struct _sys_link_knowndll 
{ 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} sys_link_knowndll, *psys_link_knowndll; 

typedef struct _sys_open_physmm 
{
	ACCESS_MASK access; //打开权限 
} sys_open_physmm, *psys_open_physmm; 

typedef struct _sys_read_physmm 
{ 
	ULONG offset; //读取偏移 
	ULONG count; //读取长度 
} sys_read_physmm, *psys_read_physmm; 

typedef sys_read_physmm sys_write_physmm, *psys_write_physmm; 

typedef struct _sys_load_kmod 
{ 
	POINTER_TYPE base_addr; 
	ULONG64 size; 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
}sys_load_kmod, *psys_load_kmod; 

typedef struct _sys_load_mod 
{ 
	POINTER_TYPE base_addr; 
	ULONG64 size; 
	ULONG time_stamp; 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
}sys_load_mod, *psys_load_mod; 

typedef sys_load_mod sys_unload_mod; 

typedef sys_load_kmod sys_load_kmod_policy; 
typedef struct _sys_enumproc 
{ 
	NTSTATUS result; //动作完成结果(NTSTATUS) 
} sys_enumproc, *psys_enumproc; 

typedef struct _sys_regsrv 
{ 
	ACCESS_MASK access; //服务创建/打开权限 
	ULONG type; //服务类型 
	ULONG starttype; //服务启动类型 
	NTSTATUS result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 
	PATH_SIZE_T srv_name_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *srv_name_ptr; 
		};
	};
} sys_regsrv, *psys_regsrv; 

typedef struct _sys_opendev 
{ 
	ULONG devtype; //设备类型 
	ULONG access; //设备打开权限 
	ULONG share; //设备共享权限 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //设备路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} sys_opendev, *psys_opendev; 

#ifndef WIN32_DEFINED
//
// Handle to an Object
//

#ifdef STRICT
typedef void *HANDLE;

#ifndef DECLARE_HANDLE
#if 0 && (_MSC_VER > 1000)
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
#else
#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
#endif
#endif //DECLARE_HANDLE

#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif

typedef HANDLE *PHANDLE;

DECLARE_HANDLE(WND_HANDLE);
//typedef HANDLE HWND; 

#define HWND WND_HANDLE

#endif //WIN32_DEFINED


typedef ULONGLONG HANDLE_TYPE; 

typedef POINTER_TYPE WPARAM_TYPE; //UINT_PTR            
typedef POINTER_TYPE LPARAM_TYPE; //LONG_PTR 
typedef LONG_PTR LRESULT; //LONG_PTR 

typedef struct _w32_postmsg 
{ 
	ULONG target_pid; //目标进程ID 
	HANDLE_TYPE hwnd; //目标窗口句柄 
	ULONG msg; //窗口消息 
	WPARAM_TYPE wparam; //消息参数1 
	LPARAM_TYPE lparam; //消息参数2 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 
		LPCWSTR path_name_ptr; 
	};

} w32_postmsg, *pw32_postmsg; 

typedef w32_postmsg w32_sendmsg, *pw32_sendmsg; 

typedef struct _w32_findwnd 
{ 
	HANDLE_TYPE parent_hwnd; //父窗口句柄 
	HANDLE_TYPE sub_hwnd; //子窗口句柄 
	NTSTATUS result; //动作完成结果(HWND) 
	PATH_SIZE_T cls_name_len; 
	PATH_SIZE_T wnd_name_len; 
	union
	{
		WCHAR cls_name[ 1 ]; //窗口类名
		LPCWSTR cls_name_ptr; 
		LPCWSTR wnd_name_ptr; 		
	}; 
} w32_findwnd, *pw32_findwnd; 

typedef struct _w32_msghook 
{ 
	ULONG target_pid; //目标进程ID 
	POINTER_TYPE modbase; //钩子模块基址 
	POINTER_TYPE hookfunc; //钩子函数地址 
	ULONG hooktype; //钩子类型 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //注册表值路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};

} w32_msghook, *pw32_msghook; 

typedef struct _w32_lib_inject 
{ 
	ULONG target_pid; //目标进程ID 
	NTSTATUS result; //动作完成结果(NTSTATUS) 
	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
			WCHAR *lib_path_ptr; 
		};
	};

} w32_lib_inject, *pw32_lib_inject; 

//MT_netmon (网络监控) 动作 路径过滤器 参数 

typedef ULONG IP_ADDR_T; 
typedef USHORT PORT_T; 

typedef struct _net_connect 
{ 
	IP_ADDR_T local_ip_addr; 
	PORT_T local_port; 
	ULONG protocol; //协议 
	IP_ADDR_T ip_addr; 
	PORT_T port; 
} net_connect, *pnet_connect; 

typedef net_connect net_create; 
typedef net_connect net_listen; 

typedef struct _net_send 
{ 
	//LPCWSTR pathname; 

	IP_ADDR_T local_ip_addr; 
	PORT_T local_port; 
	ULONG protocol; //协议 
	ULONG data_len; //发送数据长度 
	IP_ADDR_T ip_addr;//远程IP:端口 
	PORT_T port; 
} net_send, *pnet_send; 

typedef net_send net_recv; 

typedef net_recv net_accept; 

typedef struct _net_http 
{ 
	ULONG protocol; //协议 
	ULONG cmd; //HTTP命令 
	ULONG data_len; //发送数据长度 

	PATH_SIZE_T path_len; 

	union
	{
		WCHAR path_name[ 1 ]; //URL 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} net_http, *pnet_http; 

typedef struct _net_dns 
{  
	ULONG protocol; //协议  
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //URL 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};
} net_dns, *pnet_dns; 

typedef struct _net_icmp_send
{
	ULONG src_ip; 
	ULONG dest_ip; 
	BYTE type; 
	BYTE code; 
} net_icmp_send, *pnet_icmp_send; 

typedef net_icmp_send net_icmp_recv; 

//MT_behavior (行为监控) 
//行为 路径过滤器 参数 

typedef struct _ba_extract_hidden
{
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //释放文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};

} ba_extract_hidden, *pba_extract_hidden; 

typedef ba_extract_hidden ba_extract_pe; 

typedef ba_extract_hidden ba_overwrite_pe; 

typedef ba_extract_hidden ba_self_copy; 

typedef ba_extract_hidden ba_self_delete; 

typedef ba_extract_hidden ba_ulterior_exec; 

typedef struct _ba_invade_process
{
	ULONG target_pid; //目标进程ID 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //目标进程全路径 
		LPCWSTR path_name_ptr; 
	}; 
} ba_invade_process, *pba_invade_process; 

typedef ba_extract_hidden ba_infect_pe; 

	  
typedef struct _ba_register_autorun
{
	ULONG type; //自启动类型 
	PATH_SIZE_T path_len; 
	union
	{
		WCHAR path_name[ 1 ]; //自启动文件全路径 

		struct 
		{
			WCHAR *path_name_ptr; 
		};
	};

} ba_register_autorun, *pba_register_autorun; 

typedef enum _prot_type
{
	INVALID_PORT_TYPE, 
	//PROT_ICMP, 
	PROT_TCP, 
	PROT_UDP, 
	PROT_IP, 
	ALL_PROT, 
	PROT_BEGIN = PROT_TCP, 
	PROT_END = ALL_PROT, 
	MAX_PROT_TYPE
} prot_type;

#define is_valid_prot_type( type ) ( type >= PROT_BEGIN && type <= PROT_END ) 

/* types of socket action result*/
typedef enum _socket_action_ret
{
	TYPE_CONNECT = 1,
	TYPE_DATAGRAM,
	TYPE_RESOLVE_PID,
	TYPE_CONNECT_ERROR,
	TYPE_LISTEN,
	TYPE_NOT_LISTEN,
	TYPE_CONNECT_CANCELED,
	TYPE_CONNECT_RESET,
	TYPE_CONNECT_TIMEOUT,
	TYPE_CONNECT_UNREACH, 
	BEGIN_SOCKET_ACTION_RET = TYPE_CONNECT, 
	END_SOCKET_ACTION_RET = TYPE_CONNECT_UNREACH 
} socket_action_ret, *psocket_action_ret; 

#define is_valid_socket_action_result( res ) ( res >= BEGIN_SOCKET_ACTION_RET && res <= END_SOCKET_ACTION_RET ) 

#define ICMP_TYPE_ECHO 0x08
#define ICMP_TYPE_REPLY 0x00


#define DEFAULT_OUTPUT_DATA_REGION_SIZE PAGE_SIZE 
#define DEFAULT_PARAMETERS_VARIABLE_SIZE ( ( _MAX_REG_PATH_LEN + _MAX_REG_PATH_LEN ) << 1 ) 

#ifdef HWND
#undef HWND
#endif //HWND

typedef struct _sys_action_record_out
{
	//LARGE_INTEGER time; 
	sys_action_type type; 
	union
	{
#ifdef COMPATIBLE_OLD_ACTION_DEFINE
		//old action structures
		ipv4_icmp_action icmp_info; 
		ipv4_socket_action socket_info; 
		access_url_info url_info; 
		access_reg_info reg_info; 
		access_file_info file_info; 
		manage_driver_info driver_info; 
		manage_proc_info proc_info; 
		//old action structures end
#endif //COMPATIBLE_OLD_ACTION_DEFINE

		exec_create do_exec_create; 
		exec_destroy do_exec_destroy; 
		exec_module_load do_exec_module_load; 
		file_touch do_file_touch; 
		file_open do_file_open; 
		_file_read do_file_read; 
		_file_write do_file_write; 
		file_modified do_file_modified; 
		file_readdir do_file_readdir; 
		file_remove do_file_remove; 
		file_rename do_file_rename; 
		file_truncate do_file_truncate; 
		file_mklink do_file_mklink; 
		file_chmod do_file_chmod; 
		file_setsec do_file_setsec; 
		file_getinfo do_file_getinfo; 
		file_setinfo do_file_setinfo; 
		file_close do_file_close; 

		reg_openkey do_reg_openkey; 
		reg_mkkey do_reg_mkkey; 
		reg_rmkey do_reg_rmkey; 
		reg_mvkey do_reg_mvkey; 
		reg_getinfo do_reg_getinfo; 
		reg_setinfo do_reg_setinfo; 
		reg_enum_val do_reg_enum_val; 
		reg_enuminfo do_reg_enum_info; 
		reg_rmval do_reg_rmval; 
		reg_getval do_reg_getval; 
		reg_setval do_reg_setval; 
		reg_loadkey do_reg_loadkey; 
		reg_replkey do_reg_replkey; 
		reg_rstrkey do_reg_rstrkey; 
		reg_setsec do_reg_setsec; 
		proc_exec do_proc_exec; 
		proc_open do_proc_open; 
		proc_debug do_proc_debug; 
		proc_suspend do_proc_suspend; 
		proc_resume do_proc_resume; 
		proc_kill do_proc_kill; 
		proc_exit do_proc_exit; 
		proc_job do_proc_job; 
		proc_pgprot do_proc_pgprot; 
		proc_freevm do_proc_freevm; 
		proc_writevm do_proc_writevm; 
		proc_readvm do_proc_readvm; 
		thrd_create do_thrd_create; 
		thrd_remote do_thrd_remote; 
		thrd_setctxt do_thrd_setctxt; 
		thrd_suspend do_thrd_suspend; 
		thrd_resume do_thrd_resume; 
		thrd_kill do_thrd_kill; 
		thrd_exit do_thrd_exit; 
		thrd_queue_apc do_thrd_queue_apc; 
		com_access do_com_access; 
		port_read do_port_read; 
		port_write do_port_write; 
		port_urb do_port_urb; 
		sys_settime do_sys_settime; 
		sys_link_knowndll do_sys_link_knowndll; 
		sys_open_physmm do_sys_open_physmm; 
		sys_read_physmm do_sys_read_physmm; 
		sys_write_physmm do_sys_write_physmm; 
		sys_load_kmod do_sys_load_kmod; 
		sys_enumproc do_sys_enumproc; 
		sys_regsrv do_sys_regsrv; 
		sys_opendev do_sys_opendev; 
		w32_postmsg do_w32_postmsg; 
		w32_sendmsg do_w32_sendmsg; 
		w32_findwnd do_w32_findwnd; 
		w32_msghook do_w32_msghook; 
		w32_lib_inject do_w32_lib_inject; 
		net_create do_net_create; 
		net_connect do_net_connect; 
		net_listen do_net_listen; 
		net_send do_net_send; 
		net_recv do_net_recv; 
		net_accept do_net_accept; 
		net_dns do_net_dns; 
		net_http do_net_http; 
		ba_extract_hidden do_ba_extract_hidden; 
		ba_extract_pe do_ba_extract_pe; 
		ba_self_copy do_ba_self_copy; 
		ba_self_delete do_ba_self_delete; 
		ba_infect_pe do_ba_infect_pe; 
		ba_overwrite_pe do_ba_overwrite_pe; 

		ba_ulterior_exec do_ba_ulterior_exec; 
		ba_invade_process do_ba_invade_process; 
		ba_register_autorun do_ba_register_autorun; 
	}; 

	//ULONG data_len; 
	//BYTE data[ 1 ]; 
}sys_action_record_out, *psys_action_record_out; 

#define EXEC_CREATE_PARAM_COUNT 6
#define EXEC_DESTROY_PARAM_COUNT 5
#define EXEC_MODULE_LOAD_PARAM_COUNT 4
#define FILE_TOUCH_PARAM_COUNT 8 
#define FILE_OPEN_PARAM_COUNT 8
#define FILE_READ_PARAM_COUNT 4
#define MODIFY_FILE_PARAM_COUNT 1
#define MODIFY_KEY_PARAM_COUNT 1

#define BA_OVERWRITE_PE_PARAM_COUNT 2

typedef struct _action_result
{
	ULARGE_INTEGER id; 
	NTSTATUS result; 
} action_result, *paction_result; 

typedef struct _sys_action_record
{ 
	ULONG size;
	sys_action_type type; 
	
	union
	{
		exec_create do_exec_create; 
		exec_destroy do_exec_destroy; 
		exec_module_load do_exec_module_load; 
		file_touch do_file_touch; 
		file_open do_file_open; 
		_file_read do_file_read; 
		_file_write do_file_write; 
		file_modified do_file_modified; 
		file_readdir do_file_readdir; 
		file_remove do_file_remove; 
		file_rename do_file_rename; 
		file_truncate do_file_truncate; 
		file_mklink do_file_mklink; 
		file_chmod do_file_chmod; 
		file_setsec do_file_setsec; 
		file_getinfo do_file_getinfo; 
		file_setinfo do_file_setinfo; 
		file_close do_file_close; 

		reg_openkey do_reg_openkey; 
		reg_closekey do_reg_closekey; 
		reg_mkkey do_reg_mkkey; 
		reg_rmkey do_reg_rmkey; 
		reg_mvkey do_reg_mvkey; 
		reg_getinfo do_reg_getinfo; 
		reg_setinfo do_reg_setinfo; 
		reg_enum_val do_reg_enum_val; 
		reg_enuminfo do_reg_enum_info; 
		reg_rmval do_reg_rmval; 
		reg_getval do_reg_getval; 
		reg_setval do_reg_setval; 
		reg_loadkey do_reg_loadkey; 
		reg_replkey do_reg_replkey; 
		reg_rstrkey do_reg_rstrkey; 
		reg_setsec do_reg_setsec; 
		proc_exec do_proc_exec; 
		proc_open do_proc_open; 
		proc_debug do_proc_debug; 
		proc_suspend do_proc_suspend; 
		proc_resume do_proc_resume; 
		proc_kill do_proc_kill; 
		proc_exit do_proc_exit; 
		proc_job do_proc_job; 
		proc_pgprot do_proc_pgprot; 
		proc_freevm do_proc_freevm; 
		proc_writevm do_proc_writevm; 
		proc_readvm do_proc_readvm; 
		thrd_create do_thrd_create; 
		thrd_remote do_thrd_remote; 
		thrd_setctxt do_thrd_setctxt; 
		thrd_suspend do_thrd_suspend; 
		thrd_resume do_thrd_resume; 
		thrd_kill do_thrd_kill; 
		thrd_exit do_thrd_exit; 
		thrd_queue_apc do_thrd_queue_apc; 
		com_access do_com_access; 
		port_read do_port_read; 
		port_write do_port_write; 
		port_urb do_port_urb; 
		sys_settime do_sys_settime; 
		sys_link_knowndll do_sys_link_knowndll; 
		sys_open_physmm do_sys_open_physmm; 
		sys_read_physmm do_sys_read_physmm; 
		sys_write_physmm do_sys_write_physmm; 
		sys_load_kmod do_sys_load_kmod; 
		//sys_load_mod do_sys_load_mod; 
		sys_unload_mod do_sys_unload_mod; 
		sys_enumproc do_sys_enumproc; 
		sys_regsrv do_sys_regsrv; 
		sys_opendev do_sys_opendev; 
		w32_postmsg do_w32_postmsg; 
		w32_sendmsg do_w32_sendmsg; 
		w32_findwnd do_w32_findwnd; 
		w32_msghook do_w32_msghook; 
		w32_lib_inject do_w32_lib_inject; 
		net_create do_net_create; 
		net_connect do_net_connect; 
		net_listen do_net_listen; 
		net_send do_net_send; 
		net_recv do_net_recv; 
		net_accept do_net_accept; 
		net_dns do_net_dns; 
		net_http do_net_http; 
		net_icmp_send do_net_icmp_send; 
		net_icmp_recv do_net_icmp_recv; 
		ba_extract_hidden do_ba_extract_hidden; 
		ba_extract_pe do_ba_extract_pe; 
		ba_self_copy do_ba_self_copy; 
		ba_self_delete do_ba_self_delete; 
		ba_ulterior_exec do_ba_ulterior_exec; 
		ba_invade_process do_ba_invade_process; 
		ba_infect_pe do_ba_infect_pe; 
		ba_overwrite_pe do_ba_overwrite_pe; 
		ba_register_autorun do_ba_register_autorun; 
	}; 
}sys_action_record, *psys_action_record; 

typedef sys_action_type action_policy_type; 

typedef struct _action_policy
{
	sys_action_type type; 
	USHORT size; 
	union
	{
		exec_create do_exec_create; 
		exec_destroy do_exec_destroy; 
		exec_module_load do_exec_module_load; 
		file_touch do_file_touch; 
		file_open do_file_open; 
		_file_read do_file_read; 
		_file_write do_file_write; 
		file_modified do_file_modified; 
		file_readdir do_file_readdir; 
		file_remove do_file_remove; 
		file_rename do_file_rename; 
		file_truncate do_file_truncate; 
		file_mklink do_file_mklink; 
		file_chmod do_file_chmod; 
		file_setsec do_file_setsec; 
		file_getinfo do_file_getinfo; 
		file_setinfo do_file_setinfo; 
		file_close do_file_close; 

		reg_openkey do_reg_openkey; 
		reg_mkkey do_reg_mkkey; 
		reg_rmkey do_reg_rmkey; 
		reg_mvkey do_reg_mvkey; 
		reg_getinfo do_reg_getinfo; 
		reg_setinfo do_reg_setinfo; 
		reg_enum_val do_reg_enum_val; 
		reg_enuminfo do_reg_enum_info; 
		reg_rmval do_reg_rmval; 
		reg_getval do_reg_getval; 
		reg_setval do_reg_setval; 
		reg_loadkey do_reg_loadkey; 
		reg_replkey do_reg_replkey; 
		reg_rstrkey do_reg_rstrkey; 
		reg_setsec do_reg_setsec; 
		proc_exec do_proc_exec; 
		proc_open do_proc_open; 
		proc_debug do_proc_debug; 
		proc_suspend do_proc_suspend; 
		proc_resume do_proc_resume; 
		proc_kill do_proc_kill; 
		proc_exit do_proc_exit; 
		proc_job do_proc_job; 
		proc_pgprot do_proc_pgprot; 
		proc_freevm do_proc_freevm; 
		proc_writevm do_proc_writevm; 
		proc_readvm do_proc_readvm; 
		thrd_remote do_thrd_remote; 
		thrd_setctxt do_thrd_setctxt; 
		thrd_suspend do_thrd_suspend; 
		thrd_resume do_thrd_resume; 
		thrd_kill do_thrd_kill; 
		thrd_exit do_thrd_exit; 
		thrd_queue_apc do_thrd_queue_apc; 
		com_access do_com_access; 
		port_read do_port_read; 
		port_write do_port_write; 
		port_urb do_port_urb; 
		sys_settime do_sys_settime; 
		sys_link_knowndll do_sys_link_knowndll; 
		sys_open_physmm do_sys_open_physmm; 
		sys_read_physmm do_sys_read_physmm; 
		sys_write_physmm do_sys_write_physmm; 
		sys_load_kmod do_sys_load_kmod; 
		sys_enumproc do_sys_enumproc; 
		sys_regsrv do_sys_regsrv; 
		sys_opendev do_sys_opendev; 
		w32_postmsg do_w32_postmsg; 
		w32_sendmsg do_w32_sendmsg; 
		w32_findwnd do_w32_findwnd; 
		w32_msghook do_w32_msghook; 
		w32_lib_inject do_w32_lib_inject; 
		net_connect do_net_connect; 
		net_listen do_net_listen; 
		net_send do_net_send; 
		net_accept do_net_accept; 
		net_dns do_net_dns; 
		net_http do_net_http; 
		ba_extract_hidden do_ba_extract_hidden; 
		ba_extract_pe do_ba_extract_pe; 
		ba_self_copy do_ba_self_copy; 
		ba_self_delete do_ba_self_delete; 
		ba_ulterior_exec do_ba_ulterior_exec; 
		ba_invade_process do_ba_invade_process; 
		ba_infect_pe do_ba_infect_pe; 
		ba_overwrite_pe do_ba_overwrite_pe; 
		ba_register_autorun do_ba_register_autorun; 
	}; 
} action_policy, *paction_policy; 

#pragma pack( pop )

INLINE action_policy_type action_2_policy_type( sys_action_type type )
{
	if( FALSE == is_valid_action_type( type ) )
	{
		return MAX_SYS_ACTION_TYPE; 
	}

	return type; 
}

#endif //__ACTION_TYPE_H__