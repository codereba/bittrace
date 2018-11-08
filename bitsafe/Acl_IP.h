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