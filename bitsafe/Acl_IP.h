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

#ifndef __ACL_IP_H__
#define __ACL_IP_H__

#include "Acl_Common.h"

#pragma pack( push )
#pragma pack( 1 )

#ifndef TEST_IN_RING3
typedef struct __FILTER_IPV4_ADDR
{
	ULONG BlockIP;
} FILTER_IPV4_ADDR, *PFILTER_IPV4_ADDR;
#endif

typedef struct __FILTER_IPV4_SUB_NET_ADDR
{
	ULONG BlockNetIP;
	ULONG BlockNetMask;
} FILTER_IPV4_SUB_NET_ADDR, *PFILTER_IPV4_SUB_NET_ADDR;

typedef struct __FILTER_IPV4_SUB_NET_REGION
{
	ULONG BlockNetIPBegin;
	ULONG BlockNetIPEnd;
	ULONG BlockNetMask;
} FILTER_IPV4_SUB_NET_REGION, *PFILTER_IPV4_SUB_NET_REGION;

typedef struct __IPV4_SUB_NET_ADDR_LIST
{
	ULONG RuleNum;
	ULONG Flags;
	PFILTER_IPV4_SUB_NET_ADDR Rules;
} IPV4_SUB_NET_ADDR_LIST, *PIPV4_SUB_NET_ADDR_LIST;

typedef struct __IPV4_SUB_NET_REGION_LIST
{
	ULONG RuleNum;
	ULONG Flags;
	PFILTER_IPV4_SUB_NET_REGION Rules;
} IPV4_SUB_NET_REGION_LIST, *PIPV4_SUB_NET_REGION_LIST;

#ifdef SEVEN_FW_SYSMODULE
typedef struct __RULE_LIST
{
	ULONG RuleNum;
	FILTER_IPV4_ADDR *Rules;
	spin_lock RWLock;
} RULE_LIST;

typedef struct __IP_RULE_DYN_LIST
{
	ULONG IPNum;
	ULONG IPMaxNum;
	PFILTER_IPV4_ADDR IPRules;
	spin_lock RWLock;
} IP_RULE_DYN_LIST, *PIP_RULE_DYN_LIST;

#endif

typedef struct __RULE_INFO
{
	ULONG RuleNum;
	ULONG Flags;
	FILTER_IPV4_ADDR Rules[ 0 ];
} RULE_INFO;

#pragma pack( pop )

#endif //__ACL_IP_H__