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

#ifndef __ACL_DOMAIN_H__
#define __ACL_DOMAIN_H__
#include "Acl_IP.h"
#include "Acl_Common.h"

#pragma pack( push )
#pragma pack( 1 )

typedef struct __FILTER_DOMAIN_NAME
{
	ULONG Length;
	CHAR Name[ 1 ];
} FILTER_DOMAIN_NAME, *PFILTER_DOMAIN_NAME;

typedef struct __FILTER_IPV4_ADDR_RULE
{
	LIST_ENTRY IPListEntry;
	struct __FILTER_IPV4_ADDR_RULE *NameListNext;
	struct __FILTER_DOMAIN_NAME_RULE *domain_name; 
	FILTER_IPV4_ADDR IPAddr;
} FILTER_IPV4_ADDR_RULE, *PFILTER_IPV4_ADDR_RULE;

typedef struct __FILTER_DOMAIN_NAME_RULE
{
	struct __FILTER_DOMAIN_NAME_RULE *Next;
	struct __FILTER_IPV4_ADDR_RULE *OwnIPs;
	FILTER_DOMAIN_NAME Name;
} FILTER_DOMAIN_NAME_RULE, *PFILTER_DOMAIN_NAME_RULE;

#define NAME_RULE_TYPE 0
#define IP_RULE_TYPE 1
#define RULE_TYPE_NUM 2

typedef union __FILTER_INFO
{
	FILTER_IPV4_ADDR IPV4Addr;
	FILTER_DOMAIN_NAME Name;
} FILTER_INFO, *PFILTER_INFO;

typedef union __FILTER_RULE
{
	FILTER_IPV4_ADDR_RULE IPV4Addr;
	FILTER_DOMAIN_NAME_RULE DomainName;
} FILTER_RULE, *PFILTER_RULE;

#ifdef SEVEN_FW_SYSMODULE

typedef struct __FILTER_NAME_RULE_HASH_TABLE
{
	PFILTER_DOMAIN_NAME_RULE *HashTable; 
	spin_lock RWLock;
	ULONG Size;
} FILTER_NAME_RULE_HASH_TABLE, *PFILTER_NAME_RULE_HASH_TABLE;

typedef struct __FILTER_IPV4ADDR_RULE_HASH_TABLE
{
	LIST_ENTRY *HashTable;
	spin_lock RWLock;
	ULONG Size;
} FILTER_IPV4ADDR_RULE_HASH_TABLE, *PFILTER_IPV4ADDR_RULE_HASH_TABLE;

typedef union __HASH_TABLE_ELEMENT
{
	PFILTER_DOMAIN_NAME_RULE NameElement;
	LIST_ENTRY IPElement;
} HASH_TABLE_ELEMENT, *PHASH_TABLE_ELEMENT;

typedef struct __FILTER_RULE_HASH_TABLE
{
	PVOID HashTable;
	spin_lock RWLock;
	ULONG Size;
} FILTER_RULE_HASH_TABLE, *PFILTER_RULE_HASH_TABLE;

#endif

typedef struct __DOMAIN_NAME_ACL
{
	ULONG Size;
	ULONG Flags;
	FILTER_DOMAIN_NAME Names[ 0 ]; // end by all zero item;
} DOMAIN_NAME_ACL, *PDOMAIN_NAME_ACL;

#pragma pack( pop )

#define ERR_BUFF_OVERFLOW 0x11
#define ERR_ALREADY_SET_VALUE 0x12

typedef NTSTATUS ( *SET_RULE_ADDITION_INFO )( PFILTER_RULE Rule, PVOID Context, ULONG Mode );

#ifdef SEVEN_FW_SYSMODULE
#ifdef __cplusplus
extern "C" { 
#endif
NTSTATUS InitAllAclList();
VOID UninitAllAclList();
NTSTATUS IsBlockedName( CHAR* DomainName, ULONG Length, BYTE Protocol, PFILTER_DOMAIN_NAME_RULE *FoundName, BYTE *FoundType, BYTE *FoundWMode, INT32 *blocked );
BOOL IsBlockedIP( ULONG IPAddr, BYTE Protocol, PFILTER_IPV4_ADDR_RULE *FoundIP, BYTE *FoundType, BYTE *FoundWMode );
NTSTATUS SetNameAcl( PDOMAIN_NAME_ACL NameAcl, BOOL Mode );
NTSTATUS get_domain_name_from_ipv4( ULONG ipv4_addr, CHAR *domain_name, ULONG length, ULONG hash_tbl_type ); 
NTSTATUS SetFilterRule( PFILTER_INFO Info, BYTE Type, ULONG Flags, BOOL IsAdd, SET_RULE_ADDITION_INFO SetRuleAdditionInfoFunc, PVOID Context );
NTSTATUS SetFilterRuleAddition( PFILTER_RULE Rule, PVOID Context, ULONG Mode );
NTSTATUS AddTransactionId( USHORT Id, PFILTER_DOMAIN_NAME_RULE Name, BYTE Type );
VOID FindTransactionId( USHORT Id, PFILTER_DOMAIN_NAME_RULE *Name, BYTE *Type );
NTSTATUS InitTransactionIdCache();
VOID DumpAllIPRuleInfo( PFILTER_DOMAIN_NAME_RULE Name );
NTSTATUS DeleteAllNameRule( ULONG Flags );
VOID DumpRuleInfo( PFILTER_INFO Info, BYTE Type, CHAR* Title ); 
#ifdef __cplusplus
}
#endif

#endif
#endif //__ACL_DOMAIN_H__