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
 
 #ifndef __BITSAFE_CONF_H__
#define __BITSAFE_CONF_H__

typedef std::vector< action_rule_buffered* > access_rule_queue; 
extern access_rule_queue all_rules[ MAX_ACTION_RULE_TYPE ]; 
extern safe_region_list all_rules_buf[ MAX_ACTION_RULE_TYPE ]; 
//#define STL_RULE_BUFFER 

#define ADD_RULE_DEFINE 0
#define DEL_RULE_DEFINE 1
#define MODIFY_RULE_DEFINE 2

#define OPERA_ADD_RULE 0
#define OPERA_DEL_RULE 1
#define OPERA_DEL_ALL 2

LRESULT notify_rule_modify( action_rule_buffered *rule_buf, ULONG rule_cmd ); 

LRESULT init_rules_ipc_buf( BOOLEAN is_service ); 
LRESULT load_rules_from_mem( read_list_entry read_func ); 
LRESULT uninit_rules_ipc_buf(); 
LRESULT release_all_rules(); 
LRESULT input_rule_to_buffer( access_rule_type type, 
							 action_rule_record *record, 
							 ULONG flags, 
							 access_rule_desc *loaded_rule ); 
LRESULT input_default_rules( access_rule_type type, INT32 is_service ); 

#ifdef USE_SECOND_ACL
LRESULT config_url_rule( access_rule_desc *rule_input, INT32 mode ); 
LRESULT _config_url_rule( access_rule_ptr_desc *rule_input ); 
#endif //USE_SECOND_ACL

#define APP_ACTION_RULE_OP 0x00000001
#define APP_DEFINE_RULE_OP 0x00000002

LRESULT config_app_rule( access_rule_ptr_desc *rule_input, INT32 is_add, ULONG flags ); 
LRESULT config_rule( access_rule_desc *action_rule, access_rule_desc* org_action_rule, ULONG mode, ULONG flags ); 
LRESULT get_rule_column_text( LPTSTR text_out, ULONG length, access_rule_desc *rule_input, LPWSTR desc, ULONG index ); 

#if 0
LRESULT CALLBACK load_url_rule_config( access_rule_type type, action_rule_record *record, PVOID param ); 
//LRESULT CALLBACK load_url_rule_config_to_buffer( access_rule_type type, action_rule_record *record, PVOID param ); 
#endif //0

LRESULT CALLBACK load_rule_config( access_rule_type type, action_rule_record *record, PVOID param ); 

//LRESULT CALLBACK load_rule_to_buffer( access_rule_type type, action_rule_record *record, PVOID param ); 

LRESULT print_param_define( LPTSTR output, ULONG buf_len, param_all_desc *param_input ); 
LRESULT print_rule_param( LPTSTR output, ULONG buf_len, access_rule_desc *rule_input, ULONG param_index ); 
LRESULT CALLBACK load_fw_rule( access_rule_type type, action_rule_record *record, PVOID param );  
LRESULT CALLBACK alloc_fw_rule( PVOID *rule, PVOID param ); 
LRESULT add_rule_action( HWND wnd, access_rule_type cur_rule_type ); 
LRESULT modify_rule_action( HWND wnd, ULONG sel_index, access_rule_desc *action_rule, access_rule_type cur_rule_type ); 


#define DELETED_FROM_DB 0x00000001
#define DELETED_FROM_ACL 0x00000002

NTSTATUS delete_rule_action( INT32 sel_index, access_rule_type cur_rule_type, ULONG *del_state ); 

LRESULT init_url_rules_buf( ); 
LRESULT release_url_rules_buf( ); 

LRESULT add_url_rule_to_buf( LPCWSTR input ); 
LRESULT commit_url_rules( DWORD opera_mode ); 
LRESULT config_fw_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record ); 
LRESULT config_defense_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record ); 
LRESULT add_app_rule( action_response_type resp, LPCWSTR app_name, INT32 need_record ); 
LRESULT load_rules_from_mem( read_list_entry read_func ); 
LRESULT manage_rule( access_rule_type type, ULONG_PTR offset, BOOLEAN is_del ); 
LRESULT load_rule_buf( OFFSET_LIST_ENTRY *entry ); 
LRESULT free_rule_buf( action_rule_buffered *rule_buf ); 

#ifdef COMPATIBLE_OLD_ACTION_DEFINE
INLINE LRESULT init_driver_manage_rule( LPWSTR file_name, action_response_type resp, access_rule_desc *rule )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( rule != NULL ); 
	ASSERT( file_name != NULL ); 

	init_access_rule( COMMON_RULE_TYPE, rule ); 

	rule->resp = resp; 
	rule->type = COMMON_RULE_TYPE; 

	rule->desc.common.action_type = INSTALL_DRV; 
	*rule->desc.common.app.app.app_name = L'\0'; 

	if( wcslen( file_name ) >= _MAX_FILE_NAME_LEN )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	wcscpy( rule->desc.common.param0.common.name, file_name ); 

_return:
	return ret; 
}
#endif //COMPATIBLE_OLD_ACTION_DEFINE

#endif //__BITSAFE_CONF_H__