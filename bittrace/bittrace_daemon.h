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
ULONG CALLBACK init_ui_context_thread( PVOID param ); 
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
