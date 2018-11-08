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

#include "stdafx.h"
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif

#include "common_config.h"
#include "analyze_config.h"
#include "bitsafe_common.h"

const TCHAR* const kAdjustColorControlName = _T("adjcolor");
const TCHAR* const kAdjustBkControlName = _T("adjbk");

const TCHAR* const kAdjustColorSliderRControlName = _T("AdjustColorSliderR");
const TCHAR* const kAdjustColorSliderGControlName = _T("AdjustColorSliderG");
const TCHAR* const kAdjustColorSliderBControlName = _T("AdjustColorSliderB");

const TCHAR* const kHColorLayoutControlName = _T("HColorLayout");
const TCHAR* const kHBkLayoutControlName = _T("HBkLayout");

const TCHAR* const kTabControlName = _T("tabs");

LRESULT set_ui_bk_color( ULONG clr )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

LRESULT set_font()
{
	return ERROR_SUCCESS; 
}

ColorSkinWindow::ColorSkinWindow()
{
	Create(NULL, _T("colorskin"), WS_POPUP, WS_EX_TOOLWINDOW, 0, 0);
	ShowWindow(true);
}

LPCTSTR ColorSkinWindow::GetWindowClassName() const 
{ 
	return _T("ColorSkinWindow");
}

void ColorSkinWindow::OnFinalMessage(HWND hWnd)
{
	delete this;
}

void ColorSkinWindow::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("click")) == 0)
	{
		CTabLayoutUI* pTabControl = static_cast<CTabLayoutUI*>(paint_manager.FindControl(kTabControlName));
		if (pTabControl != NULL)
		{
			if (pTabControl->GetCurSel() == 0)
			{
				if (_tcsstr(msg.pSender->GetName(), _T("colour_")) != 0)
				{
					CSliderUI* AdjustColorSliderR = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderRControlName));
					CSliderUI* AdjustColorSliderG = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderGControlName));
					CSliderUI* AdjustColorSliderB = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderBControlName));
					if ((AdjustColorSliderR != NULL) && (AdjustColorSliderG != NULL) && (AdjustColorSliderB != NULL))
					{
						DWORD dwColor = msg.pSender->GetBkColor();
						AdjustColorSliderR->SetValue(static_cast<BYTE>(GetRValue(dwColor)));
						AdjustColorSliderG->SetValue(static_cast<BYTE>(GetGValue(dwColor)));
						AdjustColorSliderB->SetValue(static_cast<BYTE>(GetBValue(dwColor)));

						set_ui_bk_color( dwColor ); 
					}
				}
			}
			else if (pTabControl->GetCurSel() == 1)
			{}
		}
	}
	else if (_tcsicmp(msg.sType, _T("valuechanged")) == 0)
	{
		CTabLayoutUI* pTabControl = static_cast<CTabLayoutUI*>(paint_manager.FindControl(kTabControlName));
		if (pTabControl != NULL)
		{
			if (pTabControl->GetCurSel() == 0)
			{
				CSliderUI* AdjustColorSliderR = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderRControlName));
				CSliderUI* AdjustColorSliderG = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderGControlName));
				CSliderUI* AdjustColorSliderB = static_cast<CSliderUI*>(paint_manager.FindControl(kAdjustColorSliderBControlName));
				if ((AdjustColorSliderR != NULL) && (AdjustColorSliderG != NULL) && (AdjustColorSliderB != NULL))
				{
					if ((_tcsicmp(msg.pSender->GetName(), kAdjustColorSliderRControlName) == 0) ||
						(_tcsicmp(msg.pSender->GetName(), kAdjustColorSliderGControlName) == 0) ||
						(_tcsicmp(msg.pSender->GetName(), kAdjustColorSliderBControlName) == 0))
					{
						BYTE red = AdjustColorSliderR->GetValue();
						BYTE green = AdjustColorSliderG->GetValue();
						BYTE blue = AdjustColorSliderB->GetValue();
						COLORREF crColor = RGB(red, green, blue);
						TCHAR szBuf[MAX_PATH] = {0};
#if defined(UNDER_CE)
						_stprintf(szBuf, _T("FF%02X%02X%02X"), GetRValue(crColor), GetGValue(crColor), GetBValue(crColor));
#else
						_stprintf_s(szBuf, MAX_PATH - 1, _T("FF%02X%02X%02X"), GetRValue(crColor), GetGValue(crColor), GetBValue(crColor));
#endif
						LPTSTR pstr = NULL;
						DWORD dwColor = _tcstoul(szBuf, &pstr, 16);
						set_ui_bk_color(dwColor);
					}
				}
			}
			else if (pTabControl->GetCurSel() == 1)
			{}
		}
	}
	else if (_tcsicmp(msg.sType, _T("selectchanged")) == 0)
	{
		CTabLayoutUI* pTabControl = static_cast<CTabLayoutUI*>(paint_manager.FindControl(kTabControlName));
		if (_tcsicmp(msg.pSender->GetName(), kAdjustColorControlName) == 0)
		{
			if (pTabControl && pTabControl->GetCurSel() != 0)
			{
				pTabControl->SelectItem(0);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), kAdjustBkControlName) == 0)
		{
			if (pTabControl && pTabControl->GetCurSel() != 1)
			{
				pTabControl->SelectItem(1);
			}
		}
	}
}

