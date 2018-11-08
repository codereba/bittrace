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
#include "wdk_macros.h"
#define UNDEFINE_IRP
#define UNDEFINE_MDL
#include "ring0_2_ring3.h"
#include "action_display.h"
#include "action_logger.h"
#include "lang_en.h"

#include "volume_name_map.h"
#include <strsafe.h>

#ifdef LNAG_EN
#define FILE_STREAM_INFO_FORMAT_TEXT L"Stream name: %s, Stream size: %I64u, Stream data size: %I64u " //L"流名称: %s, 流分配空间大小: %I64u, 流数据大小: %I64u "
#define PIPE_FORMAT_TEXT L"PIPE completion mode: %u, PIPE read mode: %u" //L"管道完成模式: %u, 管理读取模式: %u"
#define PIPE_LOCAL_INFORMATION L"PIPE input quota: %u, PIPE output quota: %u, PIPE readable data size: %u, PIPE remain output quota: %u, PIPE instance count: %u, PIPE max instance count: %u, Named PIPE configure: 0x%0.8x, Named PIPE status: %u, Named PIPE type: %u, Named PIPE end: %u" //L"管道输入配额: %u, 管道输出配额: %u, 管理可读数据长度: %u, 管道写配额余量: %u, 管道当前实例数: %u, 管理最大实例数: %u, 命名管道配置: 0x%0.8x, 命名管道状态: %u, 命名管道类型: %u, 命名管道结束: %u", 
#define INVADE_PROCESS_FORMAT_TEXT L"Process id %u\n"  //L"进程ID %u\n"; 
#define REGISTER_AUTORUN_FORMAT_TEXT L"Type %u\n"  //L"类型 %u\n"
#define FILE_ATTRIBUTE_TEXT L"File attributes " //L"文件属性 "
#define FILE_NEW_ATTRIBUTE_TEXT L"New attributes " //L"新属性 "
#define FILE_ATTRIBUTE_READ_ONLY_TEXT L"Read only|" //L"只读|"
#define FILE_ATTRIBUTE_HIDE_TEXT L"Hide|" //L"隐藏|"
#define FILE_ATTRIBUTE_SYSTEM_TEXT L"System|" //L"系统|"
#define FILE_ATTRIBUTE_DIRECTORY_TEXT L"Directory|" //L"目录|"
#define FILE_ATTRIBUTE_ARCHIVE_TEXT L"Archive|" //L"存档|"
#define FILE_ATTRIBUTE_DEVICE_TEXT L"Device|" //L"设备|"
#define FILE_ATTRIBUTE_GENERAL_TEXT L"General|" //L"一般|"
#define FILE_ATTRIBUTE_TEMPORARY_TEXT L"Temporary|" //L"临时|"
#define FILE_ATTRIBUTE_SPARSE_FILE_TEXT L"Sparse file|" //L"稀疏|"
#define FILE_ATTRIBUTE_REPARSE_POINT_TEXT L"Reparse point|" //L"重定向|"
#define FILE_ATTRIBUTE_COMPRESSED_TEXT L"Compressed|" //L"压缩|"
#define FILE_ATTRIBUTE_OFFLINE_TEXT L"Offline|" //L"离线|"
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED_TEXT L"Not content indexed|" //L"无内容索引|"
#define FILE_ATTRIBUTE_ENCRYPTED_TEXT L"Encrypted|" //L"加密|"
#define FILE_ATTRIBUTE_VIRTUAL_TEXT L"Virtual|" //L"虚拟|"
#define FILE_READ_DATA_TEXT L"Read data/" //L"读数据/"
#define FILE_LIST_DIRECTORY_TEXT L"List directory/" //L"列目录/"
#define FILE_WRITE_DATA_TEXT L"Write data/" //L"写数据/"
#define FILE_ADD_FILE_TEXT L"Add file/" //L"添加文件/"
#define FILE_APPEND_DATA_TEXT L"Append data" //L"添加数据/"
#define FILE_ADD_SUBDIRECTORY_TEXT L"Add sub directory/" //L"添加子目录/"
#define FILE_CREATE_PIPE_INSTANCE_TEXT L"Create PIPE instance/" //L"创建管道实例/"
#define FILE_ACCESS_FILE_READ_EA L"Read extension attributes/" //L"读扩展属性/"
#define FILE_ACCESS_FILE_WRITE_EA L"Write extension attributes/" // L"写扩展属性/"
#define FILE_ACCESS_EXECUTE_TEXT L"File execute mapping" //L"内存映射/"
#define FILE_ACCESS_TRAVERSE_TEXT L"Directory traverse" //L"浏览文件路径/"
#define FILE_ACCESS_DELETE_CHILD_TEXT L"Delete child" //L"删除子文件(夹)/"
#define FILE_ACCESS_READ_ATTRIBUTES_TEXT L"Read attributes" //L"读属性/"
#define FILE_ACCESS_WRITE_ATTRIBUTES_TEXT L"Write attributes" //L"写属性/"
#define FILE_ACCESS_SYNCHRONIZE_TEXT L"Synchronize IO" //L"同步IO/"
#define FILE_ACCESS_DELETE_TEXT L"Delete" //L"删除对象/"
#define FILE_ACCESS_READ_CONTROL_TEXT L"Read object info" //L"读取对象信息/"
#define FILE_ACCESS_TEXT L"Access: " //L"访问权限: "
#define FILE_NAME_TEXT L"File name: " //L"文件名: "
#define FILE_ACCESS_WRITE_DAC_TEXT L"Write DAC" //L"改写访问控制列表/"
#define FILE_ACCESS_WRITE_OWNER_TEXT L"Write Owner" //L"改写所有者/"
#define FILE_OPEN_FORMAT_TEXT L"Access 0x%0.8x Initial size %u Share mode 0x%0.8x Disposition 0x%0.8x Option 0x%0.8x \n" //L"访问权限 0x%0.8x 初始大小 %u 共享属性 0x%0.8x 工作方式(disposition) 0x%0.8x 工作方式(option) 0x%0.8x \n"; 
#define REG_CREATE_KEY_FORMAT_TEXT L"Access 0x%0.8x \n"  //L"权限 0x%0.8x \n"; 
#define INJECT_LIB_FORMAT_TEXT L"DLL %s\n" //L"动态库 %s\n"; 
#define MODULE_LOAD_FORMAT_TEXT L"Base address 0x%0.8x Size %u Flags 0x%0.8x" //L"基地址 0x%0.8x 大小 %u 标志 0x%0.8x"
#define FILE_READ_DIR_FORMAT_TEXT L"Filter 0x%0.8x\n" //L"过滤条件 %u\n"; 
#define FILE_RENAME_FORMAT_TEXT L"File dest name %s File source name %s %s\n" //L"目标文件名 %s 原文件名 %s %s\n"
#define FILE_RENAME_REPLACE_TEXT L"Over write" //L"覆盖"
#define FILE_RENAME_NOT_REPLACE_TEXT L"Not over write" //L"不覆盖"
#define FILE_TRUNCATE_FORMAT_TEXT L"Size %I64u\n" //L"长度 %I64u\n"; 
#define FILE_MAKE_LINK_FORMAT_TEXT L"Hard link %s Mode 0x%0.8x\n" //L"硬链接%s 模式%u\n"; 
#define FILE_LINK_INFO_FORMAT_TEXT L"File name %s, %s" //L"文件名 %s, %s"
#define FILE_LINK_REPLACE_TEXT L"Replace exists"
#define FILE_LINK_NOT_REPLACE_TEXT L"Do not replace exists"
#define FILE_NAMES_INFO_FORMAT_TEXT L"File name: %s, Index: %u" //L"文件名: %s, 文件索引号: %u"
#define FILE_DELETE_TEXT L"Delete" //L"删除"
#define FILE_POSITION_INFO_FORMAT_TEXT L"File position: %I64u" //L"文件偏移: %I64u"
#define FILE_FULL_EA_INFO_FORMAT_TEXT L"EA name: %s, EA size: %u, EA flags: 0x%0.8x"  //L"可扩展属性名称: %s, 可扩展属性长度: %u, 可扩展属性标记: 0x%0.8x", 
#define FILE_MODE_TEXT L"File mode: 0x%0.8x" //L"文件操作模式: 0x%0.8x"
#define FILE_ALIGNMENT_INFO_FORMAT_TEXT L"Alignment: %u"  //L"文件对齐要求: %u"
#define FILE_ALL_INFO_FORMAT_TEXT L"File mode: 0x%0.8x, Access: 0x%0.8x, Alignment: %u, Allocation size: %I64u, End of file: %I64u, Position: %I64u, " \
	L"Change time: %04u-%02u-%02u %02u:%02u:%02u.%03u, Last write time: %04u-%02u-%02u %02u:%02u:%02u.%03u, Creation time: %04u-%02u-%02u %02u:%02u:%02u.%03u, Last access time: %04u-%02u-%02u %02u:%02u:%02u.%03u, " \
	L"EA size: %u, Index: %I64u, Link count: %u, %s, %s"  
//L"文件操作模式: 0x%0.8x, 访问权限: 0x%0.8x, 对齐要求: %u, 占用空间大小: %I64u, 文件数据大小: %I64u, 输入指针偏移量: %I64u, " 
//L"修改时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, " 
//L"可扩展属性大小: %u, 索引号: %I64u, 链接数: %u, %s, %s",  

#define FILE_DELETED_TEXT L"Delete pending" //L"已删除"
#define FILE_DIRECTORY_TEXT L"Directory" //L"目录"
#define FILE_ALLOCATION_INFO_FORMAT_TEXT L"Allocation size: %I64u"  //L"分配空间大小: %I64u"
#define FILE_END_OF_FILE_INFO_FORMAT_TEXT L"End of file: %I64u" //L"数据大小: %I64u"
#define FILE_ALTERNATE_NAME_INFO_FORMAT_TEXT L"Alternate name: %s" //L"短路径: %s"
#define FILE_PIPE_REMOTE_INFO_FORMAT_TEXT L"Max collection count: %u, Collection time: %0.4u-%0.2u-%0.2u %0.2u:%0.2u:%0.2u.%0.3u" //L"最大数据收集次数: %u, 收集数据时间: %0.4u-%0.2u-%0.2u %0.2u:%0.2u:%0.2u.%0.3u", 
#define FILE_MAILSLOT_QUERY_INFO_FORMAT_TEXT L"Quota: %u, Max message size: %u, Messages available: %u, Next message size: %u, Read time out: %I64ums" //L"邮槽配额: %u, 最大消息长度: %u, 缓存消息数: %u, 下个消息大小: %u, 读取超时: %I64ums", 
#define FILE_MAILSLOT_SET_INFO_FORMAT_TEXT L"Read time out: %I64ums" //L"邮槽读取超时: %I64ums"
#define FILE_COMPRESSION_INFO_FORMAT_TEXT L"Compressed size: %I64u, Compression format: %s, Chunk shift: %u, Cluster shift: %u, Compression unit shift: %u" //L"压缩大小: %I64u, 压缩格式: %s, 块大小: %u, 簇大小: %u, 压缩单元大小: %u", 
#define FILE_OBJECT_ID_INFO_FORMAT_TEXT L"File reference: 0x%I64x, Object ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x, Birth object ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x" //L"文件参考号: 0x%I64x, 对象ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x, 初始对象ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x", 
#define FILE_COMPLETION_INFO_FORMAT_TEXT L"Completion notify key: %p, Completion port: %p" //L"异步IO操作通知KEY: %p, 通知端口: %p"
#define FILE_MOVE_CLUSTER_INFO_FORMAT_TEXT L"File name: %s, Cluster count: %u" //L"文件名: %s, 簇数: %u"
#define FILE_QUOTE_INFO_FORMAT_TEXT L"Quota limit: %I64u, Quota threshold: %I64u, Quota used: %I64u, Quota change time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"配额上限: %I64u, 配额临界值: %I64u, 配额已使用: %I64u, 配额改写时间: %04u-%02u-%02u %02u:%02u:%02u.%03u", 
#define FILE_REPARSE_POINT_INFO_FORMAT_TEXT L"File referece: %u, Tag: %u" //L"重定向文件参考号: %u, 用户标记: %u", 
#define FILE_NAME_FORMAT_TEXT L"File name %s" //L"文件名:%s "
#define FILE_DIRECTORY_INFO_FORMAT_TEXT L"Allocated size:%I64u, Data size:%I64u, File index:%u, Change time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Creation time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last access time:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" //L"分配空间:%I64u, 数据大小:%I64u, 文件索引号:%u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n", 
#define FILE_BOTH_DIRECTORY_INFO_FORMAT_TEXT L"File short name:%s, Allocation size:%I64u, Data size:%I64u, EA size:%u, Change time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Creation time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last access time:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" //L"短文件名:%s, 分配空间:%I64u, 数据大小:%I64u, 扩展属性长度:%u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n", 
#define _FILE_ATTRIBUTE_TEXT L"Attributes:" //L"属性:"
#define FILE_BASIC_INFO_FORMAT_TEXT L" Change time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Creation time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last access time:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" //L" 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n", 
#define FILE_STANDARD_INFO_FORMAT_TEXT L"%s Allocation time: %I64u, Data size: %I64u, Link count: %u %s" //L"%s 分配空间: %I64u, 数据大小: %I64u, 链接数: %u %s"
#define FILE_NETWORK_OPEN_INFO_FORMAT_TEXT L"Allocation size: %I64u, End of file:%I64u, Change time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Creation time:%04u-%02u-%02u %02u:%02u:%02u.%03u, Last access time:%04u-%02u-%02u %02u:%02u:%02u.%03u \n" //L"文件占用空间大小: %I64u, 数据大小:%I64u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u \n"
#define FILE_ATTRIBUTE_TAG_INFO_FORMAT_TEXT L"Reparse tag: 0x%0.8x" //L"重定向标记: 0x%0.8x"
#define FILE_TRACKING_INFO_FORMAT_TEXT L"File tracking" //L"文件状态跟踪"
#define FILE_BOTH_DIR_FILE_INFO_FORMAT_TEXT L"File name: %s, short name: %s, Allocation size: %I64u, End of file: %I64u, File ID: %I64u, Index: %u, EA size: %u, Change time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Last write time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Creation time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Last access time: %04u-%02u-%02 %02u:%02u:%02u.%03u" //L"文件名: %s, 短文件名: %s, 分配空间大小: %I64u, 数据大小: %I64u, 文件ID: %I64u, 索引号: %u, 扩展属性大小: %u, 修改时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02 %02u:%02u:%02u.%03u"
#define FILE_FULL_DIR_INFO_FORMAT_TEXT L"Allocation size: %I64u, Data size: %I64u, File ID: %I64u, Index: %u, EA size: %u, Change time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Last write time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Creation time: %04u-%02u-%02 %02u:%02u:%02u.%03u, Last access time: %04u-%02u-%02 %02u:%02u:%02u.%03u" //L"分配空间大小: %I64u, 数据大小: %I64u, 文件ID: %I64u, 索引号: %u, 扩展属性大小: %u, 修改时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02 %02u:%02u:%02u.%03u" 
#define FILE_VALID_DATA_LENGHT_INFO_FORMAT_TEXT L"Valid data length: %I64u"  //L"有效数据长度: %I64u"
#define FILE_ALTERNATE_NAME L"File alternate name" //L"文件短名"
#define FILE_COMPLETION_NOTIFICATION_INFO_FORMAT_TEXT L"IO completion flags: 0x%0.8x" //L"IO完成标志位: 0x%0.8x", 
#define FILE_STATUS_BLOCK_RANGE_INFO_FORMAT_TEXT L"Status block range" //L"文件块区域"
#define FILE_REMOTE_FILE_TEXT L"Remote" 
#define FILE_LOCAL_FILE_TEXT L"Local"
#define FILE_STANDARD_LINK_INFO_FORMAT_TEXT L"Accessable links count: %u, Total links count: %u, %s %s" //L"可访问链接数: %u, 总链接数: %u, %s %s"

//L"远程文件"
#define _FILE_ATTRIBUTE_DIRECTORY_TEXT L"Directory" 

#define _FILE_ATTRIBUTE_FILE_TEXT L"File" 
#define FILE_DELETE_PENDING_TEXT L"Delete pending" //L"标记删除"
#define FILE_INDEX_NUMBER_TEXT L"File index: %I64u" //L"索引号: %I64u", 
#define FILE_EA_INFO_FORMAT_TEXT L"File EA size: %u" //L"扩展属性长度: %u"

#define KEY_OPEN_FORMAT_TEXT L"Access 0x%0.8x\n" //L"权限 0x%0.8x\n";
#define KEY_MOVE_KEY_FORAT_TEXT L"New key name %s \n" //L"新路径 %s \n"
#define KEY_REMOVE_VALUE_FORMAT_TEXT L"Value name %s\n" //L"键值名 %s\n"
#define KEY_GET_VALUE_INFO_FORMAT_TEXT L"Value name %s type %u size %u\n" //L"键值名 %s 类型 %u 长度 %u\n"
#define KEY_ENUM_INFO_FORMAT_TEXT L"Key index: %u" //L"键索引号: %u"
#define KEY_ENUM_VALUE_INFO_FORMAT_TEXT L"Index: %u" //L"键索引号: %u"
#define KEY_SET_VALUE_INFO_FORMAT_TEXT L"Value name %s type %s size %u\n" //L"键值名 %s 类型 %s 长度%u\n"
#define KEY_LOAD_FORMAT_TEXT L"Hive file %s\n" //L"hive 文件%s\n"
#define KEY_REPLACE_FORMAT_TEXT L"Old hive file %s, New hive file %s\n" //L"原HIVE文件 %s 新HIVE文件 %s\n" 
#define KEY_VALUE_PARTIAL_INFO_FORMAT_TEXT L"Value type: %s" //L"值类型: %s"
#define GET_PROCESS_OPEN_DETAIL_FORMAT_TEXT L"Target process %u Access %0.8x \n" //L"目标进程 %u 权限 %0.8x \n"
#define PROCESS_DEBUG_FORMAT_TEXT L"Target process %u\n" //L"目标进程 %u\n"
#define PROCESS_SUSPEND_FORMAT_TEXT L"Target process %s [%u] \n" //L"目标进程%s [%u] \n"
#define PROCESS_RESUME_FORMAT_TEXT L"Resume process %s (ID:%u) execution\n" //L"恢复进程%s (ID:%u) 执行\n"
#define PROCESS_KILL_FORMAT_TEXT L"Target process %s [%u] Exit code %u\n" //L"目标进程%s [%u] 退出代码%u\n"
#define PROCESS_JOB_FORMAT_TEXT L"Process job id %u\n" //L"进程工作组ID %u\n"
#define PROCESS_PAGE_PROTECT_FORMAT_TEXT L"Target process id %u Address 0x%0.8x Size:%u Attributes 0x%0.8x Return size %u\n" //L"目标进程ID %u 地址 0x%0.8x 长度:%u 属性 0x%0.8x 返回长度 %u\n"
#define THREAD_KILL_FORMAT_TEXT L"Target process id %u Thread id %u Exit code 0x%0.8x\n" //L"目标进程ID %u 线程ID %u 退出代码 0x%0.8x\n"
#define PROT_READ_FORMAT_TEXT L"Port:%s Read %u bytes\n" //L"端口:%s 读取数据%u字节\n"

#define PROT_WRITE_FORMAT_TEXT L"Port:%s Write %u bytes\n" //L"端口:%s 写入数据%u字节\n";
#define LOAD_KERNEL_MODULE_FORMAT_TEXT L"Base address 0x%0.8x Size %I64u" //L"基地址 0x%0.8x 大小 %I64u"

#define ICMP_SEND_FORMAT_TEXT _T( "Source ip %s Target ip %s Type %u Code %u\n" ) //_T( "源IP地址 %s 目标IP地址 %s 类型 %u 代码 %u\n" ); 
#define URB_INFO_FORMAT_TEXT L"%ws Size:%u " //L"%ws 长度:%u "
#define URB_FUNCTION_SELECT_CONFIGURATION_FORMAT_TEXT L"Configuration handle:%p INTERFACE%u Alternate setting:0x%x Class:%u Sub class:%u Protocol:%u Interface handle:%p Pipe count:%u " //L"配置句柄:%p INTERFACE%u 配置值:0x%x 类型:%u 子类型:%u 协议:%u 接口句柄:%p Pipe数量:%u ", 
#define URB_FUNCTION_SELECT_CONFIGURATION_PIPE_INFO_FORMAT_TEXT L"PIPE%u Max package size:%u PIPE address:%u Interval:%u Type:%u Handle:%p Max transfer size:%u Flags:0x%x " 
#define URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER_FORMAT_TEXT L"PIPE handle:%p Transfer flags:%x Transfer size:%u Buffer:%p MDL:%p Next URB:%p HCD:%p%p%p%p%p%p%p%p " //L"PIPE句柄:%p 传输标志:%u 长度:%u 缓存区:%p MDL:%p 下一个URB:%p HCD字段:%p%p%p%p%p%p%p%p ", 
#define URB_FUNCTION_CONTROL_TRANSFER_FORMAT_TEXT L"PIPE handle:%p Transfer flags:%x Transfer size:%u 缓存区:%p MDL:%p Next URB:%p Setup packet:%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x HCD:%p%p%p%p%p%p%p%p " //L"PIPE句柄:%p 传输标志:%u 长度:%u 缓存区:%p MDL:%p 下一个URB:%p 配置字段:%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x HCD字段:%p%p%p%p%p%p%p%p ", 

#define FILE_READ_FORMAT_TEXT _T( "Offset %I64u Size %u\n") //_T( "偏移量 %I64u 长度 %u\n")
#define NETWORK_SEND_FORMAT_TEXT _T( "Send (%s)dns request:%s\n" ) //_T( "发送 (%s)dns 请求:%s\n" )
#define NETWORK_HTTP_FORMAT_TEXT _T( "Protocol %u Command %u Command size %u \n" ); //_T( "协议 %u 命令 %u 命令长度 %u \n" );
#define NETWORK_CONNECT_FORMAT_TEXT _T( "Local %s:%u Remote %s:%u %s Port\n" ) //_T( "本地%s:%u 远程%s:%u %s端口\n" )

