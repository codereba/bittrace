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
#include "analyze_model.h"

/*********************************************************************************************
��Ҫ�ȸ�,���ֶȸߵ�ģ��: 
1.����ϵͳ����ʲô����ͨ�ţ�ͨ�ŵ���������������
2.
*********************************************************************************************/
#define MAX_ANALYZE_MODULE_DESC_LEN 200

typedef enum OPERATION_MACRO_TYPE
{
	BA_OPERATION_SEARCH, 
	BA_OPERATION_FILTER, 
	BA_EXTRACT_PARAMETERS, 
	BA_JUMP_TO, 
}OPERATION_MACRO_TYPE, *POPERATION_MACRO_TYPE; 

typedef union _OPERATION_MACRO_PARAMETERS
{
	action_filter_cond filter_condition; 
	ULONG next_command; 
} OPERATION_MACRO_PARAMETERS, *POPERATION_MACRO_PARAMETERS; 

typedef struct _OPREATION_MACRO_UNIT
{
	OPERATION_MACRO_TYPE type; 
	OPERATION_MACRO_PARAMETERS params; 
}OPERATION_MACRO_UNIT, *POPREATION_MACRO_UNIT; 

#define MAX_ANALYZE_SQL_STATEMENT_LENGTH 2048
typedef struct _ANALYZE_ACTION_UNIT
{
	WCHAR sql[ MAX_ANALYZE_SQL_STATEMENT_LENGTH ]; 
} ANALYZE_ACTION_UNIT, *PANALYZE_ACTION_UNIT; 

typedef struct _ANALYZING_MODEL
{
	WCHAR desc[ MAX_ANALYZE_MODULE_DESC_LEN ]; 
	ULONG command_count; 
	OPERATION_MACRO_UNIT command[ 1 ]; 
} ANALYZING_MODEL, *PANALYZING_MODEL; 

/************************************************************************
�����蹦�ܣ�
��һ���еĳ�������¼����������ͳһ��ִ�С�
************************************************************************/
/********************************************************************
����:
��װ�����޸�����Щ�ļ���ע�����?

��������:
ʹ�ð�װ����Ľ���ID������IMAGE·����ͨ��CREATE PROCESS�����ӽ���ID��
ȫ·����ʹ���������е��������й��ˡ�

ʵ��:
�Ƿ���Ա���һ��SQL��䣿���ԡ������ӶȺܸߡ�

����:
build����ִ������Щ����?

��������:
��¼build�������ɵ����еĽ��̣������ӽ������ɵĽ��̡������н������е�
�����¼������

ʵ��:
SQL��䣬������ű�����
*********************************************************************/

#include <vecotr>
using std; 

typedef vector< ANALYZING_MODEL* > BA_ANALYZE_MODELS; 

BA_ANALYZE_MODELS ba_models; 

INLINE LRESULT WINAPI copy_analyzing_modal( ANALYZING_MODEL *src, 
										   ANALYZING_MODEL *dst, 
										   ULONG buf_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG size; 
	
	ASSERT( src != NULL ); 
	ASSERT( dst != NULL ); 

	do 
	{
		size = FIELD_OFFSET( ANALYZING_MODEL, command ) 
			+ sizeof( OPERATION_MACRO_UNIT ) * src->command_count; 


		if( buf_size < size )
		{
			ret = ERROR_BUFFER_TOO_SMALL; 
			break; 
		}

		memcpy( dst, src, size ); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI add_analyze_modal( ANALYZING_MODEL *modal )
{
	LRESULT ret = ERROR_SUCCESS; 
	ANALYZING_MODEL *_modal = NULL; 

	do 
	{
		ASSERT( modal != NULL ); 

		_modal = ( ANALYZING_MODEL* )malloc( sizeof( ANALYZING_MODEL ) ); 
	
		if( NULL == _modal )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = copy_analyzing_modal( modal, _modal ); 

	}while( FALSE );

	if( NULL != _modal )
	{
		free( _modal ); 
	}
	return ret; 
}

typedef struct _OPERATION_MACRO_RESULT 
{
	ULONG next_instruction; 
} OPERATION_MACRO_RESULT, *POPERATION_MACRO_RESULT; 

LRESULT WINAPI execute_ba_command( OPERATION_MACRO_UNIT *command, OPERATION_MACRO_RESULT *result )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( command != NULL ); 

		switch( command->type )
		{
		case BA_EXTRACT_PARAMETERS: 
			break; 
		case BA_OPERATION_FILTER: 
			break; 
		case BA_OPERATION_SEARCH: 
			break; 
		case BA_JUMP_TO: 
			result->next_instruction = command->params.next_command; 
			break; 
		default: 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_analyzing_modal( ANALYZING_MODEL *modal )
{
	LRESULT ret = ERROR_SUCCESS; 
	OPERATION_MACRO_RESULT result; 
	INT32 i; 

	do 
	{
		ASSERT( modal ! =NULL ); 

		i = 0; 

		for( ; ; )
		{
			if( i == modal->command_count )
			{
				break; 
			}
			
			ret = execute_ba_command( &modal->command[ i ], &result ); 
			
			if( ret != ERROR_SUCCESS )
			{
				log_trace( ( MSG_FATAL_ERROR, "execute the ba command error\n" ) ); 
			}

#define INVALID_COMMAND_INDEX  ( ULONG )( -1 ) 

			if( result->next_instruction != INVALID_COMMAND_INDEX )
			{
				i = result->next_instruction; 
			}
			else
			{
				i ++; 
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_analyzing_models()
{
	LRESULT ret = ERROR_SUCCESS; 
	BA_ANALYZE_MODELS::iterator it; 

	for( it = ba_models.begin(); it != ba_models.end(); it ++ )
	{
		ASSERT( *it != NULL ); 
		free( ( *it ) ); 
	}

	ba_models.clear(); 

	return ret; 
}