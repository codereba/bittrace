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
 
 #include "stdafx.h"
#include "common_func.h"
#include "bitsafe_common.h"
#include "update.h"
#include "update_dlg.h"
#include "UIUtil.h"

LRESULT CALLBACK updating_status_notify( updating_info *info, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	update_dlg *dlg; 
	LPWSTR _update_url = NULL; 
	LPWSTR _file_name = NULL; 
	update_tip_info *tip_info; 
	file_download_tip *download_tip; 

	ASSERT( info != NULL ); 
	ASSERT( param != NULL ); 

	dlg = ( update_dlg* )param; 

	switch( info->update_status )
	{
	case UPDATE_PREPARE: 
		tip_info = ( update_tip_info* )info->context; 
		ASSERT( tip_info->url != NULL ); 
		if( *tip_info->url == '\0' )
		{
			log_trace( ( MSG_ERROR, "the update url is invalid\n" ) ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_update_url = StringConvertor::AnsiToWide( tip_info->url ); 
		if( _update_url == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		dlg->on_check_update( _update_url ); 
		break; 
	case UPDATE_FILE_DOWNLOAD_TIP: 
		
		tip_info = ( update_tip_info* )info->context; 

		ASSERT( tip_info->context != NULL ); 

		download_tip = ( file_download_tip* )tip_info->context; 
		
		if( *download_tip->file_name== '\0' )
		{
			log_trace( ( MSG_ERROR, "the update file is invalid\n" ) ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_file_name = StringConvertor::AnsiToWide( ( LPCSTR )download_tip->file_name ); 
		if( _file_name == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		dlg->on_file_download_tip( _file_name, download_tip->file_size, download_tip->file_downloaded ); 
		break; 
	case UPDATE_NO_NEED: 
		tip_info = ( update_tip_info* )info->context;  

		ret = dlg->on_no_need_update( tip_info );  
		break; 
	case UPDATE_RUNNING:
		tip_info = ( update_tip_info* )info->context;  

		ret = dlg->on_update_starting( tip_info );  
		break; 
	case UPDATE_FILE_STARGING: 
		//tip_info = ( update_tip_info* )info->context; 
		break; 
	case UPDATE_FILE_END:
		tip_info = ( update_tip_info* )info->context; 

		if( tip_info->context == NULL 
			|| *( CHAR* )tip_info->context == '\0' )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_file_name = StringConvertor::AnsiToWide( ( LPCSTR )tip_info->context ); 
		if( _file_name == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		ret = dlg->on_file_updated( _file_name ); 
		break; 
	case UPDATE_FAILED:
		tip_info = ( update_tip_info* )info->context; 
		dlg->on_update_failed( tip_info ); 
		break; 
	case UPDATE_SUCCESSFUL:
		//tip_info = ( update_tip_info* )info->context; 
		dlg->on_update_successful(); 
		break; 
	default: 
		ASSERT( FALSE && "invalid updating status\n"); 
		break; 
	}

_return:
	if( _update_url != NULL )
	{
		StringConvertor::StringFree( _update_url ); 
	}
	if( _file_name != NULL )
	{
		StringConvertor::StringFree( _file_name ); 
	}

	return ret; 
}