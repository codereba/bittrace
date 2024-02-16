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
#include "tray_icon.h"

tray_icon::tray_icon()
{
	memset( &m_tnd, 0,sizeof( m_tnd ) );
	m_bEnabled = FALSE;
	m_bHidden = FALSE;
	cur_icon_index = 0; 
	main_icon = NULL; 
	memset( animate_icons, 0, sizeof( animate_icons ) ); 
	animate_icon_num = 0; 
}

tray_icon::tray_icon( HWND wnd,UINT callback_msg, LPCTSTR szTip, HMODULE module, UINT icon_id, UINT uID, LPCTSTR icons_file ) 
{
	tray_icon(); 

	Create( wnd, callback_msg, szTip, module, icon_id, uID, icons_file ); 
	m_bHidden = FALSE;
}

LRESULT tray_icon::balloon_icon( HWND wnd, UINT callback_msg, LPCTSTR tip, LPCTSTR info_title, LPCTSTR info, HICON hicon, UINT id )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	m_tnd.cbSize = sizeof( NOTIFYICONDATA );
	m_tnd.hWnd = wnd;
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO; 
	m_tnd.uCallbackMessage = callback_msg;

#define TOOL_INFO_TITLE_LEN 64
#define TOOL_INFO_LEN 256

	ASSERT( _tcslen( tip ) <= TOOL_TIP_LEN - 1 ); 
	ASSERT( _tcslen( info_title ) <= TOOL_INFO_TITLE_LEN - 1 ); 
	ASSERT( _tcslen( info ) <= TOOL_INFO_LEN - 1 ); 

	_tcscpy( m_tnd.szTip, tip ); ;

	_tcscpy(m_tnd.szInfoTitle, info_title ); 

	_tcscpy(m_tnd.szInfo, info ); 

	m_tnd.uID = id;

	m_tnd.hIcon = hicon;
	m_tnd.dwState = NIS_SHAREDICON;
	m_tnd.uTimeout = 100;

#ifndef NIIF_USER
#define NIIF_USER 0x00000004
#endif //NIIF_USER
	m_tnd.dwInfoFlags = NIIF_INFO | NIIF_USER
;

	_ret = Shell_NotifyIcon( NIM_ADD, &m_tnd );
	m_bEnabled = _ret; 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	if( main_icon != NULL )
	{
		DestroyIcon( main_icon ); 
		main_icon = NULL; 
	}

	main_icon = hicon; 

_return:
	return ret; 
}

LRESULT tray_icon::balloon_icon( HWND wnd, 
								UINT callback_msg, 
								LPCTSTR tip, 
								LPCTSTR info_title, 
								LPCTSTR info, 
								HMODULE module, 
								UINT icon_id, 
								UINT animate_icon, 
								UINT id, 
								LPCTSTR icons_file )
{
	LRESULT ret = ERROR_SUCCESS; 
	HICON hIcon;
	LONG icon_cx; 
	LONG icon_cy; 
	HIMAGELIST dynamic_icons = NULL;
	ULONG icons_count; 
	ULONG load_image_flags; 
	INT32 i; 

	cur_icon_index = 0; 

	release_icons(); 

	icon_cx = GetSystemMetrics( SM_CXICON ); 
	icon_cy = GetSystemMetrics( SM_CYICON ); 

	if( animate_icon != NULL )
	{
		hIcon = ( HICON )LoadImage( module, 
			MAKEINTRESOURCE( icon_id ), 
			IMAGE_ICON, 
			icon_cx, 
			icon_cy, 
			LR_SHARED ); 

		if( hIcon == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		animate_icons[ 0 ] = hIcon; 
		animate_icon_num = 1; 

		hIcon = ( HICON )LoadImage( module, 
			MAKEINTRESOURCE( animate_icon ), 
			IMAGE_ICON, 
			icon_cx, 
			icon_cy, 
			LR_SHARED ); 

		if( hIcon == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		animate_icons[ 1 ] = hIcon; 
		animate_icon_num = 2; 
	}
	else if( icons_file != NULL )
	{
		if( TRUE == IS_INTRESOURCE( icons_file ) )
		{
			load_image_flags = 0; 
		}
		else
		{
			ASSERT( FALSE && "not support image list that load from bitmap list" ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		dynamic_icons = ImageList_LoadImage( module, icons_file, NORMAL_ICON_CX, 1, RGB( 255, 0, 255 ), IMAGE_BITMAP, load_image_flags ); 
		if( dynamic_icons == NULL )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "load the dynamic icons error" ) ); 
			goto _return; 
		}

		icons_count = ImageList_GetImageCount( dynamic_icons ); 

		animate_icon_num = 0; 
		for( i = 0; ( ULONG )i < icons_count; i ++ )
		{
			hIcon = ImageList_ExtractIcon( 0, dynamic_icons, cur_icon_index ); 
			if( hIcon == NULL )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "load the icon(index:%u) error\n", i ) ); 
				goto _return; 
			}

			animate_icons[ i ] = hIcon; 
			animate_icon_num ++; 
		}
	}

	
	hIcon = ( HICON )LoadImage( module, 
		MAKEINTRESOURCE( icon_id ), 
		IMAGE_ICON, 
		icon_cx, 
		icon_cy, 
		LR_SHARED ); 

	if( hIcon == NULL )
	{
		ret = GetLastError(); 
	}

	ret = balloon_icon( wnd, callback_msg, tip, info_title, info, hIcon, id ); 

