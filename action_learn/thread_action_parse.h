#ifndef __THREAD_ACTION_PARAM_H__
#define __THREAD_ACTION_PARAM_H__

#include "action_type_parse.h"

NTSTATUS parse_thread_remote_param( thrd_remote *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_set_context_param( thrd_setctxt *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_suspend_param( thrd_suspend *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_kill_param( thrd_kill *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_thread_queue_apc_param( thrd_queue_apc *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_open_phys_mm_param( sys_open_physmm *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_read_phys_mm_param( sys_read_physmm *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_load_kmod_param( sys_load_kmod *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_reg_srv_param( sys_regsrv *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_sys_open_dev_param( sys_opendev *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_w32_msg_hook( w32_msghook *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_w32_lib_inject_hook( w32_lib_inject *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_net_connect( net_connect *action, param_info *params, ULONG max_count, ULONG *count_out ); 

#define parse_net_create parse_net_connect
#define parse_net_listen parse_net_connect

NTSTATUS parse_net_send( net_send *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_net_http( net_http *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_extract_hidden( ba_extract_hidden *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_extract_pe( ba_extract_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_self_copy( ba_self_copy *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_self_delete( ba_self_delete *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_invade_process( ba_invade_process *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_infect_pe( ba_infect_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_overwrite_pe( ba_overwrite_pe *action, param_info *params, ULONG max_count, ULONG *count_out ); 

NTSTATUS parse_ba_register_autorun( ba_register_autorun *action, param_info *params, ULONG max_count, ULONG *count_out ); 

#endif //__THREAD_ACTION_PARAM_H__