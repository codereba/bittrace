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
 
#ifndef __ERC_SETTING_H__
#define __ERC_SETTING_H__

extern email_box email_box_info; 
#define REMOTE_CONTROL_BLOCK 0x0000001
#define REMOTE_CONTROL_ALLOW 0x0000002
#define REMOTE_CONTROL_ACL 0x0000003

#define DEFAULT_POP_PORT 110

ULONG CALLBACK thread_exec_remote_control( PVOID context ); 

LRESULT check_email_conf_valid( email_box *mail_box ); 
LRESULT _check_email_conf_valid( email_box *mail_box ); 
LRESULT stop_pop3_remote_control();
LRESULT pop3_exec_remote_control( PVOID context ); 
LRESULT smtp_remote_control( ULONG mode, ULONG hour, ULONG minite, PVOID context );  
LRESULT smtp_remote_control_async( ULONG mode, ULONG hour, ULONG minite, PVOID context, PVOID param ); 
LRESULT load_erc_conf(); 
LRESULT save_erc_conf(); 
LRESULT test_erc_conf(email_box *mail_box ); 
LRESULT init_erc_env( PVOID context, INT32 have_ui ); 
VOID uninit_erc_env(); 
LRESULT set_mail_box( email_box *mail_box ); 
LRESULT get_mail_box( email_box *mail_box ); 

INLINE VOID init_test_email_box( email_box *mail_box )
{
	ASSERT( mail_box != NULL ); 

	mail_box->erc_on = FALSE; 
	_tcsncpy( mail_box->email, _T( "balancesoft@sohu.com" ), MAX_EMAIL_NAME_LEN - 1 ); 
	_tcsncpy( mail_box->name, _T( "balancesoft" ), MAX_EMAIL_NAME_LEN - 1 ); 

	_tcsncpy( mail_box->pwd, _T( "hejingjing" ), MAX_EMAIL_NAME_LEN - 1 ); 
	_tcsncpy( mail_box->server, _T( "smtp.sohu.com" ), MAX_EMAIL_NAME_LEN - 1 ); 
	_tcsncpy( mail_box->pop3_server, _T( "pop.sohu.com" ), MAX_EMAIL_NAME_LEN - 1 ); 

	mail_box->need_auth = TRUE; 
	mail_box->port = 25; 

	mail_box->pop3_port = DEFAULT_POP_PORT; 
	mail_box->smtp_auth_type = 0; 
	
	memset( &mail_box->pop_ssl_info, 0, sizeof( mail_box->pop_ssl_info ) ); 
	mail_box->pop_use_ssl = FALSE; 

	memset( &mail_box->ssl_info, 0, sizeof( mail_box->ssl_info ) ); 
	mail_box->use_ssl = FALSE; 
}

#endif //__ERC_SETTING_H__