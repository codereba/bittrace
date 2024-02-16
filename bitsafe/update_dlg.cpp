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