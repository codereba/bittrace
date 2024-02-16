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
#include "acl_define.h"
#include "action_type_parse.h"

/***********************************************************************************
make sql string by compiled or by literal string.
must know the type of the member.
***********************************************************************************/ 

LPCWSTR param_type_fmts[ MAX_PARAM_TYPE ] =
{
	L"%d", //INT8_TYPE, 
		L"%d", //INT16_TYPE,
		L"%d", //INT32_TYPE, 
		L"%I64d", //INT64_TYPE, 
		L"%u", //UINT8_TYPE, 
		L"%u", //UINT16_TYPE, 
		L"%u", //UINT32_TYPE, 
		L"%I64u", //UINT64_TYPE, 
		L"0x%0.8x", //PTR_TYPE, 
		L"%s", //STRING_TYPE, 
		L"%ws", //WSTRING_TYPE, 
		//L"", //MAX_PARAM_TYPE, 
};

#define set_param_info( __param, __type, member, val ) ( __param ).type = ( __type ); ( __param ).data.member## = ( val );

NTSTATUS print_param_to_string( LPWSTR param_string, 
							   ULONG ccb_buf_len, 
							   param_info *info )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 ret; 

	do 
	{
		if( NULL == info )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		switch( info->type )
		{
		case INT8_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.int8_val ); 
			break; 

		case INT16_TYPE: 
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.int16_val ); 
			break; 

		case INT32_TYPE: 
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.int32_val ); 

			break; 

		case INT64_TYPE: 
			ret = _snwprintf( param_string, ccb_buf_len, L"%I64u", info->data.int64_val ); 

			break; 

		case UINT8_TYPE: 
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.uint8_val ); 

			break; 

		case UINT16_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.uint16_val ); 

			break; 

		case UINT32_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%u", info->data.uint32_val ); 

			break; 

		case UINT64_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%I64u", info->data.uint64_val ); 

			break; 

		case PTR_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%p", info->data.ptr_val ); 

			break; 

		case STRING_PARAM:
			ret = _snwprintf( param_string, ccb_buf_len, L"%s", info->data.string_val ); 

			break; 

		case WSTRING_TYPE:
			ret = _snwprintf( param_string, ccb_buf_len, L"%ws", info->data.wstring_val); 

			break; 
		default:
			ASSERT( FALSE && "invalid parameter type" ); 
			break; 
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_exec_create( exec_create *creat, param_info params[], ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( creat != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = EXEC_CREATE_PARAM_COUNT; 

#ifdef _DEBUG
		if( creat == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < EXEC_CREATE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, creat->pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, creat->image_base ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, creat->path_len ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, creat->cmd_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, creat->path_name ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, creat->path_name
			+ creat->path_len ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_exec_destroy( exec_destroy *destroy, 
							param_info params[], 
							ULONG max_count, 
							ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( destroy != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#ifdef _DEBUG
		if( destroy == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG
		
		*count_out = EXEC_DESTROY_PARAM_COUNT; 

		if( max_count < EXEC_DESTROY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, destroy->pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, destroy->path_len ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, destroy->cmd_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, destroy->path_name ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, destroy->path_name
			+ destroy->path_len ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_exec_module_load_param( exec_module_load *module_load, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( module_load != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#ifdef _DEBUG
		if( module_load == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		*count_out = EXEC_MODULE_LOAD_PARAM_COUNT; 

		if( max_count < EXEC_MODULE_LOAD_PARAM_COUNT  )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], PTR_TYPE, uint32_val, module_load->base ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, module_load->size ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, module_load->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, module_load->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_touch_param( file_touch *action, 
									 param_info *params, 
									 ULONG max_count, 
									 ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = FILE_TOUCH_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_TOUCH_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//ULONG access; //�ļ�����Ȩ�� 
		//ULONG alloc_size; //�ļ���ʼ���� 
		//ULONG attrib; //�ļ����� 
		//ULONG share; //�ļ��������� 
		//ULONG disposition; //�ļ���/����ѡ�� 
		//ULONG options; //�ļ���/����ѡ�� 
		////ULONG result; //������ɽ��(NTSTATUS) 
		//PATH_SIZE_T path_len; 
		//WCHAR path_name[ 1 ]; //�ļ�ȫ·�� 

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->alloc_size ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->attrib ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->share ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->disposition ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->options ); 
		//set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 7 ], WSTRING_TYPE, wstring_val, action->path_name ); 
	}while( FALSE );

	return ntstatus; 
}

/*******************************************************************************************************
2014.11.02 17:00 
ͨ�����ԣ������ļ��򿪲�����Ŀ���ļ���·������ȷ��������û����ȷ��Ŀ¼��0��β��
����������£�
\Device\HarddiskVolume10\*\bittrace\ffile source='...
����ԭ����Ҫ���м��
*******************************************************************************************************/

NTSTATUS parse_file_open_param( file_open *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = FILE_OPEN_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_OPEN_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->alloc_size ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->attrib ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->share ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->disposition ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->options ); 
		//set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 7 ], WSTRING_TYPE, wstring_val, action->path_name );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_read_param( _file_read *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = FILE_READ_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_READ_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->offset ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_write_param( _file_write *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_WRITE_PARAM_COUNT 4
		*count_out = FILE_WRITE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_WRITE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->offset ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_modified_param( file_modified *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_MODIFIED_PARAM_COUNT 2
		*count_out = FILE_MODIFIED_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_MODIFIED_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_remove_param( file_remove *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_REMOVE_PARAM_COUNT 2
		*count_out = FILE_REMOVE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_REMOVE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_rename_param( file_rename *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_RENAME_PARAM_COUNT 5
		*count_out = FILE_RENAME_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_RENAME_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->replace_existing ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 

		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->new_name_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name + action->path_len ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_truncate_param( file_truncate *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_TRUNCATE_PARAM_COUNT 3
		*count_out = FILE_TRUNCATE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_TRUNCATE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT64_TYPE, uint64_val, action->eof ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 

		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_chmod_param( file_chmod *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_CHANGE_MOD_PARAM_COUNT 3
		*count_out = FILE_CHANGE_MOD_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_CHANGE_MOD_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->attrib ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 

		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_file_set_sec_param( file_setsec *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define FILE_SET_SEC_PARAM_COUNT 2
		*count_out = FILE_SET_SEC_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < FILE_SET_SEC_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_open_key_param( reg_openkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_OPEN_KEY_PARAM_COUNT 3
		*count_out = REG_OPEN_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_OPEN_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_make_key_param( reg_mkkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MAKE_KEY_PARAM_COUNT 3
		*count_out = REG_MAKE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MAKE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_remove_key_param( reg_rmkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_REMOVE_KEY_PARAM_COUNT 2
		*count_out = REG_REMOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_REMOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_get_info_param( reg_getinfo *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MOVE_KEY_PARAM_COUNT 4
		*count_out = REG_MOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->info_class ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->info_size ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}


NTSTATUS parse_reg_set_info_param( reg_setinfo *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MOVE_KEY_PARAM_COUNT 4
		*count_out = REG_MOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->info_class ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->info_size ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}


NTSTATUS parse_reg_enum_info_param( reg_enuminfo *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MOVE_KEY_PARAM_COUNT 4
		*count_out = REG_MOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->index ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->info_class ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->info_size ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}


NTSTATUS parse_reg_enum_value_param( reg_enum_val *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MOVE_KEY_PARAM_COUNT 4
		*count_out = REG_MOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->index ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->info_class ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->info_size ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_move_key_param( reg_mvkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_MOVE_KEY_PARAM_COUNT 4
		*count_out = REG_MOVE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_MOVE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->new_name_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name + action->path_len ); 
		
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_remove_value_param( reg_rmval *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_REMOVE_VALUE_PARAM_COUNT 2
		*count_out = REG_REMOVE_VALUE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_REMOVE_VALUE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_get_value_param( reg_getval *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_GET_VALUE_PARAM_COUNT 6
		*count_out = REG_GET_VALUE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_GET_VALUE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->info_size ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->val_name_len ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 

		{
			set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name 
				+ action->path_len + 1 ); 
		}
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_set_value_param( reg_setval *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_SET_VALUE_PARAM_COUNT 6
		*count_out = REG_SET_VALUE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_SET_VALUE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->val_name_len ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_load_key_param( reg_loadkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_LOAD_KEY_PARAM_COUNT 4
		*count_out = REG_LOAD_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_LOAD_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->hive_name_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name + action->path_len ); 
		
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_repl_key_param( reg_replkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_REPLACE_KEY_PARAM_COUNT 6
		*count_out = REG_REPLACE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_REPLACE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->hive_name_len ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->new_hive_name_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len ); 

		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len 
			+ action->hive_name_len ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_reg_restore_key_param( reg_rstrkey *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define REG_RESTORE_KEY_PARAM_COUNT 4
		*count_out = REG_RESTORE_KEY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < REG_RESTORE_KEY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->hive_name_len );  
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_proc_exec_param( proc_exec *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_EXEC_PARAM_COUNT 3
		*count_out = PROC_EXEC_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_EXEC_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_proc_open_param( proc_open *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_OPEN_PARAM_COUNT 4
		*count_out = PROC_OPEN_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_OPEN_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->access ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_proc_debug_param( proc_debug *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_DEBUG_PARAM_COUNT 3
		*count_out = PROC_DEBUG_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_DEBUG_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_proc_resume_param( proc_resume *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_RESUME_PARAM_COUNT 3
		*count_out = PROC_DEBUG_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_RESUME_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_proc_kill_param( proc_kill *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_KILL_PARAM_COUNT 4
		*count_out = PROC_KILL_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_KILL_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->exitcode ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_proc_job_param( proc_job *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_JOB_PARAM_COUNT 3
		*count_out = PROC_JOB_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_JOB_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_proc_page_prot_param( proc_pgprot *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_PAGE_PROT_PARAM_COUNT 7
		*count_out = PROC_PAGE_PROT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_PAGE_PROT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->base ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->count );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->attrib ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->bytes_changed ); 
		//set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_proc_free_vm_param( proc_freevm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_FREE_VM_PARAM_COUNT 7
		*count_out = PROC_FREE_VM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_FREE_VM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->base ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->count );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->bytes_freed ); 
		//set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->path_len );  
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_proc_write_vm_param( proc_writevm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_WRITE_VM_PARAM_COUNT 6
		*count_out = PROC_WRITE_VM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_WRITE_VM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->base ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->data_len );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->bytes_written ); 
		//set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name );  

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_proc_read_vm_param( proc_readvm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_READ_VM_PARAM_COUNT 7
		*count_out = PROC_READ_VM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_READ_VM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->base ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->data_len );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->bytes_read ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->path_name );  

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_send_action( net_send *action, 
							   param_info *params, 
							   ULONG max_count, 
							   ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_SEND_PARAM_COUNT 4
		*count_out = NET_SEND_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_SEND_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocol ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->ip_addr ); 
		set_param_info( params[ 3 ], UINT16_TYPE, uint16_val, action->port );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS parse_thread_remote_param( thrd_remote *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_REMOTE_PARAM_COUNT 8
		*count_out = PROC_THREAD_REMOTE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_REMOTE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->access );  
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->suspended ); 
		set_param_info( params[ 4 ], PTR_TYPE, ptr_val, action->start_vaddr ); 
		set_param_info( params[ 5 ], PTR_TYPE, ptr_val, action->thread_param ); 
		//set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 6 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 7 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_set_context_param( thrd_setctxt *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_SET_CONTEXT_PARAM_COUNT 4
		*count_out = PROC_THREAD_SET_CONTEXT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_SET_CONTEXT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_suspend_param( thrd_suspend *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_SUSPEND_PARAM_COUNT 4
		*count_out = PROC_THREAD_SUSPEND_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_SUSPEND_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result );  
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_kill_param( thrd_kill *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_KILL_PARAM_COUNT 5
		*count_out = PROC_THREAD_KILL_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_KILL_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->exitcode );  
		//set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len );
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_thread_queue_apc_param( thrd_queue_apc *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define PROC_THREAD_QUEUE_APC_PARAM_COUNT 4
		*count_out = PROC_THREAD_QUEUE_APC_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < PROC_THREAD_QUEUE_APC_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->target_tid ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len );
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name );

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_open_phys_mm_param( sys_open_physmm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_OPEN_PHYSICAL_MM_PARAM_COUNT 1
		*count_out = SYS_OPEN_PHYSICAL_MM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_OPEN_PHYSICAL_MM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_read_phys_mm_param( sys_read_physmm *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_READ_PHYSICAL_MM_PARAM_COUNT 2
		*count_out = SYS_READ_PHYSICAL_MM_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_READ_PHYSICAL_MM_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->offset ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->count ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_load_kmod_param( sys_load_kmod *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_LOAD_KMOD_PARAM_COUNT 2
		*count_out = SYS_LOAD_KMOD_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_LOAD_KMOD_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		//set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_reg_srv_param( sys_regsrv *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_REG_SRV_PARAM_COUNT 8
		*count_out = SYS_REG_SRV_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_REG_SRV_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 2 ], PTR_TYPE, uint32_val, action->starttype ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 5 ], UINT32_TYPE, uint32_val, action->srv_name_len ); 
		set_param_info( params[ 6 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 7 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len ); 
	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_sys_open_dev_param( sys_opendev *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_OPEN_DEV_PARAM_COUNT 5
		*count_out = SYS_OPEN_DEV_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_OPEN_DEV_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->devtype ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->access ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->share ); 
		//set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_w32_msg_hook( w32_msghook *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_W32_MSG_HOOK_PARAM_COUNT 6
		*count_out = SYS_W32_MSG_HOOK_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_W32_MSG_HOOK_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], PTR_TYPE, ptr_val, action->modbase ); 
		set_param_info( params[ 2 ], PTR_TYPE, ptr_val, action->hookfunc ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->hooktype ); 
		//set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 4 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 5 ], WSTRING_TYPE, wstring_val, action->path_name ); 


	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_w32_lib_inject_hook( w32_lib_inject *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define SYS_W32_LIB_INJECT_PARAM_COUNT 5
		*count_out = SYS_W32_LIB_INJECT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < SYS_W32_LIB_INJECT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 3 ], WSTRING_TYPE, wstring_val, action->path_name ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name 
			+ action->path_len ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_connect( net_connect *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_CONNECT_PARAM_COUNT 3
		*count_out = NET_CONNECT_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_CONNECT_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocol ); 
		//set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->ip_addr ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->port ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_send( net_send *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_SEND_PARAM_COUNT 4
		*count_out = NET_SEND_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_SEND_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocol ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->ip_addr ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->port ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_net_http( net_http *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define NET_HTTP_PARAM_COUNT 6
		*count_out = NET_HTTP_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < NET_HTTP_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->protocol ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->cmd ); 
		set_param_info( params[ 2 ], UINT32_TYPE, uint32_val, action->data_len ); 
		//set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->result ); 
		set_param_info( params[ 3 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 4 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_extract_hidden( ba_extract_hidden *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_EXTRACT_HIDDEN_PARAM_COUNT 2
		*count_out = BA_EXTRACT_HIDDEN_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_EXTRACT_HIDDEN_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_extract_pe( ba_extract_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_EXTRACT_PE_PARAM_COUNT 2
		*count_out = BA_EXTRACT_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_EXTRACT_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_self_copy( ba_self_copy *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_SELF_COPY_PARAM_COUNT 2
		*count_out = BA_SELF_COPY_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_SELF_COPY_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_self_delete( ba_self_delete *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_SELF_DELETE_PARAM_COUNT 2
		*count_out = BA_SELF_DELETE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_SELF_DELETE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_invade_process( ba_invade_process *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_INVADE_PROCESS_PARAM_COUNT 3
		*count_out = BA_INVADE_PROCESS_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_INVADE_PROCESS_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->target_pid ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_infect_pe( ba_infect_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

#define BA_INFRECT_PE_PARAM_COUNT 2
		*count_out = BA_INFRECT_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_INFRECT_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_overwrite_pe( ba_overwrite_pe *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = BA_OVERWRITE_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_OVERWRITE_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 1 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS parse_ba_register_autorun( ba_register_autorun *action, param_info *params, ULONG max_count, ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( params != NULL ); 
		ASSERT( max_count > 0 ); 
		ASSERT( count_out != NULL ); 

		*count_out = BA_OVERWRITE_PE_PARAM_COUNT; 

#ifdef _DEBUG
		if( action == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		if( params == NULL )
		{
			ntstatus = STATUS_INVALID_PARAMETER_2; 
			break; 
		}
#endif //_DEBUG

		if( max_count < BA_OVERWRITE_PE_PARAM_COUNT )
		{
			ntstatus = STATUS_BUFFER_TOO_SMALL; 
			break; 
		}

		set_param_info( params[ 0 ], UINT32_TYPE, uint32_val, action->type ); 
		set_param_info( params[ 1 ], UINT32_TYPE, uint32_val, action->path_len ); 
		set_param_info( params[ 2 ], WSTRING_TYPE, wstring_val, action->path_name ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS get_params_from_action( sys_action_record *action, 
								param_info params[], 
								ULONG max_count, 
								ULONG *count_out )
{
	NTSTATUS ntstatus = STATUS_INVALID_PARAMETER; 

	do 
	{
		ASSERT( action != NULL ); 

		if( count_out != NULL )
		{
			*count_out = 0; 
		}


		switch( action->type )
		{
		case EXEC_create:
			ntstatus = parse_exec_create( &action->do_exec_create, 
				params, 
				max_count, 
				count_out ); 
			break; //�������� ����·���� ��ִ�м�أ� 

		case EXEC_destroy:
			ntstatus = parse_exec_destroy( &action->do_exec_destroy, 
				params, 
				max_count, 
				count_out ); 
			break; //�����˳� ����·���� 

		case EXEC_module_load:
			ntstatus = parse_exec_module_load_param( &action->do_exec_module_load, 
				params, 
				max_count, 
				count_out ); 
			break; //ģ����� ģ��·���� 

			//MT_filemon, 
		case FILE_touch:
			ntstatus = parse_file_touch_param( &action->do_file_touch, 
				params, 
				max_count, 
				count_out ); 
			break; //�����ļ� �ļ�ȫ·�� �ļ����

		case FILE_open:
			ntstatus = parse_file_open_param( &action->do_file_open, 
				params, 
				max_count, 
				count_out ); 

			break; //���ļ� �ļ�ȫ·�� 
		case FILE_read:
			ntstatus = parse_file_read_param( &action->do_file_read, 
				params, 
				max_count, 
				count_out ); 

			break; //��ȡ�ļ� �ļ�ȫ·�� 

		case FILE_write:
			ntstatus = parse_file_write_param( &action->do_file_write, 
				params, 
				max_count, 
				count_out ); 

			break; //д���ļ� �ļ�ȫ·�� 

		case FILE_modified:
			ntstatus = parse_file_modified_param( &action->do_file_modified, 
				params, 
				max_count, 
				count_out ); 

			break; //�ļ����޸� �ļ�ȫ·�� 
		case FILE_readdir:
			//ntstatus = parse_file_read( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //����Ŀ¼ Ŀ¼ȫ·�� 
		case FILE_remove:
			ntstatus = parse_file_remove_param( &action->do_file_remove, 
				params, 
				max_count, 
				count_out ); 
			break; //ɾ���ļ� �ļ�ȫ·�� 
		case FILE_rename:
			ntstatus = parse_file_rename_param( &action->do_file_rename, 
				params, 
				max_count, 
				count_out ); 
			break; //�������ļ� �ļ�ȫ·�� 
		case FILE_truncate:
			ntstatus = parse_file_truncate_param( &action->do_file_truncate, 
				params, 
				max_count, 
				count_out ); 
			break; //�ض��ļ� �ļ�ȫ·�� 
		case FILE_mklink:
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //�����ļ�Ӳ���� �ļ�ȫ·�� 

		case FILE_chmod:
			ntstatus = parse_file_chmod_param( &action->do_file_chmod, 
				params, 
				max_count, 
				count_out ); 
			break; //�����ļ����� �ļ�ȫ·�� 
		case FILE_setsec:
			ntstatus = parse_file_set_sec_param( &action->do_file_setsec, 
				params, 
				max_count, 
				count_out ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_getinfo:
			ntstatus = STATUS_INVALID_PARAMETER; 
			//ntstatus = parse_file_get_info_param( &action->do_file_setsec, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

		case FILE_setinfo:
			ntstatus = STATUS_INVALID_PARAMETER; 
			//ntstatus = parse_file_set_info_param( &action->do_file_setsec, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; //�����ļ���ȫ���� �ļ�ȫ·�� 

			//MT_regmon, 
		case REG_openkey:
			ntstatus = parse_reg_open_key_param( &action->do_reg_openkey, 
				params, 
				max_count, 
				count_out ); 
			break; //��ע����� ע�����·��  ��ע�����أ� 

		case REG_mkkey:
			ntstatus = parse_reg_make_key_param( &action->do_reg_mkkey, 
				params, 
				max_count, 
				count_out ); 
			break; //����ע����� ע�����·�� 
		case REG_rmkey:
			ntstatus = parse_reg_remove_key_param(  &action->do_reg_rmkey, 
				params, 
				max_count, 
				count_out ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_mvkey:
			ntstatus = parse_reg_move_key_param( &action->do_reg_mvkey, 
				params, 
				max_count, 
				count_out ); 
			break; //������ע����� ע�����·�� 

		case REG_getinfo:
			ntstatus = parse_reg_get_info_param( &action->do_reg_rmval, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case REG_setinfo:
			ntstatus = parse_reg_set_info_param( &action->do_reg_rmval, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case REG_enuminfo:
			ntstatus = parse_reg_enum_info_param( &action->do_reg_rmval, 
				params, 
				max_count, 
				count_out ); 			
			break; 
		case REG_enum_val:
			ntstatus = parse_reg_enum_value_param( &action->do_reg_rmval, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case REG_rmval:
			ntstatus = parse_reg_remove_value_param( &action->do_reg_rmval, 
				params, 
				max_count, 
				count_out ); 
			break; //ɾ��ע����� ע�����·�� 

		case REG_getval:
			ntstatus = parse_reg_get_value_param( &action->do_reg_getval, 
				params, 
				max_count, 
				count_out ); 
			break; //��ȡע���ֵ ע���ֵ·�� 

		case REG_setval:
			ntstatus = parse_reg_set_value_param( &action->do_reg_setval, 
				params, 
				max_count, 
				count_out ); 
			break; //����ע���ֵ ע���ֵ·�� 

		case REG_loadkey:
			ntstatus = parse_reg_load_key_param( &action->do_reg_loadkey, 
				params, 
				max_count, 
				count_out ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_replkey:
			ntstatus = parse_reg_repl_key_param( &action->do_reg_replkey, 
				params, 
				max_count, 
				count_out ); 
			break; //�滻ע����� ע�����·�� 

		case REG_rstrkey:
			ntstatus = parse_reg_restore_key_param( &action->do_reg_rstrkey, 
				params, 
				max_count, 
				count_out ); 
			break; //����ע���Hive�ļ� ע�����·�� 

		case REG_setsec:
			ntstatus = STATUS_INVALID_PARAMETER; 
			//ntstatus = parse_reg_set_sec_param( &action->do_reg_rstrkey, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; //����ע�������ȫ���� ע�����·�� 
			//MT_procmon, 
		case PROC_exec:
			ntstatus = parse_proc_exec_param( &action->do_proc_exec, 
				params, 
				max_count, 
				count_out ); 
			break; //�������� Ŀ�����·����  �����̼�أ�
			//case CREATE_PROC:
			//desc = L"CREATE_PROC"; 
			//	break; 
		case PROC_open:
			ntstatus = parse_proc_open_param( &action->do_proc_open, 
				params, 
				max_count, 
				count_out ); 
			break; //�򿪽��� Ŀ�����·���� 
		case PROC_debug:
			ntstatus = parse_proc_debug_param( &action->do_proc_debug, 
				params, 
				max_count, 
				count_out ); 
			break; //���Խ��� Ŀ�����·���� 
		case PROC_suspend:
			//ntstatus = parse_proc_sus( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //������� Ŀ�����·���� 
		case PROC_resume:
			ntstatus = parse_proc_resume_param( &action->do_proc_resume, 
				params, 
				max_count, 
				count_out ); 
			break; //�ָ����� Ŀ�����·���� 
		case PROC_exit:
			ntstatus = parse_proc_kill_param( &action->do_proc_kill, 
				params, 
				max_count, 
				count_out ); 
			break; //�������� Ŀ�����·���� 
			//case TERMINATE_PROC:
			//desc = L"TERMINATE_PROC"; 
			//	break; 
		case PROC_job:
			ntstatus = parse_proc_job_param( &action->do_proc_job, 
				params, 
				max_count, 
				count_out ); 
			break; //�����̼��빤���� Ŀ�����·���� 
		case PROC_pgprot:
			ntstatus = parse_proc_page_prot_param( &action->do_proc_pgprot, 
				params, 
				max_count, 
				count_out ); 
			break; //������޸��ڴ����� Ŀ�����·���� 
		case PROC_freevm:
			ntstatus = parse_proc_free_vm_param( &action->do_proc_freevm, 
				params, 
				max_count, 
				count_out ); 
			break; //������ͷ��ڴ� Ŀ�����·���� 
		case PROC_writevm:
			ntstatus = parse_proc_write_vm_param( &action->do_proc_writevm, 
				params, 
				max_count, 
				count_out ); 
			break; //�����д�ڴ� Ŀ�����·���� 
		case PROC_readvm:
			ntstatus = parse_proc_read_vm_param( &action->do_proc_readvm, 
				params, 
				max_count, 
				count_out ); 
			break; //����̶��ڴ� Ŀ�����·���� 
		case THRD_remote:
			ntstatus = parse_thread_remote_param( &action->do_thrd_remote, 
				params, 
				max_count, 
				count_out ); 
			break; //����Զ���߳� Ŀ�����·���� 
		case THRD_setctxt:
			ntstatus = parse_thread_set_context_param( &action->do_thrd_setctxt, 
				params, 
				max_count, 
				count_out ); 
			break; //����������߳������� Ŀ�����·���� 
		case THRD_suspend:
			ntstatus = parse_thread_suspend_param( &action->do_thrd_suspend, 
				params, 
				max_count, 
				count_out ); 
			break; //����̹����߳� Ŀ�����·���� 
		case THRD_resume:
			//ntstatus = parse_thread_resume_( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //����ָ̻��߳� Ŀ�����·���� 
		case THRD_exit:
			ntstatus = parse_thread_kill_param( &action->do_thrd_kill, 
				params, 
				max_count, 
				count_out ); 
			break; //����̽����߳� Ŀ�����·���� 
		case THRD_queue_apc:
			ntstatus = parse_thread_queue_apc_param( &action->do_thrd_queue_apc, 
				params, 
				max_count, 
				count_out ); 
			break; //������Ŷ�APC Ŀ�����·���� 

			//MT_common
		case COM_access:
			break; 

		case PORT_read:
			break; 

		case PORT_write:
			break; 
		case SYS_settime:
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //����ϵͳʱ�� �� 
		case SYS_link_knowndll:
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //����KnownDlls���� �����ļ��� 
		case SYS_open_physmm:

			ntstatus = parse_sys_open_phys_mm_param( &action->do_sys_open_physmm, 
				params, 
				max_count, 
				count_out ); 
			break; //�������ڴ��豸 �� 
		case SYS_read_physmm:
			ntstatus = parse_sys_read_phys_mm_param( &action->do_sys_read_physmm, 
				params, 
				max_count, 
				count_out ); 
			break; //�������ڴ� �� 
		case SYS_write_physmm:
			//ntstatus = parse_sys_write_phy( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //д�����ڴ� �� 
		case SYS_load_kmod:
			ntstatus = parse_sys_load_kmod_param( &action->do_sys_load_kmod, 
				params, 
				max_count, 
				count_out ); 
			break; //�����ں�ģ�� �ں�ģ��ȫ·�� 
		case SYS_enumproc:
			//ntstatus = parse_( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //ö�ٽ��� �� 
		case SYS_regsrv:
			ntstatus = parse_sys_reg_srv_param( &action->do_sys_regsrv, 
				params, 
				max_count, 
				count_out ); 
			break; //ע����� �������ȫ·�� 
		case SYS_opendev:
			ntstatus = parse_sys_open_dev_param( &action->do_sys_opendev, 
				params, 
				max_count, 
				count_out ); 
			break; //���豸 �豸�� 

			//MT_w32mon
		case W32_postmsg:
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; //���ʹ�����Ϣ��Post�� Ŀ�����·���� 
		case W32_sendmsg:
			break; //���ʹ�����Ϣ��Send�� Ŀ�����·���� 
		case W32_findwnd:
			break; //���Ҵ��� �� 
		case W32_msghook:
			break; //������Ϣ���� �� 
			//case INSTALL_HOOK:
			//	break; 
		case W32_lib_inject:
			ntstatus = parse_w32_lib_inject_hook( &action->do_w32_lib_inject, 
				params, 
				max_count, 
				count_out ); 

			break; //DLLע�� ע��DLL·���� 

			//MT_netmon
		case NET_create:
			ntstatus = parse_net_create( &action->do_net_create, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case NET_connect:
			ntstatus = parse_net_connect( &action->do_net_connect, 
				params, 
				max_count, 
				count_out ); 
			break; //�������� Զ�̵�ַ����ʽ��IP���˿ںţ� �������أ� 
		case NET_listen:
			ntstatus = parse_net_listen( &action->do_net_listen, 
				params, 
				max_count, 
				count_out ); 

			break; //�����˿� ������ַ����ʽ��IP���˿ںţ� 
		case NET_send:
			ntstatus = parse_net_send( &action->do_net_send, 
				params, 
				max_count, 
				count_out ); 
			break; //�������ݰ� Զ�̵�ַ����ʽ��IP���˿ںţ� 
		case NET_recv:
			ntstatus = parse_net_recv( &action->do_net_recv, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case NET_accept:
			ntstatus = parse_net_accept( &action->do_net_accept, 
				params, 
				max_count, 
				count_out ); 
			break; 
		case NET_dns: 
			//ntstatus = parse_net_dns( &action->do_net_http, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 
		case NET_http: 
			ntstatus = parse_net_http( &action->do_net_http, 
				params, 
				max_count, 
				count_out ); 
			break; //HTTP���� HTTP����·������ʽ������/URL�� 
		case NET_icmp_send:
			//ntstatus = parse_net_icmp_send( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; 
		case NET_icmp_recv:
			//ntstatus = parse_( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; 
		case BA_extract_hidden:
			ntstatus = parse_ba_extract_hidden( &action->do_ba_extract_hidden, 
				params, 
				max_count, 
				count_out ); 
			break; //�ͷ������ļ� �ͷ��ļ�·���� ����Ϊ��أ� 
		case BA_extract_pe:
			ntstatus = parse_ba_extract_pe( &action->do_ba_extract_pe, 
				params, 
				max_count, 
				count_out ); 
			break; //�ͷ�PE�ļ� �ͷ��ļ�·���� 
		case BA_self_copy:
			ntstatus = parse_ba_self_copy( &action->do_ba_self_copy, 
				params, 
				max_count, 
				count_out ); 

			break; //���Ҹ��� ����Ŀ���ļ�·���� 
		case BA_self_delete:
			ntstatus = parse_ba_self_delete( &action->do_ba_self_delete, 
				params, 
				max_count, 
				count_out ); 
			break; //����ɾ�� ɾ���ļ�·���� 
		case BA_ulterior_exec:
			//ntstatus = parse_ba_( &action->, 
			//	params, 
			//	max_count, 
			//	count_out ); 
			break; //����ִ�� ��ִ��ӳ��·���� 
		case BA_invade_process:
			ntstatus = parse_ba_invade_process( &action->do_ba_invade_process, 
				params, 
				max_count, 
				count_out ); 
			break; //���ֽ��� Ŀ�����·���� 
		case BA_infect_pe:
			ntstatus = parse_ba_infect_pe( &action->do_ba_infect_pe, 
				params, 
				max_count, 
				count_out ); 
			break; //��ȾPE�ļ� Ŀ���ļ�·���� 
		case BA_overwrite_pe:
			ntstatus = parse_ba_overwrite_pe( &action->do_ba_overwrite_pe, 
				params, 
				max_count, 
				count_out ); 
			break; //��дPE�ļ� Ŀ���ļ�·���� 
		case BA_register_autorun:
			ntstatus = parse_ba_register_autorun( &action->do_ba_register_autorun, 
				params, 
				max_count, 
				count_out ); 
			break; //ע���������� �������ļ�·���� 
		default:
			ASSERT( FALSE ); 
			ntstatus = STATUS_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE ); 

	return ntstatus; 
}