_return:

	if( dynamic_icons != NULL )
	{
		ImageList_Destroy( dynamic_icons ); 
	}
	return ret; 
}

LRESULT tray_icon::Create( HWND wnd, UINT callback_msg, LPCTSTR szTip, HICON hicon, UINT uID/*, LPCTSTR icons_file */)
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( m_bEnabled = ( GetVersion() & 0xff ) >= 4 ); 
	if( m_bEnabled == FALSE )
	{
		ret = ERROR_NOT_SUBSTED; 
		goto _return; 
	}

	ASSERT( m_bEnabled = ( ::IsWindow( wnd ) ) );
	if( m_bEnabled == FALSE ) 
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	//Confirms to the user defined message greater than WM_USER
	ASSERT( callback_msg >= WM_USER );

	//Confirms to the length of the tips text lower than 64
	ASSERT( _tcslen( szTip ) <= TOOL_TIP_LEN - 1 );

	m_tnd.cbSize = sizeof( NOTIFYICONDATA );
	m_tnd.hWnd = wnd;
	m_tnd.uID = uID;
	m_tnd.hIcon = hicon;
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tnd.uCallbackMessage = callback_msg;
	_tcscpy( m_tnd.szTip, szTip );

	//Set icon
	m_bEnabled = Shell_NotifyIcon( NIM_ADD, &m_tnd ); 
	if( m_bEnabled == FALSE )
	{
		ret = GetLastError(); 
		if( ret == ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}

		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create the tray icon failed\n" ) ); 
		goto _return; 
	}
	
	if( main_icon != NULL )
	{
		DestroyIcon( main_icon ); 
		main_icon = NULL; 
	}

	main_icon = hicon; 
	goto _return; 

_return:
	return ret;
}

LRESULT tray_icon::Create( HWND wnd, UINT callback_msg, LPCTSTR szTip, HMODULE module, UINT icon_id, UINT uID, LPCTSTR icons_file )
{
	LRESULT ret = ERROR_SUCCESS; 
	HICON hicon; 
	ULONG load_image_flags; 
	HIMAGELIST dynamic_icons = NULL; 
	ULONG icons_count; 
	INT32 i; 
	LONG icon_cx; 
	LONG icon_cy; 

	release_icons(); 

	cur_icon_index = 0; 

	icon_cx = GetSystemMetrics( SM_CXICON ); 
	icon_cy = GetSystemMetrics( SM_CYICON ); 

	if( icons_file != NULL )
	{
		if( TRUE == IS_INTRESOURCE( icons_file ) )
		{
			load_image_flags = 0; 
		}
		else
		{
			ASSERT( FALSE && "not support image list that load from bitmap list" ); 

			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		dynamic_icons = ImageList_LoadImage( NULL, MAKEINTRESOURCE( icons_file ), NORMAL_ICON_CX, 1, RGB( 0, 0, 0 ), IMAGE_BITMAP, load_image_flags ); 
		if( dynamic_icons == NULL )
		{
			SAFE_SET_ERROR_CODE( ret ); 

			goto _return; 
		}

		icons_count = ImageList_GetImageCount( dynamic_icons ); 
		animate_icon_num = 0; 
		for( i = 0; ( ULONG )i < icons_count; i ++ )
		{
			hicon = ImageList_ExtractIcon( 0, dynamic_icons, cur_icon_index ); 
			if( hicon == NULL )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				log_trace( ( DBG_MSG_AND_ERROR_OUT, "load the icon(index:%u) error\n", i ) ); 
				goto _return; 
			}

			animate_icons[ i ] = hicon; 
			animate_icon_num ++; 
		}
	}

	hicon = ( HICON )LoadImage( module, MAKEINTRESOURCE( icon_id ), IMAGE_ICON, 16, 16, LR_SHARED ); 
	if( hicon == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	ret = Create( wnd, callback_msg, szTip, hicon, uID ); ; 

_return:
	if( dynamic_icons != NULL )
	{
		ImageList_Destroy( dynamic_icons ); 

		dynamic_icons = NULL; 
	}
	return ret; 
}

LRESULT tray_icon::release_icons()
{
	INT32 i; 
	if( main_icon != NULL )
	{
		DestroyIcon( main_icon ); 
		main_icon = NULL; 
	}

	for( i = 0; i < ARRAY_SIZE( animate_icons ); i ++ )
	{
		if( NULL != animate_icons[ i ] )
		{
			DestroyIcon( animate_icons[ i ] ); 
			animate_icons[ i ] = NULL; 
		}
	}

	return ERROR_SUCCESS; 
}

tray_icon::~tray_icon()
{
	DeleteIcon(); 


	release_icons(); 
}

LRESULT tray_icon::DeleteIcon()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( !m_bEnabled ) 
	{
		goto _return; 
	}

	m_tnd.uFlags = 0;
	_ret = Shell_NotifyIcon( NIM_DELETE, &m_tnd ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 

		ASSERT( FALSE ); 
		goto _return; 
	}

	m_bEnabled = FALSE;

_return: 
	return ret; 
}

