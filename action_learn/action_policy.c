/*
 *
 * Copyright 2010 JiJie Shi
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
#include "acl_define.h"
#include "hash_table.h"
#include "ref_node.h"
#include "rbtree.h"
#include "sys_event_define.h"
#include "action_source.h"
#include "action_check.h"
#include "action_policy.h"
#include "action_group_policy.h"

NTSTATUS CALLBACK _compare_action_policy( policy_holder *src_pol, PVOID policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK compare_action_policy( policy_holder *src_pol, policy_holder *dst_pol )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ntstatus = _compare_action_policy( src_pol, &dst_pol->policy ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS check_sys_action_valid( sys_action_record *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 

		if( FALSE == is_valid_action_type( action->type ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

/*********************************************************************
组策略检测行为过程:
1.当有新的行为发生时，封装成相应的数据结构。
2.找出所有在当前行为源中没有命中的策略。
3.使用没有命中的策略集来检测此行为。
（第1版本先检测一遍所有的策略集。）
*********************************************************************/
NTSTATUS is_policy_hitted( action_source_info *source, 
						  policy_holder *policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 i; 

	do 
	{
		ASSERT( source != NULL ); 
		ASSERT( policy != NULL ); 


		for( i = 0; i < source->ctx.active_state.policy_count; i ++ )
		{
			if( policy == source->ctx.active_state.hitted_polices[ i ] )
			{
				break; 
			}
		}

		if( i == source->ctx.active_state.policy_count )
		{
			ntstatus = STATUS_NOT_FOUND; 
			break; 
		}

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS add_hitted_policy( action_source_info *source, 
						   policy_holder *policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( source != NULL ); 
		ASSERT( policy != NULL ); 

		if( source->ctx.active_state.policy_count 
			>= ARRAY_SIZE( source->ctx.active_state.hitted_polices ) )
		{
			ntstatus = STATUS_BUFFER_OVERFLOW; 
			break; 
		}

		source->ctx.active_state.hitted_polices[ source->ctx.active_state.policy_count ] = policy; 
		source->ctx.active_state.policy_count += 1; 

	}while( FALSE );

	return ntstatus; 
}

LRESULT check_policy_holder_ok( policy_holder *pol_holder )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

/******************************************************************
当行为源发生行为后，策略组检测过程:
1.找出行源是否有对应的策略。
2.如果有，将此策略添加引用，封装在一个指定封包中，记录在行为源的状
态中。
3.对所有的状态查找是否有对应的组策略，如果有，则执行相应的响应。
******************************************************************/
