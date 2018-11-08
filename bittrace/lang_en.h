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

#ifndef __LANG_EN_H__
#define __LANG_EN_H__

#include "string_common.h"

#ifdef LANG_EN
#define KEY_BASIC_INFO_FORMAT_TEXT L"Index: %u, Write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"索引号: %u, 写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u"
#define KEY_BASIC_INFO_FORMAT_TEXT2 L"Index: %u, Last write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"索引号: %u, 写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u"

#define KEY_BASIC_INFO_KEY_NAME L"Key name:" //L"键名:"
#define KEY_BASIC_INFO_KEY_CLASS L"Key class:" //L"键类型名:"
#define KEY_FULL_INFO_FORMAT_TEXT L"Max class name: %u, Max key name: %u, Max value name: %u, Max value size: %u, Max count of sub key: %u, Max count of key value: %u, Index: %u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u" //L"最大类型名长度: %u, 最大键名长度: %u, 最大值名长度: %u, 最大值数据长度: %u, 最大子键数: %u, 最大键值数: %u, 索引号: %u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u", 
#define KEY_CACHE_INFO_FORMAT_TEXT L"Key name length:%u Max sub key name:%u, Max key name: %u, Max value size: %u, Max count of sub key: %u, Max count of key value: %u, Index: %u, Last write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"键名长度:%u 最大子键名长度:%u, 最大键值名长度: %u, 最大键值数据长度: %u, 最大子键数: %u, 最大键值数: %u, 索引号: %u, 最后改写时间: %04u-%02u-%02u %02u:%02u:%02u.%03u", 
#define KEY_USER_TAG L"User tag: 0x%0.8x" //L"用户标记: 0x%0.8x"
#define KEY_VIRTUAL_INFO_FORMAT_TEXT L"%s %s %s %s %s "
#define KEY_IN_VIRTUAL_SPACE L"Virtualization candidate" //L"位于虚拟空间"
#define KEY_VIRTUAL_ENABLE L"Virtualization enabled" //L"键虚拟化"
#define KEY_VIRTUAL_TAGET L"Virtual target" //L"虚拟键"
#define KEY_VIRTUAL_STORE L"Virtual store" //L"位于虚拟存储区"
#define KEY_VIRTUAL_SOURCE L"Virtual source" //L"曾虚拟化"
#define KEY_HANDLE_TAGS_FORMAT_TEXT L"Key handle tags: 0x%0.8x" //L"句柄标记: 0x%0.8x"

#define KEY_VALUE_BASIC_INFO_FORMAT_TEXT L"Value index: %u, Value type: %s" //L"键值索引号: %u, 键值类型: %s", 
#define KEY_VALUE_NAME_TEXT L"Key value name:" //L"键值名:"
#define KEY_VALUE_INDEX_TEXT L"Key value index: %u" //L"索引号: %u", 
#define REG_VALUE_NONE L"None" //L"未知"
#define REG_VALUE_SZ L"String" //L"字符串"
#define REG_VALUE_EXPAND_SZ L"Expand string" //L"可扩展字符串"
#define REG_VALUE_BINARY L"Binary" //L"二进制"
#define REG_VALUE_DWORD L"Dword" //L"双字"
#define REG_VALUE_DWORD_BIG_ENDIAN L"Dword(Big endian)" //L"双字(大字节序)
#define REG_VALUE_LINK L"Link" //L"链接" 链接符号
#define REG_VALUE_MULTI_SZ L"Multi-String" //L"多字符串"
#define REG_VALUE_RESOURCE_LIST L"Resource list" //L"资源列表"
#define REG_VALUE_FULL_RESOURCE_DESCRIPTOR L"Full resource descriptor" //L"完整资源描述符"
#define REG_VALUE_RESOURCE_REQUIREMENTS_LIST L"Resource requirements list" //L"资源需求列表"
#define REG_VALUE_QWORD L"Qword" //L"四字"
#else
#define KEY_BASIC_INFO_FORMAT_TEXT L"索引号: %u, 写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u"
#define KEY_BASIC_INFO_FORMAT_TEXT2 L"索引号: %u, 写入时间: %04u-%02u-%02u %02u:%02u:%02u.%03u"

#define KEY_BASIC_INFO_KEY_NAME L"键名:"
#define KEY_BASIC_INFO_KEY_CLASS L"键类型名:"
#define KEY_FULL_INFO_FORMAT_TEXT L"最大类型名长度: %u, 最大键名长度: %u, 最大值名长度: %u, 最大值数据长度: %u, 最大子键数: %u, 最大键值数: %u, 索引号: %u, 修改时间:%04u-%02u-%02u %02u:%02u:%02u.%03u" 
#define KEY_CACHE_INFO_FORMAT_TEXT L"键名长度:%u 最大子键名长度:%u, 最大键值名长度: %u, 最大键值数据长度: %u, 最大子键数: %u, 最大键值数: %u, 索引号: %u, 最后改写时间: %04u-%02u-%02u %02u:%02u:%02u.%03u" 
#define KEY_USER_TAG L"用户标记: 0x%0.8x"
#define KEY_VIRTUAL_INFO_FORMAT_TEXT L"%s %s %s %s %s "
#define KEY_IN_VIRTUAL_SPACE L"位于虚拟空间"
#define KEY_VIRTUAL_ENABLE L"键虚拟化"
#define KEY_VIRTUAL_TAGET L"虚拟键"
#define KEY_VIRTUAL_STORE L"位于虚拟存储区"
#define KEY_VIRTUAL_SOURCE L"曾虚拟化"
#define KEY_HANDLE_TAGS_FORMAT_TEXT L"句柄标记: 0x%0.8x"

#define KEY_VALUE_BASIC_INFO_FORMAT_TEXT L"键值索引号: %u, 键值类型: %s" 
#define KEY_VALUE_NAME_TEXT L"键值名:"
#define KEY_VALUE_INDEX_TEXT L"索引号: %u" 
#define REG_VALUE_NONE L"未知"
#define REG_VALUE_SZ L"字符串"
#define REG_VALUE_EXPAND_SZ L"可扩展字符串"
#define REG_VALUE_BINARY L"二进制"
#define REG_VALUE_DWORD L"双字"
#define REG_VALUE_DWORD_BIG_ENDIAN L"双字(大字节序)"
#define REG_VALUE_LINK L"链接" //链接符号
#define REG_VALUE_MULTI_SZ L"多字符串"
#define REG_VALUE_RESOURCE_LIST L"资源列表"
#define REG_VALUE_FULL_RESOURCE_DESCRIPTOR L"完整资源描述符"
#define REG_VALUE_RESOURCE_REQUIREMENTS_LIST L"资源需求列表"
#define REG_VALUE_QWORD L"四字"
#endif //LANG_EN

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern pcchar_t all_strings_en[]; 

#ifdef _DEBUG
extern ULONG all_strings_en_count; 
#define LAST_EN_STRING __t( "last en string" )
#endif //_DEBUG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__LANG_EN_H__