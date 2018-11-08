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