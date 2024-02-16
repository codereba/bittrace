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

#ifndef __LANG_EN_H__
#define __LANG_EN_H__

#include "string_common.h"

#ifdef LANG_EN
#define KEY_BASIC_INFO_FORMAT_TEXT L"Index: %u, Write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"������: %u, д��ʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u"
#define KEY_BASIC_INFO_FORMAT_TEXT2 L"Index: %u, Last write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"������: %u, д��ʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u"

#define KEY_BASIC_INFO_KEY_NAME L"Key name:" //L"����:"
#define KEY_BASIC_INFO_KEY_CLASS L"Key class:" //L"��������:"
#define KEY_FULL_INFO_FORMAT_TEXT L"Max class name: %u, Max key name: %u, Max value name: %u, Max value size: %u, Max count of sub key: %u, Max count of key value: %u, Index: %u, Last write time:%04u-%02u-%02u %02u:%02u:%02u.%03u" //L"�������������: %u, ����������: %u, ���ֵ������: %u, ���ֵ���ݳ���: %u, ����Ӽ���: %u, ����ֵ��: %u, ������: %u, �޸�ʱ��:%04u-%02u-%02u %02u:%02u:%02u.%03u", 
#define KEY_CACHE_INFO_FORMAT_TEXT L"Key name length:%u Max sub key name:%u, Max key name: %u, Max value size: %u, Max count of sub key: %u, Max count of key value: %u, Index: %u, Last write time: %04u-%02u-%02u %02u:%02u:%02u.%03u" //L"��������:%u ����Ӽ�������:%u, ����ֵ������: %u, ����ֵ���ݳ���: %u, ����Ӽ���: %u, ����ֵ��: %u, ������: %u, ����дʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u", 
#define KEY_USER_TAG L"User tag: 0x%0.8x" //L"�û����: 0x%0.8x"
#define KEY_VIRTUAL_INFO_FORMAT_TEXT L"%s %s %s %s %s "
#define KEY_IN_VIRTUAL_SPACE L"Virtualization candidate" //L"λ������ռ�"
#define KEY_VIRTUAL_ENABLE L"Virtualization enabled" //L"�����⻯"
#define KEY_VIRTUAL_TAGET L"Virtual target" //L"�����"
#define KEY_VIRTUAL_STORE L"Virtual store" //L"λ������洢��"
#define KEY_VIRTUAL_SOURCE L"Virtual source" //L"�����⻯"
#define KEY_HANDLE_TAGS_FORMAT_TEXT L"Key handle tags: 0x%0.8x" //L"������: 0x%0.8x"

#define KEY_VALUE_BASIC_INFO_FORMAT_TEXT L"Value index: %u, Value type: %s" //L"��ֵ������: %u, ��ֵ����: %s", 
#define KEY_VALUE_NAME_TEXT L"Key value name:" //L"��ֵ��:"
#define KEY_VALUE_INDEX_TEXT L"Key value index: %u" //L"������: %u", 
#define REG_VALUE_NONE L"None" //L"δ֪"
#define REG_VALUE_SZ L"String" //L"�ַ���"
#define REG_VALUE_EXPAND_SZ L"Expand string" //L"����չ�ַ���"
#define REG_VALUE_BINARY L"Binary" //L"������"
#define REG_VALUE_DWORD L"Dword" //L"˫��"
#define REG_VALUE_DWORD_BIG_ENDIAN L"Dword(Big endian)" //L"˫��(���ֽ���)
#define REG_VALUE_LINK L"Link" //L"����" ���ӷ���
#define REG_VALUE_MULTI_SZ L"Multi-String" //L"���ַ���"
#define REG_VALUE_RESOURCE_LIST L"Resource list" //L"��Դ�б�"
#define REG_VALUE_FULL_RESOURCE_DESCRIPTOR L"Full resource descriptor" //L"������Դ������"
#define REG_VALUE_RESOURCE_REQUIREMENTS_LIST L"Resource requirements list" //L"��Դ�����б�"
#define REG_VALUE_QWORD L"Qword" //L"����"
#else
#define KEY_BASIC_INFO_FORMAT_TEXT L"������: %u, д��ʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u"
#define KEY_BASIC_INFO_FORMAT_TEXT2 L"������: %u, д��ʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u"

#define KEY_BASIC_INFO_KEY_NAME L"����:"
#define KEY_BASIC_INFO_KEY_CLASS L"��������:"
#define KEY_FULL_INFO_FORMAT_TEXT L"�������������: %u, ����������: %u, ���ֵ������: %u, ���ֵ���ݳ���: %u, ����Ӽ���: %u, ����ֵ��: %u, ������: %u, �޸�ʱ��:%04u-%02u-%02u %02u:%02u:%02u.%03u" 
#define KEY_CACHE_INFO_FORMAT_TEXT L"��������:%u ����Ӽ�������:%u, ����ֵ������: %u, ����ֵ���ݳ���: %u, ����Ӽ���: %u, ����ֵ��: %u, ������: %u, ����дʱ��: %04u-%02u-%02u %02u:%02u:%02u.%03u" 
#define KEY_USER_TAG L"�û����: 0x%0.8x"
#define KEY_VIRTUAL_INFO_FORMAT_TEXT L"%s %s %s %s %s "
#define KEY_IN_VIRTUAL_SPACE L"λ������ռ�"
#define KEY_VIRTUAL_ENABLE L"�����⻯"
#define KEY_VIRTUAL_TAGET L"�����"
#define KEY_VIRTUAL_STORE L"λ������洢��"
#define KEY_VIRTUAL_SOURCE L"�����⻯"
#define KEY_HANDLE_TAGS_FORMAT_TEXT L"������: 0x%0.8x"

#define KEY_VALUE_BASIC_INFO_FORMAT_TEXT L"��ֵ������: %u, ��ֵ����: %s" 
#define KEY_VALUE_NAME_TEXT L"��ֵ��:"
#define KEY_VALUE_INDEX_TEXT L"������: %u" 
#define REG_VALUE_NONE L"δ֪"
#define REG_VALUE_SZ L"�ַ���"
#define REG_VALUE_EXPAND_SZ L"����չ�ַ���"
#define REG_VALUE_BINARY L"������"
#define REG_VALUE_DWORD L"˫��"
#define REG_VALUE_DWORD_BIG_ENDIAN L"˫��(���ֽ���)"
#define REG_VALUE_LINK L"����" //���ӷ���
#define REG_VALUE_MULTI_SZ L"���ַ���"
#define REG_VALUE_RESOURCE_LIST L"��Դ�б�"
#define REG_VALUE_FULL_RESOURCE_DESCRIPTOR L"������Դ������"
#define REG_VALUE_RESOURCE_REQUIREMENTS_LIST L"��Դ�����б�"
#define REG_VALUE_QWORD L"����"
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