#define FILE_SFIO_VOLUME_INFORMATION_FORMAT_TEXT L"Max requests per peroid: %u, Min period: %u, Min transfer size: %u" //L"每周期最大请求数: %u, 最小周期长度: %u, 最小传输大小: %u", 
#define FILE_IO_PRIORITY_HINT_INFO_FORMAT_TEXT L"IO priority hint: %u" //L"推荐IO优先级: %u", 
#define FILE_SFIO_RESERVE_INFO_FORMAT_TEXT L"Requests per period: %u, Peroid: %u, Retry failures: %u, %s, Request size: %u, Outstanding requests: %u" //L"每周期请求数: %u, 周期数: %u, 失败重试次数: %u, %s, 请求大小: %u, 排列请求数: %u", 
#define FILE_SFIO_DISCARDABLE_TEXT L"Discardable" //L"可丢弃" 
#define FILE_HARD_LINK_TEXT L"File hard link" //L"文件硬链接"
#define FILE_PROCESS_IDS_USING_FILE_INFO_TEXT1 L"Processes: " //L"进程"
#define FILE_PROCESS_IDS_USING_FILE_INFO_TEXT2 L"Is using file" //L"占用文件"
#define FILE_NORMALIZED_NAME_INFO_FORMAT_TEXT L"Normalized name" //L"标准格式文件名: "
#define FILE_NETWORK_PHYSICAL_NAME_INFO_FORMAT_TEXT L"Network physical name:%s" //L"网络物理名 %s"
#define FILE_REMOTE_PROTOCOL_INFO_FORMAT_TEXT L"Protocol: %u, Protocol major version:%u, Protocol minor version:%u, Protocol revision: %u, Flags: 0x%0.8x" //L"远程协议: %u, 远程协议 主版本:%u, 次版本:%u, 修定号: %u, 标志位: %u"
#define FILE_NUMA_NODE_INFO_FORMAT_TEXT L"NUMA node No. %u" //L"NUMA结点号: %u"
#define FILE_ATTRIBUTE_CACHE_INFO_FORMAT_TEXT L"File attribute cache" //L"文件属性缓存: "
#define FILE_ID_GLOBAL_TX_DIR_INFO_FORMAT_TEXT L"File id global tx directory information"  
#else
#define FILE_STREAM_INFO_FORMAT_TEXT L"流名称: %s, 流分配空间大小: %I64u, 流数据大小: %I64u "
#define PIPE_FORMAT_TEXT  L"管道完成模式: %u, 管理读取模式: %u"
#define PIPE_LOCAL_INFORMATION L"管道输入配额: %u, 管道输出配额: %u, 管理可读数据长度: %u, 管道写配额余量: %u, 管道当前实例数: %u, 管理最大实例数: %u, 命名管道配置: 0x%0.8x, 命名管道状态: %u, 命名管道类型: %u, 命名管道结束: %u" 
#define INVADE_PROCESS_FORMAT_TEXT L"进程ID %u\n" 
#define REGISTER_AUTORUN_FORMAT_TEXT L"类型 %u\n"
#define FILE_ATTRIBUTE_TEXT L"文件属性 "
#define FILE_NEW_ATTRIBUTE_TEXT L"新属性 "
#define FILE_ATTRIBUTE_READ_ONLY_TEXT L"只读|"
#define FILE_ATTRIBUTE_HIDE_TEXT L"隐藏|"
#define FILE_ATTRIBUTE_SYSTEM_TEXT L"系统|"
#define FILE_ATTRIBUTE_DIRECTORY_TEXT L"目录|"
#define FILE_ATTRIBUTE_ARCHIVE_TEXT L"存档|"
#define FILE_ATTRIBUTE_DEVICE_TEXT L"设备|"
#define FILE_ATTRIBUTE_GENERAL_TEXT L"一般|"
#define FILE_ATTRIBUTE_TEMPORARY_TEXT L"临时|"
#define FILE_ATTRIBUTE_SPARSE_FILE_TEXT L"稀疏|"
#define FILE_ATTRIBUTE_REPARSE_POINT_TEXT L"重定向|"
#define FILE_ATTRIBUTE_COMPRESSED_TEXT L"压缩|"
#define FILE_ATTRIBUTE_OFFLINE_TEXT L"离线|"
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED_TEXT L"无内容索引|"
#define FILE_ATTRIBUTE_ENCRYPTED_TEXT L"加密|"
#define FILE_ATTRIBUTE_VIRTUAL_TEXT L"虚拟|"
#define FILE_READ_DATA_TEXT L"读数据/"
#define FILE_LIST_DIRECTORY_TEXT L"列目录/"
#define FILE_WRITE_DATA_TEXT L"写数据/"
#define FILE_ADD_FILE_TEXT L"添加文件/"
#define FILE_APPEND_DATA_TEXT L"添加数据/"
#define FILE_ADD_SUBDIRECTORY_TEXT L"添加子目录/"
#define FILE_CREATE_PIPE_INSTANCE_TEXT L"创建管道实例/"
#define FILE_ACCESS_FILE_READ_EA L"读扩展属性/"
#define FILE_ACCESS_FILE_WRITE_EA L"写扩展属性/"
#define FILE_ACCESS_EXECUTE_TEXT L"内存映射/"
#define FILE_ACCESS_TRAVERSE_TEXT L"浏览文件路径/"
#define FILE_ACCESS_DELETE_CHILD_TEXT L"删除子文件(夹)/"
#define FILE_ACCESS_READ_ATTRIBUTES_TEXT L"读属性/"
#define FILE_ACCESS_WRITE_ATTRIBUTES_TEXT L"写属性/"
#define FILE_ACCESS_SYNCHRONIZE_TEXT L"同步IO/"
#define FILE_ACCESS_DELETE_TEXT L"删除对象/"
#define FILE_ACCESS_READ_CONTROL_TEXT L"读取对象信息/"
#define FILE_ACCESS_TEXT L"访问权限: "
#define FILE_NAME_TEXT L"文件名: "
#define FILE_ACCESS_WRITE_DAC_TEXT L"改写访问控制列表/"
#define FILE_ACCESS_WRITE_OWNER_TEXT L"改写所有者/"
#define FILE_OPEN_FORMAT_TEXT L"访问权限 0x%0.8x 初始大小 %u 共享属性 0x%0.8x 工作方式(disposition) 0x%0.8x 工作方式(option) 0x%0.8x \n" 
#define REG_CREATE_KEY_FORMAT_TEXT L"权限 0x%0.8x \n" 
#define INJECT_LIB_FORMAT_TEXT L"动态库 %s\n" 
#define MODULE_LOAD_FORMAT_TEXT L"基地址 0x%0.8x 大小 %u 标志 0x%0.8x"
#define FILE_READ_DIR_FORMAT_TEXT L"过滤条件 %u\n" 
#define FILE_RENAME_FORMAT_TEXT L"目标文件名 %s 原文件名 %s %s\n"
#define FILE_RENAME_REPLACE_TEXT L"覆盖"
#define FILE_RENAME_NOT_REPLACE_TEXT L"不覆盖"
#define FILE_TRUNCATE_FORMAT_TEXT L"长度 %I64u\n" 
#define FILE_MAKE_LINK_FORMAT_TEXT L"硬链接%s 模式%u\n" 
#define FILE_LINK_INFO_FORMAT_TEXT L"文件名 %s, %s"
#define FILE_LINK_REPLACE_TEXT L"覆盖"
#define FILE_LINK_NOT_REPLACE_TEXT L"不覆盖"
#define FILE_NAMES_INFO_FORMAT_TEXT L"文件名: %s, 文件索引号: %u"
#define FILE_DELETE_TEXT L"删除"
#define FILE_POSITION_INFO_FORMAT_TEXT L"文件偏移: %I64u"
#define FILE_FULL_EA_INFO_FORMAT_TEXT L"可扩展属性名称: %s, 可扩展属性长度: %u, 可扩展属性标记: 0x%0.8x" 
#define FILE_MODE_TEXT L"文件操作模式: 0x%0.8x"
#define FILE_ALIGNMENT_INFO_FORMAT_TEXT L"文件对齐要求: %u"
#define FILE_ALL_INFO_FORMAT_TEXT L"文件操作模式: 0x%0.8x, 访问权限: 0x%0.8x, 对齐要求: %u, 占用空间大小: %I64u, 文件数据大小: %I64u, 输入指针偏移量: %I64u, " \
	L"修改时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02u %02u:%02u:%02u.%03u, " \
	L"可扩展属性大小: %u, 索引号: %I64u, 链接数: %u, %s, %s"  

#define FILE_DELETED_TEXT L"已删除"
#define FILE_DIRECTORY_TEXT L"目录"
#define FILE_ALLOCATION_INFO_FORMAT_TEXT L"分配空间大小: %I64u"
#define FILE_END_OF_FILE_INFO_FORMAT_TEXT L"数据大小: %I64u"
#define FILE_ALTERNATE_NAME_INFO_FORMAT_TEXT L"短路径: %s"
#define FILE_PIPE_REMOTE_INFO_FORMAT_TEXT L"最大数据收集次数: %u, 收集数据时间: %0.4u-%0.2u-%0.2u %0.2u:%0.2u:%0.2u.%0.3u" 
#define FILE_MAILSLOT_QUERY_INFO_FORMAT_TEXT L"邮槽配额: %u, 最大消息长度: %u, 缓存消息数: %u, 下个消息大小: %u, 读取超时: %I64ums" 
#define FILE_MAILSLOT_SET_INFO_FORMAT_TEXT L"邮槽读取超时: %I64ums"
#define FILE_COMPRESSION_INFO_FORMAT_TEXT L"压缩大小: %I64u, 压缩格式: %s, 块大小: %u, 簇大小: %u, 压缩单元大小: %u" 
#define FILE_OBJECT_ID_INFO_FORMAT_TEXT L"文件参考号: 0x%I64x, 对象ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x, 初始对象ID: %0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x" 
#define FILE_COMPLETION_INFO_FORMAT_TEXT L"异步IO操作通知KEY: %p, 通知端口: %p"
#define FILE_MOVE_CLUSTER_INFO_FORMAT_TEXT L"文件名: %s, 簇数: %u"
#define FILE_QUOTE_INFO_FORMAT_TEXT L"配额上限: %I64u, 配额临界值: %I64u, 配额已使用: %I64u, 配额改写时间: %04u-%02u-%02u %02u:%02u:%02u.%03u" 
#define FILE_REPARSE_POINT_INFO_FORMAT_TEXT L"重定向文件参考号: %u, 用户标记: %u" 
#define FILE_NAME_FORMAT_TEXT L"文件名:%s "
#define FILE_DIRECTORY_INFO_FORMAT_TEXT L"分配空间:%I64u, 数据大小:%I64u, 文件索引号:%u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" 
#define FILE_BOTH_DIRECTORY_INFO_FORMAT_TEXT L"短文件名:%s, 分配空间:%I64u, 数据大小:%I64u, 扩展属性长度:%u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" 
#define _FILE_ATTRIBUTE_TEXT L"属性:"
#define FILE_BASIC_INFO_FORMAT_TEXT L" 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n" 
#define FILE_STANDARD_INFO_FORMAT_TEXT L"%s 分配空间: %I64u, 数据大小: %I64u, 链接数: %u %s"
#define FILE_NETWORK_OPEN_INFO_FORMAT_TEXT L"文件占用空间大小: %I64u, 数据大小:%I64u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u, 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u \n"
#define FILE_ATTRIBUTE_TAG_INFO_FORMAT_TEXT L"重定向标记: 0x%0.8x"
#define FILE_TRACKING_INFO_FORMAT_TEXT L"文件状态跟踪"
#define FILE_BOTH_DIR_FILE_INFO_FORMAT_TEXT L"文件名: %s, 短文件名: %s, 分配空间大小: %I64u, 数据大小: %I64u, 文件ID: %I64u, 索引号: %u, 扩展属性大小: %u, 修改时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02 %02u:%02u:%02u.%03u"
#define FILE_FULL_DIR_INFO_FORMAT_TEXT L"分配空间大小: %I64u, 数据大小: %I64u, 文件ID: %I64u, 索引号: %u, 扩展属性大小: %u, 修改时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后写入时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 生成时间: %04u-%02u-%02 %02u:%02u:%02u.%03u, 最后访问时间: %04u-%02u-%02 %02u:%02u:%02u.%03u" 
#define FILE_VALID_DATA_LENGHT_INFO_FORMAT_TEXT L"有效数据长度: %I64u"
#define FILE_ALTERNATE_NAME L"文件短名"
#define FILE_COMPLETION_NOTIFICATION_INFO_FORMAT_TEXT L"IO完成标志位: 0x%0.8x" 
#define FILE_STATUS_BLOCK_RANGE_INFO_FORMAT_TEXT L"文件块区域"
#define FILE_REMOTE_FILE_TEXT L"远程" 
#define FILE_LOCAL_FILE_TEXT L"本地"
#define FILE_STANDARD_LINK_INFO_FORMAT_TEXT L"可访问链接数: %u, 总链接数: %u, %s %s"

//L"远程文件"
#define _FILE_ATTRIBUTE_DIRECTORY_TEXT L"目录" 

#define _FILE_ATTRIBUTE_FILE_TEXT L"文件" 
#define FILE_DELETE_PENDING_TEXT L"标记删除"
#define FILE_INDEX_NUMBER_TEXT L"索引号: %I64u" 
#define FILE_EA_INFO_FORMAT_TEXT L"扩展属性长度: %u"

#define KEY_OPEN_FORMAT_TEXT L"权限 0x%0.8x\n"
#define KEY_MOVE_KEY_FORAT_TEXT L"新路径 %s \n"
#define KEY_REMOVE_VALUE_FORMAT_TEXT L"键值名 %s\n"
#define KEY_GET_VALUE_INFO_FORMAT_TEXT L"键值名 %s 类型 %u 长度 %u\n"
#define KEY_ENUM_INFO_FORMAT_TEXT L"键索引号: %u"
#define KEY_ENUM_VALUE_INFO_FORMAT_TEXT L"键索引号: %u"
#define KEY_SET_VALUE_INFO_FORMAT_TEXT L"键值名 %s 类型 %s 长度%u\n"
#define KEY_LOAD_FORMAT_TEXT L"hive 文件%s\n"
#define KEY_REPLACE_FORMAT_TEXT L"原HIVE文件 %s 新HIVE文件 %s\n" 
#define KEY_VALUE_PARTIAL_INFO_FORMAT_TEXT L"值类型: %s"
#define GET_PROCESS_OPEN_DETAIL_FORMAT_TEXT L"目标进程 %u 权限 %0.8x \n"
#define PROCESS_DEBUG_FORMAT_TEXT L"目标进程 %u\n"
#define PROCESS_SUSPEND_FORMAT_TEXT L"目标进程%s [%u] \n"
#define PROCESS_RESUME_FORMAT_TEXT L"恢复进程%s (ID:%u) 执行\n"
#define PROCESS_KILL_FORMAT_TEXT L"目标进程%s [%u] 退出代码%u\n"
#define PROCESS_JOB_FORMAT_TEXT L"进程工作组ID %u\n"
#define PROCESS_PAGE_PROTECT_FORMAT_TEXT L"目标进程ID %u 地址 0x%0.8x 长度:%u 属性 0x%0.8x 返回长度 %u\n"
#define THREAD_KILL_FORMAT_TEXT L"目标进程ID %u 线程ID %u 退出代码 0x%0.8x\n"
#define PROT_READ_FORMAT_TEXT L"端口:%s 读取数据%u字节\n"

#define PROT_WRITE_FORMAT_TEXT L"端口:%s 写入数据%u字节\n"
#define LOAD_KERNEL_MODULE_FORMAT_TEXT L"基地址 0x%0.8x 大小 %I64u"

#define ICMP_SEND_FORMAT_TEXT L"源IP地址 %s 目标IP地址 %s 类型 %u 代码 %u\n"
#define URB_INFO_FORMAT_TEXT L"%ws 长度:%u "
#define URB_FUNCTION_SELECT_CONFIGURATION_FORMAT_TEXT L"配置句柄:%p INTERFACE%u 配置值:0x%x 类型:%u 子类型:%u 协议:%u 接口句柄:%p Pipe数量:%u " 
#define URB_FUNCTION_SELECT_CONFIGURATION_PIPE_INFO_FORMAT_TEXT L"PIPE%u Max package size:%u PIPE address:%u Interval:%u Type:%u Handle:%p Max transfer size:%u Flags:0x%x " 
#define URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER_FORMAT_TEXT L"PIPE句柄:%p 传输标志:%u 长度:%u 缓存区:%p MDL:%p 下一个URB:%p HCD字段:%p%p%p%p%p%p%p%p " 
#define URB_FUNCTION_CONTROL_TRANSFER_FORMAT_TEXT L"PIPE句柄:%p 传输标志:%u 长度:%u 缓存区:%p MDL:%p 下一个URB:%p 配置字段:%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x HCD字段:%p%p%p%p%p%p%p%p " 

#define FILE_READ_FORMAT_TEXT _T( "Offset %I64u Size %u\n") //_T( "偏移量 %I64u 长度 %u\n")
#define NETWORK_SEND_FORMAT_TEXT _T( "Send (%s)dns request:%s\n" ) //_T( "发送 (%s)dns 请求:%s\n" )
#define NETWORK_HTTP_FORMAT_TEXT _T( "Protocol %u Command %u Command size %u \n" ); //_T( "协议 %u 命令 %u 命令长度 %u \n" );
#define NETWORK_CONNECT_FORMAT_TEXT _T( "Local %s:%u Remote %s:%u %s Port\n" ) //_T( "本地%s:%u 远程%s:%u %s端口\n" )

#define FILE_SFIO_VOLUME_INFORMATION_FORMAT_TEXT L"每周期最大请求数: %u, 最小周期长度: %u, 最小传输大小: %u" 
#define FILE_IO_PRIORITY_HINT_INFO_FORMAT_TEXT L"推荐IO优先级: %u" 
#define FILE_SFIO_RESERVE_INFO_FORMAT_TEXT L"每周期请求数: %u, 周期数: %u, 失败重试次数: %u, %s, 请求大小: %u, 排列请求数: %u" 
#define FILE_SFIO_DISCARDABLE_TEXT L"可丢弃" 
#define FILE_HARD_LINK_TEXT L"文件硬链接"
#define FILE_PROCESS_IDS_USING_FILE_INFO_TEXT1 L"进程"
#define FILE_PROCESS_IDS_USING_FILE_INFO_TEXT2 L"占用文件"
#define FILE_NORMALIZED_NAME_INFO_FORMAT_TEXT L"标准格式文件名: "
#define FILE_NETWORK_PHYSICAL_NAME_INFO_FORMAT_TEXT L"网络物理名 %s"
#define FILE_REMOTE_PROTOCOL_INFO_FORMAT_TEXT L"远程协议: %u, 远程协议 主版本:%u, 次版本:%u, 修定号: %u, 标志位: %u"
#define FILE_NUMA_NODE_INFO_FORMAT_TEXT L"NUMA结点号: %u"
#define FILE_ATTRIBUTE_CACHE_INFO_FORMAT_TEXT L"文件属性缓存: "
#define FILE_ID_GLOBAL_TX_DIR_INFO_FORMAT_TEXT L"文件ID 全局TX目录信息"  

#endif //LNAG_EN

LPCWSTR WINAPI get_return_code_desc( NTSTATUS return_code )
{
	LPCWSTR code_desc = L""; 

	switch( return_code )
	{
	case STATUS_INVALID_PARAMETER: 
			break; 
	default: 
		break; 
	}

	return code_desc; 
}

LRESULT WINAPI get_net_recv_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_accept_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR param = NULL; 

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

		*tip = L'\0'; 

	
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_net_send_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_create_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_net_dns_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		if( action->type != NET_dns )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = NETWORK_SEND_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			
			get_ip_proto_text( action->do_net_dns.protocol ), 
			action->do_net_dns.path_name ); 

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

LRESULT WINAPI get_net_http_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		fmt = NETWORK_HTTP_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_ba_extract_hidden_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	

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

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_extract_pe_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		if( action->type != BA_extract_pe )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_self_copy_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_self_delete_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_ulterior_exec_detail( sys_action_record *action, 
								  action_context *ctx, 
								  prepared_params_desc *params_desc, 
								  LPWSTR tip, 
								  ULONG ccb_buf_len, 
								  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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



		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_invade_process_detail( sys_action_record *action, 
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

		fmt = INVADE_PROCESS_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_ba_infect_pe_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		//fmt = L"感染(写入病毒代码)可执行(PE)文件 %s\n"; 

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_overwrite_pe_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		//fmt = L"修改可执行(PE)文件%s\n"; 

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_ba_register_autorun_detail( sys_action_record *action, 
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

		fmt = REGISTER_AUTORUN_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			//action->do_ba_register_autorun.path_name, 
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

LRESULT WINAPI get_file_remove_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 
	}while( FALSE );
	return ret; 
}

LRESULT WINAPI get_file_read_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		if( action->type != FILE_read )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = FILE_READ_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

	return ret; 
}

