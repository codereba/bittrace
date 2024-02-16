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
 
#ifndef __PIPE_LINE_H__
#define __PIPE_LINE_H__

#define RASBASE 600
#define MAX_PIPE_DATA_LEN 4096
#define ERROR_BUFFER_TOO_SMALL               (RASBASE+3)

typedef struct _pipe_ipc_point
{
	LPWSTR pipe_name;
	HANDLE pipe; 
	HANDLE lock; 
} pipe_ipc_point, *ppipe_ipc_point; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

INLINE LRESULT __stdcall check_name_pipe_exist( LPCWSTR pipe_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret = FALSE;

	_ret = WaitNamedPipe(
		pipe_name, 
		NMPWAIT_USE_DEFAULT_WAIT
		);

	if( FALSE == _ret )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "check server pipe line failed\n" ) ); 

		if( ERROR_PIPE_BUSY != ret )
		{
			goto _return; 
		}
		else 
		{
			ret = ERROR_SUCCESS; 
		}
	}

_return:
	return ret; 
}

LRESULT create_name_pipe( LPCTSTR pipe_name, HANDLE *pipe_out ); 
UINT read_from_name_pipe( HANDLE pipe, CHAR* data, ULONG data_len, ULONG *ret_len ); 
LRESULT disconnect_name_pipe( HANDLE pipe ); 
LRESULT write_to_name_pipe( HANDLE pipe, CHAR* data, ULONG data_len, ULONG *ret_len ); 
LRESULT create_non_name_pipe( HANDLE *read_pipe, HANDLE *write_pipe ); 

#define CLIENT_PIPE 0x00000001
#define SERVER_PIPE 0x00000002
#define MAX_PIPE_RETRY_TIME 600 

LRESULT init_pipe_point( pipe_ipc_point *point, LPCWSTR pipe_name, ULONG flags, ULONG try_time ); 

INLINE LRESULT uninit_pipe_point( pipe_ipc_point *point )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( point != NULL ); 
	ASSERT( point->pipe != NULL ); 
	
	CloseHandle( point->pipe ); 
	uninit_mutex( point->lock ); 
	point->pipe = NULL; 
	point->lock = NULL; 

	//if( point->pipe_name != NULL )
	{
		free( point->pipe_name ); 
	}

	return ret; 
}

LRESULT __stdcall _exec_cmd_from_pipe( pipe_ipc_point *point, 
									  const BYTE* method,	// �����Ϊ�գ��ͻ��˻᷵��һ�� ���������� ��Ϣ
									  const BYTE* data,			// �����Ϊ�գ��ͻ��˻᷵��һ�� ���������� ��Ϣ
									  ULONG data_len,
									  VOID** data_ret,
									  ULONG* data_ret_len
									  ); 

/**
* @brief �ͷ�ͨ�����������ڴ�
* @param[in] pBuffer ��Ҫ�ͷŵ��ڴ��ַ
* @return 0 �ɹ�������Ϊʧ�ܴ�����
*/
INLINE LRESULT __stdcall release_pipe_buf(
	PVOID *data
	)
{
	free( *data ); ;
	*data= NULL;
	return ERROR_SUCCESS;
}

/**
* @brief ���������������ܵ�������������
* @return  0 �ɹ�������Ϊʧ�ܴ�����
*/
LRESULT __stdcall connect_to_pipe( LPCWSTR pipe_name, HANDLE *pipe_out ); 


LRESULT __stdcall write_pipe_sync( 
								  pipe_ipc_point *point, 
								  const BYTE* data, 
								  const ULONG data_len
								  ); 
LRESULT __stdcall read_pipe_sync( 
								 pipe_ipc_point *point, 
								 BYTE* data, 
								 const ULONG data_len ); 

INLINE LRESULT exec_cmd_from_pipe(
								  pipe_ipc_point *point, 
								  const BYTE* method,
								  const BYTE* param,
								  ULONG param_size,
								  BYTE **data_ret, 
								  ULONG *data_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( point != NULL ); 
	ASSERT( point->pipe_name != NULL ); 

	ret = check_name_pipe_exist( point->pipe_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _exec_cmd_from_pipe( point, 
		method, 
		param,
		param_size,
		( VOID ** )data_ret,
		data_ret_len );

	if( ret != ERROR_SUCCESS ) 		{
		goto _return; 
	}

_return: 
#ifdef _DEBUG
	if( ret != ERROR_SUCCESS )
	{
		if( data_ret != NULL )
		{
			ASSERT( *data_ret != NULL ); 
		}
	}
#endif //_DEBUG
	
	return ret; 
}

LRESULT exec_cmd_on_pipe( LPCWSTR pipe_name, 
						BYTE* cmd, 
						ULONG cmd_len, 
						BYTE **resp, 
						ULONG *resp_len ); 

#ifdef __cplusplus
}
#endif //__cplusplus


#endif //__PIPE_LINE_H__