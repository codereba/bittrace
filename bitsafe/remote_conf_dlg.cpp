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
		_T( "远程控制设置" ) ); 

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
			_T( "设置远程控制邮箱成功" ) ); 
		show_msg( NULL, tmp_text ); 
	}
	else
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:
	return ret; 
}