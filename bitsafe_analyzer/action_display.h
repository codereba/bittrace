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

#ifndef __ACTION_DISPLAY_H__
#define __ACTION_DISPLAY_H__

#include "acl_define.h"

#define LOG_MSG 0x0000001

#define IPV4_ADDR_LEN 32
#define SOCKET_PARAM_NUM 8 
#define FILE_ACTION_PARAM_NUM 1

typedef struct _prepared_params_desc
{
	LPCWSTR desc; 
	ULONG desc_len; 
} prepared_params_desc, *pprepared_params_desc; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	INLINE LPCWSTR get_ip_proto_text( ULONG type )
	{
		LPCWSTR desc = L""; 

		switch( type )
		{
		case PROT_TCP: 
			desc = L"TCP"; //L"TCP协议"; 
			break; 
		case PROT_UDP:
			desc = L"UDP"; //L"UDP协议"; 
			break; 
		default:
			desc = L"*"; 
			break; 
		}

		return desc; 
	}

	LPCWSTR WINAPI get_reg_value_type_desc( ULONG type ); 

	LRESULT WINAPI get_reg_info_detail( ULONG key_class, 
		PVOID key_info, 
		ULONG info_size, 
		LPWSTR text, 
		ULONG cc_buf_len, 
		ULONG *cc_ret_len ); 

	LRESULT WINAPI get_reg_value_info_detail( ULONG value_class, 
		PVOID value_info, 
		ULONG info_size, 
		LPWSTR text, 
		ULONG cc_buf_len, 
		ULONG *cc_ret_len ); 


LRESULT WINAPI _get_action_main_type( sys_action_type type, action_main_type *main_type ); 


LRESULT WINAPI get_net_recv_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_send_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI get_net_connect_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_http_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_file_remove_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI get_file_write_desc( sys_action_record *action, 
							action_context *ctx, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags ); 

