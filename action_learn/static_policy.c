#include "common_func.h"
#include "ring0_2_ring3.h"
#include "acl_define.h"
#include "hash_table.h"
#include "ref_node.h"
#include "rbtree.h"
#include "sys_event_define.h"
#include "action_type_parse.h"
#include "action_source.h"
#include "static_policy.h"
#include "action_policy_common.h"

/*****************************************************************************************
level policy design:
1.base system action type:
	disk
	file system 
	network system
	windows object

1.depart system action to these main type:
	1.system behavior
	2.network
	3.win32
	4.system common action
	5.com component action
	6.process and thread (include execute)
	7.registry
	8.file 
	9.disk io

2. one main action type corresponds to one policy type
3. policies structure:
	1.hash table (for simple name or simple path)
	2.red black tree (for numeric region)
	3.btree ( for complex path )
	4.atomic single linked list (for simple and small policies)
4. policies structure architecture feature:
	1.front cache support (these actions is repeated regularly.

5. policies communications between kernel and user layer feature:
	1.irp communications by pending mode.
	2.log transfer from kernel to user by shared circle buffer.

6. policies type and matched structure:
	1.system behavior ( hash table )
	2.network ( rbtree )
	3.win32 ( hash table )
	4.system common action ( hash table )
	5.com component action ( hash table )
	6.process and thread (include execute) (hash table)
	7.registry (btree)
	8.file ( btree )
	9.disk io
*****************************************************************************************/
static NTSTATUS check_manage_proc( manage_proc_info *proc_info ); 
//old action structures end
static NTSTATUS check_exec_create( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_exec_destroy( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_exec_module_load( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_touch( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_open( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_read( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_write( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_modified( sys_action_info *action, 
							 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_readdir( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_remove( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_rename( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_truncate( sys_action_info *action, 
							 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_mklink( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_chmod( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_file_setsec( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_openkey( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_mkkey( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rmkey( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_mvkey( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rmval( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_getval( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_setval( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_loadkey( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_replkey( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_rstrkey( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_reg_setsec( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

//static NTSTATUS check_proc_exec( sys_action_info *action, 
//						 action_reply *reply )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//
//	do 
//	{	
//		ASSERT( action != NULL );  
//
//	}while( FALSE );
//
//	return ntstatus; 
//}

static NTSTATUS check_proc_open( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_debug( sys_action_info *action, 
						  action_reply *resp )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_suspend( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_resume( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_kill( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_job( sys_action_info *action, 
						action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_pgprot( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_freevm( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_writevm( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_proc_readvm( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_remote( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_setctxt( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_suspend( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_resume( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_kill( sys_action_info *action, 
						 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_thrd_queue_apc( sys_action_info *action, 
							  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_settime( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_link_knowndll( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_open_physmm( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_read_physmm( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_write_physmm( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}


static NTSTATUS check_sys_load_kmod( sys_action_info *action, 
							 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}


static NTSTATUS _check_sys_load_kmod( action_source_info *source, 
							  policy_holder *sub_policy, 
							  sys_action_info *action )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS load_important_file_policies()
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	INT32 _Ret; 
	WCHAR windows_path[ MAX_PATH ]; 

	do 
	{
		_Ret = GetWindowsDirectoryW( windows_path, ARRAYSIZE( windows_path ) ); 
		if( _Ret >= ARRAYSIZE( windows_path ) )
		{
			ntstatus = STATUS_INVALID_PARAMETER_1; 
			break; 
		}

		//add_action_rule( )
	}while( FALSE ); 

	return ntstatus; 
}

static NTSTATUS check_sys_enumproc( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_regsrv( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_sys_opendev( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_postmsg( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_sendmsg( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_findwnd( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_msghook( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_w32_lib_inject( sys_action_info *action, 
							  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_connect( sys_action_info *action, 
						   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_listen( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_accept( sys_action_info *action, 
						  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_send( sys_action_info *action, 
						action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_net_http( sys_action_info *action, 
						action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_extract_hidden( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_extract_pe( sys_action_info *action, 
							 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_self_copy( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_self_delete( sys_action_info *action, 
							  action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_ulterior_exec( sys_action_info *action, 
								action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_invade_process( sys_action_info *action, 
								 action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_infect_pe( sys_action_info *action, 
							action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_overwrite_pe( sys_action_info *action, 
							   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

static NTSTATUS check_ba_register_autorun( sys_action_info *action, 
								   action_reply *reply )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{	
		ASSERT( action != NULL );  

	}while( FALSE );

	return ntstatus; 
}

NTSTATUS CALLBACK init_name_iteration_ex( PVOID param, 
										 PULONG hash_code, 
										 ULONG tbl_size, 
										 PVOID *param_out, 
										 PVOID *context )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPWSTR name; 
	ULONG _hash_code; 
	param_info *all_params; 
	ULONG name_len; 
	fs_style_interator *fs_style_path_iter = NULL; 

	ASSERT( context != NULL ); 
	ASSERT( tbl_size > 0 ); 
	ASSERT( hash_code != NULL ); 

	if( param_out == NULL )
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	if( param == NULL )
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	*param_out = NULL; 
	*context = NULL; 
	*hash_code = INVALID_HASH_CODE; 

	//if( ( buf_len % 2 ) != 0 )
	//{
	//	ASSERT( FALSE ); 
	//	log_trace( ( MSG_WARNING, "buffer length is not wide char size aligned\n" ) ); 
	//}

	all_params = ( param_info* )param; 

	name = ( LPWSTR )all_params[ 3 ].data.wstring_val; 

	//if( name[ buf_len / sizeof( WCHAR ) ] != L'\0' )
	//{
	//	name[ buf_len / sizeof( WCHAR ) ] = L'\0'; 
	//}

	name_len = wcslen( name ); 

	fs_style_path_iter = ( fs_style_interator* )ALLOC_TAG_POOL( sizeof( fs_style_interator ) ); 
	if( fs_style_path_iter == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	fs_style_path_iter->index = name_len; 
	fs_style_path_iter->length = name_len + 1; 
	fs_style_path_iter->org_path = NULL; 
	fs_style_path_iter->iterator = NULL; 

	fs_style_path_iter->org_path = ( LPWSTR )ALLOC_TAG_POOL( ( ( name_len + 1 )* sizeof( WCHAR ) ) * 2 ) ; 

	if( fs_style_path_iter->org_path == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	fs_style_path_iter->iterator = ( LPWSTR )fs_style_path_iter->org_path + name_len + 1; 

	memcpy( ( PVOID )fs_style_path_iter->org_path, name, ( name_len + 1 ) * sizeof( WCHAR ) ); 
	memcpy( fs_style_path_iter->iterator, name, ( name_len + 1 ) * sizeof( WCHAR ) ); 

	unicode_str_to_upper( fs_style_path_iter->iterator ); 

	if( fs_style_path_iter->org_path[ name_len - 1 ] == PATH_DELIM )
	{
		( ( LPWSTR )fs_style_path_iter->org_path )[ name_len - 1 ] = L'\0'; 
	}

	if( fs_style_path_iter->iterator[ name_len - 1 ] == PATH_DELIM )
	{
		fs_style_path_iter->iterator[ name_len - 1 ] = L'\0'; 
	}

	_hash_code = unicode_str_hash( fs_style_path_iter->iterator, tbl_size ); 
	*param_out = fs_style_path_iter->iterator; 
	*context = ( PVOID )fs_style_path_iter; 
	*hash_code = _hash_code; 

_return:
	return ntstatus; 
}

NTSTATUS CALLBACK uninit_name_iteration_ex( PVOID context )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	fs_style_interator *fs_style_path_iter; 

	ASSERT( context != NULL ); 

	fs_style_path_iter = ( fs_style_interator* )context; 
	if( fs_style_path_iter->org_path != NULL )
	{
		FREE_TAG_POOL( ( PVOID )fs_style_path_iter->org_path ); 
	}
	else
	{
		ASSERT( FALSE && "uninitialize a not initialized fs tyle name iterator" ); 
	}

	FREE_TAG_POOL( fs_style_path_iter ); 

	return ntstatus; 
}

ULONG CALLBACK calc_socket_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	ULONG hash_code; 
	param_info *all_params; 
	ASSERT( param != NULL ); 

	all_params = ( param_info* )param; 

	ASSERT( all_params[ 2 ].type == UINT32_TYPE ); 

	hash_code = long_hash( ( ULONG )all_params[ 2 ].data.uint32_val, table_size ); 

	return hash_code; 
}

INT32 _compare_socket_info_ex( action_rule_define *socket_define, action_desc *socket )
{
	return 0; 
}

INT32 CALLBACK compare_socket_rule_ex( PVOID param, 
								   PLIST_ENTRY list_item, 
								   PVOID param_iteration )
{
	INT32 ret = TRUE; 
	param_info *all_params; 
	action_rule_item *rule_item; 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	rule_item = CONTAINING_RECORD( list_item, action_rule_item, entry ); 

	all_params = ( param_info* )param; 

	ASSERT( rule_item->rule.rule.socket.dest_ip != NULL 
		|| rule_item->rule.rule.socket.dest_port != NULL 
		|| rule_item->rule.rule.socket.src_ip != NULL 
		|| rule_item->rule.rule.socket.src_port != NULL ); 

	if( rule_item->rule.rule.socket.app != NULL )
	{
		ret = compare_name_define( rule_item->rule.rule.socket.app->param.app.app_name, 
			all_params[ 0 ].data.wstring_val ); 

		if( ret == FALSE )
		{
			goto _return; 
		}
	}

	ret = _compare_socket_info_ex( &rule_item->rule.rule, all_params ); 

_return:
	return ret; 
}

INT32 CALLBACK compare_reg_rule_ex( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration )
{
	INT32 ret = TRUE; 
	param_info *all_params; 
	action_rule_item *rule_item; 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
	rule_item = CONTAINING_RECORD( list_item, action_rule_item, entry ); 

	all_params = ( param_info* )param; 

	ASSERT( rule_item->rule.rule.reg.reg_path != NULL ); 
	ASSERT( all_params[ 0 ].type == WSTRING_TYPE ); 

	if( rule_item->rule.rule.reg.app != NULL )
	{
		//log_trace( ( MSG_INFO, "record app name is %ws cur action app name is %ws \n", rule_item->rule.rule.reg.app->param.app.app_name, all_params->reg.app.app.app_name ) ); 

		ret = compare_name_define( rule_item->rule.rule.reg.app->param.app.app_name, 
			all_params[ 0 ].data.wstring_val ); 

		if( ret == FALSE )
		{
			goto _return; 
		}
	}

	ASSERT( all_params[ 3 ].type == WSTRING_TYPE 
		&& all_params[ 3 ].data.wstring_val != NULL ); 

	if( rule_item->rule.rule.reg.reg_path->param.reg.reg_path[ 0 ] == L'\0' 
		|| all_params[ 3 ].data.wstring_val == L'\0' )
	{
		ASSERT( FALSE ); 
		log_trace( ( MSG_INFO, "*** check reg rule, but the reg path is null! ( rule 0x%0.8x ( %ws ), check 0x%0.8x ( %ws ) ) *** \n", 
			rule_item->rule.rule.reg.reg_path->param.reg.reg_path, 
			rule_item->rule.rule.reg.reg_path->param.reg.reg_path, 
			all_params[ 3 ].data.wstring_val, 
			all_params[ 3 ].data.wstring_val ) ); 
		goto _return; 
	}
	else
	{
		//ULONG name_len; 
		log_trace( ( MSG_INFO, "record registry path is %ws compare with %ws, iteration name is %ws\n", rule_item->rule.rule.reg.reg_path->param.reg.reg_path, 
			all_params[ 3 ].data.wstring_val, 
			( LPWSTR )param_iteration ) ); 

		//name_len = wcsnlen( params->reg.reg_path.reg.reg_path, _MAX_REG_PATH_LEN ); 
		//ASSERT( name_len < _MAX_REG_PATH_LEN ); 

		//if( params->reg.reg_path.reg.reg_path[ name_len - 1 ] == L'\\' 
		//	|| params->reg.reg_path.reg.reg_path[ name_len - 1 ] == L'/' )
		//{
		//	params->reg.reg_path.reg.reg_path[ name_len - 1 ] = L'\0';  
		//}

		if( param_iteration != NULL )
		{
			ret = compare_name_define( rule_item->rule.rule.reg.reg_path->param.reg.reg_path, 
				( LPWSTR )param_iteration ); 

		}
		else
		{
			ret = compare_name_define( rule_item->rule.rule.reg.reg_path->param.reg.reg_path, 
				all_params[ 3 ].data.wstring_val ); 
		}

		if( ret == FALSE )
		{
			goto _return; 
		}
	}

_return: 
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

#define NAME_PARAM_INDEX 1
INLINE ULONG calc_name_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	ULONG hash_code; 
	param_info *_param; 
	ASSERT( param != NULL ); 

	_param = ( param_info* )param; 

	ASSERT( _param->type == WSTRING_TYPE ); 
	ASSERT( _param->data.wstring_val != NULL ); 

	hash_code = unicode_str_hash( _param->data.wstring_val, table_size ); 

	return hash_code; 
}

ULONG CALLBACK calc_url_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	ULONG hash_code; // = 0; 
	param_info *all_params; 

	ASSERT( param != NULL ); 

	all_params = ( param_info* )param; 

	ASSERT( all_params[ 3 ].type == WSTRING_TYPE ); 

	hash_code = calc_url_name_hash_code( all_params[ 3 ].data.wstring_val, table_size ); 

	goto _return; 
_return:
	return hash_code; 
}

ULONG CALLBACK calc_file_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	return calc_name_rule_hash_code_ex( param, table_size ); 
}

ULONG CALLBACK calc_reg_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	return calc_name_rule_hash_code_ex( param, table_size ); 
}

ULONG CALLBACK calc_com_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	return calc_name_rule_hash_code_ex( param, table_size ); 
}

ULONG CALLBACK calc_common_rule_hash_code_ex( PVOID param, ULONG table_size )
{
	return calc_name_rule_hash_code_ex( param, table_size ); 
}

INT32 CALLBACK compare_url_rule_ex( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration )
{
	INT32 ret = TRUE; 
	param_info *all_params; 
	action_rule_item *rule_item; 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	rule_item = CONTAINING_RECORD( list_item, action_rule_item, entry ); 

	all_params = ( param_info* )param; 

#ifdef DBG
	if( param == NULL )
	{
		DBG_BP(); 
	}

	if( rule_item == NULL || rule_item->rule.rule.url.url == NULL )
	{
		DBG_BP(); 
	}
#endif //DBG

	ASSERT( all_params[ 0 ].type == WSTRING_TYPE ); 

	if( rule_item->rule.rule.socket.app!= NULL )
	{
		ret = compare_name_define( rule_item->rule.rule.url.app->param.app.app_name, 
			all_params[ 0 ].data.wstring_val ); 
		if( ret == FALSE )
		{
			goto _return; 
		}
	}

	ASSERT( rule_item->rule.rule.url.url != NULL ); 
	ASSERT( all_params[ 3 ].type == WSTRING_TYPE ); 

	if( rule_item->rule.rule.url.url->param.url.url[ 0 ] == L'\0' 
		|| all_params[ 3 ].data.wstring_val[ 0 ] == L'\0' )
	{
		log_trace( ( MSG_INFO, "*** check reg rule, but the reg path is null! ( rule 0x%0.8x ( %ws ), check 0x%0.8x ( %ws ) ) *** \n", 
			rule_item->rule.rule.url.url->param.url.url, 
			rule_item->rule.rule.url.url->param.url.url, 
			all_params[ 3 ].data.wstring_val, 
			all_params[ 3 ].data.wstring_val ) ); 
		goto _return; 
	}
	else
	{
		ret = compare_url_define( rule_item->rule.rule.url.url->param.url.url, 
			all_params[ 3 ].data.wstring_val ); 
		if( ret == FALSE )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

#define DOMAIN_NAME_DELIM_UNICODE L'.'
#define DOMAIN_NAME_DELIM '.'

NTSTATUS CALLBACK init_url_iteration_ex( PVOID param, 
										PULONG hash_code, 
										ULONG tbl_size, 
										PVOID *param_out, 
										PVOID *context )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	LPWSTR name; 
	ULONG _hash_code; 
	param_info *all_params; 
	ULONG name_len; 
	fs_style_interator *fs_style_path_iter = NULL; 
	USHORT domain_name_off; 
	USHORT file_name_off; 
	USHORT domain_name_len; 
	USHORT file_name_len; 

	ASSERT( context != NULL ); 
	ASSERT( tbl_size > 0 ); 
	ASSERT( hash_code != NULL ); 

	if( param_out == NULL )
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}; 

	if( param == NULL )
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	*param_out = NULL; 
	*context = NULL; 
	*hash_code = INVALID_HASH_CODE; 

	//if( ( buf_len % 2 ) != 0 )
	//{
	//	ASSERT( FALSE ); 
	//	log_trace( ( MSG_WARNING, "buffer length is not wide char size aligned\n" ) ); 
	//}

	all_params = ( param_info* )param; 

	ntstatus = parse_url( all_params[ 3 ].data.wstring_val, 
		wcslen( all_params[ 3 ].data.wstring_val ) + 1, 
		wcslen( all_params[ 3 ].data.wstring_val ) + 1, 
		&domain_name_off, 
		&domain_name_len, 
		&file_name_off, 
		&file_name_len ); 

	if( !NT_SUCCESS( ntstatus ) )
	{
		goto _return; 
	}

	name = ( LPWSTR )all_params[ 3 ].data.wstring_val 
		+ domain_name_off; 

	//if( name[ buf_len / sizeof( WCHAR ) ] != L'\0' )
	//{
	//	name[ buf_len / sizeof( WCHAR ) ] = L'\0'; 
	//}

	name_len = domain_name_len; 

	fs_style_path_iter = ( fs_style_interator* )ALLOC_TAG_POOL( sizeof( fs_style_interator ) ); 
	if( fs_style_path_iter == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	fs_style_path_iter->index = 0; 
	fs_style_path_iter->length = name_len + 1; 
	fs_style_path_iter->org_path = NULL; 
	fs_style_path_iter->iterator = NULL; 

	fs_style_path_iter->org_path = ( LPWSTR )ALLOC_TAG_POOL( ( ( name_len + 1 )* sizeof( WCHAR ) ) * 2 ) ; 

	if( fs_style_path_iter->org_path == NULL )
	{
		ntstatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _return; 
	}

	fs_style_path_iter->iterator = ( LPWSTR )fs_style_path_iter->org_path + name_len + 1; 

	memcpy( ( PVOID )fs_style_path_iter->org_path, name, ( name_len + 1 ) * sizeof( WCHAR ) ); 
	memcpy( fs_style_path_iter->iterator, name, ( name_len + 1 ) * sizeof( WCHAR ) ); 

	( ( LPWSTR )fs_style_path_iter->org_path )[ name_len ] = L'\0'; 
	fs_style_path_iter->iterator[ name_len ] = L'\0'; 

	unicode_str_to_upper( fs_style_path_iter->iterator ); 

	if( fs_style_path_iter->org_path[ name_len - 1 ] == DOMAIN_NAME_DELIM_UNICODE )
	{
		( ( LPWSTR )fs_style_path_iter->org_path )[ name_len - 1 ] = L'\0'; 
	}

	if( fs_style_path_iter->iterator[ name_len - 1 ] == DOMAIN_NAME_DELIM_UNICODE )
	{
		fs_style_path_iter->iterator[ name_len - 1 ] = L'\0'; 
	}

	if( fs_style_path_iter->iterator[ 0 ] == DOMAIN_NAME_DELIM_UNICODE )
	{
		fs_style_path_iter->index = 1; 
	}

	_hash_code = unicode_str_hash( fs_style_path_iter->iterator + fs_style_path_iter->index, tbl_size ); 
	*param_out = fs_style_path_iter->iterator + fs_style_path_iter->index; 
	*context = ( PVOID )fs_style_path_iter; 
	*hash_code = _hash_code; 

_return:
	if( !NT_SUCCESS( ntstatus ) )
	{
		if( fs_style_path_iter != NULL )
		{
			if( fs_style_path_iter->org_path != NULL )
			{
				FREE_TAG_POOL( ( PVOID )fs_style_path_iter->org_path ); 
			}

			FREE_TAG_POOL( fs_style_path_iter ); 
		}
	}
	return ntstatus; 
}

NTSTATUS CALLBACK uninit_url_iteration_ex( PVOID context )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	fs_style_interator *fs_style_path_iter; 

	ASSERT( context != NULL ); 

	fs_style_path_iter = ( fs_style_interator* )context; 
	if( fs_style_path_iter->org_path != NULL )
	{
		FREE_TAG_POOL( ( PVOID )fs_style_path_iter->org_path ); 
	}
	else
	{
		ASSERT( FALSE && "uninitialize a not initialized fs tyle name iterator" ); 
	}

	FREE_TAG_POOL( fs_style_path_iter ); 

	return ntstatus; 
}

NTSTATUS CALLBACK interate_url_path_ex( PVOID param, PULONG hash_code, ULONG tbl_size, PVOID *param_out )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	fs_style_interator *url_iter; 
	ULONG _hash_code; 

	//ASSERT( param != NULL ); 
	ASSERT( param_out != NULL ); 
	ASSERT( hash_code != NULL ); 
	ASSERT( tbl_size > 0 ); 

	if( param == NULL )
	{
		ASSERT( FALSE ); 
		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

	*param_out = NULL; 
	*hash_code = 0; 

	url_iter = ( fs_style_interator* )param; 

	if( url_iter->index >= url_iter->length - 1 )
	{
		ASSERT( ( LONG )url_iter->index >= 0 ); 

		ntstatus = STATUS_INVALID_PARAMETER; 
		goto _return; 
	}

#ifdef DBG
	if( url_iter->index == url_iter->length - 1 )
	{
		ASSERT( url_iter->iterator[ url_iter->index - 1 ] != DOMAIN_NAME_DELIM_UNICODE ); 
	}
	else if( url_iter->index != 0 )
	{
		ASSERT( url_iter->iterator[ url_iter->index ] == DOMAIN_NAME_DELIM_UNICODE ); 
	}
#endif //DBG

	for( ; ; )
	{
		if( url_iter->iterator[ url_iter->index ] == DOMAIN_NAME_DELIM_UNICODE )
		{
			url_iter->index ++; 
			break; 
		}
		else if( url_iter->iterator[ url_iter->index ] == L'\0' )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			goto _return; 
		}
		else if( url_iter->index == url_iter->length - 1 )
		{
			ntstatus = STATUS_UNSUCCESSFUL; 
			goto _return; 
		}

		url_iter->index ++; 
	}

	*param_out = url_iter->iterator + url_iter->index; 
	_hash_code = unicode_str_hash( url_iter->iterator + url_iter->index, tbl_size ); 
	*hash_code = _hash_code; 

_return:
	return ntstatus; 
}

INT32 CALLBACK compare_com_rule_ex( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration )
{
	INT32 ret = TRUE; 
	action_desc *params; 
	action_rule_item *rule_item; 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	rule_item = ( action_rule_item* )CONTAINING_RECORD( list_item, action_rule_item, entry ); 

	params = ( action_desc* )param; 

	ASSERT( rule_item->rule.rule.com.com_name != NULL ); 

	if( rule_item->rule.rule.com.app != NULL )
	{
		ret = compare_name_define( rule_item->rule.rule.com.app->param.app.app_name, 
			params->com.app.app.app_name ); 
		if( ret == FALSE )
		{
			goto _return; 
		}
	}

	if( rule_item->rule.rule.com.com_name->param.com.com_name[ 0 ] == L'\0' 
		|| params->com.com.app.app_name[ 0 ] == L'\0' )
	{
		log_trace( ( MSG_INFO, "*** check reg rule, but the reg path is null! ( rule 0x%0.8x ( %ws ), check 0x%0.8x ( %ws ) ) *** \n", 
			rule_item->rule.rule.com.com_name->param.com.com_name, 
			rule_item->rule.rule.com.com_name->param.com.com_name, 
			params->com.com.app.app_name, 
			params->com.com.app.app_name ) ); 
		goto _return; 
	}
	else
	{
		ret = compare_name_define( rule_item->rule.rule.com.com_name->param.com.com_name, params->com.com.app.app_name ); 
		if( ret == FALSE  )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

INT32 CALLBACK compare_file_rule_ex( PVOID param, 
									PLIST_ENTRY list_item, 
									PVOID param_iteration )
{
	INT32 ret = TRUE; 
	param_info *all_params; 
	action_rule_item *rule_item; 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	do 
	{
		rule_item = ( action_rule_item* )CONTAINING_RECORD( list_item, action_rule_item, entry ); 

		all_params = ( param_info* )param; 

		ASSERT( rule_item->rule.rule.file.file_path != NULL ); 

		if( rule_item->rule.rule.file.app != NULL )
		{
			log_trace( ( MSG_INFO, "record file name is %ws action file name is %ws \n", rule_item->rule.rule.app.app->param.app.app_name, all_params[ 0 ].data.wstring_val ) ); 

			if( all_params[ 0 ].type != WSTRING_TYPE )
			{
				ASSERT( FALSE ); 
				ret = FALSE; 
				break; 
			}

			ret = compare_name_define( rule_item->rule.rule.file.app->param.app.app_name, 
				all_params[ 0 ].data.wstring_val ); 
			if( ret == FALSE )
			{
				break; 
			}
		}

		if( all_params[ 3 ].type != WSTRING_TYPE )
		{
			ASSERT( FALSE ); 
			ret = FALSE; 
			break; 
		}

		if( rule_item->rule.rule.file.file_path->param.file.file_path[ 0 ] == L'\0' 
			|| all_params[ 3 ].data.wstring_val[ 0 ] == L'\0' )
		{
			ASSERT( FALSE ); 
			log_trace( ( MSG_INFO, "*** check reg rule, but the reg path is null! ( rule 0x%0.8x ( %ws ), check 0x%0.8x ( %ws ) ) *** \n", 
				rule_item->rule.rule.file.file_path->param.file.file_path, 
				rule_item->rule.rule.file.file_path->param.file.file_path, 
				all_params[ 3 ].data.wstring_val, 
				all_params[ 3 ].data.wstring_val ) ); 

			break; 
		}
		else
		{
			log_trace( ( MSG_INFO, "record file name is %ws action file name is %ws \n", 
				rule_item->rule.rule.file.file_path->param.file.file_path, 
				all_params[ 3 ].data.wstring_val ) ); 

			if( param_iteration != NULL )
			{
				ret = compare_name_define( rule_item->rule.rule.file.file_path->param.file.file_path, 
					( LPWSTR )param_iteration ); 

			}
			else
			{
				ret = compare_name_define( rule_item->rule.rule.file.file_path->param.file.file_path, 
					all_params[ 3 ].data.wstring_val ); 
			}
		}

		//ASSERT( params->type == rule_item->rule.type ); 

	}while( FALSE );

	log_trace( ( MSG_INFO, "enter %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	return ret; 
}

//INT32 CALLBACK compare_com_rule_ex( PVOID param, PLIST_ENTRY list_item, PVOID param_iteration )
//{
//	INT32 ret = TRUE; 
//	param_info *all_params; 
//	action_rule_item *rule_item; 
//
//	ASSERT( param != NULL ); 
//	ASSERT( list_item != NULL ); 
//
//	rule_item = ( action_rule_item* )CONTAINING_RECORD( list_item, action_rule_item, entry ); 
//
//	all_params = ( param_info* )param; 
//
//	ASSERT( rule_item->rule.rule.com.com_name != NULL ); 
//
//	if( rule_item->rule.rule.com.app != NULL )
//	{
//		ASSERT( all_params[ 0 ].type == WSTRING_TYPE ); 
//
//		ret = compare_name_define( rule_item->rule.rule.com.app->param.app.app_name, 
//			all_params[ 0 ].data.wstring_val ); 
//		if( ret == FALSE )
//		{
//			goto _return; 
//		}
//	}
//
//	ASSERT( all_params[ 3 ].type == WSTRING_TYPE ); 
//	ASSERT( all_params[ 3 ].data.wstring_val != NULL ); 
//
//	if( rule_item->rule.rule.com.com_name->param.com.com_name[ 0 ] == L'\0' 
//		|| all_params[ 3 ].data.wstring_val[ 0 ] == L'\0' )
//	{
//		log_trace( ( MSG_INFO, "*** check reg rule, but the reg path is null! ( rule 0x%0.8x ( %ws ), check 0x%0.8x ( %ws ) ) *** \n", 
//			rule_item->rule.rule.com.com_name->param.com.com_name, 
//			rule_item->rule.rule.com.com_name->param.com.com_name, 
//			all_params[ 3 ].data.wstring_val, 
//			all_params[ 3 ].data.wstring_val ) ); 
//		goto _return; 
//	}
//	else
//	{
//		ret = compare_name_define( rule_item->rule.rule.com.com_name->param.com.com_name, 
//			all_params[ 3 ].data.wstring_val ); 
//		if( ret == FALSE  )
//		{
//			goto _return; 
//		}
//	}
//
//_return:
//	return ret; 
//}

INT32 CALLBACK compare_common_rule_ex( PVOID param, 
									  PLIST_ENTRY list_item, 
									  PVOID param_iteration )
{
	INT32 ret = TRUE; 
	param_info *all_params; 
	action_rule_item *rule_item; 

	ASSERT( param != NULL ); 
	ASSERT( list_item != NULL ); 

	rule_item = CONTAINING_RECORD( list_item, action_rule_item, entry ); 

	all_params = ( param_info* )param; 

	ASSERT( rule_item->rule.rule.common.app != NULL 
		&& rule_item->rule.rule.common.param0 != NULL 
		|| rule_item->rule.rule.common.param1 != NULL 
		|| rule_item->rule.rule.common.param2 != NULL 
		|| rule_item->rule.rule.common.param3 != NULL ); 

	if( rule_item->rule.rule.common.action_type != ( sys_action_type )all_params[ 0 ].data.uint32_val )
	{
		ret = FALSE; 
		goto _return; 
	}

	if( rule_item->rule.rule.common.app != NULL )
	{
		ret = compare_name_define( rule_item->rule.rule.reg.app->param.app.app_name, 
			all_params[ 1 ].data.wstring_val ); 
		if( ret == FALSE )
		{
			goto _return; 
		}
	}

	if( rule_item->rule.rule.common.param0 != NULL )
	{
		ret = compare_name_define( rule_item->rule.rule.common.param0->param.common.name, 
			all_params[ 3 ].data.wstring_val ); 
		if( ret == FALSE )
		{
			goto _return; 
		}
	}

_return:
	return ret; 
}

INLINE VOID get_action_rule_func( access_rule_type rule_type, 
								 init_iteration_callback *init_iteration_func, 
								 uninit_iteration_callback *uninit_iteration_func, 
								 iterate_name_callback *iteration_func, 
								 calc_hash_code_callback *hash_code_func, 
								 compare_hash_table_item_callback *compare_func )
{
	switch( rule_type )
	{
	case URL_RULE_TYPE: 
		*init_iteration_func = init_url_iteration_ex; 
		*uninit_iteration_func = uninit_url_iteration_ex; 
		*iteration_func = interate_url_path_ex; 
		*hash_code_func = calc_url_rule_hash_code_ex; 
		*compare_func = compare_url_rule_ex; 
		break; 
	case SOCKET_RULE_TYPE: 
		*init_iteration_func = NULL; 
		*uninit_iteration_func = NULL; 
		*iteration_func = NULL; 
		*hash_code_func = calc_socket_rule_hash_code_ex; 
		*compare_func = compare_socket_rule_ex; 
		break; 
	case FILE_RULE_TYPE:
		*init_iteration_func = init_name_iteration_ex; 
		*uninit_iteration_func = uninit_name_iteration_ex; 
		*iteration_func = iterate_fs_style_path; 
		*hash_code_func = calc_file_rule_hash_code_ex; 
		*compare_func = compare_file_rule_ex; 
		break; 
	case REG_RULE_TYPE: 
		*init_iteration_func = init_name_iteration_ex; 
		*uninit_iteration_func = uninit_name_iteration_ex; 
		*iteration_func = iterate_fs_style_path; 
		*hash_code_func = calc_reg_rule_hash_code_ex; 
		*compare_func = compare_reg_rule_ex; 
		break; 
	case COM_RULE_TYPE: 
		*init_iteration_func = NULL; 
		*uninit_iteration_func = NULL; 
		*iteration_func = NULL; 
		*hash_code_func = calc_com_rule_hash_code_ex; 
		*compare_func = compare_com_rule_ex; 
		break; 
	case COMMON_RULE_TYPE: 
		*init_iteration_func = NULL; 
		*uninit_iteration_func = NULL; 
		*iteration_func = NULL; 
		*hash_code_func = calc_common_rule_hash_code_ex; 
		*compare_func = compare_common_rule_ex; 
		break; 
	default:
		ASSERT( FALSE ); 
		break; 
	}
}

VOID _get_action_rule_func_and_param_ex( access_rule_type rule_type, 
									 param_info *all_params, 
									 init_iteration_callback *init_iteration_func, 
									 uninit_iteration_callback *uninit_iteration_func, 
									 iterate_name_callback *iterate_func, 
									 calc_hash_code_callback *hash_code_func, 
									 compare_hash_table_item_callback *compare_func, 
									 PVOID *param )
{
	ASSERT( is_valid_access_rule_type( rule_type ) == TRUE ); 

	*param = ( PVOID )all_params; 
	
	get_action_rule_func( rule_type, 
		init_iteration_func, 
		uninit_iteration_func, 
		iterate_func, 
		hash_code_func, 
		compare_func ); 
}

//NTSTATUS find_action_rule_ex( access_rule_type type, 
//						  param_info *all_params, 
//						  data_trace_option *trace_option, 
//						  action_reply*response )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	common_hash_table *table; 
//	PLIST_ENTRY item_found; 
//	action_rule_item *rule_found; 
//	PVOID param; 
//	init_iteration_callback init_iteration_func; 
//	uninit_iteration_callback uninit_iteration_func; 
//	iterate_name_callback iterate_func; 
//	calc_hash_code_callback hash_code_func; 
//	compare_hash_table_item_callback compare_func; 
//	action_reply response = ACTION_ALLOW; 
//
//	ASSERT( is_valid_access_rule_type( type ) == TRUE ); 
//	//ASSERT( is_rbtree_link_rule( type ) == FALSE ); 
//	ASSERT( all_params != NULL ); 
//
//	do 
//	{
//		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 
//
//		if( FALSE == is_valid_access_rule_type( type ) )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_1; 
//			break; 
//		}
//
//		if( NULL == all_params )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_2; 
//			break; 
//		}
//
//		table = get_action_rule_table( type ); 
//
//		get_action_rule_func_and_param( type, 
//			all_params, 
//			&init_iteration_func, 
//			&uninit_iteration_func, 
//			&iterate_func, 
//			&hash_code_func, 
//			&compare_func, 
//			&param ); 
//
//		log_trace( ( MSG_INFO, "init iteration fucntion is 0x%0.8x, uninit iteration fuction is 0x%0.8x iteration function is 0x%0.8x hash function is 0x%0.8x, compare function is 0x%0.8x parameter is 0x%0.8x hash table is 0x%0.8x\n", init_iteration_func, uninit_iteration_func, iterate_func, hash_code_func, compare_func, param, table ) ); 
//
//		hold_hash_table_lock( table ); 
//		ntstatus = find_in_hash_table_lock_free( ( PVOID )&cur_action->desc, init_iteration_func, uninit_iteration_func, iterate_func, hash_code_func, compare_func, table, &item_found ); 
//
//		if( !NT_SUCCESS( ntstatus ) )
//		{
//			log_trace( ( MSG_ERROR, "!!!find this action rule failed\n" ) ); 
//			goto _return; 
//		}
//
//		ASSERT( item_found != NULL ); 
//		rule_found = ( action_rule_item* )CONTAINING_RECORD( item_found, action_rule_item, entry ); 
//
//		if( NULL != trace_option )
//		{
//			*trace_option = rule_found->rule.trace_option; 
//		}
//
//		cur_action->resp = rule_found->rule.action; 
//	}while( FALSE );
//
//_return: 
//	release_hash_table_lock( table ); 
//
//	if( response != NULL )
//	{
//		*response = _response; 
//	}
//
//	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ntstatus ) ); 
//
//	return ntstatus; 
//}

/******************************************************************
HOOK type:
previous hook for blocking executing
post hook fro logging
******************************************************************/

/*****************************************************************
policies group type:
have order
:do that by saving state

have not order
:do that by condition array.

******************************************************************/
//NTSTATUS check_sys_load_kmod( action_source_info *info, 
//							 action_context *ctx, 
//							 policy_holder *sub_policy, 
//							 sys_action_record *load_kmod )
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	sys_action_policy *policy; 
//
//	do 
//	{
//		ASSERT( action != NULL );  
//		ASSERT( sub_policy != NULL ); 
//		ASSERT( load_kmod != NULL ); 
//
//		policy = ( sys_action_policy* )sub_policy->policy; 
//
//		if( policy == NULL )
//		{
//			ntstatus = STATUS_NOT_FOUND; 
//			break; 
//		}
//
//		//if( ctx == NULL )
//		//{
//		//	ntstatus = STATUS_INVALID_PARAMETER_1; 
//		//	break; 
//		//}
//
//		//policy = ctx->prev_state.pol_locator; 
//		//if( policy == NULL )
//		//{
//		//	ntstatus = STATUS_NOT_FOUND; 
//		//	break; 
//		//}
//
//		//pol_holder->check_policy_func( driver_info ); 
//
//		ntstatus = is_valid_mem_region( policy, calc_sys_action_record_size( SYS_load_kmod ) ); 
//		if( ntstatus != STATUS_SUCCESS )
//		{
//			break; 
//		}
//
//		if( 0 != wcsncmp( policy->policy.do_sys_load_kmod.path_name, 
//			load_kmod->do_sys_load_kmod.path_name, 
//			MAX_PATH ) )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_2; 
//			break; 
//		}
//
//		if( info->thread_id != ( HANDLE )ctx->thread_id )
//		{
//			ntstatus = STATUS_INVALID_PARAMETER_3; 
//			break; 
//		}
//
//	}while( FALSE );
//
//	return ntstatus; 
//}

NTSTATUS check_sys_enumproc( sys_enumproc do_sys_enumproc ); 
NTSTATUS check_sys_regsrv( sys_regsrv do_sys_regsrv ); 
NTSTATUS check_sys_opendev( sys_opendev do_sys_opendev ); 
NTSTATUS check_w32_postmsg( w32_postmsg do_w32_postmsg ); 
NTSTATUS check_w32_sendmsg( w32_sendmsg do_w32_sendmsg ); 
NTSTATUS check_w32_findwnd( w32_findwnd do_w32_findwnd ); 
NTSTATUS check_w32_msghook( w32_msghook do_w32_msghook ); 
NTSTATUS check_w32_lib_inject( w32_lib_inject do_w32_lib_inject ); 
NTSTATUS check_net_connect( net_connect do_net_connect ); 
NTSTATUS check_net_listen( net_listen do_net_listen ); 
NTSTATUS check_net_send( net_send do_net_send ); 
NTSTATUS check_net_http( net_http do_net_http ); 
NTSTATUS check_ba_extract_hidden( ba_extract_hidden do_ba_extract_hidden ); 
NTSTATUS check_ba_extract_pe( ba_extract_pe do_ba_extract_pe ); 
NTSTATUS check_ba_self_copy( ba_self_copy do_ba_self_copy ); 

NTSTATUS check_socket_action( LPCWSTR proc_name, 
							 ULONG proc_id, 
							 LPCWSTR user_name, 
							 IP_ADDR_T local_addr, 
							 PORT_T local_port, 
							 IP_ADDR_T remote_addr, 
							 PORT_T remote_port, 
							 ULONG protocol, 
							 PVOID data, 
							 ULONG data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_info all_params[ 7 ]; 

	do
	{
		set_param_info( all_params[ 0 ], WSTRING_TYPE, wstring_val, proc_name ); 
		set_param_info( all_params[ 1 ], UINT32_TYPE, uint32_val, proc_id ); 
		set_param_info( all_params[ 2 ], WSTRING_TYPE, wstring_val, user_name ); 

		set_param_info( all_params[ 3 ], UINT32_TYPE, uint32_val, local_addr ); 
		set_param_info( all_params[ 4 ], UINT16_TYPE, uint16_val, local_port ); 
		set_param_info( all_params[ 5 ], UINT32_TYPE, uint32_val, remote_addr ); 
		set_param_info( all_params[ 6 ], UINT32_TYPE, uint16_val, remote_port ); 
		set_param_info( all_params[ 7 ], UINT32_TYPE, uint32_val, protocol ); 
		set_param_info( all_params[ 8 ], PTR_TYPE, ptr_val, data ); 
		set_param_info( all_params[ 9 ], UINT32_TYPE, uint32_val, data_len ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS check_reg_action( 	LPCWSTR proc_name, 
							 ULONG proc_id, 
							 LPCWSTR user_name, 
							 LPCWSTR reg_key_path, 
							 LPCWSTR reg_val_name, 
							 ULONG reg_val_type, 
							 PVOID reg_val_data, 
							 ULONG val_data_len )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_info all_params[ 7 ]; 

	do
	{
		set_param_info( all_params[ 0 ], WSTRING_TYPE, wstring_val, proc_name ); 
		set_param_info( all_params[ 1 ], UINT32_TYPE, uint32_val, proc_id ); 
		//set_param_info( all_params[ 2 ], WSTRING_TYPE, wstring_val, user_name ); 

		//set_param_info( all_params[ 3 ], UINT32_TYPE, uint32_val, local_addr ); 
		//set_param_info( all_params[ 4 ], UINT16_TYPE, uint16_val, local_port ); 
		//set_param_info( all_params[ 5 ], UINT32_TYPE, uint32_val, remote_addr ); 
		//set_param_info( all_params[ 6 ], UINT32_TYPE, uint16_val, remote_port ); 
		//set_param_info( all_params[ 7 ], UINT32_TYPE, uint32_val, protocol ); 
		//set_param_info( all_params[ 8 ], PTR_TYPE, ptr_val, data ); 
		//set_param_info( all_params[ 9 ], UINT32_TYPE, uint32_val, data_len ); 

	}while( FALSE ); 

	return ntstatus; 
}

NTSTATUS check_file_action( sys_action_type type, 
						   LPCWSTR proc_name, 
						   LPCWSTR user_name, 
						   LPCWSTR file_name, 
						   PVOID data, 
						   ULONG data_len, 
						   data_trace_option *trace_option, 
						   ULONG flags, 
						   action_reply*response )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	param_info all_params[ 5 ]; 
	access_rule_type rule_type; 
	action_reply _response = ACTION_ALLOW; 

	do 
	{
		//ASSERT( need_log != NULL ); 
		ASSERT( file_name != NULL ); 

		log_trace( ( MSG_INFO, "enter %s, action is %ws\n", __FUNCTION__, get_action_desc( type ) ) ); ; 

		//if( response != NULL )
		//{
		//	*response = ACTION_ALLOW; 
		//}

		//ntstatus = check_sys_action_input_valid( cur_action ); 
		//if( !NT_SUCCESS( ntstatus ) ) 
		//{
		//	ASSERT( FALSE ); 
		//	break; 
		//}

		ntstatus = find_app_response_record( proc_name, 
			wcslen( proc_name ) + 1, 
			trace_option, 
			&_response ); 

		if( NT_SUCCESS( ntstatus ) )
		{
			ASSERT( _response == ACTION_ALLOW 
				|| _response == ACTION_BLOCK ); 

			break; 
		}

		ntstatus = find_response_record( type, 
			proc_name, 
			wcslen( proc_name ) + 1, 
			trace_option, 
			&_response ); 

		if( NT_SUCCESS( ntstatus ) )
		{
			ASSERT( _response == ACTION_ALLOW 
				|| _response == ACTION_BLOCK ); 

			break; 
		}

		rule_type = acl_type( type ); 

		set_param_info( all_params[ 0 ], 
			WSTRING_TYPE, 
			wstring_val, 
			proc_name ); 

		set_param_info( all_params[ 1 ], 
			WSTRING_TYPE, 
			wstring_val, 
			user_name ); 

		set_param_info( all_params[ 2 ], 
			WSTRING_TYPE, 
			wstring_val, 
			file_name ); 

		set_param_info( all_params[ 3 ], 
			PTR_TYPE, 
			ptr_val, 
			data ); 

		set_param_info( all_params[ 4 ], 
			UINT32_TYPE, 
			uint32_val, 
			data_len ); 

		//ntstatus = find_action_rule_ex( rule_type, 
		//	all_params, 
		//	ARRAY_SIZE( all_params ), 
		//	trace_option ); 

		//if( !NT_SUCCESS( ntstatus ) )
		//{
		//	break; 
		//}

#ifdef CHECK_TRACE_OPTION
		if( trace_option != NULL )
		{
			log_trace( ( MSG_INFO, "trace mode is 0x%0.8x, trace data size is %u\n", 
				trace_option->trace_mode, 
				trace_option->tracing_size ) ); 
		}
#endif //CHECK_TRACE_OPTION

	}while( FALSE ); 


	if( response != NULL )
	{
		*response = _response; 
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ntstatus ) ); 

	return ntstatus; 
}

NTSTATUS check_sys_common_action( LPCWSTR proc_name, 
								 LPCWSTR user_name, 
								 LPCWSTR reg_path, 
								 ULONG reg_path_len, 
								 LPCWSTR value_name, 
								 ULONG value_name_len, 
								 sys_action_type type )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS check_sys_proc_thread_action( LPCWSTR proc_name, 
								 LPCWSTR user_name, 
								 LPCWSTR reg_path, 
								 ULONG reg_path_len, 
								 LPCWSTR value_name, 
								 ULONG value_name_len, 
								 sys_action_type type )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS check_sys_w32_action( LPCWSTR proc_name, 
							  LPCWSTR user_name, 
							  LPCWSTR reg_path, 
							  ULONG reg_path_len, 
							  LPCWSTR value_name, 
							  ULONG value_name_len, 
							  sys_action_type type )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ntstatus; 
}

NTSTATUS check_sys_ba_action( LPCWSTR proc_name, 
							  LPCWSTR user_name, 
							  LPCWSTR obj_path, 
							  sys_action_type type )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 

	do 
	{

	}while( FALSE );

	return ntstatus; 
}

action_check_dispatch action_dispatch; 
//NTSTATUS init_static_policy_dispatch()
//{
//	NTSTATUS ntstatus = STATUS_SUCCESS; 
//	INT32 i; 
//
//	do 
//	{
//		for( i = 0; i < ARRAY_SIZE( action_dispatch.action_check_functions ); i ++ )
//		{
//			action_dispatch.action_check_functions[ i ] = ( CHECK_ACTION_FUCTION )NULL; 
//		}	
//	
//		//ntstatus = register_action_check_dispatch( &action_dispatch ); 
//
//	}while( FALSE ); 
//
//	return ntstatus; 
//}
//

NTSTATUS check_static_action( action_source_info *source, 
					  sys_action_info *action, 
					  policy_holder *policy )
{
	NTSTATUS ntstatus = STATUS_SUCCESS; 
	
#if 0
	//action_response_type _resp = ACTION_ALLOW; 

	switch( action->action.type )
	{
	case SOCKET_CONNECT:
	case SOCKET_SEND:
	case SOCKET_RECV:


#ifdef _DEBUG
		{
			if( debugging_action == TRUE )
			{
				in_addr addr; 
				addr.s_addr = action->action.socket_info.dest_ip; 
				set_socket_debug_info( inet_ntoa( addr ), action->action.socket_info.dest_port ); 
			}
		} 
		check_socket_debug_info( action->action.socket_info.dest_ip, action->action.socket_info.dest_port ); 
#endif //_DEBUG
		ntstatus = check_socket_acl_direct( 
			action->ctx.proc_name, 
			&action->action, 
			NULL, 
			&_resp ); 

		resp->action = _resp; 

		if( _resp == ACTION_LEARN )
		{
			if( action_desc_out != NULL )
			{
				ntstatus = generate_action_desc_from_action( action, action_desc_out ); 
				if( ntstatus != STATUS_SUCCESS )
				{
					break; 
				}
			}
		}

		break; 


	case LOCATE_URL:
		ASSERT( FALSE && "can't reach here check url rule "); 
		//ntstatus = check_name_acl( action_info->action.type, action_info->ctx.proc_name, action_info->action.url_info.file_path, action_info->action.url_info.file_path, &need_log ); 

		//ntstatus = url_filter( action_info->action.url_info.domain_name, action_info->action.url_info.file_path ); 

		//if( NT_SUCCESS( ntstatus ) )
		//{
		//	response = ACTION_BLOCK; 
		//}
		//else
		//{
		//	response = ACTION_ALLOW;
		//}
		action_checked = FALSE; 
		break; 
	case INSTALL_DRV:
		name_param = action->action.driver_info.path_name; 
		break; 
	case ACCESS_COM:
		break; 
	case MODIFY_KEY:
		name_param = action->action.reg_info.path_name; 
		break; 
	case MODIFY_FILE: 
	case DELETE_FILE: 
		name_param = action->action.file_info.path_name; 
		break; 
	case INSTALL_HOOK: 
		break; 
	case CREATE_PROC:
		name_param = action->action.proc_info.path_name; 
		break; 
	case TERMINATE_PROC:
		name_param = action->action.proc_info.path_name; 
		break; 
	case ACCESS_MEM:
		break; 
	case OTHER_ACTION: 
		break; 
		//case INSTALL_DRV: 
		//case INSTALL_HOOK:
		//case CREATE_PROC:
		//case ACCESS_MEM: 
		//case OTHER_ACTION:
	case EXEC_create://进程启动 进程路径名 （执行监控） 
#define SYS_ACTION_RECORD_FRONT_PART_SIZE ( ULONG )( FIELD_OFFSET( sys_action_record, do_exec_create ) )
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 
	case EXEC_destroy://进程退出 进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case EXEC_module_load: //模块加载 模块路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

		//MT_procmon: 
	case PROC_exec: //创建进程 目标进程路径名  （进程监控）
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_open: //打开进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_debug: //调试进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_suspend: //挂起进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_resume: //恢复进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_exit: //结束进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_job: //将进程加入工作集 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_pgprot: //跨进程修改内存属性 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_freevm: //跨进程释放内存 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_writevm: //跨进程写内存 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case PROC_readvm: //跨进程读内存 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_remote: //创建远程线程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_create:
		{
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( thrd_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ASSERT( FALSE ); 
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_thrd_create, 
				&resp->action ); 
		}
		break; 
	case THRD_setctxt: //跨进程设置线程上下文 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_thread_create( &action->ctx, 
				&action->action.do_thrd_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_suspend: //跨进程挂起线程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_resume: //跨进程恢复线程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_exit: //跨进程结束线程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case THRD_queue_apc: //跨进程排队APC 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 


		//MT_common
	case COM_access: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

		//MT_sysmon
	case SYS_settime: //设置系统时间 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_link_knowndll: //建立KnownDlls链接 链接文件名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_open_physmm: //打开物理内存设备 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_read_physmm: //读物理内存 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_write_physmm: //写物理内存 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_load_kmod: //加载内核模块 内核模块全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_enumproc: //枚举进程 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_regsrv: //注册服务 服务进程全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case SYS_opendev: //打开设备 设备名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 


		//MT_w32mon
	case W32_postmsg: //发送窗口消息（Post） 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case W32_sendmsg: //发送窗口消息（Send） 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case W32_findwnd: //查找窗口 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case W32_msghook: //设置消息钩子 无 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case W32_lib_inject: //DLL注入 注入DLL路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 


		//MT_behavior: 
	case BA_extract_hidden: //释放隐藏文件 释放文件路径名 （行为监控） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_extract_pe: //释放PE文件 释放文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_self_copy: //自我复制 复制目标文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_self_delete: //自我删除 删除文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_ulterior_exec: //隐秘执行 被执行映像路径名 

		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_invade_process: //入侵进程 目标进程路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_infect_pe: //感染PE文件 目标文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_overwrite_pe: //覆写PE文件 目标文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case BA_register_autorun: //注册自启动项 自启动文件路径名 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

		//case BA_other: 
		ASSERT( action->action.type == INSTALL_DRV ); 
		resp->action = default_response; 
		break; 

		//MT_regmon: 
	case REG_openkey: //打开注册表键 注册表键路径  （注册表监控） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_mkkey: //创建注册表键 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_rmkey: //删除注册表键 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_mvkey: //重命名注册表键 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_rmval: //删除注册表键 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_getval: //获取注册表值 注册表值路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_setval: //设置注册表值 注册表值路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_loadkey: //挂载注册表Hive文件 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_replkey: //替换注册表键 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_rstrkey: //导入注册表Hive文件 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case REG_setsec: //设置注册表键安全属性 注册表键路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

		resp->action = default_response; 
		break; 

		//case MODIFY_FILE:
		//case DELETE_FILE:
		//MT_filemon: 
	case FILE_touch: //创建文件 文件全路径 （文件监控） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_open: //打开文件 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_read: //读取文件 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_write: //写入文件 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_modified: //文件被修改 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_readdir: //遍历目录 目录全路径 

		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 
	case FILE_remove: //删除文件 文件全路径 

		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 
	case FILE_rename: //重命名文件 文件全路径 

		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 
	case FILE_truncate: //截断文件 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_mklink: //建立文件硬链接 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_chmod: //设置文件属性 文件全路径 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case FILE_setsec: //设置文件安全属性 文件全路径 

		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 
		ntstatus = check_file_action( ctx, action, &resp->action ); 
		resp->action = default_response; 
		break; 

		//case SOCKET_CONNECT:
		//case SOCKET_SEND:
		//case SOCKET_RECV:
		//MT_netmon: 
	case NET_create: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_connect: //网络连接 远程地址（格式：IP：端口号） （网络监控） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_listen: //监听端口 本机地址（格式：IP：端口号） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_send: //发送数据包 远程地址（格式：IP：端口号） 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_recv: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_accept: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( net_accept ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_net_accept( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 


	case NET_http: //HTTP请求 HTTP请求路径（格式：域名/URL） 
		1				{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_icmp_send: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

	case NET_icmp_recv: 
		{		
			if( action->action.size < 
				( SYS_ACTION_RECORD_FRONT_PART_SIZE + sizeof( exec_create ) + ( ( action->action.do_exec_create.path_len + 1 ) << 1 ) ) )
			{
				ntstatus = STATUS_INVALID_PARAMETER; 
				break; 
			}

			ntstatus = check_exec_create( &action->ctx, 
				&action->action.do_exec_create, 
				&resp->action ); 
		}	
		break; 

		//_resp = ACTION_LEARN; 
		resp->action = default_response; 
		break; 

	default:
		ASSERT( "invalid sys action type" && FALSE ); 
		ASSERT( FALSE && "unknown sys action" ); 
		ret = ERROR_INVALID_PARAMETER; 
		_action_checked = TRUE; 
		action_checked = FALSE; 
		break; 	
	}

	do 
	{
		if( TRUE == _action_checked )
		{
			break; 
		}

		if( name_param == NULL )
		{
			break; 
		}

		ntstatus = check_name_acl( action->action.type, action->ctx.proc_name, ( LPWSTR )name_param, NULL, &_resp, action_desc_out );  

		if( !NT_SUCCESS( ntstatus ) 
			/*&& ntstatus != STATUS_NOT_FOUND */ )
		{
			ASSERT( _resp == ACTION_ALLOW ); 
			//?? add the log granularity control function 
			ASSERT( need_log == FALSE ); 
			action_checked = FALSE; 
		}
		//else
		//{

		//}

		resp->action = _resp; 
	}while( FALSE ); 

#endif //0

	return ntstatus; 
}

/**************************************************************
original code
**************************************************************/
