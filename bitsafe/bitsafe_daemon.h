#ifndef __BITSAFE_DEAMON_H__
#define __BITSAFE_DEAMON_H__

#pragma pack( push )
#pragma pack( 1 )
typedef struct _pipe_cmd
{
	ULONG cmd; 
	BYTE param[ 1 ]; 
} pipe_cmd, *ppipe_cmd; 

typedef struct _ctrl_rule_cmd
{
	ULONG cmd; 
	ULONG rule_type; 
	ULONG_PTR offset; 
} ctrl_rule_cmd, *pctrl_rule_cmd; 

typedef struct _ctrl_rule_resp
{
	ULONG cmd; 
	ULONG_PTR param; 
} ctrl_rule_resp, *pctrl_rule_resp; 

typedef struct _pipe_response
{
	ULONG cmd; 
	BYTE response[ 1 ]; 
} pipe_response, *ppipe_response; 

typedef enum _bitsafe_pipe_cmd
{
	CMD_ADD_RULE, 
	CMD_DEL_RULE, 
	CMD_RULE_LOAD_STATUS
} bitsafe_pipe_cmd, *pbitsafe_pipe_cmd; 
#pragma pack( pop )

#define is_valid_rule_cmd( cmd ) ( cmd >= CMD_ADD_RULE && cmd <= CMD_DEL_RULE ) 

#define BITSAFE_SERVICE_PIPE L"\\\\.\\Pipe\\RulesPipe"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

LRESULT pipe_daemon(); 
LRESULT init_service_context_async(); 
LRESULT init_ui_context_async( PVOID param ); 
LRESULT uninit_ui_context(); 
LRESULT uninit_service_context(); 
LRESULT unistall_bitsafe(); 
LRESULT safe_open_bitsafe_conf(); 
LRESULT safe_output_themes(); 
ULONG CALLBACK thread_pipe_daemon( PVOID param ); 
LRESULT config_bitsafe(); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__BITSAFE_DEAMON_H__