LRESULT WINAPI get_reg_mk_key_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 
LRESULT WINAPI get_proc_com_access_desc( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 
LRESULT WINAPI get_proc_readvm_desc( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 
LRESULT WINAPI get_proc_exec_desc( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags ); 
LRESULT WINAPI get_w32_msg_hook_desc( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 
LRESULT WINAPI get_sys_load_kmod_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *params_desc, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

#define LOAD_DATA_PTR_PARAM 0x00000001

#define MAX_ACTION_TIP_LEN 2048


LRESULT WINAPI get_key_data_text( LPWSTR data_dump, ULONG ccb_buf_len, PVOID data, ULONG data_len ); 


#define SAVE_INFO_BY_POINTER 0x000000080
#define PRINT_PROCESS_PATH 0x00000100

LRESULT WINAPI get_object_path( sys_action_record *action, 
							   action_context *ctx, 
							   LPWSTR obj_path, 
							   ULONG cc_buf_len, 
							   LPCWSTR *obj_path_ptr, 
							   ULONG flags, 
							   ULONG *cc_ret_len ); 

LRESULT WINAPI get_net_recv_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_accept_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_send_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_create_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_dns_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_http_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_ba_extract_hidden_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_ba_extract_pe_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_ba_self_copy_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_ba_self_delete_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_ba_ulterior_exec_detail( sys_action_record *action, 
								  action_context *ctx, 
								  prepared_params_desc *params_desc, 
								  LPWSTR tip, 
								  ULONG ccb_buf_len, 
								  ULONG flags ); 

LRESULT WINAPI get_ba_invade_process_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_ba_infect_pe_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_ba_overwrite_pe_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_ba_register_autorun_detail( sys_action_record *action, 
									 action_context *ctx, 
									 prepared_params_desc *params_desc, 
									 LPWSTR tip, 
									 ULONG ccb_buf_len, 
									 ULONG flags ); 

LRESULT WINAPI get_file_open_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_file_read_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_file_remove_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_file_read_desc( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_file_write_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI get_reg_mk_key_detail( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags ); 

LRESULT WINAPI get_proc_com_access_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_proc_readvm_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_proc_exec_detail( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags ); 

LRESULT WINAPI get_w32_msg_hook_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_w32_lib_inject_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_net_connect_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_net_listen_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI get_exec_create_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_exec_module_load_detail( sys_action_record *action, 
									action_context *ctx, 
									prepared_params_desc *params_desc, 
									LPWSTR tip, 
									ULONG ccb_buf_len, 
									ULONG flags ); 

LRESULT WINAPI get_exec_destroy_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_file_touch_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_file_modified_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_file_readdir_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_file_rename_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_file_truncate_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_file_mklink_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_file_chmod_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_file_setsec_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_file_attr_desc( ULONG file_attr, LPWSTR text, ULONG cc_buf_len, LPWSTR *text_remain, size_t *cc_remain_len ); 

LRESULT WINAPI get_file_setinfo_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_file_close_detail( sys_action_record *action, 
									 action_context *ctx, 
									 prepared_params_desc *params_desc, 
									 LPWSTR tip, 
									 ULONG ccb_buf_len, 
									 ULONG flags ); 

#ifndef FILE_INFORMATION_CLASS_DEFINED

#define FILE_INFORMATION_CLASS_DEFINED 1

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation         = 1,
	FileFullDirectoryInformation,   // 2
	FileBothDirectoryInformation,   // 3
	FileBasicInformation,           // 4
	FileStandardInformation,        // 5
	FileInternalInformation,        // 6
	FileEaInformation,              // 7
	FileAccessInformation,          // 8
	FileNameInformation,            // 9
	FileRenameInformation,          // 10
	FileLinkInformation,            // 11
	FileNamesInformation,           // 12
	FileDispositionInformation,     // 13
	FilePositionInformation,        // 14
	FileFullEaInformation,          // 15
	FileModeInformation,            // 16
	FileAlignmentInformation,       // 17
	FileAllInformation,             // 18
	FileAllocationInformation,      // 19
	FileEndOfFileInformation,       // 20
	FileAlternateNameInformation,   // 21
	FileStreamInformation,          // 22
	FilePipeInformation,            // 23
	FilePipeLocalInformation,       // 24
	FilePipeRemoteInformation,      // 25
	FileMailslotQueryInformation,   // 26
	FileMailslotSetInformation,     // 27
	FileCompressionInformation,     // 28
	FileObjectIdInformation,        // 29
	FileCompletionInformation,      // 30
	FileMoveClusterInformation,     // 31
	FileQuotaInformation,           // 32
	FileReparsePointInformation,    // 33
	FileNetworkOpenInformation,     // 34
	FileAttributeTagInformation,    // 35
	FileTrackingInformation,        // 36
	FileIdBothDirectoryInformation, // 37
	FileIdFullDirectoryInformation, // 38
	FileValidDataLengthInformation, // 39
	FileShortNameInformation,       // 40
	FileIoCompletionNotificationInformation, // 41
	FileIoStatusBlockRangeInformation,       // 42
	FileIoPriorityHintInformation,           // 43
	FileSfioReserveInformation,              // 44
	FileSfioVolumeInformation,               // 45
	FileHardLinkInformation,                 // 46
	FileProcessIdsUsingFileInformation,      // 47
	FileNormalizedNameInformation,           // 48
	FileNetworkPhysicalNameInformation,      // 49
	FileIdGlobalTxDirectoryInformation,      // 50
	FileIsRemoteDeviceInformation,           // 51
	FileAttributeCacheInformation,           // 52
	FileNumaNodeInformation,                 // 53
	FileStandardLinkInformation,             // 54
	FileRemoteProtocolInformation,           // 55
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS; 

#endif //FILE_INFORMATION_CLASS_DEFINED

LRESULT WINAPI get_file_info_desc( FILE_INFORMATION_CLASS info_class, 
								  PVOID info, 
								  ULONG info_size, 
								  LPWSTR text, 
								  ULONG cc_buf_len, 
								  ULONG *cc_ret_len ); 

LRESULT WINAPI get_file_getinfo_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_reg_open_key_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_reg_mkkey_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_reg_rmkey_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_reg_mvkey_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_reg_enum_value_detail( sys_action_record *action, 
										 action_context *ctx, 
										 prepared_params_desc *params_desc, 
										 LPWSTR tip, 
										 ULONG ccb_buf_len, 
										 ULONG flags ); 

LRESULT WINAPI get_reg_enum_info_detail( sys_action_record *action, 
										action_context *ctx, 
										prepared_params_desc *params_desc, 
										LPWSTR tip, 
										ULONG ccb_buf_len, 
										ULONG flags ); 


LRESULT WINAPI get_reg_setinfo_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags ); 

LRESULT WINAPI get_reg_getinfo_detail( sys_action_record *action, 
									  action_context *ctx, 
									  prepared_params_desc *params_desc, 
									  LPWSTR tip, 
									  ULONG ccb_buf_len, 
									  ULONG flags ); 

LRESULT WINAPI get_reg_rmval_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_reg_getval_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_reg_setval_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_load_key_detail( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags ); 

LRESULT WINAPI get_rstr_key_detail( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags ); 

LRESULT WINAPI get_reg_set_sec_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_repl_key_detail( sys_action_record *action, 
							action_context *ctx, 
							prepared_params_desc *params_desc, 
							LPWSTR tip, 
							ULONG ccb_buf_len, 
							ULONG flags ); 

LRESULT WINAPI get_close_key_detail( sys_action_record *action, 
									action_context *ctx, 
									prepared_params_desc *params_desc, 
									LPWSTR tip, 
									ULONG ccb_buf_len, 
									ULONG flags ); 

LRESULT WINAPI get_proc_open_detail( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags ); 

LRESULT WINAPI get_proc_debug_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_proc_suspend_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_proc_resume_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_proc_kill_detail( sys_action_record *action, 
							 action_context *ctx, 
							 prepared_params_desc *params_desc, 
							 LPWSTR tip, 
							 ULONG ccb_buf_len, 
							 ULONG flags ); 

LRESULT WINAPI get_proc_job_detail( sys_action_record *action, 
						  action_context *ctx, 
						  prepared_params_desc *params_desc, 
						  LPWSTR tip, 
						  ULONG ccb_buf_len, 
						  ULONG flags ); 

LRESULT WINAPI get_proc_pg_prot_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_proc_free_vm_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_proc_write_vm_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_proc_read_vm_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_thread_remote_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_thread_set_ctxt_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_thread_suspend_detail( sys_action_record *action, 
								  action_context *ctx, 
								  prepared_params_desc *params_desc, 
								  LPWSTR tip, 
								  ULONG ccb_buf_len, 
								  ULONG flags ); 

LRESULT WINAPI get_thread_resume_detail( sys_action_record *action, 
								 action_context *ctx, 
								 prepared_params_desc *params_desc, 
								 LPWSTR tip, 
								 ULONG ccb_buf_len, 
								 ULONG flags ); 

LRESULT WINAPI get_thread_kill_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_thread_queue_apc_detail( sys_action_record *action, 
									action_context *ctx, 
									prepared_params_desc *params_desc, 
									LPWSTR tip, 
									ULONG ccb_buf_len, 
									ULONG flags ); 

LRESULT WINAPI get_port_read_detail( sys_action_record *action, 
									action_context *ctx, 
									PVOID data, 
									ULONG data_size, 
									prepared_params_desc *params_desc, 
									LPWSTR tip, 
									ULONG ccb_buf_len, 
									ULONG flags ); 

LRESULT WINAPI get_port_write_detail( sys_action_record *action, 
									 action_context *ctx, 
									 PVOID data, 
									 ULONG data_size, 
									 prepared_params_desc *params_desc, 
									 LPWSTR tip, 
									 ULONG ccb_buf_len, 
									 ULONG flags ); 

LRESULT WINAPI get_port_urb_detail( sys_action_record *action, 
								   action_context *ctx, 
								   PVOID data, 
								   ULONG data_size, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_sys_settime_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_sys_link_knowndll_detail( sys_action_record *action, 
									 action_context *ctx, 
									 prepared_params_desc *params_desc, 
									 LPWSTR tip, 
									 ULONG ccb_buf_len, 
									 ULONG flags ); 

LRESULT WINAPI get_sys_open_physmm_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_sys_read_physmm_detail( sys_action_record *action, 
								   action_context *ctx, 
								   prepared_params_desc *params_desc, 
								   LPWSTR tip, 
								   ULONG ccb_buf_len, 
								   ULONG flags ); 

LRESULT WINAPI get_sys_write_physmm_detail( sys_action_record *action, 
									action_context *ctx, 
									prepared_params_desc *params_desc, 
									LPWSTR tip, 
									ULONG ccb_buf_len, 
									ULONG flags ); 

LRESULT WINAPI get_sys_load_kmod_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *params_desc, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_sys_enumproc_detail( sys_action_record *action, 
								action_context *ctx, 
								prepared_params_desc *params_desc, 
								LPWSTR tip, 
								ULONG ccb_buf_len, 
								ULONG flags ); 

LRESULT WINAPI get_sys_regsrv_detail( sys_action_record *action, 
							  action_context *ctx, 
							  prepared_params_desc *params_desc, 
							  LPWSTR tip, 
							  ULONG ccb_buf_len, 
							  ULONG flags ); 

LRESULT WINAPI get_sys_opendev_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_w32_postmsg_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI get_sendmsg_detail( sys_action_record *action, 
						   action_context *ctx, 
						   prepared_params_desc *params_desc, 
						   LPWSTR tip, 
						   ULONG ccb_buf_len, 
						   ULONG flags ); 

LRESULT WINAPI get_w32_findwnd_detail( sys_action_record *action, 
							   action_context *ctx, 
							   prepared_params_desc *params_desc, 
							   LPWSTR tip, 
							   ULONG ccb_buf_len, 
							   ULONG flags ); 

LRESULT WINAPI _get_icmp_send_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI _get_icmp_recv_detail( sys_action_record *action, action_context *ctx, prepared_params_desc *param_descs, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_urb_info( LPWSTR text, 
							ULONG cc_buf_len, 
							PVOID data, 
							ULONG data_size, 
							ULONG *cc_ret_len ); 

LRESULT WINAPI get_action_tip( sys_action_record *action, action_context *ctx, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 

LRESULT WINAPI get_action_detail( sys_action_record *action, action_context *ctx, LPWSTR tip, ULONG ccb_buf_len, ULONG flags ); 
LRESULT WINAPI get_action_data( sys_action_record *action, LPWSTR data_dump, ULONG ccb_buf_len, ULONG flags ); 

INLINE LRESULT WINAPI get_action_detail_ex( sys_action_record *action, 
									 action_context *ctx, 
									 PVOID data, 
									 ULONG data_size, 
									 LPWSTR tip, 
									 ULONG ccb_buf_len, 
									 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCTSTR fmt = NULL; 

#define MAX_ACTION_DESC_SIZE ( ULONG )( 64 )

#define MAX_PARAMS_NUM ( ULONG )( 16 )
	prepared_params_desc prepared_params; 

	ASSERT( action != NULL ); 
	ASSERT( tip != NULL ); 
	ASSERT( ccb_buf_len > 0 ); 

	do 
	{
		if( action == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ctx == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( tip == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ccb_buf_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*tip = L'\0'; 
		tip[ ccb_buf_len - 1 ] = L'\0'; 

		switch( action->type )
		{
		case PORT_read:
			ret = get_port_read_detail( action, 
				ctx, 
				data, 
				data_size, 
				&prepared_params, 
				tip, 
				ccb_buf_len, 
				flags ); 

			if( tip[ ccb_buf_len - 1 ] != L'\0' )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				ASSERT( FALSE ); 
				tip[ ccb_buf_len - 1 ] = L'\0'; 
			}

			break; 

		case PORT_write:
			ret = get_port_write_detail( action, 
				ctx, 
				data, 
				data_size, 
				&prepared_params, 
				tip, 
				ccb_buf_len, 
				flags ); 

			if( tip[ ccb_buf_len - 1 ] != L'\0' )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				ASSERT( FALSE ); 
				tip[ ccb_buf_len - 1 ] = L'\0'; 
			}

			break; 
		case PORT_urb:
			ret = get_port_urb_detail( action, 
				ctx, 
				data, 
				data_size, 
				&prepared_params, 
				tip, 
				ccb_buf_len, 
				flags ); 

			if( tip[ ccb_buf_len - 1 ] != L'\0' )
			{
				ret = ERROR_BUFFER_OVERFLOW; 
				ASSERT( FALSE ); 
				tip[ ccb_buf_len - 1 ] = L'\0'; 
			}

			break; 

		default:
			{
				ret = get_action_detail( action, ctx, tip, ccb_buf_len, flags ); 

				break; 
			}
		}
	}while( FALSE ); 

	return ret; 
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__ACTION_DISPLAY_H__