LRESULT WINAPI get_file_open_detail( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPCWSTR fmt; 
	LPWSTR text_output; 
	size_t remain_len; 

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

		text_output = tip; 
		remain_len = ccb_buf_len; 
		hr = StringCchCopyExW( text_output, 
			remain_len, 
			FILE_ATTRIBUTE_TEXT, 
			&text_output, 
			&remain_len, 
			0 ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

		ret = get_file_attr_desc( action->do_file_touch.attrib, text_output, remain_len, &text_output, &remain_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		fmt = FILE_OPEN_FORMAT_TEXT; 

		hr = StringCchPrintfExW( text_output, 
			remain_len, 
			&text_output, 
			&remain_len, 
			0, 
			fmt, 
			action->do_file_touch.access, 
			action->do_file_touch.alloc_size, 
			action->do_file_touch.share, 
			action->do_file_touch.disposition, 
			action->do_file_touch.options ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

	}while( FALSE );

	return ret; 
}


LRESULT WINAPI get_file_write_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		if( action->type != FILE_write )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = FILE_READ_FORMAT_TEXT; //_T( "偏移量 %I64u 长度 %u\n"); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

	return ret; 
}

LRESULT WINAPI get_reg_mk_key_detail( sys_action_record *action, 
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

		if( action->type != REG_mkkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = REG_CREATE_KEY_FORMAT_TEXT; 

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_proc_com_access_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_readvm_detail( sys_action_record *action, 
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

		fmt = _T( "进程ID %u 地址 0x%0.8x 长度 %u 返回长度 %u\n" ); 

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
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
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_proc_exec_detail( sys_action_record *action, 
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

		if( action->type != PROC_exec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = _T( "进程ID %u\n" ); 

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_proc_exec.target_pid ); 

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

LRESULT WINAPI get_w32_msg_hook_detail( sys_action_record *action, 
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

		if( action->type != W32_msghook )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		fmt = _T( "目标进程ID %u 类型 %u 钩子函数地址 0x%0.8x 钩子模块基址 0x%0.8x\n" ); 

		_ret = _sntprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_msghook.hookfunc, 
			action->do_w32_msghook.hooktype, 
			action->do_w32_msghook.target_pid, 
			action->do_w32_msghook.modbase ); 

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

LRESULT WINAPI get_w32_lib_inject_detail( sys_action_record *action, 
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
		
		fmt = INJECT_LIB_FORMAT_TEXT; 

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

LRESULT WINAPI get_net_connect_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		fmt = NETWORK_CONNECT_FORMAT_TEXT; 

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

LRESULT WINAPI get_net_listen_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_exec_create_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR cmd_line; 

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

		fmt = _T( "命令行:%s 父进程ID:%u 进程ID:%u 基地址:%u" ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			cmd_line = action->do_exec_create.cmd_line_ptr; 
		}
		else
		{
			cmd_line = action->do_exec_create.path_name + action->do_exec_create.path_len + 1; 
		}

		if( cmd_line != NULL )
		{
			ASSERT( cmd_line[ action->do_exec_create.cmd_len] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_create.cmd_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			cmd_line, 
			action->do_exec_create.parent_pid, 
			action->do_exec_create.pid, 
			action->do_exec_create.image_base ); 

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

LRESULT WINAPI get_exec_module_load_detail( sys_action_record *action, 
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

		if( action->type != EXEC_module_load )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}


		fmt = MODULE_LOAD_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_exec_module_load.base, 
			action->do_exec_module_load.size, 
			action->do_exec_module_load.flags ); 

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

LRESULT WINAPI get_exec_destroy_detail( sys_action_record *action, 
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
	LPCWSTR cmd_line; 

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

		fmt =  _T( "命令行 %s 父进程ID %u 进程ID %u\n" ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			cmd_line = action->do_exec_destroy.cmd_line_ptr; 
		}
		else
		{
			cmd_line = action->do_exec_destroy.path_name + action->do_exec_destroy.path_len + 1; 
		}

		if( cmd_line != NULL )
		{
			ASSERT( cmd_line[ action->do_exec_destroy.cmd_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_exec_destroy.cmd_len == 0 ); 
		}

		if( cmd_line[ action->do_exec_destroy.cmd_len ] != L'\0' )
		{
			( ( WCHAR* )cmd_line )[ action->do_exec_destroy.cmd_len ] = L'\0'; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			cmd_line, 
			action->do_exec_destroy.parent_pid, 
			action->do_exec_destroy.pid ); 

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

LRESULT WINAPI get_file_touch_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS;  
	HRESULT hr; 
	LPCWSTR fmt; 
	LPWSTR text_output; 
	size_t remain_len; 

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

		//{
		//	//ULONG pid; //动作发起者进程ID 
		//	//ULONG tid; //动作发起者线程ID 
		//	ULONG access; //文件访问权限 
		//	ULONG alloc_size; //文件初始长度 
		//	ULONG attrib; //文件属性 
		//	ULONG share; //文件共享属性 
		//	ULONG disposition; //文件打开/创建选项 
		//	ULONG options; //文件打开/创建选项 
		//	//ULONG result; //动作完成结果(NTSTATUS) 
		//	PATH_SIZE_T path_len; 
		//	union
		//	{
		//		WCHAR path_name[ 1 ]; //文件全路径 

		//		struct 
		//		{
		//			WCHAR *path_name_ptr; 
		//		};
		//	}; 
		//	//WCHAR path_name[ 1 ]; //文件全路径 

		text_output = tip; 
		remain_len = ccb_buf_len; 

		hr = StringCchCopyExW( text_output, 
			remain_len, 
			FILE_ATTRIBUTE_TEXT, 
			&text_output, 
			&remain_len, 
			0 ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

		fmt = FILE_OPEN_FORMAT_TEXT; //L"访问权限 0x%0.8x 初始大小 %u 共享属性 0x%0.8x 工作方式(disposition) 0x0.8x 工作方式(option) 0x%0.8x \n"; 

		hr = StringCchPrintfExW( text_output, 
			remain_len, 
			&text_output, 
			&remain_len, 
			0, 
			fmt, 
			action->do_file_touch.access, 
			action->do_file_touch.alloc_size, 
			action->do_file_touch.attrib, 
			action->do_file_touch.share, 
			action->do_file_touch.disposition, 
			action->do_file_touch.options ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_file_modified_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		tip[ 0 ] = L'\0'; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_readdir_detail( sys_action_record *action, 
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

		if( action->type != FILE_readdir )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = FILE_READ_DIR_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_file_readdir.filter ); 

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

LRESULT WINAPI get_file_rename_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR new_name; 
	LPCWSTR fmt; 
	ULONG _name_len; 
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

		fmt = FILE_RENAME_FORMAT_TEXT; 

		//ULONG replace_existing; //是否覆盖已存在文件 
		//ULONG result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T new_name_len; 
		//WCHAR path_name[ 1 ]; //文件全路径 

		_name_len = action->do_file_rename.path_len; 

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

		_name_len = action->do_file_rename.new_name_len; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			new_name = action->do_file_rename.new_name_ptr; 
		}
		else
		{
			new_name = action->do_file_rename.path_name + action->do_file_rename.path_len + 1; 
		}

		if( new_name != NULL )
		{
			ASSERT( new_name[ action->do_file_rename.new_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_file_rename.new_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			new_name, 
			path_name, 
			action->do_file_rename.replace_existing ?  FILE_RENAME_REPLACE_TEXT: FILE_RENAME_NOT_REPLACE_TEXT ); 

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

LRESULT WINAPI get_file_truncate_detail( sys_action_record *action, 
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

		if( action->type != FILE_truncate )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = FILE_TRUNCATE_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

	return ret; 
}

LRESULT WINAPI get_file_mklink_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR fmt; 
	LPCWSTR link_name; 
	ULONG _name_len; 

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

		fmt = FILE_MAKE_LINK_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			link_name = action->do_file_mklink.link_name_ptr; 
		}
		else
		{
			link_name = action->do_file_mklink.path_name + action->do_file_mklink.path_len + 1; 
		}

		_name_len = action->do_file_mklink.link_name_len; 

		if( link_name != NULL )
		{
			ASSERT( link_name[ _name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( _name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			link_name, 
			action->do_file_mklink.flags ); 

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

LRESULT WINAPI get_file_chmod_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR text_output; 
	size_t remain_len; 

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

		//param = ( TCHAR* )malloc( sizeof( TCHAR ) * MAX_PATH ); 
		//if( param == NULL )
		//{
		//	ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	break; 
		//}

		//ULONG attrib; //文件属性 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //文件全路径 

		text_output = tip; 
		remain_len = ccb_buf_len; 

		hr = StringCchCopyExW( text_output, 
			remain_len, 
			FILE_NEW_ATTRIBUTE_TEXT, 
			&text_output, 
			&remain_len, 
			0 ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			break; 
		}

		ret = get_file_attr_desc( action->do_file_chmod.attrib, 
			text_output, 
			remain_len, 
			&text_output, 
			&remain_len ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_attr_desc( ULONG file_attr, LPWSTR text, ULONG cc_buf_len, LPWSTR *text_remain, size_t *cc_remain_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR _text; 
	size_t cc_remain_buf_len; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( cc_buf_len > 0 ); 
		ASSERT( text_remain != NULL ); 
		ASSERT( cc_remain_len != NULL ); 

		_text = text; 
		cc_remain_buf_len = cc_buf_len; 

		do 
		{
			if( file_attr & FILE_ATTRIBUTE_READONLY )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_READ_ONLY_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_HIDDEN )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_HIDE_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_SYSTEM )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_SYSTEM_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_DIRECTORY )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_DIRECTORY_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_ARCHIVE )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_ARCHIVE_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_DEVICE )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_DEVICE_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_NORMAL )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_GENERAL_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_TEMPORARY )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_TEMPORARY_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_SPARSE_FILE )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_SPARSE_FILE_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_REPARSE_POINT )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_REPARSE_POINT_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_COMPRESSED )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_COMPRESSED_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_OFFLINE )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_OFFLINE_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_ENCRYPTED )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_ENCRYPTED_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}

			if( file_attr & FILE_ATTRIBUTE_VIRTUAL )
			{
				hr = StringCchCatExW( text, cc_remain_buf_len, FILE_ATTRIBUTE_VIRTUAL_TEXT, &_text, &cc_remain_buf_len, 0 ); 
				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}
		}while( FALSE );

		*cc_remain_len = cc_remain_buf_len; 
		*text_remain = _text; 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_getinfo_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 

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

		if( action->type != FILE_getinfo )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 

		if( ctx->last_result == STATUS_SUCCESS 
			|| ctx->last_result == STATUS_BUFFER_OVERFLOW )
		{
			ULONG valid_info_size; 
			ASSERT( action->do_file_getinfo.info_size > 0 ); 

			valid_info_size = action->size - 
				( ( ( action->do_file_getinfo.path_len + 1 ) << 1 ) + 
				ACTION_RECORD_SIZE_BY_TYPE( file_getinfo ) ); 

			if( action->do_file_getinfo.info_size < valid_info_size ) 
			{
				valid_info_size = action->do_file_getinfo.info_size; 
			}
			
			try
			{
				ret = get_file_info_desc( ( FILE_INFORMATION_CLASS )action->do_file_getinfo.info_class, 
					( action->do_file_getinfo.path_name + action->do_file_getinfo.path_len + 1 ), 
					valid_info_size, 
					tip, 
					ccb_buf_len, 
					&cc_ret_len ); 
			}
			catch( ... )
			{
				DBG_BP(); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}
		else
		{
			ASSERT( action->do_file_getinfo.info_size == 0 ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_access_mask_desc( LPWSTR text, ULONG cc_buf_len, ULONG access_flags, ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR text_output; 
	size_t remain_len; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( cc_buf_len > 0 ); 
		ASSERT( cc_ret_len != NULL ); 

		text_output = text; 
		remain_len = cc_buf_len; 

		if( access_flags & GENERIC_ALL )
		{
			access_flags = ( FILE_GENERIC_EXECUTE | FILE_GENERIC_READ | FILE_GENERIC_WRITE ); 
		}
		else
		{
			access_flags = 0; 
			if( access_flags & GENERIC_READ )
			{
				access_flags |= FILE_GENERIC_READ; 
			}

			if( access_flags & GENERIC_WRITE )
			{
				access_flags |= FILE_GENERIC_WRITE; 
			}

			if( access_flags & GENERIC_EXECUTE )
			{
				access_flags |= FILE_GENERIC_EXECUTE; 
			}
		}

		*text_output = L'\0'; 

		if( access_flags & FILE_READ_DATA )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_READ_DATA_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_LIST_DIRECTORY )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_LIST_DIRECTORY_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_WRITE_DATA )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_WRITE_DATA_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_ADD_FILE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ADD_FILE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_APPEND_DATA )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_APPEND_DATA_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_ADD_SUBDIRECTORY )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ADD_SUBDIRECTORY_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_CREATE_PIPE_INSTANCE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_CREATE_PIPE_INSTANCE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_READ_EA )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_FILE_READ_EA, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_WRITE_EA )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_FILE_WRITE_EA, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_EXECUTE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_EXECUTE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_TRAVERSE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_TRAVERSE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_DELETE_CHILD )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_DELETE_CHILD_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_READ_ATTRIBUTES )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_READ_ATTRIBUTES_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & FILE_WRITE_ATTRIBUTES )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_WRITE_ATTRIBUTES_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & SYNCHRONIZE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_SYNCHRONIZE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & DELETE )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_DELETE_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & READ_CONTROL )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_READ_CONTROL_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & WRITE_DAC )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_WRITE_DAC_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		if( access_flags & WRITE_OWNER )
		{
			hr = StringCchCatExW( text_output, remain_len, FILE_ACCESS_WRITE_OWNER_TEXT, &text_output, &remain_len, 0 ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_BUFFER_TOO_SMALL; 
				break; 
			}
		}

		text[ cc_buf_len - remain_len - 1 ] = L'\0'; 

		*cc_ret_len = cc_buf_len - remain_len - 1; 

	}while( FALSE );

	return ret; 
}

typedef struct _FILE_ALTERNATE_NAME_INFORMATION
{
	UNICODE_STRING ShortName; 
	WCHAR ShortNameBuf[ 8 + 1 + 3 ]; 
	ULONG ShortNameLength; 
}FILE_ALTERNATE_NAME_INFORMATION, *PFILE_ALTERNATE_NAME_INFORMATION; 

LPCWSTR WINAPI get_file_compress_format_desc( ULONG format )
{
	LPCWSTR format_desc = L""; 

	switch( format )
	{
	case COMPRESSION_FORMAT_NONE: 
		format_desc = L"COMPRESSION_FORMAT_NONE"; 
		break; 
	case COMPRESSION_FORMAT_DEFAULT: 
		format_desc = L"COMPRESSION_FORMAT_DEFAULT"; 
		break; 
	case COMPRESSION_FORMAT_LZNT1: 
		format_desc = L"COMPRESSION_FORMAT_LZNT1"; 
		break; 
	//case COMPRESSION_ENGINE_STANDARD: 
	//	format_desc = L"COMPRESSION_ENGINE_STANDARD"; 
	//	break; 
	case COMPRESSION_ENGINE_MAXIMUM: 
		format_desc = L"COMPRESSION_ENGINE_MAXIMUM"; 
		break; 
	case COMPRESSION_ENGINE_HIBER: 
		format_desc = L"COMPRESSION_ENGINE_HIBER"; 
		break; 
	default: 
		break; 
	}

	return format_desc; 
}

