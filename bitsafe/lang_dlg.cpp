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
#include "resource.h"
#include "ui_config.h"
#include "bitsafe_common.h"
#include "ui_ctrl.h"
#include "lang_dlg.h"
#include "tinyxml.h"
#include "string_mng.h"
#include "memory_mng.h"

CRITICAL_SECTION lang_setting_lock; 
#define BITSAFE_LANG_CONF_FILE _T( "data7.dat" )

LRESULT get_lang_conf_file_name( WCHAR *conf_file_path, ULONG name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 

	ASSERT( conf_file_path != NULL ); 
	ASSERT( name_len > 0 ); 

	ret = get_app_path( conf_file_path, name_len, &cc_ret_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	wcsncat( conf_file_path, L"\\", name_len - wcslen( conf_file_path ) );


	wcsncat( conf_file_path, BITSAFE_LANG_CONF_FILE, name_len - wcslen( conf_file_path ) );

	if( conf_file_path[ name_len - 1 ] != L'\0' )
	{
		conf_file_path[ name_len - 1 ] = L'\0'; 
	}

	log_trace( ( MSG_INFO, "user configure file name is %ws\n", conf_file_path ) ); 

_return:
	return ret; 
}

#define LANG_CONF "language"
#define LANG_ID_SETTING "language_id"

LRESULT test_xml_doc()
{
	std::string test; 

	TiXmlDocument xml_doc; 
	return ERROR_SUCCESS; 
}

LRESULT load_lang_conf()
{
	LRESULT ret = ERROR_SUCCESS; 

	TiXmlDocument xml_doc; 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *email_conf = NULL; 
	CHAR *tmp_str = NULL; 
	TCHAR erc_file_name[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	LPCSTR attr_val; 
	bool _ret; 
	ULONG lang_id_setting = LANG_ID_CN; 
	LPCTSTR tmp_text; 

	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_file_exist( erc_file_name ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
		goto _return; 
	}

	file_name = alloc_tchar_to_char( erc_file_name ); 
	if( file_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	_ret = xml_doc.LoadFile( file_name ); 
	if( _ret == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	root = xml_doc.RootElement(); 
	if( root == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	node = root->FirstChild( LANG_CONF ); 
	if( node == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	email_conf = node->ToElement(); 


	attr_val = email_conf->Attribute( LANG_ID_SETTING ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	lang_id_setting = strtol( attr_val, &tmp_str, 10 ); 
	
	if( FALSE == is_valid_lang_id( lang_id_setting ) )
	{
		ASSERT( FALSE ); 

		tmp_text = _get_string_by_id( TEXT_LANG_LOAD_LANG_CONFIG_ERROR, 
			_T( "������������ʧ��" ) ); 

		show_msg( NULL, tmp_text ); 
		lang_id_setting = LANG_ID_CN; 
	}


#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

_return:

	if( file_name != NULL )
	{
		mem_free( ( VOID** )&file_name ); 
	}

	set_cur_lang( ( LANG_ID )lang_id_setting ); 

	return ret; 
}

LRESULT save_lang_conf()
{
	TiXmlDocument xml_doc; 
	TiXmlDeclaration Declaration( "1.0","gb2312", "yes" ); 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *email_conf = NULL; 
	CHAR* tmp_str = NULL; 

	LRESULT ret = ERROR_SUCCESS; 
	TCHAR erc_file_name[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	if( FALSE == is_valid_lang_id( get_cur_lang_id() ) ) 
	{
		ASSERT( FALSE && "how can got the invalid language id here" ); 
		goto _return; 
	}

	xml_doc.InsertEndChild( Declaration ); 

	root = new TiXmlElement( LANG_CONF ); 
	if( root == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = xml_doc.InsertEndChild( *root ); 

	ASSERT( node != NULL ); 

	log_trace( ( MSG_INFO, "root is 0x%0.8x, root of node is 0x%0.8x\n", root, node->ToElement() ) ); 

	delete root; 

	root = node->ToElement(); 

	email_conf = new TiXmlElement( LANG_CONF ); 

	if( email_conf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = root->InsertEndChild( *email_conf ); 
	if( node == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; ; 
	}

	delete email_conf; 

	email_conf = node->ToElement(); 

	email_conf->SetAttribute( LANG_ID_SETTING, ( INT32 )get_cur_lang_id() ); 

	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	file_name = alloc_tchar_to_char( erc_file_name ); 
	if( file_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

	xml_doc.SaveFile( file_name ); 

_return:

	if( file_name != NULL )
	{
		mem_free( ( void** )&file_name ); 
	}

	return ret; 
}


LRESULT chg_original_lang( action_ui_notify *ui_notify, BOOLEAN *conf_file_exist )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( conf_file_exist != NULL )
	{
		*conf_file_exist = TRUE; 
	}

	ret = load_lang_conf(); 

	if( ret != ERROR_SUCCESS )
	{
		chg_lang_dlg dlg( ui_notify );
		dlg_ret_state ret_state; 
		LRESULT _ret; 
		LPCTSTR tmp_text; 

		if( ret != ERROR_FILE_NOT_FOUND )
		{
			tmp_text = _T( "ϵͳ�ؼ����ݱ��ƻ�,����������Ϣ��Ҫ��������!\n\n" ) \
						_T( "Language configuration file is corrupted, need setting again." ); 

			show_msg( NULL, tmp_text, NULL, _T( "" ), 0 ); 
		}
		else
		{
			if( conf_file_exist != NULL )
			{
				*conf_file_exist = FALSE; 
			}
		}

		tmp_text = _T( "���ڻ�û����������,����������,�ſ�����ȷ��ʾ����.\n\n" ) \
			__t( "Have not set language now, please set it now, then ui can display correctly" ); 

		_ret = show_msg( NULL, tmp_text, &ret_state, 0 ); 
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "create message box failed to indicate null password failed\n" ) ); 
		}

		if( ret_state == OK_STATE )
		{
			tmp_text = _get_string_by_id( TEXT_LANG_TITLE, 
				_T("������������") ); 
			dlg.Create( NULL, tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
			dlg.SetIcon( IDI_MAIN_ICON ); 
			dlg.CenterWindow();
			dlg.ShowModal(); 

			save_lang_conf(); 
		}

		goto _return; 
	}

_return:
	return ret; 
}

LRESULT del_lang_conf()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	TCHAR erc_file_name[ MAX_PATH ]; 
	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "get language configuration file name error" ); 
		goto _return; 
	}
	_ret = DeleteFile( erc_file_name ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the configuration file error\n" ) ); 
	}

_return:
	return ret; 
}

LRESULT del_option_conf()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	TCHAR erc_file_name[ MAX_PATH ]; 

	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE && "get language configuration file name error" ); 
		goto _return; 
	}

	_ret = DeleteFile( erc_file_name ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the configuration file error\n" ) ); 
	}

_return:
	return ret; 
}

