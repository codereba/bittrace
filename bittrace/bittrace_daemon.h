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