LRESULT tray_icon::HideIcon()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( m_bEnabled && !m_bHidden )
	{
		m_tnd.uFlags = NIF_ICON;
		
		_ret = Shell_NotifyIcon( NIM_DELETE, &m_tnd );
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		m_bHidden = TRUE;
	}

_return:
	return ret; 
}

LRESULT tray_icon::ShowIcon()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( m_bEnabled && m_bHidden )
	{
		m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		
		_ret = Shell_NotifyIcon( NIM_ADD, &m_tnd );
		if( _ret == FALSE )
		{
			ASSERT( FALSE ); 
			ret = GetLastError(); 
			goto _return; 
		}

		m_bHidden = FALSE;
	}

_return:
	return ret; 
}

LRESULT tray_icon::SetIcon(HICON hIcon)
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( !m_bEnabled ) 
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	m_tnd.uFlags = NIF_ICON;
	m_tnd.hIcon = hIcon;
	_ret = Shell_NotifyIcon(NIM_MODIFY,&m_tnd);
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		if( ret == ERROR_SUCCESS )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
		}

		goto _return; 
	}

_return:
	return ret; 
}

LRESULT tray_icon::SetIcon( LPCTSTR icon_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HICON icon; 

	icon = LoadIcon( NULL, icon_name );
	if( icon == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	ret = SetIcon( icon ); 

_return: 
	return ret; 
}

LRESULT tray_icon::SetIcon( UINT uIDResource )
{
	LRESULT ret = ERROR_SUCCESS; 
	HICON icon; 

	icon = LoadIcon( NULL, MAKEINTRESOURCE(  uIDResource ) ); 
	if( icon == NULL )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	ret = SetIcon( icon ); 
_return:
	return ret; 
}

inline LRESULT tray_icon::SetStandardIcon( LPCTSTR icon_name )
{
	return SetIcon( icon_name );
}

inline LRESULT tray_icon::SetStandardIcon( UINT icon_id )
{
	return SetIcon( icon_id ); 
}

HICON tray_icon::GetIcon() const
{
	HICON hIcon = NULL; 

	if( m_bEnabled )
	{
		hIcon = m_tnd.hIcon; 
	}

	return hIcon;
}

LRESULT tray_icon::SetToolTipText(LPCTSTR pszTip)
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( m_bEnabled == FALSE ) 
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	m_tnd.uFlags = NIF_TIP; 

	_tcscpy( m_tnd.szTip, pszTip ); 

	_ret = Shell_NotifyIcon( NIM_MODIFY, &m_tnd ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT tray_icon::SetToolTipText(UINT uID)
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	TCHAR tool_tip_text[ TOOL_TIP_LEN ]; 

	_ret = LoadString( NULL, uID, tool_tip_text, TOOL_TIP_LEN ); 
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	ret = SetToolTipText( tool_tip_text );

_return:
	return ret; 

}

LPCTSTR tray_icon::GetToolTipText() const
{
	LPCTSTR tool_tip_text = NULL; 
	
	if( m_bEnabled )
		tool_tip_text = m_tnd.szTip; 
	
	return tool_tip_text;
}

LRESULT tray_icon::SetNotificationWnd( CWindowWnd *pWnd )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( pWnd != NULL );
	ret = SetNotificationWnd( pWnd->GetHWND() ); 

	return ret; 
}

LRESULT tray_icon::SetNotificationWnd( HWND hWnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	ASSERT( hWnd && IsWindow( hWnd ) );

	if( m_bEnabled == FALSE ) 
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	m_tnd.hWnd = hWnd;
	m_tnd.uFlags = 0;
	_ret = Shell_NotifyIcon( NIM_MODIFY, &m_tnd ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return:
	return ret; 
}

HWND tray_icon::GetNotificationWnd() const
{
	return m_tnd.hWnd;
}

LRESULT tray_icon::set_cur_icon_index( ULONG index )
{
	cur_icon_index = index; 

	return ERROR_SUCCESS; 
}

LRESULT tray_icon::restore_main_icon()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 

	if( main_icon == NULL )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	m_tnd.hIcon = main_icon;

	_ret = Shell_NotifyIcon( NIM_MODIFY, &m_tnd ); 

	if( FALSE == _ret )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT tray_icon::show_next_icon()
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HICON cur_icon;  
	if( animate_icon_num == 0 )
	{
		ret = ERROR_NOT_READY; 
		goto _return; 
	}

	cur_icon_index ++; 
	if( cur_icon_index >= animate_icon_num )
		cur_icon_index = 0; 

	cur_icon = animate_icons[ cur_icon_index ]; 
	ASSERT( cur_icon != NULL ); 

	m_tnd.hIcon = cur_icon;

	_ret = Shell_NotifyIcon( NIM_MODIFY, &m_tnd ); 

	if( FALSE == _ret )
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}

_return:

	return ret; 
}