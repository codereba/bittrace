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
 
#include "StdAfx.h"
#include "ui_config.h"
#include "remote_conf_dlg.h"

LRESULT set_erc_config_now()
{
	LRESULT ret = ERROR_SUCCESS; 
	email_box mail_box; 
	LPCTSTR tmp_text; 

	ret = get_mail_box( &mail_box ); 
	if( ret != ERROR_SUCCESS )
	{
		memset( &mail_box, 0, sizeof( mail_box ) ); 
	}

	email_setting_dlg dlg( &mail_box ); 

	//dlg.set_output_param( &email_box_info ); 

	tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_TITLE, 
		_T( "Զ�̿�������" ) ); 

	dlg.Create( NULL, tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
	dlg.SetIcon( IDI_MAIN_ICON ); 
	dlg.CenterWindow();
	dlg.ShowModal(); 

	if( dlg.setting_success() == TRUE )
	{
		ret = set_mail_box( &mail_box ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			goto _return; 
		}
	
		email_box_info.erc_on = TRUE; 

		save_erc_conf(); 

		tmp_text = _get_string_by_id( TEXT_ERC_ACCOUNT_SETTING_SUCCESSFULLY, 
			_T( "����Զ�̿�������ɹ�" ) ); 
		show_msg( NULL, tmp_text ); 
	}
	else
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:
	return ret; 
}