LRESULT WINAPI get_sid_string( SID *sid, LPWSTR sid_string, ULONG cc_buf_len, ULONG *cc_ret_len )
{
	HRESULT hr; 
	LPWSTR text_output; 
	size_t remain_len; 
	LRESULT ret; 

	ASSERT( sid != NULL ); 
	ASSERT( sid_string != NULL ); 
	ASSERT( cc_buf_len > 0 ); 

	do 
	{
		text_output = sid_string; 
		remain_len = cc_buf_len; 

		hr = StringCchPrintfExW( text_output, 
			remain_len, 
			NULL, 
			&remain_len, 
			0, 
			L"%0.2x%0.2x%0.2x%0.2x%0.2x%0.2x", 
			sid->Revision,
			sid->IdentifierAuthority.Value[ 0 ], 
			sid->IdentifierAuthority.Value[ 1 ], 
			sid->IdentifierAuthority.Value[ 2 ], 
			sid->IdentifierAuthority.Value[ 3 ], 
			sid->IdentifierAuthority.Value[ 4 ], 
			sid->IdentifierAuthority.Value[ 5 ] ); 

		if( FAILED( hr ) ) 
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		{
			INT32 i; 
			
			for( i = 0; i < sid->SubAuthorityCount; i ++ )
			{
				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					L"%0.2x", 
					sid->SubAuthority[ i ] ); 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

INLINE VOID set_time_to_null( SYSTEMTIME *time )
{
	ASSERT( time != NULL ); 

	memset( time, 0, sizeof( *time ) ); 
}

LRESULT WINAPI get_file_info_desc( FILE_INFORMATION_CLASS info_class, 
								  PVOID info, 
								  ULONG info_size, 
								  LPWSTR text, 
								  ULONG cc_buf_len, 
								  ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		ASSERT( info != NULL ); 
		ASSERT( text != NULL ); 
		ASSERT( cc_buf_len > 0 ); 
		ASSERT( cc_ret_len != NULL ); 

		switch( info_class )
		{
		case FileDirectoryInformation: //         = 1, 
			{
				HRESULT hr; 
				WCHAR *text_output; 
				size_t remain_len; 
				FILE_DIRECTORY_INFORMATION *file_dir_info; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				file_dir_info = ( FILE_DIRECTORY_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				for( ; ; )
				{
					do 
					{
						_ret = FileTimeToSystemTime( ( FILETIME* )&file_dir_info->CreationTime, &creation_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &creation_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_dir_info->LastAccessTime, &last_access_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &last_access_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_dir_info->ChangeTime, &change_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &change_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_dir_info->LastWriteTime, &write_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &write_time ); 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_NAME_FORMAT_TEXT, 
							file_dir_info->FileName ); 


						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						ret = get_file_attr_desc( file_dir_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_DIRECTORY_INFO_FORMAT_TEXT, 
							file_dir_info->EndOfFile.QuadPart, 
							file_dir_info->AllocationSize.QuadPart, 
							file_dir_info->FileIndex, 

							change_time.wYear, 
							change_time.wMonth, 
							change_time.wDay, 
							change_time.wHour, 
							change_time.wMinute, 
							change_time.wSecond, 
							change_time.wMilliseconds, 

							write_time.wYear, 
							write_time.wMonth, 
							write_time.wDay, 
							write_time.wHour, 
							write_time.wMinute, 
							write_time.wSecond, 
							write_time.wMilliseconds, 

							creation_time.wYear, 
							creation_time.wMonth, 
							creation_time.wDay, 
							creation_time.wHour, 
							creation_time.wMinute, 
							creation_time.wSecond, 
							creation_time.wMilliseconds, 

							last_access_time.wYear, 
							last_access_time.wMonth, 
							last_access_time.wDay, 
							last_access_time.wHour, 
							last_access_time.wMinute, 
							last_access_time.wSecond, 
							last_access_time.wMilliseconds ); 

						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

					}while( FALSE );

					if( file_dir_info->NextEntryOffset == 0 )
					{
						break; 
					}

					file_dir_info = ( FILE_DIRECTORY_INFORMATION* )( ( BYTE* )file_dir_info + file_dir_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileFullDirectoryInformation: // 2
			{
				FILE_FULL_DIR_INFORMATION *file_full_dir_info; 
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				text_output = text; 
				remain_len = cc_buf_len; 

				file_full_dir_info = ( FILE_FULL_DIR_INFORMATION* )info; 

				for( ; ; )
				{
					do 
					{
						_ret = FileTimeToSystemTime( ( FILETIME* )&file_full_dir_info->CreationTime, &creation_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &creation_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_full_dir_info->LastAccessTime, &last_access_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &last_access_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_full_dir_info->ChangeTime, &change_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &change_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_full_dir_info->LastWriteTime, &write_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &write_time ); 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_NAME_FORMAT_TEXT, 
							file_full_dir_info->FileName ); 


						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						ret = get_file_attr_desc( file_full_dir_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_DIRECTORY_INFO_FORMAT_TEXT, //L"分配空间:%I64u 数据大小:%I64u 索引号:%u 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u 最后写入时间:%04u-%02u-%02u %02u:%02u:%02u.%03u 生成时间:%04u-%02u-%02u %02u:%02u:%02u.%03u 最后访问时间:%04u-%02u-%02u %02u:%02u:%02u.%03u\n", 
							file_full_dir_info->AllocationSize.QuadPart, 
							file_full_dir_info->EndOfFile.QuadPart, 

							file_full_dir_info->FileAttributes, 
							file_full_dir_info->FileIndex, 

							change_time.wYear, 
							change_time.wMonth, 
							change_time.wDay, 
							change_time.wHour, 
							change_time.wMinute, 
							change_time.wSecond, 
							change_time.wMilliseconds, 

							write_time.wYear, 
							write_time.wMonth, 
							write_time.wDay, 
							write_time.wHour, 
							write_time.wMinute, 
							write_time.wSecond, 
							write_time.wMilliseconds, 

							creation_time.wYear, 
							creation_time.wMonth, 
							creation_time.wDay, 
							creation_time.wHour, 
							creation_time.wMinute, 
							creation_time.wSecond, 
							creation_time.wMilliseconds, 
							
							last_access_time.wYear, 
							last_access_time.wMonth, 
							last_access_time.wDay, 
							last_access_time.wHour, 
							last_access_time.wMinute, 
							last_access_time.wSecond, 
							last_access_time.wMilliseconds ); 

						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW;  
							break; 
						}

					}while( FALSE ); 

					if( 0 == file_full_dir_info->NextEntryOffset )
					{
						break; 
					}

					file_full_dir_info = ( FILE_FULL_DIR_INFORMATION* )( ( BYTE* )file_full_dir_info + file_full_dir_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileBothDirectoryInformation: // 3
			{
				FILE_BOTH_DIRECTORY_INFORMATION *file_both_dir_info; 
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				file_both_dir_info = ( FILE_BOTH_DIRECTORY_INFORMATION* )info; 
				text_output = text; 
				remain_len = cc_buf_len; 


				for( ; ; )
				{
					if( 0 == file_both_dir_info->NextEntryOffset )
					{
						break; 
					}

					do 
					{
						_ret = FileTimeToSystemTime( ( FILETIME* )&file_both_dir_info->CreationTime, &creation_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &creation_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_both_dir_info->LastAccessTime, &last_access_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &last_access_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_both_dir_info->ChangeTime, &change_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &change_time ); 
						}

						_ret = FileTimeToSystemTime( ( FILETIME* )&file_both_dir_info->LastWriteTime, &write_time ); 
						if( FALSE == _ret )
						{
							set_time_to_null( &write_time ); 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_NAME_FORMAT_TEXT, 
							file_both_dir_info->FileName ); 


						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						ret = get_file_attr_desc( file_both_dir_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_BOTH_DIRECTORY_INFO_FORMAT_TEXT, 
							file_both_dir_info->AlternateName, 

							file_both_dir_info->AllocationSize.QuadPart, 
							file_both_dir_info->EndOfFile.QuadPart, 

							file_both_dir_info->EaInformationLength, 

							creation_time.wYear, 
							creation_time.wMonth, 
							creation_time.wDay, 
							creation_time.wHour, 
							creation_time.wMinute, 
							creation_time.wSecond, 
							creation_time.wMilliseconds, 

							last_access_time.wYear, 
							last_access_time.wMonth, 
							last_access_time.wDay, 
							last_access_time.wHour, 
							last_access_time.wMinute, 
							last_access_time.wSecond, 
							last_access_time.wMilliseconds, 

							change_time.wYear, 
							change_time.wMonth, 
							change_time.wDay, 
							change_time.wHour, 
							change_time.wMinute, 
							change_time.wSecond, 
							change_time.wMilliseconds, 

							write_time.wYear, 
							write_time.wMonth, 
							write_time.wDay, 
							write_time.wHour, 
							write_time.wMinute, 
							write_time.wSecond, 
							write_time.wMilliseconds ); 

					}while( FALSE ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
						break; 
					}

					file_both_dir_info = ( FILE_BOTH_DIRECTORY_INFORMATION* )( ( BYTE* )file_both_dir_info + file_both_dir_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}	
			break; 
		case FileBasicInformation: // 4
			{
				FILE_BASIC_INFORMATION *basic_info; 
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				basic_info = ( FILE_BASIC_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				_ret = FileTimeToSystemTime( ( FILETIME* )&basic_info->CreationTime, &creation_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &creation_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&basic_info->LastAccessTime, &last_access_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &last_access_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&basic_info->ChangeTime, &change_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &change_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&basic_info->LastWriteTime, &write_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &write_time ); 
				}

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					FILE_ATTRIBUTE_TEXT, 
					&text_output, 
					&remain_len, 
					0 ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}

				ret = get_file_attr_desc( basic_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					FILE_BASIC_INFO_FORMAT_TEXT, 

					change_time.wYear, 
					change_time.wMonth, 
					change_time.wDay, 
					change_time.wHour, 
					change_time.wMinute, 
					change_time.wSecond, 
					change_time.wMilliseconds, 

					write_time.wYear, 
					write_time.wMonth, 
					write_time.wDay, 
					write_time.wHour, 
					write_time.wMinute, 
					write_time.wSecond, 
					write_time.wMilliseconds, 

					creation_time.wYear, 
					creation_time.wMonth, 
					creation_time.wDay, 
					creation_time.wHour, 
					creation_time.wMinute, 
					creation_time.wSecond, 
					creation_time.wMilliseconds, 

					last_access_time.wYear, 
					last_access_time.wMonth, 
					last_access_time.wDay, 
					last_access_time.wHour, 
					last_access_time.wMinute, 
					last_access_time.wSecond, 
					last_access_time.wMilliseconds ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileStandardInformation: // 5
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_STANDARD_INFORMATION *standard_info; 

				standard_info = ( FILE_STANDARD_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_STANDARD_INFO_FORMAT_TEXT, 
					standard_info->Directory == TRUE ? _FILE_ATTRIBUTE_DIRECTORY_TEXT : _FILE_ATTRIBUTE_FILE_TEXT, 
					standard_info->AllocationSize.QuadPart, 
					standard_info->EndOfFile.QuadPart, 
					standard_info->NumberOfLinks, 
					standard_info->DeletePending == TRUE ? FILE_DELETE_PENDING_TEXT : L"" ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{ 
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileInternalInformation: // 6
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_INTERNAL_INFORMATION *internal_info; 

				internal_info = ( FILE_INTERNAL_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_INDEX_NUMBER_TEXT, 
					internal_info->IndexNumber.QuadPart ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileEaInformation: // 7
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_EA_INFORMATION *ea_info; 

				ea_info = ( FILE_EA_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_EA_INFO_FORMAT_TEXT, 
					ea_info->EaSize ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileAccessInformation: // 8
			{
				HRESULT hr; 
				FILE_ACCESS_INFORMATION *access_info; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 

				access_info = ( FILE_ACCESS_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					FILE_ACCESS_TEXT, 
					&text_output, 
					&remain_len, 
					0 ); 

				if( FAILED( hr ) )
				{
					*cc_ret_len = _cc_ret_len + cc_buf_len - remain_len; // + 1; 

					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}

				ret = get_access_mask_desc( text_output, 
					( ULONG )remain_len, 
					access_info->AccessFlags, 
					&_cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					*cc_ret_len = _cc_ret_len + cc_buf_len - remain_len; // + 1; 
					break; 
				}

				*cc_ret_len = _cc_ret_len + cc_buf_len - remain_len; // + 1; 
			}
			break; 
		case FileNameInformation: // 9
			{
				HRESULT hr; 
				FILE_NAME_INFORMATION *name_info; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 

				name_info = ( FILE_NAME_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					FILE_NAME_TEXT, 
					&text_output, 
					&remain_len, 
					0 ); 

				if( FAILED( hr ) )
				{
					*cc_ret_len = cc_buf_len - remain_len; // + 1; 

					ret = ERROR_BUFFER_OVERFLOW;  
					break; 
				}

				WCHAR old_ch; 

				ULONG max_name_len; 
				ULONG cc_name_len; 
				max_name_len = ( info_size - FIELD_OFFSET( FILE_NAME_INFORMATION, FileName ) ); 

				if( max_name_len >= name_info->FileNameLength )
				{
					cc_name_len = ( name_info->FileNameLength >> 1 ); 

					if( name_info->FileName[ cc_name_len - 1 ] != L'\0' )
					{
						if( cc_name_len + 1 <= ( max_name_len >> 1 ) )
						{
						}
						else
						{
							cc_name_len -= 1; 
						}
					}
					else
					{
						cc_name_len -= 1; 
					}
				}
				else
				{
					cc_name_len = ( max_name_len >> 1 ); 
					cc_name_len -= 1; 
				}

				old_ch = name_info->FileName[ cc_name_len ]; 
				name_info->FileName[ ( cc_name_len ) ] = L'\0'; 

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					name_info->FileName, 
					&text_output, 
					&remain_len, 
					0 ); 

				name_info->FileName[ cc_name_len ] = old_ch ; 

				if( FAILED( hr ) )
				{
					*cc_ret_len = _cc_ret_len + cc_buf_len - remain_len; // + 1; 

					ret = ERROR_BUFFER_OVERFLOW;  
					break; 
				}

				*cc_ret_len = _cc_ret_len + cc_buf_len - remain_len; // + 1; 
			}
			break; 
		case FileRenameInformation: // 10
			{
			}
			break; 
		case FileLinkInformation: // 11
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_LINK_INFORMATION *link_info; 

				link_info = ( FILE_LINK_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_LINK_INFO_FORMAT_TEXT, 
					link_info->FileName, 
					link_info->ReplaceIfExists == FALSE ? FILE_LINK_NOT_REPLACE_TEXT : FILE_LINK_REPLACE_TEXT ); 
				
				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					if( ret != ERROR_BUFFER_TOO_SMALL )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
					}

					break; 
				}
			}
			break; 
		case FileNamesInformation: // 12
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_NAMES_INFORMATION *names_info; 
				LPWSTR text_output; 

				names_info = ( FILE_NAMES_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				for( ; ; )
				{
					if( 0 == names_info->NextEntryOffset )
					{
						break; 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						FILE_NAMES_INFO_FORMAT_TEXT, 
						names_info->FileName, 
						names_info->FileIndex ); 
					
					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
						break; 
					}

					names_info = ( FILE_NAMES_INFORMATION* )( ( BYTE* )names_info + names_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileDispositionInformation: // 13
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_DISPOSITION_INFORMATION *disp_info; 

				disp_info = ( FILE_DISPOSITION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					L"%s", 
					disp_info->DeleteFile == TRUE ? FILE_DELETE_TEXT : L"" ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FilePositionInformation: // 14
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_POSITION_INFORMATION *pos_info; 

				pos_info = ( FILE_POSITION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_POSITION_INFO_FORMAT_TEXT, 
					pos_info->CurrentByteOffset.QuadPart ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileFullEaInformation: // 15
			{
				HRESULT hr; 
				FILE_FULL_EA_INFORMATION *full_ea_info; 
				LPWSTR text_output; 
				size_t remain_len; 

				full_ea_info = ( FILE_FULL_EA_INFORMATION* )info; 

				for( ; ; )
				{
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						FILE_FULL_EA_INFO_FORMAT_TEXT, 
						full_ea_info->EaName, 
						full_ea_info->EaValueLength, 
						full_ea_info->Flags ); 
				
					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
						break; 
					}

					if( 0 == full_ea_info->NextEntryOffset )
					{
						break; 
					}

					full_ea_info = ( FILE_FULL_EA_INFORMATION* )( ( BYTE* )full_ea_info + full_ea_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileModeInformation: // 16
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_MODE_INFORMATION* mode_info; 

				mode_info = ( FILE_MODE_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_MODE_TEXT, 
					mode_info->Mode ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileAlignmentInformation: // 17
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_ALIGNMENT_INFORMATION* align_info; 

				align_info = ( FILE_ALIGNMENT_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_ALIGNMENT_INFO_FORMAT_TEXT, 
					align_info->AlignmentRequirement ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileAllInformation: // 18
			{
				HRESULT hr; 
				size_t cc_remain_len; 
				LPWSTR _text; 
				INT32 _ret; 
				FILE_ALL_INFORMATION *all_info; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME write_time; 
				SYSTEMTIME change_time; 

				all_info = ( FILE_ALL_INFORMATION* )info; 

				if( info_size < sizeof( FILE_ALL_INFORMATION ) )
				{
					*text = L'\0'; 
					*cc_ret_len = 0; 

					dbg_print( MSG_FATAL_ERROR, "the full file information size is too small( < sizeof(FILE_ALL_INFORMATION)\n" ); 
					break; 
				}
				else
				{
					*( LPWSTR )( ( BYTE* )all_info + ( info_size & 0xfffffffe ) - sizeof( WCHAR ) ) = L'\0'; 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&all_info->BasicInformation.CreationTime, &creation_time ); 
				if( _ret == FALSE )
				{
					set_time_to_null( &creation_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&all_info->BasicInformation.LastAccessTime, &last_access_time ); 
				if( _ret == FALSE )
				{
					set_time_to_null( &last_access_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&all_info->BasicInformation.LastWriteTime, &write_time ); 
				if( _ret == FALSE )
				{
					set_time_to_null( &write_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&all_info->BasicInformation.ChangeTime, &change_time ); 
				if( _ret == FALSE )
				{
					set_time_to_null( &change_time ); 
				}

				{
					//不可能小于0
					ULONG cc_max_name_len; 

					cc_max_name_len = ( ULONG )( ( info_size 
						- FIELD_OFFSET( FILE_ALL_INFORMATION, NameInformation ) 
						+ FIELD_OFFSET( FILE_NAME_INFORMATION, FileName ) ) >> 1 ); 

					if( all_info->NameInformation.FileNameLength > ( cc_max_name_len << 1 ) )
					{
						all_info->NameInformation.FileNameLength = ( cc_max_name_len << 1 ); 
					}

					ASSERT( cc_max_name_len >= 1 ); 
				}

				do 
				{
					_text = text; 
					cc_remain_len = cc_buf_len; 

					hr = StringCchCopyExW( _text, 
						cc_remain_len, 
						FILE_NAME_TEXT, 
						&_text, 
						&cc_remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						*cc_ret_len = cc_buf_len - cc_remain_len; 
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( all_info->NameInformation.FileNameLength > 0 )
					{
						if( cc_remain_len < ( all_info->NameInformation.FileNameLength >> 1 ) + 1 )
						{
							memcpy( _text, all_info->NameInformation.FileName, ( cc_remain_len << 1 ) - sizeof( WCHAR ) ); 
							_text[ cc_remain_len - 1 ] = L'\0'; 
							cc_remain_len = 0; 
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						memcpy( _text, all_info->NameInformation.FileName, all_info->NameInformation.FileNameLength ); 

						if( _text[ ( all_info->NameInformation.FileNameLength >> 1 ) - 1 ] == L'\0' )
						{
							cc_remain_len -= ( ( all_info->NameInformation.FileNameLength >> 1 ) - 1 ); 
							_text += ( ( all_info->NameInformation.FileNameLength >> 1 ) - 1 ); 						
						}
						else
						{
							cc_remain_len -= ( all_info->NameInformation.FileNameLength >> 1 ); 
							_text += ( all_info->NameInformation.FileNameLength >> 1 ); 
						}
					}

					hr = StringCchPrintfExW( _text, 
						cc_remain_len, 
						NULL, 
						&cc_remain_len, 
						0, 
						FILE_ALL_INFO_FORMAT_TEXT, 
						all_info->ModeInformation.Mode, 
						all_info->AccessInformation.AccessFlags, 
						all_info->AlignmentInformation.AlignmentRequirement, 
						all_info->StandardInformation.AllocationSize.QuadPart, 
						all_info->StandardInformation.EndOfFile.QuadPart, 
						all_info->PositionInformation.CurrentByteOffset.QuadPart, 

						change_time.wYear, 
						change_time.wMonth, 
						change_time.wDay, 
						change_time.wHour, 
						change_time.wMinute, 
						change_time.wSecond, 
						change_time.wMilliseconds, 

						write_time.wYear, 
						write_time.wMonth, 
						write_time.wDay, 
						write_time.wHour, 
						write_time.wMinute, 
						write_time.wSecond, 
						write_time.wMilliseconds, 

						creation_time.wYear, 
						creation_time.wMonth, 
						creation_time.wDay, 
						creation_time.wHour, 
						creation_time.wMonth, 
						creation_time.wSecond, 
						creation_time.wMilliseconds, 

						last_access_time.wYear, 
						last_access_time.wMonth, 
						last_access_time.wDay, 
						last_access_time.wHour, 
						last_access_time.wMinute, 
						last_access_time.wSecond, 
						last_access_time.wMilliseconds, 

						all_info->EaInformation.EaSize, 
						all_info->InternalInformation.IndexNumber.QuadPart, 
						all_info->StandardInformation.NumberOfLinks, 
						all_info->StandardInformation.DeletePending == TRUE ? FILE_DELETED_TEXT : L"", 
						all_info->StandardInformation.Directory == TRUE ? FILE_DIRECTORY_TEXT : L"" ); 

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - cc_remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileAllocationInformation: // 19
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_ALLOCATION_INFORMATION *alloc_info; 

				alloc_info = ( FILE_ALLOCATION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_ALLOCATION_INFO_FORMAT_TEXT, 
					alloc_info->AllocationSize.QuadPart ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileEndOfFileInformation: // 20
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_END_OF_FILE_INFORMATION *end_of_file_info; 

				end_of_file_info = ( FILE_END_OF_FILE_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_END_OF_FILE_INFO_FORMAT_TEXT, 
					end_of_file_info->EndOfFile.QuadPart ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileAlternateNameInformation: // 21
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_ALTERNATE_NAME_INFORMATION *alternate_name_info; 

				alternate_name_info = ( FILE_ALTERNATE_NAME_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_ALTERNATE_NAME_INFO_FORMAT_TEXT, 
					alternate_name_info->ShortNameBuf ); 
				
				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileStreamInformation: // 22
			{
				HRESULT hr; 
				FILE_STREAM_INFORMATION *stream_info; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG remain_info_size; 

				text_output = text; 
				remain_len = cc_buf_len; 

				remain_info_size = info_size; 
				stream_info = ( FILE_STREAM_INFORMATION* )info; 

				for( ; ; )
				{
					if( sizeof( *stream_info ) > remain_info_size )
					{
						*text_output = L'\0'; 
						break; 
					}

					if( stream_info->NextEntryOffset > remain_info_size )
					{
						*text_output = L'\0'; 
						break; 
					}

					{
						ULONG _info_size; 

						_info_size = ( info_size & 0xfffffffe ); 

						*( ( WCHAR* )( ( BYTE* )stream_info + _info_size - sizeof( WCHAR ) ) ) = L'\0'; 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						FILE_STREAM_INFO_FORMAT_TEXT, 
						stream_info->StreamName, 
						stream_info->StreamAllocationSize.QuadPart, 
						stream_info->StreamSize.QuadPart ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW;  
						break; 
					}

					if( 0 == stream_info->NextEntryOffset )
					{
						break; 
					}

					if( stream_info->NextEntryOffset > remain_info_size )
					{
						break; 
					}
					else
					{
						remain_info_size -= stream_info->NextEntryOffset; 
					}

					stream_info = ( FILE_STREAM_INFORMATION* )( ( BYTE* )stream_info + stream_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FilePipeInformation: // 23
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				FILE_PIPE_INFORMATION *pipe_info; 

				pipe_info = ( FILE_PIPE_INFORMATION* )info; 
				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					PIPE_FORMAT_TEXT, 
					pipe_info->CompletionMode, 
					pipe_info->ReadMode ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FilePipeLocalInformation: // 24
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_PIPE_LOCAL_INFORMATION *pipe_local_info; 

				pipe_local_info = ( FILE_PIPE_LOCAL_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					PIPE_LOCAL_INFORMATION, 
					pipe_local_info->InboundQuota, 
					pipe_local_info->OutboundQuota, 
					pipe_local_info->ReadDataAvailable, 
					pipe_local_info->WriteQuotaAvailable, 
					pipe_local_info->CurrentInstances, 
					pipe_local_info->MaximumInstances, 
					pipe_local_info->NamedPipeConfiguration, 
					pipe_local_info->NamedPipeState, 
					pipe_local_info->NamedPipeType, 
					pipe_local_info->NamedPipeEnd ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FilePipeRemoteInformation: // 25
			{
				HRESULT hr; 
				INT32 _ret; 
				size_t remain_len; 
				SYSTEMTIME time; 
				FILE_PIPE_REMOTE_INFORMATION *pipe_remote_info; 

				pipe_remote_info = ( FILE_PIPE_REMOTE_INFORMATION* )info; 

				_ret = FileTimeToSystemTime( ( FILETIME* )&pipe_remote_info->CollectDataTime, 
					&time ); 

				if( FALSE == _ret )
				{
					set_time_to_null( &time ); 
				}

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_PIPE_REMOTE_INFO_FORMAT_TEXT, 
					pipe_remote_info->MaximumCollectionCount, 
					time.wYear, 
					time.wMonth, 
					time.wDay, 
					time.wHour, 
					time.wMinute, 
					time.wSecond, 
					time.wMilliseconds ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileMailslotQueryInformation: // 26
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_MAILSLOT_QUERY_INFORMATION *mail_query_info; 

				mail_query_info = ( FILE_MAILSLOT_QUERY_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_MAILSLOT_QUERY_INFO_FORMAT_TEXT, 
					mail_query_info->MailslotQuota, 
					mail_query_info->MaximumMessageSize, 
					mail_query_info->MessagesAvailable, 
					mail_query_info->NextMessageSize, 
					mail_query_info->ReadTimeout.QuadPart / ( 10000 ) ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileMailslotSetInformation: // 27
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_MAILSLOT_SET_INFORMATION *mailslot_set_info; 

				mailslot_set_info = ( FILE_MAILSLOT_SET_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_MAILSLOT_SET_INFO_FORMAT_TEXT, 
					( mailslot_set_info->ReadTimeout->QuadPart / 10000 ) ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileCompressionInformation: // 28
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_COMPRESSION_INFORMATION *compress_info; 

				compress_info = ( FILE_COMPRESSION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_COMPRESSION_INFO_FORMAT_TEXT, 
					compress_info->CompressedFileSize.QuadPart, 
					get_file_compress_format_desc( compress_info->CompressionFormat ), 
					( 1 << compress_info->ChunkShift ), 
					( 1 << compress_info->ClusterShift ), 
					( 1 << compress_info->CompressionUnitShift ) ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileObjectIdInformation: // 29
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_OBJECTID_INFORMATION *object_id_info; 

				object_id_info = ( FILE_OBJECTID_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_OBJECT_ID_INFO_FORMAT_TEXT, 
					object_id_info->FileReference, 
					object_id_info->ObjectId[ 0 ], 
					object_id_info->ObjectId[ 1 ], 
					object_id_info->ObjectId[ 2 ], 
					object_id_info->ObjectId[ 3 ], 
					object_id_info->ObjectId[ 4 ], 
					object_id_info->ObjectId[ 5 ], 
					object_id_info->ObjectId[ 6 ], 
					object_id_info->ObjectId[ 7 ], 
					object_id_info->ObjectId[ 8 ], 
					object_id_info->ObjectId[ 9 ], 
					object_id_info->ObjectId[ 10 ], 
					object_id_info->ObjectId[ 11 ], 
					object_id_info->ObjectId[ 12 ], 
					object_id_info->ObjectId[ 13 ], 
					object_id_info->ObjectId[ 14 ], 
					object_id_info->ObjectId[ 15 ], 

					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 0 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 1 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 2 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 3 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 4 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 5 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 6 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 7 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 8 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 9 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 10 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 11 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 12 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 13 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 14 ], 
					object_id_info->DUMMYSTRUCTNAME.BirthObjectId[ 15 ] ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileCompletionInformation: // 30
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_COMPLETION_INFORMATION *completion_info; 

				completion_info = ( FILE_COMPLETION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_COMPLETION_INFO_FORMAT_TEXT, 
					completion_info->Key, 
					completion_info->Port ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileMoveClusterInformation: // 31
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_MOVE_CLUSTER_INFORMATION *move_cluster_info; 

				move_cluster_info = ( FILE_MOVE_CLUSTER_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_MOVE_CLUSTER_INFO_FORMAT_TEXT, 
					move_cluster_info->FileName, 
					move_cluster_info->ClusterCount ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileQuotaInformation: // 32
			{
				HRESULT hr; 
				INT32 _ret; 
				LPWSTR text_output;
				size_t remain_len; 
				SYSTEMTIME time; 
				FILE_QUOTA_INFORMATION *quota_info; 

				quota_info = ( FILE_QUOTA_INFORMATION* )info; 

				for( ; ; )
				{
					do 
					{
						_ret = FileTimeToSystemTime( ( FILETIME* )&quota_info->ChangeTime, 
							&time ); 

						if( FALSE == _ret )
						{
							set_time_to_null( &time ); 
						}

						text_output = text; 
						remain_len = cc_buf_len; 

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							FILE_QUOTE_INFO_FORMAT_TEXT, 
							quota_info->QuotaLimit.QuadPart, 
							quota_info->QuotaThreshold.QuadPart, 
							quota_info->QuotaUsed.QuadPart, 
							time.wYear, 
							time.wMonth, 
							time.wDay, 
							time.wHour, 
							time.wMinute, 
							time.wSecond, 
							time.wMilliseconds ); 

						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_TOO_SMALL;  
							break; 
						}

						{
							ULONG _cc_ret_len; 
							ret = get_sid_string( &quota_info->Sid, text_output, remain_len, &_cc_ret_len ); 

							if( ret != ERROR_SUCCESS )
							{
								*text_output = L'\0'; 
								break; 
							} 

							ASSERT( _cc_ret_len <= remain_len ); 
							text_output += _cc_ret_len - 1; 
							remain_len -= _cc_ret_len - 1; 
						}

					}while( FALSE ); 

					if( 0 == quota_info->NextEntryOffset )
					{
						break; 
					}

					quota_info = ( FILE_QUOTA_INFORMATION* )( ( BYTE* )quota_info + quota_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len;
			}
			break; 
		case FileReparsePointInformation: // 33
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_REPARSE_POINT_INFORMATION *repase_point_info; 

				repase_point_info = ( FILE_REPARSE_POINT_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_REPARSE_POINT_INFO_FORMAT_TEXT, 
					repase_point_info->FileReference, 
					repase_point_info->Tag ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileNetworkOpenInformation: // 34
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 

				FILE_NETWORK_OPEN_INFORMATION *network_open_info; 

				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				network_open_info = ( FILE_NETWORK_OPEN_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				_ret = FileTimeToSystemTime( ( FILETIME* )&network_open_info->CreationTime, &creation_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &creation_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&network_open_info->LastAccessTime, &last_access_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &last_access_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&network_open_info->ChangeTime, &change_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &change_time ); 
				}

				_ret = FileTimeToSystemTime( ( FILETIME* )&network_open_info->LastWriteTime, &write_time ); 
				if( FALSE == _ret )
				{
					set_time_to_null( &write_time ); 
				}

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					_FILE_ATTRIBUTE_TEXT, 
					&text_output, 
					&remain_len, 
					0 ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}
				
				
				ret = get_file_attr_desc( network_open_info->FileAttributes, text_output, remain_len, &text_output, &remain_len );  
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					FILE_NETWORK_OPEN_INFO_FORMAT_TEXT, 

					network_open_info->AllocationSize.QuadPart, 
					network_open_info->EndOfFile.QuadPart, 

					change_time.wYear, 
					change_time.wMonth, 
					change_time.wDay, 
					change_time.wHour, 
					change_time.wMinute, 
					change_time.wSecond, 
					change_time.wMilliseconds, 

					write_time.wYear, 
					write_time.wMonth, 
					write_time.wDay, 
					write_time.wHour, 
					write_time.wMinute, 
					write_time.wSecond, 
					write_time.wMilliseconds, 

					creation_time.wYear, 
					creation_time.wMonth, 
					creation_time.wDay, 
					creation_time.wHour, 
					creation_time.wMinute, 
					creation_time.wSecond, 
					creation_time.wMilliseconds, 

					last_access_time.wYear, 
					last_access_time.wMonth, 
					last_access_time.wDay, 
					last_access_time.wHour, 
					last_access_time.wMinute, 
					last_access_time.wSecond, 
					last_access_time.wMilliseconds ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileAttributeTagInformation: // 35
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				FILE_ATTRIBUTE_TAG_INFORMATION *attr_tag_info; 
				
				attr_tag_info = ( FILE_ATTRIBUTE_TAG_INFORMATION* )info; 

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					ret = get_file_attr_desc( attr_tag_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						NULL, 
						&remain_len, 
						0, 
						FILE_ATTRIBUTE_TAG_INFO_FORMAT_TEXT, 
						attr_tag_info->ReparseTag ); 
					
					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileTrackingInformation: // 36
			{
				HRESULT hr; 
				size_t remain_len; 

				dbg_print( MSG_FATAL_ERROR, "get the file tracking information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_TRACKING_INFO_FORMAT_TEXT, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileIdBothDirectoryInformation: // 37
			{
				HRESULT hr; 
				INT32 _ret; 
				size_t remain_len; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				FILE_ID_BOTH_DIR_INFORMATION* file_id_both_dir_info; 

				file_id_both_dir_info = ( FILE_ID_BOTH_DIR_INFORMATION* )info; 

				for( ; ; )
				{
					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_both_dir_info->CreationTime, &creation_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &creation_time ); 
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_both_dir_info->LastAccessTime, &last_access_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &last_access_time );  
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_both_dir_info->LastWriteTime, &write_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &write_time ); 
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_both_dir_info->ChangeTime, &change_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &change_time ); 
					}

					hr = StringCchPrintfExW( text, 
						cc_buf_len, 
						NULL, 
						&remain_len, 
						0, 
						FILE_BOTH_DIR_FILE_INFO_FORMAT_TEXT, 
						file_id_both_dir_info->FileName, 
						file_id_both_dir_info->ShortName, 

						file_id_both_dir_info->AllocationSize.QuadPart, 
						file_id_both_dir_info->EndOfFile.QuadPart, 
						file_id_both_dir_info->FileId.QuadPart, 
						file_id_both_dir_info->FileIndex, 
						file_id_both_dir_info->EaSize, 

						change_time.wYear, 
						change_time.wMonth, 
						change_time.wDay, 
						change_time.wHour, 
						change_time.wMinute, 
						change_time.wSecond, 
						change_time.wMilliseconds, 

						write_time.wYear, 
						write_time.wMonth, 
						write_time.wDay, 
						write_time.wHour, 
						write_time.wMinute, 
						write_time.wSecond, 
						write_time.wMilliseconds, 

						creation_time.wYear, 
						creation_time.wMonth, 
						creation_time.wDay, 
						creation_time.wHour, 
						creation_time.wMinute, 
						creation_time.wSecond, 
						creation_time.wMilliseconds, 

						last_access_time.wYear, 
						last_access_time.wMonth, 
						last_access_time.wDay, 
						last_access_time.wHour, 
						last_access_time.wMinute, 
						last_access_time.wSecond, 
						last_access_time.wMilliseconds ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
						break; 
					}

					if( 0 == file_id_both_dir_info->NextEntryOffset )
					{
						break; 
					}

					file_id_both_dir_info = ( FILE_ID_BOTH_DIR_INFORMATION* )( ( BYTE* )file_id_both_dir_info + file_id_both_dir_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileIdFullDirectoryInformation: // 38
			{
				HRESULT hr; 
				INT32 _ret; 
				size_t remain_len; 
				LPWSTR text_output; 
				SYSTEMTIME write_time; 
				SYSTEMTIME last_access_time; 
				SYSTEMTIME creation_time; 
				SYSTEMTIME change_time; 

				FILE_ID_FULL_DIR_INFORMATION* file_id_full_dir_info; 

				file_id_full_dir_info = ( FILE_ID_FULL_DIR_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				for( ; ; )
				{
					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_full_dir_info->CreationTime, &creation_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &creation_time ); 
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_full_dir_info->LastAccessTime, &last_access_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &last_access_time ); 
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_full_dir_info->LastWriteTime, &write_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &write_time ); 
					}

					_ret = FileTimeToSystemTime( ( FILETIME* )&file_id_full_dir_info->ChangeTime, &change_time ); 
					if( _ret == FALSE )
					{
						set_time_to_null( &change_time ); 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						FILE_NAME_FORMAT_TEXT, 
						file_id_full_dir_info->FileName ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					ret = get_file_attr_desc( file_id_full_dir_info->FileAttributes, text_output, remain_len, &text_output, &remain_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						FILE_FULL_DIR_INFO_FORMAT_TEXT, 
						file_id_full_dir_info->AllocationSize.QuadPart, 
						file_id_full_dir_info->EndOfFile.QuadPart, 
						file_id_full_dir_info->FileId.QuadPart, 
						file_id_full_dir_info->FileIndex, 
						file_id_full_dir_info->EaSize, 

						change_time.wYear, 
						change_time.wMonth, 
						change_time.wDay, 
						change_time.wHour, 
						change_time.wMinute, 
						change_time.wSecond, 
						change_time.wMilliseconds, 

						write_time.wYear, 
						write_time.wMonth, 
						write_time.wDay, 
						write_time.wHour, 
						write_time.wMinute, 
						write_time.wSecond, 
						write_time.wMilliseconds, 

						creation_time.wYear, 
						creation_time.wMonth, 
						creation_time.wDay, 
						creation_time.wHour, 
						creation_time.wMinute, 
						creation_time.wSecond, 
						creation_time.wMilliseconds, 

						last_access_time.wYear, 
						last_access_time.wMonth, 
						last_access_time.wDay, 
						last_access_time.wHour, 
						last_access_time.wMinute, 
						last_access_time.wSecond, 
						last_access_time.wMilliseconds ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL;  
						break; 
					}

					if( 0 == file_id_full_dir_info->NextEntryOffset )
					{
						break; 
					}

					file_id_full_dir_info = ( FILE_ID_FULL_DIR_INFORMATION* )( ( BYTE* )file_id_full_dir_info + file_id_full_dir_info->NextEntryOffset ); 
				}

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileValidDataLengthInformation: // 39
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_VALID_DATA_LENGTH_INFORMATION *data_length_info; 

				data_length_info = ( FILE_VALID_DATA_LENGTH_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_VALID_DATA_LENGHT_INFO_FORMAT_TEXT, 
					data_length_info->ValidDataLength.QuadPart ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileShortNameInformation: // 40
			{
				HRESULT hr; 
				size_t remain_len; 
				
				dbg_print( MSG_FATAL_ERROR, "get the file short information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_ALTERNATE_NAME, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileIoCompletionNotificationInformation: // 41
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_IO_COMPLETION_NOTIFICATION_INFORMATION *io_completion_info; 

				io_completion_info = ( FILE_IO_COMPLETION_NOTIFICATION_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_COMPLETION_NOTIFICATION_INFO_FORMAT_TEXT, 
					io_completion_info->Flags ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileIoStatusBlockRangeInformation: // 42
			{
				HRESULT hr; 
				size_t remain_len; 

				dbg_print( MSG_FATAL_ERROR, "get the file status block range information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_STATUS_BLOCK_RANGE_INFO_FORMAT_TEXT, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileIoPriorityHintInformation: // 43
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_IO_PRIORITY_HINT_INFO *priority_hint_info; 

				priority_hint_info = ( FILE_IO_PRIORITY_HINT_INFO* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_IO_PRIORITY_HINT_INFO_FORMAT_TEXT, 
					priority_hint_info->PriorityHint ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileSfioReserveInformation: // 44
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_SFIO_RESERVE_INFORMATION *sfio_reserve_info; 

				sfio_reserve_info = ( FILE_SFIO_RESERVE_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_SFIO_RESERVE_INFO_FORMAT_TEXT, 
					sfio_reserve_info->RequestsPerPeriod, 
					sfio_reserve_info->Period, 
					sfio_reserve_info->RetryFailures, 
					sfio_reserve_info->Discardable == TRUE ? FILE_SFIO_DISCARDABLE_TEXT : L"", 
					sfio_reserve_info->RequestSize, 
					sfio_reserve_info->NumOutstandingRequests ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileSfioVolumeInformation: // 45
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_SFIO_VOLUME_INFORMATION *sfio_volume_info; 

				sfio_volume_info = ( FILE_SFIO_VOLUME_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_SFIO_VOLUME_INFORMATION_FORMAT_TEXT, 
					sfio_volume_info->MaximumRequestsPerPeriod, 
					sfio_volume_info->MinimumPeriod, 
					sfio_volume_info->MinimumTransferSize ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL;  
					break; 
				}
			}
			break; 
		case FileHardLinkInformation: // 46
			{
				HRESULT hr; 
				size_t remain_len; 
				
				dbg_print( MSG_FATAL_ERROR, "get the file hard link information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_HARD_LINK_TEXT, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileProcessIdsUsingFileInformation: // 47
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				LONG i; 
				FILE_PROCESS_IDS_USING_FILE_INFORMATION *process_ids_using_file_info; 

				process_ids_using_file_info = ( FILE_PROCESS_IDS_USING_FILE_INFORMATION* )info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchCopyExW( text_output, remain_len, FILE_PROCESS_IDS_USING_FILE_INFO_TEXT1, &text_output, &remain_len, 0 ); 
				if( FAILED(  hr ) )
				{
					*cc_ret_len = cc_buf_len - remain_len; 
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}

				for( i = 0; ( ULONG )i < process_ids_using_file_info->NumberOfProcessIdsInList; i ++ )
				{
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						L"%u, ", 
						process_ids_using_file_info->ProcessIdList[ i ] ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}
				}

				if( FAILED(  hr ) )
				{
					*cc_ret_len = cc_buf_len - remain_len; 
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}

				hr = StringCchCopyExW( text_output, remain_len, FILE_PROCESS_IDS_USING_FILE_INFO_TEXT2, &text_output, &remain_len, 0 ); 
				if( FAILED(  hr ) )
				{
					*cc_ret_len = cc_buf_len - remain_len; 
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}
				
				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case FileNormalizedNameInformation: // 48
			{
				HRESULT hr; 
				size_t remain_len; 

				dbg_print( MSG_FATAL_ERROR, "get the file normalized name information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_NORMALIZED_NAME_INFO_FORMAT_TEXT, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}
			}
			break; 
		case FileNetworkPhysicalNameInformation: // 49
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_NETWORK_PHYSICAL_NAME_INFORMATION *network_physical_info; 

				network_physical_info = ( FILE_NETWORK_PHYSICAL_NAME_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_NETWORK_PHYSICAL_NAME_INFO_FORMAT_TEXT, 
					network_physical_info->FileName ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileIdGlobalTxDirectoryInformation: // 50
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_ID_GLOBAL_TX_DIR_INFORMATION *global_tx_dir_info; 

				global_tx_dir_info = ( FILE_ID_GLOBAL_TX_DIR_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_ID_GLOBAL_TX_DIR_INFO_FORMAT_TEXT ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileIsRemoteDeviceInformation: // 51
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_IS_REMOTE_DEVICE_INFORMATION *remote_dev_info; 

				remote_dev_info = ( FILE_IS_REMOTE_DEVICE_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					L"%s", 
					remote_dev_info->IsRemote == TRUE ? FILE_REMOTE_FILE_TEXT : FILE_LOCAL_FILE_TEXT );  

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileAttributeCacheInformation: // 52
			{
				HRESULT hr; 
				size_t remain_len; 

				dbg_print( MSG_FATAL_ERROR, "get the file attribute cache information!!!\n" ); 

				hr = StringCchCopyExW( text, 
					cc_buf_len, 
					FILE_ATTRIBUTE_CACHE_INFO_FORMAT_TEXT, 
					NULL, 
					&remain_len, 
					0 ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}
			}
			break; 
		case FileNumaNodeInformation: // 53
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_NUMA_NODE_INFORMATION *numa_node_info; 

				numa_node_info = ( FILE_NUMA_NODE_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_NUMA_NODE_INFO_FORMAT_TEXT, 
					numa_node_info->NodeNumber ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
		case FileStandardLinkInformation: // 54
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_STANDARD_LINK_INFORMATION *standard_link_info; 

				standard_link_info = ( FILE_STANDARD_LINK_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_STANDARD_LINK_INFO_FORMAT_TEXT, 
					standard_link_info->NumberOfAccessibleLinks, 
					standard_link_info->TotalNumberOfLinks, 
					standard_link_info->DeletePending == TRUE ? FILE_DELETED_TEXT : L"", 
					standard_link_info->Directory == TRUE ? FILE_DIRECTORY_TEXT : L"" ); 

				*cc_ret_len = cc_buf_len - remain_len; 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break;
		case FileRemoteProtocolInformation: // 55
			{
				HRESULT hr; 
				size_t remain_len; 
				FILE_REMOTE_PROTOCOL_INFORMATION *remote_prot_info; 

				remote_prot_info = ( FILE_REMOTE_PROTOCOL_INFORMATION* )info; 

				hr = StringCchPrintfExW( text, 
					cc_buf_len, 
					NULL, 
					&remain_len, 
					0, 
					FILE_REMOTE_PROTOCOL_INFO_FORMAT_TEXT, 
					remote_prot_info->Protocol, // Protocol (WNNC_NET_*) defined in winnetwk.h or ntifs.h.
					// Protocol Version & Type
					remote_prot_info->ProtocolMajorVersion, 
					remote_prot_info->ProtocolMinorVersion, 
					remote_prot_info->ProtocolRevision, 

					// Protocol-Generic Information
					remote_prot_info->Flags ); 
				
				*cc_ret_len = cc_buf_len - remain_len; 
				
				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}
			}
			break; 
			//case FileMaximumInformation:
			//	break; 
		default: 
			dbg_print( MSG_FATAL_ERROR, "invalid file information class %u\n", 
				info_class ); 
			ASSERT( FALSE ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_close_detail( sys_action_record *action, 
									   action_context *ctx, 
									   prepared_params_desc *params_desc, 
									   LPWSTR tip, 
									   ULONG ccb_buf_len, 
									   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		if( action->type != FILE_close )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_setinfo_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 

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

		*tip = L'\0'; 


		if( action->do_file_setinfo.info_size > 0 )
		{
			ULONG valid_info_size; 

			valid_info_size = action->size - 
				( ( ( action->do_file_getinfo.path_len + 1 ) << 1 ) + 
				ACTION_RECORD_SIZE_BY_TYPE( file_getinfo ) ); 

			if( action->do_file_getinfo.info_size < valid_info_size ) 
			{
				valid_info_size = action->do_file_getinfo.info_size; 
			}

			try
			{
				ret = get_file_info_desc( ( FILE_INFORMATION_CLASS )action->do_file_setinfo.info_class, 
					( PVOID )( action->do_file_setinfo.path_name + action->do_file_setinfo.path_len + 1 ), 
					valid_info_size, 
					tip, 
					ccb_buf_len, 
					&cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}
			catch( ... )
			{
				DBG_BP(); 
				ASSERT( ctx->last_result != STATUS_SUCCESS ); 
				ASSERT( ctx->last_result == STATUS_INVALID_PARAMETER ); 				

				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}
		else
		{
			DBG_BP(); 
			ASSERT( ctx->last_result == STATUS_INVALID_PARAMETER ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_file_setsec_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_reg_open_key_detail( sys_action_record *action, 
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

		if( action->type != REG_openkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = KEY_OPEN_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_reg_mkkey_detail( sys_action_record *action, 
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

		if( action->type != REG_mkkey )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = KEY_OPEN_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_reg_rmkey_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_mvkey_detail( sys_action_record *action, 
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

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T new_name_len; 
		////WCHAR path_name[ 1 ]; //注册表键路径 
		////WCHAR new_keyname[ 0 ]; //目标注册表键名 
		//WCHAR path_name[ 1 ]; //注册表键路径

		fmt = KEY_MOVE_KEY_FORAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			new_path_name = action->do_reg_mvkey.new_key_name_ptr; 

		}
		else
		{
			new_path_name = action->do_reg_mvkey.path_name + action->do_reg_mvkey.path_len + 1; 
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

LRESULT WINAPI get_reg_rmval_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LPCWSTR val_name; 
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

		if( action->type != REG_rmval )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = KEY_REMOVE_VALUE_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			val_name = action->do_reg_rmval.val_name_ptr; 
		}
		else
		{
			val_name = action->do_reg_rmval.path_name + action->do_reg_rmval.path_len + 1; 
		}

		if( val_name != NULL )
		{
			ASSERT( val_name[ action->do_reg_rmval.val_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_rmval.val_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			val_name ); 

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

LRESULT WINAPI get_reg_getval_detail( sys_action_record *action, 
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
	size_t cc_remain_len; 
	LPWSTR _text; 
	ULONG cc_ret_len; 
	PVOID info; 

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

		fmt = KEY_GET_VALUE_INFO_FORMAT_TEXT; 

		//ULONG type; //注册表值类型 
		//ULONG data_len; //数据长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T val_name_len; 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //注册表值路径 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			val_name = action->do_reg_getval.value_name_ptr;  
		}
		else
		{
			val_name = action->do_reg_getval.path_name + action->do_reg_getval.path_len + 1; 
		}

		if( val_name != NULL )
		{
		}
		else
		{
			ASSERT( action->do_reg_getval.val_name_len == 0 ); 
		}

		_text = tip; 
		cc_remain_len = ccb_buf_len; 

		_ret = StringCchPrintfExW( _text, 
			cc_remain_len, 
			&_text, 
			&cc_remain_len, 
			0, 
			fmt, 
			val_name, 
			action->do_reg_getval.type, 
			action->do_reg_getval.info_size ); 

		ASSERT( ( ULONG )_ret <= ccb_buf_len ); 

		if( cc_remain_len == 0 )
		{
			break; 
		} 

		if(  ctx->last_result == STATUS_SUCCESS 
			|| ctx->last_result == STATUS_BUFFER_OVERFLOW )
		{
			ASSERT( action->do_reg_getval.info_size > 0 ); 

			try
			{
				info = ( PVOID )( action->do_reg_getval.path_name 
					+ action->do_reg_getval.path_len 
					+ 1 
					+ action->do_reg_getval.val_name_len 
					+ 1 ); 

				ret = get_reg_value_info_detail( action->do_reg_getval.type, 
					info, 
					action->do_reg_getval.info_size, 
					_text, 
					cc_remain_len, 
					&cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					tip[ ccb_buf_len - 1 ] = L'\0'; 
					ret = ERROR_BUFFER_OVERFLOW; 
				}
			}
			catch( ... )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				DBG_BP(); 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else
		{
			ASSERT( action->do_reg_enum_val.info_size == 0 ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_reg_getinfo_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 
	PVOID info; 

	do 
	{
		if( action->do_reg_getinfo.info_size > 0 )
		{
			try
			{
				info = ( PVOID )( action->do_reg_getinfo.path_name + action->do_reg_getinfo.path_len + 1 ); 

				ret = get_reg_info_detail( action->do_reg_getinfo.info_class, 
					info, 
					action->do_reg_getinfo.info_size, 
					tip, 
					ccb_buf_len, 
					&cc_ret_len ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}
			catch( ... )
			{
				DBG_BP(); 
				ASSERT( ctx->last_result != STATUS_SUCCESS ); 
				ASSERT( ctx->last_result == STATUS_INVALID_PARAMETER ); 				

				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}
		else
		{
			DBG_BP(); 
			ASSERT( ctx->last_result == STATUS_BUFFER_TOO_SMALL 
				|| ctx->last_result == STATUS_INVALID_PARAMETER ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_reg_setinfo_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 
	PVOID info; 

	do 
	{
		if( action->do_reg_setinfo.info_size > 0 )
		{
			try
			{
				info = ( PVOID )( action->do_reg_setinfo.path_name + action->do_reg_setinfo.path_len + 1 ); 

				ret = get_reg_info_detail( action->do_reg_setinfo.info_class, 
					info, 
					action->do_reg_setinfo.info_size, 
					tip, 
					ccb_buf_len, 
					&cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}
			catch( ... )
			{
				DBG_BP(); 
				ASSERT( ctx->last_result != STATUS_SUCCESS ); 
				ASSERT( ctx->last_result == STATUS_INVALID_PARAMETER ); 				

				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
		}
		else
		{
			DBG_BP(); 
			ASSERT( ctx->last_result == STATUS_INVALID_PARAMETER ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_reg_enum_info_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR text_output; 
	size_t remain_len; 
	PVOID info; 
	ULONG _cc_ret_len = 0; 

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

		if( action->type != REG_enuminfo )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		text_output = tip; 
		remain_len = ccb_buf_len; 

		hr = StringCchPrintfExW( text_output, 
			remain_len, 
			&text_output, 
			&remain_len, 
			0, 
			KEY_ENUM_INFO_FORMAT_TEXT, 
			action->do_reg_enum_info.index ); 

		//ULONG index; 
		//PATH_SIZE_T path_len; 
		//ULONG info_class; 
		//ULONG info_size; 

		//WCHAR path_name[ 1 ]; //文件全路径 

		if( ctx->last_result == STATUS_SUCCESS 
			|| ctx->last_result == STATUS_BUFFER_OVERFLOW )
		{
			ASSERT( action->do_reg_enum_info.info_size > 0 ); 
			info = ( PVOID )( action->do_reg_enum_info.path_name + action->do_reg_enum_info.path_len + 1 ); 

			try
			{
				ret = get_reg_info_detail( action->do_reg_enum_info.info_class, 
					info, 
					action->do_reg_enum_info.info_size, 
					text_output, 
					remain_len, 
					&_cc_ret_len ); 
			}
			catch( ... )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else
		{
			ASSERT( action->do_reg_enum_info.info_size == 0 ); 
		}

	}while( FALSE ); 
	
	return ret; 
}

LRESULT WINAPI get_reg_enum_value_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR text_output; 
	size_t remain_len; 
	PVOID info; 
	ULONG _cc_ret_len = 0; 

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

		if( action->type != REG_enum_val )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		text_output = tip; 
		remain_len = ccb_buf_len; 

		hr = StringCchPrintfExW( text_output, 
			remain_len, 
			&text_output, 
			&remain_len, 
			0, 
			KEY_ENUM_VALUE_INFO_FORMAT_TEXT, 
			action->do_reg_enum_val.index ); 

		//ULONG index; 
		//PATH_SIZE_T path_len; 
		//ULONG info_class; 
		//ULONG info_size; 

		//WCHAR path_name[ 1 ]; //文件全路径 

		if(  ctx->last_result == STATUS_SUCCESS 
			|| ctx->last_result == STATUS_BUFFER_OVERFLOW )
		{
			ASSERT( action->do_reg_enum_val.info_size > 0 ); 

			try
			{
				info = ( PVOID )( action->do_reg_enum_val.path_name + action->do_reg_enum_val.path_len + 1 ); 

				ret = get_reg_value_info_detail( action->do_reg_enum_val.info_class, 
					info, 
					action->do_reg_enum_val.info_size, 
					text_output, 
					remain_len, 
					&_cc_ret_len ); 
			}
			catch( ... )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				DBG_BP(); 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else
		{
			ASSERT( action->do_reg_enum_val.info_size == 0 ); 
		}

	}while( FALSE ); 
	
	return ret; 
}


LRESULT WINAPI get_reg_setval_detail( sys_action_record *action, 
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

		//ULONG tid; //动作发起者线程ID 
		//ULONG type; //注册表值类型 
		//ULONG data_len; //数据长度 

		fmt = KEY_SET_VALUE_INFO_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			val_name = action->do_reg_setval.value_name_ptr;  
		}
		else
		{
			val_name = action->do_reg_setval.path_name + action->do_reg_setval.path_len + 1; 
		}

		if( val_name != NULL )
		{
			ASSERT( val_name[ action->do_reg_setval.val_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_setval.val_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			val_name, 
			get_reg_value_type_desc( action->do_reg_setval.type ), 
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

LRESULT WINAPI get_load_key_detail( sys_action_record *action, 
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

		fmt = KEY_LOAD_FORMAT_TEXT; 

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//WCHAR path_name[ 1 ]; //注册表键路径名 

		//ASSERT( action->do_reg_loadkey.path_name[ action->do_reg_loadkey.path_len ] == L'\0' ); 

		hive_file_name = action->do_reg_loadkey.path_name + action->do_reg_loadkey.path_len + 1; 

		ASSERT( hive_file_name[ action->do_reg_loadkey.hive_name_len ] == L'\0' ); 

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

LRESULT WINAPI get_close_key_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		if( action->type != REG_closekey ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_rstr_key_detail( sys_action_record *action, 
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

		fmt = KEY_LOAD_FORMAT_TEXT; 

		//NTSTATUS result;//动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//WCHAR path_name[ 1 ]; //注册表键路径名 

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

LRESULT WINAPI get_reg_set_sec_detail( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		if( action->type != REG_setsec )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		tip[ 0 ] = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_repl_key_detail( sys_action_record *action, 
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

		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T hive_name_len; 
		//PATH_SIZE_T new_hive_name_len; 
		//WCHAR path_name[ 1 ]; //注册表键路径名 

		fmt = KEY_REPLACE_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			old_hive_file_name = action->do_reg_replkey.hive_name_ptr; 
			new_hive_file_name = action->do_reg_replkey.new_hive_name_ptr; 
		}
		else
		{
			old_hive_file_name = action->do_reg_replkey.path_name + action->do_reg_replkey.path_len + 1; 
			new_hive_file_name = action->do_reg_replkey.path_name + action->do_reg_replkey.path_len 
				+ action->do_reg_replkey.hive_name_len
				+ 2; 
		}

		if( old_hive_file_name != NULL )
		{
			ASSERT( old_hive_file_name[ action->do_reg_replkey.hive_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_replkey.hive_name_len == 0 ); 
		}

		if( new_hive_file_name != NULL )
		{
			ASSERT( new_hive_file_name[ action->do_reg_replkey.new_hive_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_reg_replkey.new_hive_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_proc_open_detail( sys_action_record *action, 
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

		if( action->type != PROC_open )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//ULONG access; //进程打开权限 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = GET_PROCESS_OPEN_DETAIL_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_open.path_name; 
		}
		else
		{
			path_name = action->do_proc_open.path_name_ptr; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_proc_open.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_proc_open.path_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
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

LRESULT WINAPI get_proc_debug_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = PROCESS_DEBUG_FORMAT_TEXT; 

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

LRESULT WINAPI get_proc_suspend_detail( sys_action_record *action, 
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

		if( action->type != PROC_suspend )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//ULONG result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = PROCESS_SUSPEND_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_suspend.path_name_ptr; 
		}
		else
		{
			path_name = action->do_proc_suspend.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( action->do_proc_suspend.path_name[ action->do_proc_suspend.path_len ] == L'\0' );  
		}
		else
		{
			ASSERT( action->do_proc_suspend.path_len == 0 );  
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
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

LRESULT WINAPI get_proc_resume_detail( sys_action_record *action, 
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

		if( action->type != PROC_resume )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = PROCESS_RESUME_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_resume.path_name_ptr; 
		}
		else
		{
			path_name = action->do_proc_resume.path_name; 
		}

		if( path_name != NULL )
		{
			ASSERT( path_name[ action->do_proc_resume.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_proc_resume.path_len == 0 ); 
			path_name = L""; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
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

LRESULT WINAPI get_proc_kill_detail( sys_action_record *action, 
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

		if( action->type != PROC_exit )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = PROCESS_KILL_FORMAT_TEXT; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			path_name = action->do_proc_kill.path_name_ptr; 
		}
		else
		{
			path_name = action->do_proc_kill.path_name; 
		}

		//ULONG target_pid; //目标进程ID 
		//ULONG exitcode; //进程推出码 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		if( path_name !=NULL )
		{
			ASSERT( path_name[ action->do_proc_kill.path_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_proc_kill.path_len == 0 ); 
			path_name = L""; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			path_name, 
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

LRESULT WINAPI get_proc_job_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR pathname[ 1 ]; //目标进程全路径

		fmt = PROCESS_JOB_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_proc_pg_prot_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//PVOID base; //修改内存基址 
		//ULONG count; //修改内存长度 
		//ACCESS_MASK attrib; //修改内存属性 
		//ULONG bytes_changed; //成功修改长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = PROCESS_PAGE_PROTECT_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_proc_free_vm_detail( sys_action_record *action, 
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

		fmt = L"进程ID:%u 地址 0x%0.8x 长度 %u 类型 %u 返回长度:%u\n\n"; 

		//ULONG target_pid; //目标进程ID 
		//PVOID base; //释放内存基址 
		//ULONG count; //释放内存长度 
		//ULONG type; //释放内存类型 
		//ULONG bytes_freed; //成功释放长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_proc_write_vm_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//PVOID base; //写入内存基址 
		//ULONG data_len; //试图写入长度 
		//ULONG bytes_written; //成功写入长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"进程ID %u 地址 0x%0.8x 长度:%u 返回长度 %u\n"; 

		ASSERT( action->do_proc_writevm.path_name[ action->do_proc_writevm.path_len ] == L'\0' ); 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			//action->do_proc_writevm.path_name, 
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

LRESULT WINAPI get_proc_read_vm_detail( sys_action_record *action, 
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

		fmt = L"进程ID %u 地址 0x%0.8x 长度 %u 返回长度 %u\n"; 

		//ULONG target_pid; //目标进程ID 
		//PVOID base; //读取内存基址 
		//ULONG data_len; //试图读取长度 
		//ULONG bytes_read; //成功读取长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //目标进程全路径 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_thread_remote_detail( sys_action_record *action, 
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


		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//ACCESS_MASK access; //线程创建权限 
		//BOOLEAN suspended; //是否挂起方式创建 
		//PVOID start_vaddr; //线程起始地址 
		//PVOID thread_param; //线程参数 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		////WCHAR path_name[ 1 ]; //目标进程全路径 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"进程ID %u 远程线程ID:%u 权限 %u 挂起 %u 启始地址 0x%0.8x 参数 0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_thread_set_ctxt_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"进程ID %u 线程:%u \n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_thread_suspend_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"进程ID %u 线程ID %u\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_thread_resume_detail( sys_action_record *action, 
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

		fmt = L"目标进程ID %u 线程ID %u \n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_thread_kill_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//ULONG exitcode; //线程退出码 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = THREAD_KILL_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_port_read_detail( sys_action_record *action, 
									action_context *ctx, 
									PVOID data, 
									ULONG data_size, 
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

		if( action->type != PORT_read )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径

		fmt = PROT_READ_FORMAT_TEXT; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_port_read.path_name, 
			data_size ); 

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


LRESULT WINAPI get_port_urb_detail( sys_action_record *action, 
   action_context *ctx, 
   PVOID data, 
   ULONG data_size, 
   prepared_params_desc *params_desc, 
   LPWSTR tip, 
   ULONG ccb_buf_len, 
   ULONG flags )
{
   LRESULT ret = ERROR_SUCCESS; 

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

	   if( action->type != PORT_urb )
	   {
		   ret = ERROR_INVALID_PARAMETER; 
		   break; 
	   }

	   //ULONG target_pid; //目标进程ID 
	   //ULONG target_tid; //目标线程ID 
	   //NTSTATUS result; //动作完成结果(NTSTATUS) 
	   //PATH_SIZE_T path_len; 
	   //WCHAR path_name[ 1 ]; //目标进程全路径

	   {
		   size_t cc_remain_len; 
		   LPWSTR remain_text; 
		   ULONG cc_ret_len; 

		   cc_remain_len = ccb_buf_len; 
		   remain_text = tip; 

		   if( data_size == 0 
			   || data == NULL )
		   {
			   *remain_text = L'\0'; 
			   cc_ret_len = 0; 
			   break; 
		   }

		   ret = get_urb_info( remain_text, cc_remain_len, data, data_size, &cc_ret_len ); 
	   }
   }while( FALSE ); 

   return ret; 
}

LRESULT WINAPI get_port_write_detail( sys_action_record *action, 
   action_context *ctx, 
   PVOID data, 
   ULONG data_size, 
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

	   if( action->type != PORT_write )
	   {
		   ret = ERROR_INVALID_PARAMETER; 
		   break; 
	   }

	   //ULONG target_pid; //目标进程ID 
	   //ULONG target_tid; //目标线程ID 
	   //NTSTATUS result; //动作完成结果(NTSTATUS) 
	   //PATH_SIZE_T path_len; 
	   //WCHAR path_name[ 1 ]; //目标进程全路径

	   fmt = PROT_WRITE_FORMAT_TEXT; 

	   _ret = _snwprintf( tip, 
		   ccb_buf_len, 
		   fmt, 
		   action->do_port_write.path_name, 
		   data_size ); 


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

LRESULT WINAPI get_thread_queue_apc_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//ULONG target_tid; //目标线程ID 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径

		fmt = L"目标进程ID %u 线程ID %u APC函数地址 0x%0.8x 参数1 0x%0.8x 参数2 0x%0.8x 参数3 0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_thrd_queue_apc.target_pid, 
			action->do_thrd_queue_apc.target_tid, 
			action->do_thrd_queue_apc.apc_routine, 
			action->do_thrd_queue_apc.arg1, 
			action->do_thrd_queue_apc.arg2, 
			action->do_thrd_queue_apc.arg3 ); 

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

LRESULT WINAPI get_sys_settime_detail( sys_action_record *action, 
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

		//ULONG time_delta; //设置时间与动作发起时间差 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 

		fmt = L"系统时间 (相对时间) %u\n"; 

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

LRESULT WINAPI get_sys_link_knowndll_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sys_open_physmm_detail( sys_action_record *action, 
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

		//ACCESS_MASK access; //打开权限 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 

		fmt = L"权限: 0x%0.8x\n"; 

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

LRESULT WINAPI get_sys_read_physmm_detail( sys_action_record *action, 
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

		//ULONG offset; //读取偏移 
		//ULONG count; //读取长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 

		fmt = L"偏移:%u 长度%u\n"; 

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

LRESULT WINAPI get_sys_write_physmm_detail( sys_action_record *action, 
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

		//ULONG offset; //写入偏移 
		//ULONG count; //写入长度 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 

		fmt = L"偏移 %u 长度 %u\n"; 

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


LRESULT WINAPI get_sys_load_kmod_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *params_desc, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		if( action->type != SYS_load_kmod )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		fmt = LOAD_KERNEL_MODULE_FORMAT_TEXT; 


		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_sys_load_kmod.base_addr, 
			action->do_sys_load_kmod.size ); 

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

LRESULT WINAPI get_sys_enumproc_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 

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

		*tip = L'\0'; 

	}while( FALSE ); 

	return ret; 
}

#ifndef KEY_INFORMATION_CLASS_DEFINED
#define KEY_INFORMATION_CLASS_DEFINED 1

typedef enum _KEY_INFORMATION_CLASS {
	KeyBasicInformation,
	KeyNodeInformation,
	KeyFullInformation,
	KeyNameInformation,
	KeyCachedInformation,
	KeyFlagsInformation,
	KeyVirtualizationInformation,
	KeyHandleTagsInformation,
	MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;
#endif //KEY_INFORMATION_CLASS_DEFINED

typedef struct _KEY_BASIC_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   ClassOffset;
	ULONG   ClassLength;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable length string
	//          Class[1];           // Variable length string not declared
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   ClassOffset;
	ULONG   ClassLength;
	ULONG   SubKeys;
	ULONG   MaxNameLen;
	ULONG   MaxClassLen;
	ULONG   Values;
	ULONG   MaxValueNameLen;
	ULONG   MaxValueDataLen;
	WCHAR   Class[1];           // Variable length
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

typedef struct _KEY_WRITE_TIME_INFORMATION {
	LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_WOW64_FLAGS_INFORMATION {
	ULONG   UserFlags;
} KEY_WOW64_FLAGS_INFORMATION, *PKEY_WOW64_FLAGS_INFORMATION;

typedef struct _KEY_HANDLE_TAGS_INFORMATION {
	ULONG   HandleTags;
} KEY_HANDLE_TAGS_INFORMATION, *PKEY_HANDLE_TAGS_INFORMATION;

typedef struct _KEY_CONTROL_FLAGS_INFORMATION {
	ULONG   ControlFlags;
} KEY_CONTROL_FLAGS_INFORMATION, *PKEY_CONTROL_FLAGS_INFORMATION;

typedef struct _KEY_SET_VIRTUALIZATION_INFORMATION {
	ULONG   VirtualTarget           : 1; // Tells if the key is a virtual target key. 
	ULONG   VirtualStore	        : 1; // Tells if the key is a virtual store key.
	ULONG   VirtualSource           : 1; // Tells if the key has been virtualized at least one (virtual hint)
	ULONG   Reserved                : 29;   
} KEY_SET_VIRTUALIZATION_INFORMATION, *PKEY_SET_VIRTUALIZATION_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
	KeyWriteTimeInformation,
	KeyWow64FlagsInformation,
	KeyControlFlagsInformation,
	KeySetVirtualizationInformation,
	KeySetDebugInformation,
	KeySetHandleTagsInformation,
	MaxKeySetInfoClass  // MaxKeySetInfoClass should always be the last enum
} KEY_SET_INFORMATION_CLASS;

//
// Value entry query structures
//

typedef struct _KEY_VALUE_BASIC_INFORMATION {
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   DataOffset;
	ULONG   DataLength;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable size
	//          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   DataLength;
	UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
	ULONG   Type;
	ULONG   DataLength;
	UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
	PUNICODE_STRING ValueName;
	ULONG           DataLength;
	ULONG           DataOffset;
	ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
	KeyValueBasicInformation,
	KeyValueFullInformation,
	KeyValuePartialInformation,
	KeyValueFullInformationAlign64,
	KeyValuePartialInformationAlign64,
	MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_NAME_INFORMATION {
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable length string
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   SubKeys;
	ULONG   MaxNameLen;
	ULONG   Values;
	ULONG   MaxValueNameLen;
	ULONG   MaxValueDataLen;
	ULONG   NameLength;
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_FLAGS_INFORMATION {
	ULONG   UserFlags;
} KEY_FLAGS_INFORMATION, *PKEY_FLAGS_INFORMATION;

typedef struct _KEY_VIRTUALIZATION_INFORMATION {
	ULONG   VirtualizationCandidate : 1; // Tells whether the key is part of the virtualization namespace scope (only HKLM\Software for now)
	ULONG   VirtualizationEnabled   : 1; // Tells whether virtualization is enabled on this key. Can be 1 only if above flag is 1.
	ULONG   VirtualTarget           : 1; // Tells if the key is a virtual key. Can be 1 only if above 2 are 0. Valid only on the virtual store key handles.
	ULONG   VirtualStore            : 1; // Tells if the key is a part of the virtual sore path. Valid only on the virtual store key handles.
	ULONG   VirtualSource           : 1; // Tells if the key has ever been virtualized, Can be 1 only if VirtualizationCandidate is 1
	ULONG   Reserved                : 27;   
} KEY_VIRTUALIZATION_INFORMATION, *PKEY_VIRTUALIZATION_INFORMATION;

LRESULT WINAPI get_reg_info_detail( ULONG key_class, 
								   PVOID key_info, 
								   ULONG info_size, 
								   LPWSTR text, 
								   ULONG cc_buf_len, 
								   ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( cc_ret_len != NULL ); 

		*text = L'\0'; 
		*cc_ret_len = 0; 

		switch( key_class )
		{
		case KeyBasicInformation: 
			{
				HRESULT hr; 
				INT32 _ret; 
				LPWSTR text_output; 
				size_t remain_len; 
				SYSTEMTIME time; 
				KEY_BASIC_INFORMATION *basic_info; 

				do 
				{
					if( info_size < FIELD_OFFSET( KEY_BASIC_INFORMATION, Name ) ) 
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					basic_info = ( KEY_BASIC_INFORMATION* )key_info; 

					_ret = FileTimeToSystemTime( ( FILETIME* )&basic_info->LastWriteTime, 
						&time ); 

					if( FALSE == _ret ) 
					{
						set_time_to_null( &time ); 
					}

					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_NAME, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					{
						ULONG cc_max_name_len; 

						cc_max_name_len = ( ULONG )( ( info_size - FIELD_OFFSET( KEY_BASIC_INFORMATION, Name ) ) >> 1 ); 

						if( basic_info->NameLength > ( cc_max_name_len << 1 ) )
						{
							basic_info->NameLength = ( cc_max_name_len << 1 ); 
						}
					}

					if( basic_info->NameLength > 0 )
					{
						if( remain_len < ( basic_info->NameLength >> 1 ) + 1 )
						{
							memcpy( text_output, basic_info->Name, ( remain_len << 1 ) - sizeof( WCHAR ) ); 
							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						memcpy( text_output, basic_info->Name, basic_info->NameLength ); 

						if( text_output[ ( basic_info->NameLength >> 1 ) - 1 ] == L'\0' )
						{
							remain_len -= ( ( basic_info->NameLength >> 1 ) - 1 ); 
							text_output += ( ( basic_info->NameLength >> 1 ) - 1 ); 						
						}
						else
						{
							remain_len -= ( basic_info->NameLength >> 1 ); 
							text_output += ( basic_info->NameLength >> 1 ); 
						}
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_BASIC_INFO_FORMAT_TEXT, 
						basic_info->TitleIndex, 
						time.wYear, 
						time.wMonth, 
						time.wDay, 
						time.wHour, 
						time.wMinute, 
						time.wSecond, 
						time.wMilliseconds ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyNodeInformation: 
			{
				HRESULT hr; 
				INT32 _ret; 
				LPWSTR text_output; 
				size_t remain_len; 
				SYSTEMTIME time; 
				KEY_NODE_INFORMATION *node_info; 

				if( info_size < FIELD_OFFSET( KEY_NODE_INFORMATION, Name ) )
				{
					*text = L'\0'; 
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				node_info = ( KEY_NODE_INFORMATION* )key_info; 

				{
					ULONG cc_max_name_len; 

					cc_max_name_len = ( ULONG )( ( info_size - FIELD_OFFSET( KEY_NODE_INFORMATION, Name ) ) >> 1 ); 

					if( node_info->NameLength > ( cc_max_name_len << 1 ) )
					{
						node_info->NameLength = ( cc_max_name_len << 1 ); 
					}
				}

				do 
				{
					_ret = FileTimeToSystemTime( (FILETIME*) &node_info->LastWriteTime, 
						&time ); 

					if( FALSE == _ret ) 
					{
						set_time_to_null( &time ); 
					}

					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_NAME, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( node_info->NameLength > 0 )
					{
						if( ( remain_len << 1 ) < ( node_info->NameLength >> 1 ) + 1  )
						{
							memcpy( text_output, node_info->Name, ( remain_len << 1 ) - sizeof( WCHAR ) ); 
							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						memcpy( text_output, node_info->Name, node_info->NameLength ); 

						if( text_output[ ( node_info->NameLength >> 1 ) - 1 ] == L'\0' )
						{
							remain_len -= ( ( node_info->NameLength >> 1 ) - 1 ); 
							text_output += ( ( node_info->NameLength >> 1 ) - 1 ); 						
						}
						else
						{
							remain_len -= ( node_info->NameLength >> 1 ); 
							text_output += ( node_info->NameLength >> 1 ); 
						}
					}

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_CLASS, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( node_info->ClassLength > 0 )
					{
						ASSERT( node_info->ClassOffset != 0xFFFFFFFF ); 

						if( ( remain_len << 1 ) < node_info->ClassLength + sizeof( WCHAR ) )
						{
							memcpy( text_output, ( ( BYTE* )node_info + node_info->ClassOffset ), ( remain_len << 1 ) - sizeof( WCHAR ) ); 
							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						memcpy( text_output, ( ( BYTE* )node_info + node_info->ClassOffset ), node_info->ClassLength ); 

						if( text_output[ ( node_info->ClassLength >> 1 ) - 1 ] == L'\0' )
						{
							remain_len -= ( ( node_info->ClassLength >> 1 ) - 1 ); 
							text_output += ( ( node_info->ClassLength >> 1 ) - 1 ); 						
						}
						else
						{
							remain_len -= ( node_info->ClassLength >> 1 ); 
							text_output += ( node_info->ClassLength >> 1 ); 
						}
					}
					else
					{
						ASSERT( node_info->ClassOffset == 0xFFFFFFFF ); 
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_BASIC_INFO_FORMAT_TEXT2, 
						node_info->TitleIndex, 
						time.wYear, 
						time.wMonth, 
						time.wDay, 
						time.wHour, 
						time.wMinute, 
						time.wSecond, 
						time.wMilliseconds ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyFullInformation: 
			{			
				HRESULT hr; 
				INT32 _ret; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG name_len; 
				SYSTEMTIME time; 
				KEY_FULL_INFORMATION *full_info; 

				/**********************************************************************
				可以进行最精确的针对单个字段的判断
				现在使用不定长字段之前的字段长度判断方式。
				**********************************************************************/
				if( info_size < FIELD_OFFSET( KEY_FULL_INFORMATION, Class ) ) 
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				do 
				{
					full_info = ( KEY_FULL_INFORMATION* )key_info; 

					{
						ULONG cc_max_name_len; 

						cc_max_name_len = ( ULONG )( ( info_size - FIELD_OFFSET( KEY_FULL_INFORMATION, Class ) ) >> 1 ); 

						if( full_info->ClassLength > ( cc_max_name_len << 1 ) )
						{
							full_info->ClassLength = ( cc_max_name_len << 1 ); 
						}
					}

					_ret = FileTimeToSystemTime( (FILETIME*) &full_info->LastWriteTime, 
						&time ); 

					if( FALSE == _ret ) 
					{
						set_time_to_null( &time ); 
						//break; 
					}

					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_CLASS, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( full_info->ClassLength > 0 )
					{
						name_len = ( full_info->ClassLength >> 1 ); 

						if( remain_len < name_len + 1 )
						{
							memcpy( text_output, full_info->Class/*( ( BYTE* )full_info + full_info->ClassOffset )*/, ( remain_len << 1 ) - sizeof( WCHAR ) ); 
							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						memcpy( text_output, /*( ( BYTE* )full_info + full_info->ClassOffset )*/full_info->Class, full_info->ClassLength ); 

						if( text_output[ name_len - 1 ] == L'\0' ) 
						{
							remain_len -= ( name_len - 1 ); 
							text_output += ( name_len - 1 ); 						
						}
						else
						{
							text_output[ name_len ] = L'\0'; 
							remain_len -= name_len ; 
							text_output += name_len ; 
						}
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						
						KEY_FULL_INFO_FORMAT_TEXT, 
						full_info->MaxClassLen, 
						full_info->MaxNameLen, 
						full_info->MaxValueNameLen, 
						full_info->MaxValueDataLen, 
						full_info->SubKeys, 
						full_info->Values, 
						full_info->TitleIndex, 

						time.wYear, 
						time.wMonth, 
						time.wDay, 
						time.wHour, 
						time.wMinute, 
						time.wSecond, 
						time.wMilliseconds ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyNameInformation: 
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 
				KEY_NAME_INFORMATION *name_info; 

				name_info = ( KEY_NAME_INFORMATION* )key_info; 

				if( info_size < FIELD_OFFSET( KEY_NAME_INFORMATION, Name ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchCopyExW( text_output, 
					remain_len, 
					KEY_BASIC_INFO_KEY_NAME, 
					&text_output, 
					&remain_len, 
					0 ); 

				if( FAILED( hr ) ) 
				{
					ret = ERROR_BUFFER_OVERFLOW; 
					break; 
				}

				{
					ULONG cc_max_name_len; 

					cc_max_name_len = ( ULONG )( ( info_size - FIELD_OFFSET( KEY_NAME_INFORMATION, Name ) ) >> 1 ); 

					if( name_info->NameLength > ( cc_max_name_len << 1 ) )
					{
						name_info->NameLength = ( cc_max_name_len << 1 ); 
					}
				}

				if( name_info->NameLength > 0 )
				{
					if( ( remain_len << 1 ) < ( name_info->NameLength >> 1 ) + 1 )
					{
						memcpy( text_output, name_info->Name, ( remain_len << 1 ) - sizeof( WCHAR ) ); 
						text_output[ remain_len - 1 ] = L'\0'; 
						remain_len = 0; 
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					memcpy( text_output, name_info->Name, name_info->NameLength ); 

					if( text_output[ ( name_info->NameLength >> 1 ) - 1 ] == L'\0' )
					{
						remain_len -= ( ( name_info->NameLength >> 1 ) - 1 ); 
						text_output += ( ( name_info->NameLength >> 1 ) - 1 ); 						
					}
					else
					{
						remain_len -= ( name_info->NameLength >> 1 ); 
						text_output += ( name_info->NameLength >> 1 ); 
					}

					text_output[ 0 ] = L'\0'; 
				}
			}
			break; 
		case KeyCachedInformation: 
			{
				HRESULT hr; 
				INT32 _ret; 
				SYSTEMTIME time; 
				size_t remain_len; 
				ULONG name_len; 
				LPWSTR text_output; 
				KEY_CACHED_INFORMATION *cache_info; 

				cache_info = ( KEY_CACHED_INFORMATION* )key_info; 

				if( info_size < sizeof( KEY_CACHED_INFORMATION ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					_ret = FileTimeToSystemTime( (FILETIME*) &cache_info->LastWriteTime, 
						&time ); 

					if( _ret == FALSE )
					{
						set_time_to_null( &time ); 
					}

					name_len = ( cache_info->NameLength >> 1 ); 

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_CACHE_INFO_FORMAT_TEXT, 
						name_len, 
						cache_info->MaxNameLen, 
						cache_info->MaxValueNameLen, 
						cache_info->MaxValueDataLen, 
						cache_info->SubKeys, 
						cache_info->Values, 
						cache_info->TitleIndex, 
						time.wYear, 
						time.wMonth, 
						time.wDay, 
						time.wHour, 
						time.wMinute, 
						time.wSecond, 
						time.wMilliseconds ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE );

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyFlagsInformation: 
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				KEY_FLAGS_INFORMATION *flags_info; 

				if( info_size < sizeof( KEY_FLAGS_INFORMATION ) ) 
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				do 
				{
					flags_info = ( KEY_FLAGS_INFORMATION* )key_info; 

					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_USER_TAG, 
						flags_info->UserFlags ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE );

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyVirtualizationInformation: 
			{
				HRESULT hr; 
				KEY_VIRTUALIZATION_INFORMATION *virtual_info; 
				LPWSTR text_output; 
				size_t remain_len; 

				if( info_size < sizeof( KEY_VIRTUALIZATION_INFORMATION ) ) 
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					virtual_info = ( KEY_VIRTUALIZATION_INFORMATION* )key_info; 
					
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_VIRTUAL_INFO_FORMAT_TEXT, 
						virtual_info->VirtualizationCandidate == 1 ? KEY_IN_VIRTUAL_SPACE : L"", 
						virtual_info->VirtualizationEnabled == 1 ? KEY_VIRTUAL_ENABLE : L"", 
						virtual_info->VirtualTarget== 1 ? KEY_VIRTUAL_TAGET : L"", 
						virtual_info->VirtualStore == 1 ? KEY_VIRTUAL_STORE : L"", 
						virtual_info->VirtualSource == 1 ? KEY_VIRTUAL_SOURCE : L"" );  

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyHandleTagsInformation: 
			{
				HRESULT hr; 
				KEY_HANDLE_TAGS_INFORMATION *handle_tag_info; 
				size_t remain_len; 
				LPWSTR text_output; 

				if( info_size < sizeof( KEY_HANDLE_TAGS_INFORMATION ) )
				{
					//*text = L'\0'; 
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					handle_tag_info = ( KEY_HANDLE_TAGS_INFORMATION* )key_info; 
					
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_HANDLE_TAGS_FORMAT_TEXT, 
						handle_tag_info->HandleTags ); 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		default: 
			{
				ret = ERROR_INVALID_PARAMETER; 
				DBG_BP(); 
				*text = L'\0'; 
				*cc_ret_len = 0; 
			}
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

LPCWSTR WINAPI get_reg_value_type_desc( ULONG type )
{
	LPCWSTR type_desc = L""; 

	switch( type )
	{
	case REG_NONE: // No value type
		type_desc = REG_VALUE_NONE; 
		break; 
	case REG_SZ: // Unicode nul terminated string
		type_desc = REG_VALUE_SZ; 
		break; 
	case REG_EXPAND_SZ: // Unicode nul terminated string
		type_desc = REG_VALUE_EXPAND_SZ; 
		break; 
		// (with environment variable references)
	case REG_BINARY: // Free form binary
		type_desc = REG_VALUE_BINARY; 
		break; 
	case REG_DWORD: // 32-bit number
		type_desc = REG_VALUE_DWORD; 
		break; 
	case REG_LINK: // Symbolic Link (unicode)
		type_desc = REG_VALUE_LINK; 
		break; 
	case REG_MULTI_SZ: // Multiple Unicode strings
		type_desc = REG_VALUE_MULTI_SZ; 
		break; 
	case REG_RESOURCE_LIST: // Resource list in the resource map
		type_desc = REG_VALUE_RESOURCE_LIST; 
		break; 
	case REG_FULL_RESOURCE_DESCRIPTOR: // Resource list in the hardware description
		type_desc = REG_VALUE_FULL_RESOURCE_DESCRIPTOR; 
		break; 
	case REG_RESOURCE_REQUIREMENTS_LIST: 
		type_desc = REG_VALUE_RESOURCE_REQUIREMENTS_LIST; 
		break; 
	case REG_QWORD: // 64-bit number
		type_desc = REG_VALUE_QWORD; 
		break; 
	default: 
		break; 
	}

	return type_desc; 
}

LRESULT WINAPI printf_reg_value( LPWSTR text, 
								ULONG cc_buf_len, 
								ULONG value_type, 
								PVOID data, 
								ULONG data_size, 
								ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( data != NULL ); 
		ASSERT( cc_ret_len != NULL ); 
		
		if( data_size == 0 )
		{
			*cc_ret_len = 0; 
			break; 
		}

		switch( value_type )
		{
		case REG_NONE: //                    ( 0 )   // No value type
			*cc_ret_len = 0; 
			break; 
		case REG_SZ: //                      ( 1 )   // Unicode nul terminated string
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 
				ULONG name_len; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_SZ L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					name_len = ( data_size >> 1 ); 

					if( remain_len < name_len + 1 )
					{
						memcpy( text_output, data, ( ( remain_len - 1 ) << 1 ) ); 
						text_output[ remain_len - 1 ] = L'\0'; 
						remain_len = 0; 
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					memcpy( text_output, data, ( ( name_len ) << 1 ) ); 

					if( text_output[ name_len - 1 ] == L'\0' )
					{
						remain_len -= ( name_len - 1 ); 
						text_output += ( name_len - 1 ); 						
					}
					else
					{
						text_output[ name_len ] = L'\0'; 

						remain_len -= name_len; 
						text_output += name_len; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
		case REG_EXPAND_SZ: 
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 
				ULONG name_len; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_EXPAND_SZ L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					name_len = ( data_size >> 1 ); 

					if( remain_len < name_len + 1 )
					{
						memcpy( text_output, data, ( ( remain_len - 1 ) << 1 ) ); 
						text_output[ remain_len - 1 ] = L'\0'; 
						remain_len = 0; 
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					memcpy( text_output, data, ( ( name_len ) << 1 ) ); 

					if( text_output[ name_len - 1 ] == L'\0' )
					{
						remain_len -= ( name_len - 1 ); 
						text_output += ( name_len - 1 ); 						
					}
					else
					{
						text_output[ name_len ] = L'\0'; 

						remain_len -= name_len; 
						text_output += name_len; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
			//               ( 2 )   // Unicode nul terminated string
			// (with environment variable references)
		case REG_BINARY: //                  ( 3 )   // Free form binary
			{
				HRESULT hr; 
				ULONG i; 
				size_t remain_len; 
				LPWSTR text_output; 
				BYTE *_data; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_BINARY L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					_data = ( BYTE* )data; 

#define DATA_LINE_BYTE_TEXT_COUNT 8
					for( i = 0; i + DATA_LINE_BYTE_TEXT_COUNT < data_size; i += DATA_LINE_BYTE_TEXT_COUNT )
					{
						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							L"%02x%02x%02x%02x%02x%02x%02x%02x", 
							_data[ i + 0 ], 
							_data[ i + 1 ], 
							_data[ i + 2 ], 
							_data[ i + 3 ], 
							_data[ i + 4 ], 
							_data[ i + 5 ], 
							_data[ i + 6 ], 
							_data[ i + 7 ] ); 

						if( FAILED( hr ) ) 
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}
					}

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					for( ; i < data_size; i ++ )
					{
						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							L"%02x", 
							_data[ i ] ); 

						if( FAILED( hr ) ) 
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_DWORD: //                   ( 4 )   // 32-bit number
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					ASSERT( data_size >= sizeof( ULONG ) ); 
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						REG_VALUE_DWORD L": %u", 
						*( ULONG* )data ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW;  
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len;  
			}
			break; 
			//case REG_DWORD_LITTLE_ENDIAN: //     ( 4 )   // 32-bit number (same as REG_DWORD)
			//break; 
		case REG_DWORD_BIG_ENDIAN: //        ( 5 )   // 32-bit number
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 
				ULONG big_endian_value; 
				BYTE *_data; 
				
				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					ASSERT( data_size >= sizeof( ULONG ) ); 

					_data = ( BYTE* )data; 
					big_endian_value = ( _data[ 0 ] << 24 | _data[ 1 ] << 16 | _data[ 2 ] << 8 | _data[ 3 ] ); 
					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						REG_VALUE_DWORD_BIG_ENDIAN L": %u", 
						big_endian_value ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			} 
			break; 
		case REG_LINK: //                    ( 6 )   // Symbolic Link (unicode)
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 
				ULONG name_len; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_LINK L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					name_len = ( data_size >> 1 ); 

					if( remain_len < name_len + 1 )
					{
						memcpy( text_output, data, ( ( remain_len - 1 ) << 1 ) ); 
						text_output[ remain_len - 1 ] = L'\0'; 
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					memcpy( text_output, data, ( ( name_len ) << 1 ) ); 
					text_output[ remain_len ] = L'\0'; 

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_MULTI_SZ: //                ( 7 )   // Multiple Unicode strings
			{
				HRESULT hr; 
				size_t old_remain_len; 
				size_t remain_len; 
				LPWSTR text_output; 
				LPWSTR name; 
				ULONG name_len; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_MULTI_SZ L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					_try
					{
					name = ( LPWSTR )data; 
					name[ ( ( data_size & 0xFFFFFFFE ) >> 1 ) - 1 ] = L'\0'; 

					for( ; ; )
					{
						old_remain_len = remain_len; 

						hr = StringCchPrintfExW( text_output, 
							remain_len, 
							&text_output, 
							&remain_len, 
							0, 
							L"%s ", 
							name ); 
					
						if( FAILED( hr ) )
						{
							ret = ERROR_BUFFER_OVERFLOW; 
							break; 
						}

						name_len = old_remain_len - remain_len; 

						name += name_len; 

						if( *name == L'\0' )
						{
							break; 
						}
						else if( ( ( BYTE* )name - ( BYTE* )data ) >= ( LONG )data_size ) 
						{
							dbg_print( MSG_FATAL_ERROR, "the content of the multiple string is invalid (overflowed)\n" ); 
							break; 
						}
					}
					}
					_except( EXCEPTION_EXECUTE_HANDLER )
					{
						dbg_print( MSG_FATAL_ERROR, "exception happen when get the content of the multiple string (%x)\n", 
							GetExceptionCode() ); 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_RESOURCE_LIST: //           ( 8 )   // Resource list in the resource map
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_RESOURCE_LIST L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_FULL_RESOURCE_DESCRIPTOR: // ( 9 )  // Resource list in the hardware description
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_FULL_RESOURCE_DESCRIPTOR L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_RESOURCE_REQUIREMENTS_LIST: // ( 10 )
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						REG_VALUE_RESOURCE_REQUIREMENTS_LIST L": ", 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case REG_QWORD: //                   ( 11 )  // 64-bit number
			{
				HRESULT hr; 
				size_t remain_len; 
				LPWSTR text_output; 

				do
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					ASSERT( data_size >= sizeof( ULONGLONG ) ); 

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						REG_VALUE_QWORD L": %I64u", 
						*( ULONGLONG* )data ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}
				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		default: 
			ret = ERROR_INVALID_PARAMETER; 
			*cc_ret_len = 0; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_reg_value_info_detail( ULONG value_class, 
										 PVOID value_info, 
										 ULONG info_size, 
										 LPWSTR text, 
										 ULONG cc_buf_len, 
										 ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( text != NULL ); 
		ASSERT( cc_ret_len != NULL ); 

		*text = L'\0'; 
		*cc_ret_len = 0; 

		switch( value_class )
		{
		case KeyValueBasicInformation: 
			{
				HRESULT hr; 
				size_t remain_len; 
				ULONG name_len; 
				LPWSTR text_output; 
				KEY_VALUE_BASIC_INFORMATION *basic_info; 

				basic_info = ( KEY_VALUE_BASIC_INFORMATION* )value_info; 

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_NAME, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( basic_info->NameLength > 0 )
					{
						name_len = ( basic_info->NameLength >> 1 ); 

						if( remain_len < name_len + 1 )
						{
							memcpy( text_output, 
								basic_info->Name, 
								( remain_len << 1 ) - sizeof( WCHAR ) ); 

							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_TOO_SMALL; 
							break; 
						}

						memcpy( text_output, basic_info->Name, basic_info->NameLength ); 

						if( text_output[ name_len - 1 ] == L'\0' )
						{
							remain_len -= ( name_len - 1 ); 
							text_output += ( name_len - 1 ); 						
						}
						else
						{
							text_output[ name_len ] = L'\0'; 
							remain_len -= name_len; 
							text_output += name_len; 
						}
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_VALUE_BASIC_INFO_FORMAT_TEXT, 
						basic_info->TitleIndex, 
						get_reg_value_type_desc( basic_info->Type ) ); ; 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyValueFullInformation: 
			{
				HRESULT hr; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 
				ULONG name_len; 
				LPWSTR text_output; 
				KEY_VALUE_FULL_INFORMATION *full_info; 

				full_info = ( KEY_VALUE_FULL_INFORMATION* )value_info; 

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_VALUE_NAME_TEXT, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( full_info->NameLength > 0 )
					{
						name_len = ( full_info->NameLength >> 1 ); 

						if( remain_len < name_len + 1 )
						{
							memcpy( text_output, 
								full_info->Name, 
								( remain_len << 1 ) - sizeof( WCHAR ) ); 

							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_TOO_SMALL; 
							break; 
						}

						memcpy( text_output, full_info->Name, full_info->NameLength ); 

						if( text_output[ name_len - 1 ] == L'\0' )
						{
							remain_len -= ( name_len - 1 ); 
							text_output += ( name_len - 1 ); 						
						}
						else
						{
							text_output[ name_len ] = L'\0'; 
							remain_len -= name_len; 
							text_output += name_len; 
						}
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_VALUE_BASIC_INFO_FORMAT_TEXT, 
						full_info->TitleIndex, 
						get_reg_value_type_desc( full_info->Type ) ); ; 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					{
						ULONG valid_data_size; 
						valid_data_size = full_info->DataLength; 
						if( valid_data_size > info_size - full_info->DataOffset )
						{
							valid_data_size = info_size - full_info->DataOffset; 
						}

						ret = printf_reg_value( text_output, ( ULONG )remain_len, full_info->Type, ( ( BYTE* )full_info + full_info->DataOffset ), valid_data_size, &_cc_ret_len ); 
					}
				
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( _cc_ret_len <= remain_len ); 
					remain_len -= _cc_ret_len; 

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyValuePartialInformation: 
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 
				KEY_VALUE_PARTIAL_INFORMATION *value_part_info; 

				value_part_info = ( KEY_VALUE_PARTIAL_INFORMATION* )value_info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					KEY_VALUE_INDEX_TEXT, 
					value_part_info->TitleIndex ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}

				if( value_part_info->DataLength > info_size - FIELD_OFFSET( KEY_VALUE_PARTIAL_INFORMATION, Data ) )
				{
					value_part_info->DataLength = info_size - FIELD_OFFSET( KEY_VALUE_PARTIAL_INFORMATION, Data ); 
				}

				ret = printf_reg_value( text_output, 
					remain_len, 
					value_part_info->Type, 
					value_part_info->Data, 
					value_part_info->DataLength, 
					&_cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( remain_len >= _cc_ret_len ); 

				remain_len -= _cc_ret_len; 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyValueFullInformationAlign64: 
			{
				HRESULT hr; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 
				ULONG name_len; 
				LPWSTR text_output; 
				KEY_VALUE_FULL_INFORMATION *full_info; 

				full_info = ( KEY_VALUE_FULL_INFORMATION* )value_info; 

				do 
				{
					text_output = text; 
					remain_len = cc_buf_len; 

					hr = StringCchCopyExW( text_output, 
						remain_len, 
						KEY_BASIC_INFO_KEY_NAME, 
						&text_output, 
						&remain_len, 
						0 ); 

					if( FAILED( hr ) ) 
					{
						ret = ERROR_BUFFER_OVERFLOW; 
						break; 
					}

					if( full_info->NameLength > 0 )
					{
						name_len = ( full_info->NameLength >> 1 ); 

						if( remain_len < name_len + 1 )
						{
							memcpy( text_output, 
								full_info->Name, 
								( remain_len << 1 ) - sizeof( WCHAR ) ); 

							text_output[ remain_len - 1 ] = L'\0'; 
							remain_len = 0; 
							ret = ERROR_BUFFER_TOO_SMALL; 
							break; 
						}

						memcpy( text_output, full_info->Name, full_info->NameLength ); 

						if( text_output[ name_len - 1 ] == L'\0' )
						{
							remain_len -= ( name_len - 1 ); 
							text_output += ( name_len - 1 ); 						
						}
						else
						{
							text_output[ name_len ] = L'\0'; 
							remain_len -= name_len; 
							text_output += name_len; 
						}
					}

					hr = StringCchPrintfExW( text_output, 
						remain_len, 
						&text_output, 
						&remain_len, 
						0, 
						KEY_VALUE_BASIC_INFO_FORMAT_TEXT, 
						full_info->TitleIndex, 
						get_reg_value_type_desc( full_info->Type ) ); ; 

					if( FAILED( hr ) )
					{
						ret = ERROR_BUFFER_TOO_SMALL; 
						break; 
					}

					ret = printf_reg_value( text_output, remain_len, full_info->Type, ( ( BYTE* )full_info + full_info->DataOffset ), full_info->DataLength, &_cc_ret_len ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( _cc_ret_len <= remain_len ); 
					remain_len -= _cc_ret_len; 

				}while( FALSE ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		case KeyValuePartialInformationAlign64: 
			{
				HRESULT hr; 
				LPWSTR text_output; 
				size_t remain_len; 
				ULONG _cc_ret_len = 0; 
				KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 *value_part_info; 

				value_part_info = ( KEY_VALUE_PARTIAL_INFORMATION_ALIGN64* )value_info; 

				text_output = text; 
				remain_len = cc_buf_len; 

				hr = StringCchPrintfExW( text_output, 
					remain_len, 
					&text_output, 
					&remain_len, 
					0, 
					KEY_VALUE_PARTIAL_INFO_FORMAT_TEXT, 
					get_reg_value_type_desc( value_part_info->Type ) ); 

				if( FAILED( hr ) )
				{
					ret = ERROR_BUFFER_TOO_SMALL; 
					break; 
				}

				ret = printf_reg_value( text_output, 
					remain_len, 
					value_part_info->Type, 
					value_part_info->Data, 
					value_part_info->DataLength, 
					&_cc_ret_len ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( remain_len >= _cc_ret_len ); 

				*cc_ret_len = cc_buf_len - remain_len; 
			}
			break; 
		default:
			{
				ret = ERROR_INVALID_PARAMETER; 
				DBG_BP(); 
				*text = L'\0'; 
				*cc_ret_len = 0; 
			}
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_sys_regsrv_detail( sys_action_record *action, 
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

		//ACCESS_MASK access; //服务创建/打开权限 
		//ULONG type; //服务类型 
		//ULONG starttype; //服务启动类型 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//PATH_SIZE_T srv_name_len; 
		//WCHAR path_name[ 1 ]; //驱动/服务文件全路径 

		fmt = L"服务(驱动)名 %s 类型 %u 启动类型 %u 权限 0x%0.8x\n"; 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			srv_name = action->do_sys_regsrv.srv_name_ptr; 
		}
		else
		{
			srv_name = action->do_sys_regsrv.path_name + action->do_sys_regsrv.path_len + 1; 
		}

		if( srv_name != NULL )
		{
			ASSERT( srv_name[ action->do_sys_regsrv.srv_name_len ] == L'\0' ); 
		}
		else
		{
			srv_name = L""; 
			ASSERT( action->do_sys_regsrv.srv_name_len == 0 ); 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			srv_name, 
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

LRESULT WINAPI get_sys_opendev_detail( sys_action_record *action, 
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

		//ULONG devtype; //设备类型 
		//ULONG access; //设备打开权限 
		//ULONG share; //设备共享权限 
		//NTSTATUS result; //动作完成结果(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//union
		//{
		//	WCHAR path_name[ 1 ]; //设备路径 

		//	struct 
		//	{
		//		WCHAR *path_name_ptr; 
		//	};
		//};

		fmt = L"设备类型 %u 权限 0x%0.8x 共享权限 0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_w32_postmsg_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//HWND hwnd; //目标窗口句柄 
		//ULONG msg; //窗口消息 
		//WPARAM wparam; //消息参数1 
		//LPARAM lparam; //消息参数2 
		//NTSTATUS result; //动作完成结果(BOOL) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"PID:%u 窗体 0x%0.8x 发送消息 %u 参数1 0x%0.8x 参数2 0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_sendmsg_detail( sys_action_record *action, 
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

		//ULONG target_pid; //目标进程ID 
		//HWND hwnd; //目标窗口句柄 
		//ULONG msg; //窗口消息 
		//WPARAM wparam; //消息参数1 
		//LPARAM lparam; //消息参数2 
		//NTSTATUS result; //动作完成结果(LRESULT) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //目标进程全路径 

		fmt = L"PID:%u 窗体 0x%0.8x 消息 %u 参数1 0x%0.8x 参数2 0x%0.8x\n"; 

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
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

LRESULT WINAPI get_w32_findwnd_detail( sys_action_record *action, 
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
	LPCWSTR cls_name; 

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

		//HWND parent_hwnd; //父窗口句柄 
		//HWND sub_hwnd; //子窗口句柄 
		//NTSTATUS result; //动作完成结果(HWND) 
		//PATH_SIZE_T cls_name_len; 
		//PATH_SIZE_T wnd_name_len; 
		//WCHAR cls_name[ 1 ]; //窗口类名

		fmt = L"父窗体 0x%0.8x 子窗体 0x%0.8x (类名 %s 窗体名 %s)\n"; 

		ASSERT( action->do_w32_findwnd.cls_name[ action->do_w32_findwnd.cls_name_len ] == L'\0' ); 

		if( flags & SAVE_INFO_BY_POINTER )
		{
			cls_name = action->do_w32_findwnd.cls_name_ptr; 
			wnd_name = action->do_w32_findwnd.wnd_name_ptr; 
		}
		else
		{
			cls_name = action->do_w32_findwnd.cls_name; 
			wnd_name = action->do_w32_findwnd.cls_name + action->do_w32_findwnd.cls_name_len + 1; 
		}

		if( cls_name != NULL )
		{
			ASSERT( cls_name[ action->do_w32_findwnd.cls_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_w32_findwnd.cls_name_len == 0 ); 
			cls_name = L""; 
		}

		if( wnd_name != NULL )
		{
			ASSERT( wnd_name[ action->do_w32_findwnd.wnd_name_len ] == L'\0' ); 
		}
		else
		{
			ASSERT( action->do_w32_findwnd.wnd_name_len == 0 ); 
			cls_name = L""; 
		}

		_ret = _snwprintf( tip, 
			ccb_buf_len, 
			fmt, 
			action->do_w32_findwnd.parent_hwnd, 
			action->do_w32_findwnd.sub_hwnd, 
			cls_name, 
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

LRESULT WINAPI _get_icmp_send_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		fmt = ICMP_SEND_FORMAT_TEXT; 

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

LRESULT WINAPI _get_icmp_recv_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags )
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

		fmt = ICMP_SEND_FORMAT_TEXT; 

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

#include <usb.h>
LRESULT WINAPI get_urb_info( LPWSTR text, 
							ULONG cc_buf_len, 
							PVOID data, 
							ULONG data_size, 
							ULONG *cc_remain_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	PURB urb; 
	LPCWSTR func_name = L"UNKNOWN"; 
	LPWSTR _text; 
	size_t _cc_remain_len = 0; 
	LONG i; 

	do 
	{
		ASSERT( data != NULL ); 
		ASSERT( data_size > 0 ); 

		ASSERT( cc_remain_len != NULL ); 
		ASSERT( NULL != text ); 

		*text = L'\0'; 
		*cc_remain_len = cc_buf_len; 

		if( data_size < sizeof( urb->UrbHeader ) )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		urb = ( PURB )data; 

		switch( urb->UrbHeader.Function )
		{
		case URB_FUNCTION_SELECT_CONFIGURATION: 
			func_name = L"URB_FUNCTION_SELECT_CONFIGURATION"; 
			break; 
		case URB_FUNCTION_SELECT_INTERFACE: 
			func_name = L"URB_FUNCTION_SELECT_INTERFACE";  break; 
		case URB_FUNCTION_ABORT_PIPE: 
			func_name = L"URB_FUNCTION_ABORT_PIPE";  
			break; 
		case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL: 
			func_name = L"URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL"; 
			break; 
		case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL: 
			func_name = L"URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL";  break; 
		case URB_FUNCTION_GET_FRAME_LENGTH: 
			func_name = L"URB_FUNCTION_GET_FRAME_LENGTH";  break; 
		case URB_FUNCTION_SET_FRAME_LENGTH: 
			func_name = L"URB_FUNCTION_SET_FRAME_LENGTH";  break; 
		case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER: 
			func_name = L"URB_FUNCTION_GET_CURRENT_FRAME_NUMBER";  break; 
		case URB_FUNCTION_CONTROL_TRANSFER: 
			func_name = L"URB_FUNCTION_CONTROL_TRANSFER";  break; 
		case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER: 
			func_name = L"URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER";  break; 
		case URB_FUNCTION_ISOCH_TRANSFER: 
			func_name = L"URB_FUNCTION_ISOCH_TRANSFER";  break; 
		case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE: 
			func_name = L"URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE";  break; 
		case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE: 
			func_name = L"URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE";  break; 
		case URB_FUNCTION_SET_FEATURE_TO_DEVICE: 
			func_name = L"URB_FUNCTION_SET_FEATURE_TO_DEVICE";  break; 
		case URB_FUNCTION_SET_FEATURE_TO_INTERFACE: 
			func_name = L"URB_FUNCTION_SET_FEATURE_TO_INTERFACE";  break; 
		case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT: 
			func_name = L"URB_FUNCTION_SET_FEATURE_TO_ENDPOINT";  break; 
		case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE: 
			func_name = L"URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE";  break; 
		case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE: 
			func_name = L"URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE";  break; 
		case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT: 
			func_name = L"URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT";  break; 
		case URB_FUNCTION_GET_STATUS_FROM_DEVICE: 
			func_name = L"URB_FUNCTION_GET_STATUS_FROM_DEVICE";  break; 
		case URB_FUNCTION_GET_STATUS_FROM_INTERFACE: 
			func_name = L"URB_FUNCTION_GET_STATUS_FROM_INTERFACE";  break; 
		case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT: 
			func_name = L"URB_FUNCTION_GET_STATUS_FROM_ENDPOINT";  break; 
		case URB_FUNCTION_RESERVED_0X0016: 
			func_name = L"URB_FUNCTION_RESERVED_0X0016";  break; 
		case URB_FUNCTION_VENDOR_DEVICE: 
			func_name = L"URB_FUNCTION_VENDOR_DEVICE";  break; 
		case URB_FUNCTION_VENDOR_INTERFACE: 
			func_name = L"URB_FUNCTION_VENDOR_INTERFACE";  break; 
		case URB_FUNCTION_VENDOR_ENDPOINT: 
			func_name = L"URB_FUNCTION_VENDOR_ENDPOINT";  break; 
		case URB_FUNCTION_CLASS_DEVICE: 
			func_name = L"URB_FUNCTION_CLASS_DEVICE";  break; 
		case URB_FUNCTION_CLASS_INTERFACE: 
			func_name = L"URB_FUNCTION_CLASS_INTERFACE";  break; 
		case URB_FUNCTION_CLASS_ENDPOINT: 
			func_name = L"URB_FUNCTION_CLASS_ENDPOINT";  break; 
		case URB_FUNCTION_RESERVE_0X001D: 
			func_name = L"URB_FUNCTION_RESERVE_0X001D";  break; 
			// previously URB_FUNCTION_RESET_PIPE
		case URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL: 
			func_name = L"URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL";  break; 
		case URB_FUNCTION_CLASS_OTHER: 
			func_name = L"URB_FUNCTION_CLASS_OTHER";  break; 
		case URB_FUNCTION_VENDOR_OTHER: 
			func_name = L"URB_FUNCTION_VENDOR_OTHER";  break; 
		case URB_FUNCTION_GET_STATUS_FROM_OTHER: 
			func_name = L"URB_FUNCTION_GET_STATUS_FROM_OTHER";  break; 
		case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER: 
			func_name = L"URB_FUNCTION_CLEAR_FEATURE_TO_OTHER";  break; 
		case URB_FUNCTION_SET_FEATURE_TO_OTHER: 
			func_name = L"URB_FUNCTION_SET_FEATURE_TO_OTHER";  break; 
		case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT: 
			func_name = L"URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT";  break; 
		case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT: 
			func_name = L"URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT";  break; 
		case URB_FUNCTION_GET_CONFIGURATION: 
			func_name = L"URB_FUNCTION_GET_CONFIGURATION";  break; 
		case URB_FUNCTION_GET_INTERFACE: 
			func_name = L"URB_FUNCTION_GET_INTERFACE";  break; 

		case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE: 
			func_name = L"URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE";  break; 
		case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE: 
			func_name = L"URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE";  break; 

			// USB 2.0 calls start at 0x0030

#if (_WIN32_WINNT >= 0x0501)

		case URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR: 
			func_name = L"URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR";  break; 
		case URB_FUNCTION_SYNC_RESET_PIPE: 
			func_name = L"URB_FUNCTION_SYNC_RESET_PIPE";  break; 
		case URB_FUNCTION_SYNC_CLEAR_STALL: 
			func_name = L"URB_FUNCTION_SYNC_CLEAR_STALL";  break; 

#endif

#if (_WIN32_WINNT >= 0x0600)

		case URB_FUNCTION_CONTROL_TRANSFER_EX: 
			func_name = L"URB_FUNCTION_CONTROL_TRANSFER_EX";  break; 
		case URB_FUNCTION_RESERVE_0X0033: 
			func_name = L"URB_FUNCTION_RESERVE_0X0033";  break; 
		case URB_FUNCTION_RESERVE_0X0034: 
			func_name = L"URB_FUNCTION_RESERVE_0X0034";  break; 

#endif

			// Reserve 0x002B-0x002F
		case URB_FUNCTION_RESERVE_0X002B: 
			func_name = L"URB_FUNCTION_RESERVE_0X002B";  break; 
		case URB_FUNCTION_RESERVE_0X002C: 
			func_name = L"URB_FUNCTION_RESERVE_0X002C";  break; 
		case URB_FUNCTION_RESERVE_0X002D: 
			func_name = L"URB_FUNCTION_RESERVE_0X002D";  break; 
		case URB_FUNCTION_RESERVE_0X002E: 
			func_name = L"URB_FUNCTION_RESERVE_0X002E";  break; 
		case URB_FUNCTION_RESERVE_0X002F: 
			func_name = L"URB_FUNCTION_RESERVE_0X002F";  break; 
		default:
			break; 
		}

		_text = text; 
		_cc_remain_len = cc_buf_len; 

		hr = StringCchPrintfExW( _text, 
			_cc_remain_len, 
			&_text, 
			&_cc_remain_len, 
			0, 
			URB_INFO_FORMAT_TEXT, 
			func_name, 
			urb->UrbHeader.Length ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		try
		{
			switch( urb->UrbHeader.Function )
			{
			case URB_FUNCTION_SELECT_CONFIGURATION: 
				{
					hr = StringCchPrintfExW( _text, 
						_cc_remain_len, 
						&_text, 
						&_cc_remain_len, 
						0, 
						URB_FUNCTION_SELECT_CONFIGURATION_FORMAT_TEXT, 
						urb->UrbSelectConfiguration.ConfigurationHandle, 
						urb->UrbSelectConfiguration.Interface.InterfaceNumber, 
						urb->UrbSelectConfiguration.Interface.AlternateSetting, 
						urb->UrbSelectConfiguration.Interface.Class, 
						urb->UrbSelectConfiguration.Interface.SubClass, 
						urb->UrbSelectConfiguration.Interface.Protocol, 
						urb->UrbSelectConfiguration.Interface.InterfaceHandle, 
						urb->UrbSelectConfiguration.Interface.NumberOfPipes ); 

					if( FAILED( hr ) )
					{
						break; 
					}

#if 0 //have not copy the additional struct data.
					if( NULL != urb->UrbSelectConfiguration.ConfigurationDescriptor ) 
					{
						hr = StringCchPrintfExW( _text, 
							_cc_remain_len, 
							&_text, 
							&_cc_remain_len, 
							0, 
							L"描述符类型:%u 总长度:%u INTERFACE数:%u 配置值:%u 配置索引%u 配置属性 %u 最大电量%u ", 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->bDescriptorType, 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->wTotalLength, 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->bNumInterfaces, 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->bConfigurationValue, 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->iConfiguration, 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->bmAttributes , 
							urb->UrbSelectConfiguration.ConfigurationDescriptor->MaxPower ); 
					}
#endif //0

					for( i = 0; 
						( ULONG )i < urb->UrbSelectConfiguration.Interface.NumberOfPipes; 
						i ++ )
					{
						hr = StringCchPrintfExW( _text, 
							_cc_remain_len, 
							&_text, 
							&_cc_remain_len, 
							0, 
							URB_FUNCTION_SELECT_CONFIGURATION_PIPE_INFO_FORMAT_TEXT, 
							i, 
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].MaximumPacketSize, // Maximum packet size for this pipe
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].EndpointAddress,      // 8 bit USB endpoint address (includes direction)
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].Interval,             // Polling interval in ms if interrupt pipe
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].PipeType,    // PipeType identifies type of transfer valid for this pipe
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].PipeHandle, 
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].MaximumTransferSize, 
							urb->UrbSelectConfiguration.Interface.Pipes[ i ].PipeFlags ); 

						if( FAILED( hr ) )
						{
							break; 
						}
					}
				}
				break; 
			case URB_FUNCTION_SELECT_INTERFACE: 
				break; 
			case URB_FUNCTION_ABORT_PIPE: 
				break; 
			case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL: 
				break; 
			case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL: 
				break; 
			case URB_FUNCTION_GET_FRAME_LENGTH: 
				
				break; 
			case URB_FUNCTION_SET_FRAME_LENGTH: 
				break; 
			case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER: 
				break; 
			case URB_FUNCTION_CONTROL_TRANSFER: 
				{
					hr = StringCchPrintfExW( _text, 
						_cc_remain_len, 
						&_text, 
						&_cc_remain_len, 
						0, 
						URB_FUNCTION_CONTROL_TRANSFER_FORMAT_TEXT, 
						urb->UrbControlTransfer.PipeHandle, 
						urb->UrbControlTransfer.TransferFlags,                 // note: the direction bit will be set by USBD
						urb->UrbControlTransfer.TransferBufferLength, 
						urb->UrbControlTransfer.TransferBuffer, 
						urb->UrbControlTransfer.TransferBufferMDL,              // *optional*
						urb->UrbControlTransfer.UrbLink,                // *optional* link to next urb request
						urb->UrbControlTransfer.SetupPacket[ 0 ], 
						urb->UrbControlTransfer.SetupPacket[ 1 ], 
						urb->UrbControlTransfer.SetupPacket[ 2 ], 
						urb->UrbControlTransfer.SetupPacket[ 3 ], 
						urb->UrbControlTransfer.SetupPacket[ 4 ], 
						urb->UrbControlTransfer.SetupPacket[ 5 ], 
						urb->UrbControlTransfer.SetupPacket[ 6 ], 
						urb->UrbControlTransfer.SetupPacket[ 7 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 0 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 1 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 2 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 3 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 4 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 5 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 6 ], 
						urb->UrbControlTransfer.hca.Reserved8[ 7 ] ); 

					if( FAILED( hr ) ) 
					{
						break; 
					}
				}
				break; 
			case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER: 
				{
					hr = StringCchPrintfExW( _text, 
						_cc_remain_len, 
						&_text, 
						&_cc_remain_len, 
						0, 
						URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER_FORMAT_TEXT, 
						urb->UrbBulkOrInterruptTransfer.PipeHandle, 
						urb->UrbBulkOrInterruptTransfer.TransferFlags,                 // note: the direction bit will be set by USBD
						urb->UrbBulkOrInterruptTransfer.TransferBufferLength, 
						urb->UrbBulkOrInterruptTransfer.TransferBuffer, 
						urb->UrbBulkOrInterruptTransfer.TransferBufferMDL,              // *optional*
						urb->UrbBulkOrInterruptTransfer.UrbLink,                // *optional* link to next urb request
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 0 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 1 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 2 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 3 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 4 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 5 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 6 ], 
						urb->UrbBulkOrInterruptTransfer.hca.Reserved8[ 7 ] ); 

					if( FAILED( hr ) ) 
					{
						break; 
					}
				}
				break; 	
			case URB_FUNCTION_ISOCH_TRANSFER: 
				break; 
			case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE: 
				break; 
			case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE: 
				break; 
			case URB_FUNCTION_SET_FEATURE_TO_DEVICE: 
				break; 
			case URB_FUNCTION_SET_FEATURE_TO_INTERFACE: 
				break; 
			case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT: 
				break; 
			case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE: 
				break; 
			case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE: 
				break; 
			case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT: 
				break; 
			case URB_FUNCTION_GET_STATUS_FROM_DEVICE: 
				break; 
			case URB_FUNCTION_GET_STATUS_FROM_INTERFACE: 
				break; 
			case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT: 
				break; 
			case URB_FUNCTION_RESERVED_0X0016: 
				break; 
			case URB_FUNCTION_VENDOR_DEVICE: 
				break; 
			case URB_FUNCTION_VENDOR_INTERFACE: 
				break; 
			case URB_FUNCTION_VENDOR_ENDPOINT: 
				break; 
			case URB_FUNCTION_CLASS_DEVICE: 
				break; 
			case URB_FUNCTION_CLASS_INTERFACE: 
				break; 
			case URB_FUNCTION_CLASS_ENDPOINT: 
				break; 
			case URB_FUNCTION_RESERVE_0X001D: 
				break; 
				// previously URB_FUNCTION_RESET_PIPE
			case URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL: 
				break; 
			case URB_FUNCTION_CLASS_OTHER: 
				break; 
			case URB_FUNCTION_VENDOR_OTHER: 
				break; 
			case URB_FUNCTION_GET_STATUS_FROM_OTHER: 
				break; 
			case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER: 
				break; 
			case URB_FUNCTION_SET_FEATURE_TO_OTHER: 
				break; 
			case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT: 
				break; 
			case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT: 
				break; 
			case URB_FUNCTION_GET_CONFIGURATION: 
				break; 
			case URB_FUNCTION_GET_INTERFACE: 
				break; 

			case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE: 
				break; 
			case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE: 
				break; 

				// USB 2.0 calls start at 0x0030

#if (_WIN32_WINNT >= 0x0501)

			case URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR: 
				break; 
			case URB_FUNCTION_SYNC_RESET_PIPE: 
				break; 
			case URB_FUNCTION_SYNC_CLEAR_STALL: 
				break; 

#endif

#if (_WIN32_WINNT >= 0x0600)

			case URB_FUNCTION_CONTROL_TRANSFER_EX: 
				break; 
			case URB_FUNCTION_RESERVE_0X0033: 
				break; 
			case URB_FUNCTION_RESERVE_0X0034: 
				break; 

#endif

			// Reserve 0x002B-0x002F
			case URB_FUNCTION_RESERVE_0X002B: 
				break; 
			case URB_FUNCTION_RESERVE_0X002C: 
				break; 
			case URB_FUNCTION_RESERVE_0X002D: 
				break; 
			case URB_FUNCTION_RESERVE_0X002E: 
				break; 
			case URB_FUNCTION_RESERVE_0X002F: 
				break; 
			default:
				break; 
			}
		}
		catch( ... )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			*_text = L'\0'; 
		}
	}while( FALSE );

	*cc_remain_len = ( ULONG )_cc_remain_len; 

	return ret; 
}