void ColorSkinWindow::Init()
{
	SIZE size = paint_manager.GetInitSize();
	MoveWindow(m_hWnd, parent_window_rect_.right - size.cx, parent_window_rect_.top, size.cx, size.cy, FALSE);
}

CStdString ColorSkinWindow::GetSkinFile()
{
	return _T("skin_clr.xml");
}

CStdString ColorSkinWindow::GetSkinFolder()
{
	return CStdString(CPaintManagerUI::GetInstancePath()) + _T("skin\\");
}

LRESULT ColorSkinWindow::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Close();
	return 0;
}

LRESULT get_common_conf_file_name( WCHAR *conf_file_path, ULONG name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	BOOL _ret; 
	ULONG cc_ret_len; 

	ASSERT( conf_file_path != NULL ); 
	ASSERT( name_len > 0 ); 

	//_ret = GetSystemDirectory( conf_file_path, name_len ); 
	//if( _ret == FALSE )
	//{
	//	ret = GetLastError(); 
	//	goto _return; 
	//}

	ret = get_app_path( conf_file_path, name_len, &cc_ret_len ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	hr = StringCchCatW( conf_file_path, name_len, L"\\" );

	if( FAILED( hr ) ) 
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	hr = StringCchCatW( conf_file_path, name_len, BITSAFE_UI_CONF_FILE );

	if( FAILED( hr ) ) 
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	//if( conf_file_path[ name_len - 1 ] != L'\0' )
	//{
	//	conf_file_path[ name_len - 1 ] = L'\0'; 
	//}

	log_trace( ( MSG_INFO, "user configure file name is %ws\n", conf_file_path ) ); 

_return:
	return ret; 
}

#define UI_CONF "skin"
#define UI_TRANSPARENT_SETTING "transparent"
#define UI_THEME_CONF "theme"
#define UI_THEME_NO_SETTING "no"
#define UI_THEME_NAME_SETTING "name"
#define UI_SLOGAN "slogan"
#define UI_SLOGAN_SHOW_SETTING "show"

#include "tinyxml.h"
#include "string_mng.h"
#include "memory_mng.h"

typedef struct _init_themes_param
{
	CPaintManagerUI *pm; 
	ULONG index; 
} init_themes_param, *pinit_themes_param; 

LRESULT get_theme_define( LPCTSTR theme_path, theme_define *theme_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	CStdString conf_file; 

	ASSERT( theme_path != NULL ); 
	ASSERT( theme_out != NULL ); 

	log_trace( ( MSG_INFO, "enter %s\n", __FUNCTION__ ) ); 
	log_trace( ( MSG_INFO, "theme path is %ws\n", theme_path ) ); 

	_tcsncpy( theme_out->bk_file, theme_path, MAX_PATH ); 
	if( theme_out->bk_file[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_tcsncat( theme_out->bk_file, _T( "\\bk.jpg" ), MAX_PATH - _tcslen( theme_out->bk_file ) ); 

	if( theme_out->bk_file[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_tcsncpy( theme_out->thumb_file, theme_path, MAX_PATH ); 
	if( theme_out->thumb_file[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_tcsncat( theme_out->thumb_file, _T( "\\thumb.jpg" ), MAX_PATH - _tcslen( theme_out->thumb_file ) ); 

	if( theme_out->thumb_file[ MAX_PATH - 1 ] != _T( '\0' ) )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}


	conf_file = theme_path; 
	conf_file.Append( _T( "\\config.xml" ) ); 

	ret = load_theme_conf( conf_file.GetData(), &theme_out->setting ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

_return:
	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

LRESULT get_theme_define_from_path( LPCTSTR theme_path, theme_define *theme_found )
{
	LRESULT ret = ERROR_SUCCESS; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	memset( theme_found, 0, sizeof( *theme_found ) ); 

	*theme_found->setting.theme_desc = _T( '\0' ); 

	ret = get_theme_define( theme_path, theme_found ); 
	if( ret != ERROR_SUCCESS )
	{
		log_trace( ( MSG_ERROR, "load the theme path %ws error 0x%0.8x\n", theme_path, ret ) ); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT CALLBACK on_themes_init_load( LPCTSTR root_path, WIN32_FIND_DATA *find_data, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	init_themes_param *param; 
	CStdString theme_path; 
	theme_define theme_set; 

	param = ( init_themes_param* )context; 

	ASSERT( find_data != NULL ); 

	if( 0 != ( find_data->dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY ) )
	{
		if( 0 != _tcscmp( find_data->cFileName, _T( ".." ) ) 
			&& 0 != _tcscmp( find_data->cFileName, _T( "." ) ) )
		{
			do 
			{
				if( param->index == get_theme_no() )
				{
					theme_path = ( LPCTSTR )root_path; 
					theme_path.Append( _T( "\\" ) ); 
					theme_path.Append( ( LPCTSTR )find_data->cFileName ); 

					ret = get_theme_define_from_path( theme_path.GetData(), &theme_set ); 
					if( ret != ERROR_SUCCESS )
					{
						//will never enter this branch to load ui configuration again
						break; 
					}

					ASSERT( param->pm != NULL ); 

					ret = _change_skin_theme( param->pm, &theme_set ); 
				}

			}while( FALSE );

			param->index ++; 
		}
	}
	//else
	//{
	//	ret = ERROR_INVALID_PARAMETER; 
	//}

_return:
	return ret; 
}

LRESULT load_theme_setting( CPaintManagerUI *main_wnd_pm )
{
	LRESULT ret = ERROR_SUCCESS; 
	init_themes_param param; 

	param.pm = main_wnd_pm; 
	param.index = 0; 

	ret = load_all_themes( on_themes_init_load, ( PVOID )&param ); 

	return ret; 
}

LRESULT load_all_themes( file_found_callback on_file_found, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	CStdString theme_path; 

	ret = get_themes_path( theme_path ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = find_whole_path( theme_path.GetData(), on_file_found, context ); 

_return:
	return ret; 
}

#define ANALYZE_HELP_CONF "help"
#define ANALYZE_SYMBOL_PATH_CONF "symbol"
#define ANALYZE_DBG_HELP_PATH_CONF "dbg_help"
#define ANALYZE_HELP_CONF_FLAGS "flags"

LRESULT load_common_conf( on_ui_conf_loaded ui_loaded_callback, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	TiXmlDocument xml_doc; 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *common_conf = NULL; 
	CHAR *tmp_str = NULL; 
	TCHAR common_conf_file[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	LPCSTR attr_val; 
	bool _ret; 
	LRESULT __ret; 
	ULONG transparent_val; 
	ULONG theme_no; 
	INT32 show_slogan = FALSE; 
	WCHAR *dbg_help_path = NULL; 
	WCHAR *symbol_path = NULL; 
	ULONG help_flags; 
	WCHAR *_dbg_help_path = NULL; 
	WCHAR *_symbol_path = NULL; 

	//analyze_setting analyze_help; 
	
	//TCHAR *theme_name = NULL; 
	//LPCTSTR tmp_text; 

	//lock_cs( lang_setting_lock ); 
	ret = get_common_conf_file_name( common_conf_file, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = check_file_exist( common_conf_file ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
		goto _return; 
	}

	file_name = alloc_tchar_to_char( common_conf_file ); 
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

	do 
	{
#define DEF_UI_TRANSPARENT_VALUE ( ULONG )( 230 )
		transparent_val = DEF_UI_TRANSPARENT_VALUE; 
	node = root->FirstChild( UI_CONF ); 
	if( node == NULL )
	{
		break; 
		//ret = ERROR_ERRORS_ENCOUNTERED; 
		//goto _return; 
	}

	common_conf = node->ToElement(); 

	//tmp_str = ui_conf_file; 

	attr_val = common_conf->Attribute( UI_TRANSPARENT_SETTING ); 
	if( attr_val == NULL )
	{
		break; 
		//ret = ERROR_ERRORS_ENCOUNTERED; 
		//goto _return; 
	}
	}while( FALSE ); 

	transparent_val = strtol( attr_val, &tmp_str, 10 ); 

#define is_valid_transparent_val( val ) ( val >= 50 && val <= 255 ) 
	if( FALSE == is_valid_transparent_val( transparent_val ) )
	{
		ASSERT( FALSE ); 

		//tmp_text = _get_string_by_id( TEXT_LOAD_CONFIG_ERROR, 
		//	_T( "加载配置失败" ) ); 

		//show_msg( NULL, tmp_text ); 

		transparent_val = DEF_UI_TRANSPARENT_VALUE; 
		//goto _return; 
	}

	do 
	{
		theme_no = 0; 

		node = root->FirstChild( UI_THEME_CONF ); 
		if( node == NULL )
		{
			break; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}

		common_conf = node->ToElement(); 

		attr_val = common_conf->Attribute( UI_THEME_NO_SETTING ); 
		if( attr_val == NULL )
		{
			break; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}

		theme_no = strtol( attr_val, &tmp_str, 10 ); 

		//attr_val = common_conf->Attribute( UI_THEME_NAME_SETTING ); 
		//if( attr_val == NULL )
		//{
		//	//ret = ERROR_ERRORS_ENCOUNTERED; 
		//	//goto _return; 
		//	break; 
		//}

		//theme_name = alloc_char_to_tchar( attr_val ); 
		//if( theme_name == NULL )
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}
	}while( FALSE );

	do 
	{
		show_slogan = 1; 
		node = root->FirstChild( UI_SLOGAN ); 
		if( node == NULL )
		{
			break; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}

		common_conf = node->ToElement(); 

		attr_val = common_conf->Attribute( UI_SLOGAN_SHOW_SETTING ); 
		if( attr_val == NULL )
		{
			break; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}

		show_slogan = strtol( attr_val, &tmp_str, 10 ); 

	} while ( FALSE );
	
	do 
	{
		symbol_path = L""; 
		dbg_help_path = L""; 
		help_flags = 0; 

		node = root->FirstChild( ANALYZE_HELP_CONF ); 
		if( node == NULL )
		{
			break; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}

		common_conf = node->ToElement(); 

		attr_val = common_conf->Attribute( ANALYZE_SYMBOL_PATH_CONF ); 
		if( attr_val == NULL )
		{
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}
		else
		{
			//char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
			_symbol_path = alloc_char_to_tchar( ( LPSTR )attr_val ); 
			symbol_path = _symbol_path; 
		}

		attr_val = common_conf->Attribute( ANALYZE_DBG_HELP_PATH_CONF ); 

		if( attr_val == NULL )
		{
			dbg_help_path = L""; 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}
		else
		{
			//char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
			_dbg_help_path = alloc_char_to_tchar( ( LPSTR )attr_val ); 
			dbg_help_path = _dbg_help_path; 
		}

		attr_val = common_conf->Attribute( ANALYZE_HELP_CONF_FLAGS ); 

		if( attr_val == NULL )
		{
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			//goto _return; 
		}
		else
		{
			help_flags = strtol( attr_val, &tmp_str, 10 ); 
		}

	} while ( FALSE );

	set_slogan_show( show_slogan ); 
	set_transparent( transparent_val ); 
	set_theme_no( theme_no ); 

	__ret = set_analyze_help_conf( symbol_path, dbg_help_path, help_flags ); 
	if( __ret != ERROR_SUCCESS )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "set the analyze help configuration error\n", _ret ) ); 
	}

	//set_theme_name( theme_name ); 

	if( ui_loaded_callback != NULL )
	{
		LRESULT _ret; 
		_ret = ui_loaded_callback( context ); 

#ifdef _DEBUG
		if( _ret != ERROR_SUCCESS )
		{
			log_trace( ( MSG_ERROR, "update ui configuration on callback function error\n" ) ); 
		}
#endif //_DEBUG

	}

#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

_return:
	//unlock_cs( lang_setting_lock ); 

	if( file_name != NULL )
	{
		mem_free( ( VOID** )&file_name ); 
	}

	if( _dbg_help_path != NULL )
	{
		mem_free( ( VOID** )&_dbg_help_path ); 
	}

	if( _symbol_path != NULL )
	{
		mem_free( ( VOID** )&_symbol_path ); 
	}

	//if( theme_name != NULL )
	//{
	//	mem_free( ( VOID** )&theme_name ); 
	//}
	return ret; 
}

LRESULT save_common_conf()
{
	TiXmlDocument xml_doc; 
	TiXmlDeclaration Declaration( "1.0","gb2312", "yes" ); 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *common_conf = NULL; 
	CHAR* tmp_str = NULL; 

	LRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	WCHAR ui_conf_file[ MAX_PATH ]; 
	CHAR *file_name = NULL; 
	analyze_setting *help_conf; 
	LPSTR symbol_path = NULL; 
	LPSTR dbg_help_path = NULL; 
	//CHAR lang_id_str[ 16 ]; 
	//bool _ret; 

	//lock_cs( lang_setting_lock ); 

	if( FALSE == is_valid_transparent_val( get_tranparent() ) ) 
	{
		ASSERT( FALSE && "how can got the invalid transparent value here" ); 
		goto _return; 
	}

	//if( FALSE == is_valid_transparent_val( get_theme_no() ) ) 
	//{
	//	ASSERT( FALSE && "how can got the invalid transparent value here" ); 
	//	goto _return; 
	//}

	//_ret = xml_doc.LoadFile( file_name ); 
	//if( _ret == false )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	xml_doc.InsertEndChild( Declaration ); 

	//root = xml_doc.RootElement(); 
	//if( root == NULL )
	//{
	root = new TiXmlElement( UI_CONF ); 
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

	common_conf = new TiXmlElement( UI_CONF ); 

	if( common_conf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = root->InsertEndChild( *common_conf ); 
	if( node == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; ; 
	}

	delete common_conf; 

	common_conf = node->ToElement(); 

	common_conf->SetAttribute( UI_TRANSPARENT_SETTING, ( INT32 )get_tranparent() ); 

	//insert theme configuration.
	common_conf = new TiXmlElement( UI_THEME_CONF ); 

	if( common_conf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = root->InsertEndChild( *common_conf ); 
	if( node == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	delete common_conf; 

	common_conf = node->ToElement(); 

	common_conf->SetAttribute( UI_THEME_NO_SETTING, ( INT32 )get_theme_no() ); 
	common_conf->SetAttribute( UI_THEME_NAME_SETTING, ( LPCSTR )"" ); 

	//insert theme configuration.
	common_conf = new TiXmlElement( UI_SLOGAN ); 

	if( common_conf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = root->InsertEndChild( *common_conf ); 
	if( node == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; ; 
	}

	delete common_conf; 

	common_conf = node->ToElement(); 

	common_conf->SetAttribute( UI_SLOGAN_SHOW_SETTING, ( INT32 )get_slogan_show() ); 

	//analyze help configuration
	common_conf = new TiXmlElement( ANALYZE_HELP_CONF ); 

	if( common_conf == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	node = root->InsertEndChild( *common_conf ); 
	if( node == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; ; 
	}

	delete common_conf; 

	common_conf = node->ToElement(); 

	help_conf = get_analyze_help_conf(); 

	symbol_path = alloc_tchar_to_char( help_conf->symbol_path ); 
	if( symbol_path == NULL )
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "%s:%u allocate and convert string error\n", __FUNCTION__, __LINE__ ) ); 
		common_conf->SetAttribute( ANALYZE_SYMBOL_PATH_CONF, "" ); 
	}
	else
	{
		common_conf->SetAttribute( ANALYZE_SYMBOL_PATH_CONF, symbol_path ); 
	}

	//if( symbol_path != NULL )
	//{
	//	mem_free( ( PVOID* )&symbol_path ); 
	//}
	//symbol_path = alloc_tchar_to_char( help_conf->symbol_path ); 

	common_conf->SetAttribute( ANALYZE_DBG_HELP_PATH_CONF, ""/*help_conf->dbg_help_path */); 
	common_conf->SetAttribute( ANALYZE_HELP_CONF_FLAGS, ( INT32 )get_analyze_ui_flags() ); 

	//save the ui configuration file
	ret = get_common_conf_file_name( ui_conf_file, MAX_PATH ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	file_name = alloc_tchar_to_char( ui_conf_file ); 
	if( file_name == NULL )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

	_ret = xml_doc.SaveFile( file_name ); 
	if( _ret == false )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:
	//unlock_cs( lang_setting_lock ); 

	if( file_name != NULL )
	{
		mem_free( ( void** )&file_name ); 
	}

	if( symbol_path != NULL )
	{
		mem_free( ( PVOID* )&symbol_path ); 
	}

	return ret; 
}

LRESULT load_theme_conf( LPCWSTR theme_conf_file, theme_setting *theme_out )
{
	LRESULT ret = ERROR_SUCCESS; 

	TiXmlDocument xml_doc; 
	TiXmlElement *root = NULL; 
	TiXmlNode *node = NULL; 
	TiXmlElement *ui_conf = NULL; 
	CHAR *tmp_str = NULL; 
	CHAR *file_name = NULL; 
	LPCSTR attr_val; 
	bool _ret; 
	//theme_setting cur_theme; 
	LPCTSTR tmp_text = NULL; 

	ASSERT( theme_conf_file != NULL ); 
	ASSERT( theme_out != NULL ); 

	memset( theme_out, 0, sizeof( *theme_out ) ); 
	
	ret = check_file_exist( theme_conf_file ); 
	if( ret != ERROR_SUCCESS )
	{
		ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
		goto _return; 
	}

	file_name = alloc_tchar_to_char( ( LPWSTR )theme_conf_file ); 
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

#define THEME_CONF "theme"
#define THEME_BAR_COLOR_SETTING "bar_clr"
#define THEME_MAIN_COLOR_SETTING "main_clr"
#define THEME_DESC_SETTING "desc"

	if( strcmp( THEME_CONF, root->Value() ) != 0 )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	//node = root->FirstChild( THEME_CONF ); 
	//if( node == NULL )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	ui_conf = root; //node->ToElement(); 

	//tmp_str = ui_conf_file; 

	attr_val = ui_conf->Attribute( THEME_BAR_COLOR_SETTING ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	theme_out->bar_clr = strtoul( attr_val, &tmp_str, 16 ); 

	attr_val = ui_conf->Attribute( THEME_MAIN_COLOR_SETTING ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	theme_out->bk_clr = strtoul( attr_val, &tmp_str, 16 ); 

	attr_val = ui_conf->Attribute( THEME_DESC_SETTING ); 
	if( attr_val == NULL )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	tmp_text = alloc_char_to_tchar( ( CHAR* )attr_val ); 

	_tcsncpy( theme_out->theme_desc, tmp_text, MAX_PATH ); 
	theme_out->theme_desc[ MAX_PATH - 1 ] = _T( '\0' ); 

#ifdef _DEBUG
	xml_doc.Print(); 
#endif //_DEBUG

_return:
	//unlock_cs( lang_setting_lock ); 

	if( file_name != NULL )
	{
		mem_free( ( VOID** )&file_name ); 
	}

	if( tmp_text != NULL )
	{
		mem_free( ( VOID** )&tmp_text ); 
	}

	return ret; 
}

LRESULT CALLBACK on_themes_found( LPCTSTR root_path, WIN32_FIND_DATA *find_data, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 

	ui_custom_dlg *dlg; 

	dlg = ( ui_custom_dlg* )context; 

	ASSERT( find_data != NULL ); 

	if( 0 != ( find_data->dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY ) )
	{
		if( 0 != _tcscmp( find_data->cFileName, _T( ".." ) ) 
			&& 0 != _tcscmp( find_data->cFileName, _T( "." ) ) )
		{
			dlg->SendMessage( WM_THEME_NOTIFY, ( WPARAM )root_path, ( LPARAM )find_data->cFileName ); 
		}
	}
	//else
	//{
	//	ret = ERROR_INVALID_PARAMETER; 
	//}

	return ret; 
}

//#include "stdafx.h"
//#include "ui_config.h"
//#include "bitsafe_common.h"
//#include "lang_dlg.h"
//#include "tinyxml.h"
//#include "string_mng.h"
//#include "memory_mng.h"
//
//CRITICAL_SECTION lang_setting_lock; 
//#define BITSAFE_LANG_CONF_FILE _T( "data7.dat" )
//
//LRESULT get_lang_conf_file_name( WCHAR *conf_file_path, ULONG name_len )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	BOOL _ret; 
//
//	ASSERT( conf_file_path != NULL ); 
//	ASSERT( name_len > 0 ); 
//
//	_ret = GetSystemDirectory( conf_file_path, name_len ); 
//	if( _ret == FALSE )
//	{
//		ret = GetLastError(); 
//		goto _return; 
//	}
//
//	wcsncat( conf_file_path, L"\\", name_len - wcslen( conf_file_path ) );
//
//	//if( conf_file_path[ name_len - 1 ] != L'\0' )
//	//{
//	//	//ASSERT( FALSE ); 
//	//	ret = ERROR_BUFFER_TOO_SMALL; 
//	//	goto _return; 
//	//}
//
//	wcsncat( conf_file_path, BITSAFE_LANG_CONF_FILE, name_len - wcslen( conf_file_path ) );
//
//	if( conf_file_path[ name_len - 1 ] != L'\0' )
//	{
//		conf_file_path[ name_len - 1 ] = L'\0'; 
//		//ASSERT( FALSE ); 
//		//ret = ERROR_BUFFER_TOO_SMALL; 
//		//goto _return; 
//	}
//
//	log_trace( ( MSG_INFO, "user configure file name is %ws\n", conf_file_path ) ); 
//
//_return:
//	return ret; 
//}
//
//#define LANG_CONF "language"
//#define LANG_ID_SETTING "language_id"
//
//LRESULT load_lang_conf()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	TiXmlDocument xml_doc; 
//	TiXmlElement *root = NULL; 
//	TiXmlNode *node = NULL; 
//	TiXmlElement *email_conf = NULL; 
//	CHAR *tmp_str = NULL; 
//	TCHAR erc_file_name[ MAX_PATH ]; 
//	CHAR *file_name = NULL; 
//	LPCSTR attr_val; 
//	bool _ret; 
//	ULONG lang_id_setting = LANG_ID_CN; 
//	LPCTSTR tmp_text; 
//
//	//lock_cs( lang_setting_lock ); 
//	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		goto _return; 
//	}
//
//	ret = check_file_exist( erc_file_name ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
//		goto _return; 
//	}
//
//	file_name = alloc_tchar_to_char( erc_file_name ); 
//	if( file_name == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//	_ret = xml_doc.LoadFile( file_name ); 
//	if( _ret == false )
//	{
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//
//	root = xml_doc.RootElement(); 
//	if( root == NULL )
//	{
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//
//	node = root->FirstChild( LANG_CONF ); 
//	if( node == NULL )
//	{
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//
//	email_conf = node->ToElement(); 
//
//	//tmp_str = erc_file_name; 
//
//	attr_val = email_conf->Attribute( LANG_ID_SETTING ); 
//	if( attr_val == NULL )
//	{
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//		goto _return; 
//	}
//
//	lang_id_setting = strtol( attr_val, &tmp_str, 10 ); 
//
//	if( FALSE == is_valid_lang_id( lang_id_setting ) )
//	{
//		ASSERT( FALSE ); 
//
//		tmp_text = _get_string_by_id( TEXT_LANG_LOAD_LANG_CONFIG_ERROR, 
//			_T( "加载语言配置失败" ) ); 
//
//		show_msg( NULL, tmp_text ); 
//		lang_id_setting = LANG_ID_CN; 
//	}
//
//	//char_to_tchar( attr_val, tmp_str, MAX_PATH ); 
//
//#ifdef _DEBUG
//	xml_doc.Print(); 
//#endif //_DEBUG
//
//_return:
//	//unlock_cs( lang_setting_lock ); 
//
//	if( file_name != NULL )
//	{
//		mem_free( ( VOID** )&file_name ); 
//	}
//
//	set_cur_lang( ( LANG_ID )lang_id_setting ); 
//
//	return ret; 
//}
//
//LRESULT save_lang_conf()
//{
//	TiXmlDocument xml_doc; 
//	TiXmlDeclaration Declaration( "1.0","gb2312", "yes" ); 
//	TiXmlElement *root = NULL; 
//	TiXmlNode *node = NULL; 
//	TiXmlElement *email_conf = NULL; 
//	CHAR* tmp_str = NULL; 
//
//	LRESULT ret = ERROR_SUCCESS; 
//	TCHAR erc_file_name[ MAX_PATH ]; 
//	CHAR *file_name = NULL; 
//	//CHAR lang_id_str[ 16 ]; 
//	//bool _ret; 
//
//	//lock_cs( lang_setting_lock ); 
//
//	if( FALSE == is_valid_lang_id( get_cur_lang_id() ) ) 
//	{
//		ASSERT( FALSE && "how can got the invalid language id here" ); 
//		goto _return; 
//	}
//
//	//_ret = xml_doc.LoadFile( file_name ); 
//	//if( _ret == false )
//	//{
//	//	ret = ERROR_ERRORS_ENCOUNTERED; 
//	//	goto _return; 
//	//}
//
//	xml_doc.InsertEndChild( Declaration ); 
//
//	//root = xml_doc.RootElement(); 
//	//if( root == NULL )
//	//{
//	root = new TiXmlElement( LANG_CONF ); 
//	if( root == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//	node = xml_doc.InsertEndChild( *root ); 
//
//	ASSERT( node != NULL ); 
//
//	log_trace( ( MSG_INFO, "root is 0x%0.8x, root of node is 0x%0.8x\n", root, node->ToElement() ) ); 
//
//	delete root; 
//
//	root = node->ToElement(); 
//
//	email_conf = new TiXmlElement( LANG_CONF ); 
//
//	if( email_conf == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//	node = root->InsertEndChild( *email_conf ); 
//	if( node == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; ; 
//	}
//
//	delete email_conf; 
//
//	email_conf = node->ToElement(); 
//	//}
//	//else
//	//{
//	//	node = root->FirstChild( EMAIL_CONF ); 
//	//	if( node == NULL )
//	//	{
//	//		root->Clear(); 
//	//	}
//
//	//	email_conf = new TiXmlElement( EMAIL_CONF ); 
//
//	//	if( email_conf == NULL )
//	//	{
//	//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//	//		goto _return; 
//	//	}
//
//	//	node = root->InsertEndChild( *email_conf ); 
//
//	//	email_conf = node->ToElement(); 
//	//}
//
//	//tmp_str = ( CHAR* )erc_file_name; 
//
//	email_conf->SetAttribute( LANG_ID_SETTING, ( INT32 )get_cur_lang_id() ); 
//
//	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		goto _return; 
//	}
//
//	file_name = alloc_tchar_to_char( erc_file_name ); 
//	if( file_name == NULL )
//	{
//		ret = ERROR_NOT_ENOUGH_MEMORY; 
//		goto _return; 
//	}
//
//#ifdef _DEBUG
//	xml_doc.Print(); 
//#endif //_DEBUG
//
//	xml_doc.SaveFile( file_name ); 
//
//_return:
//	//unlock_cs( lang_setting_lock ); 
//
//	if( file_name != NULL )
//	{
//		mem_free( ( void** )&file_name ); 
//	}
//
//	return ret; 
//}
//
//
//LRESULT chg_original_lang( action_ui_notify *ui_notify, BOOLEAN *conf_file_exist )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	//ASSERT( ui_notify != NULL ); 
//	//ASSERT( IsWindow( parent_wnd ) == TRUE ); 
//
//	if( conf_file_exist != NULL )
//	{
//		*conf_file_exist = TRUE; 
//	}
//
//	ret = load_lang_conf(); 
//
//	if( ret != ERROR_SUCCESS )
//	{
//		chg_lang_dlg dlg( ui_notify );
//		dlg_ret_state ret_state; 
//		LRESULT _ret; 
//		LPCTSTR tmp_text; 
//
//		if( ret != ERROR_FILE_NOT_FOUND )
//		{
//			//tmp_text = _get_string_by_id( TEXT_LANG_LANG_FILE_CURRPTED, 
//			//	_T( "系统关键数据被破坏,语言配置信息需要重新设置!" ) ); 
//
//			tmp_text = _T( "系统关键数据被破坏,语言配置信息需要重新设置!\n\n" ) \
//				_T( "Language configuration file is corrupted, need setting again." ); 
//
//			show_msg( NULL, tmp_text, NULL, _T( "" ), 0 ); 
//
//			//PostQuitMessage( 0 ); 
//		}
//		else
//		{
//			if( conf_file_exist != NULL )
//			{
//				*conf_file_exist = FALSE; 
//			}
//		}
//
//		tmp_text = _T( "现在还没有设置语言,请立即设置,才可以正确显示界面.\n\n" ) \
//			__t( "Have not set language now, please set it now, then ui can display correctly" ); 
//
//		_ret = show_msg( NULL, tmp_text, &ret_state, 0 ); 
//		if( _ret != ERROR_SUCCESS )
//		{
//			log_trace( ( MSG_ERROR, "create message box failed to indicate null password failed\n" ) ); 
//			//goto _return; 
//		}
//
//		if( ret_state == OK_STATE )
//		{
//			tmp_text = _get_string_by_id( TEXT_LANG_TITLE, 
//				_T("立即设置语言") ); 
//			dlg.Create( NULL, tmp_text, UI_WNDSTYLE_DIALOG, 0L, 0, 0, 1024, 738 );
//			dlg.SetIcon( IDI_MAIN_ICON ); 
//			dlg.CenterWindow();
//			dlg.ShowModal(); 
//
//			save_lang_conf(); 
//		}
//
//		goto _return; 
//	}
//
//_return:
//	return ret; 
//}
//
//LRESULT del_lang_conf()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	INT32 _ret; 
//	TCHAR erc_file_name[ MAX_PATH ]; 
//	//lock_cs( lang_setting_lock ); 
//	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		ASSERT( FALSE && "get language configuration file name error" ); 
//		goto _return; 
//	}
//
//	//ret = check_file_exist( erc_file_name ); 
//	//if( ret != ERROR_SUCCESS )
//	//{
//	//	ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
//	//	goto _return; 
//	//}
//
//	_ret = DeleteFile( erc_file_name ); 
//	if( _ret == FALSE )
//	{
//		ret = GetLastError(); 
//
//		log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the configuration file error\n" ) ); 
//	}
//
//_return:
//	return ret; 
//}
//
//LRESULT del_option_conf()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	INT32 _ret; 
//	TCHAR erc_file_name[ MAX_PATH ]; 
//	//lock_cs( lang_setting_lock ); 
//	ret = get_lang_conf_file_name( erc_file_name, MAX_PATH ); 
//	if( ret != ERROR_SUCCESS )
//	{
//		ASSERT( FALSE && "get language configuration file name error" ); 
//		goto _return; 
//	}
//
//	//ret = check_file_exist( erc_file_name ); 
//	//if( ret != ERROR_SUCCESS )
//	//{
//	//	ASSERT( ret == ERROR_FILE_NOT_FOUND ); 
//	//	goto _return; 
//	//}
//
//	_ret = DeleteFile( erc_file_name ); 
//	if( _ret == FALSE )
//	{
//		ret = GetLastError(); 
//
//		log_trace( ( DBG_MSG_AND_ERROR_OUT, "delete the configuration file error\n" ) ); 
//	}
//
//_return:
//	return ret; 
//}
//
