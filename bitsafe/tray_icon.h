/*
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 * This file is part of NetMonitor.
 *
 * NetMonitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NetMonitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NetMonitor.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  2007.6.10	Ji Jie Shi modified it for this ui lib. 
 */

#ifndef __TRAY_ICON_H__
#define __TRAY_ICON_H__

#include <shellapi.h>

#define TOOL_TIP_LEN 128

#define SMALL_ICON_CX 16
#define NORMAL_ICON_CX 32


class tray_icon
{
public:
	tray_icon();
	tray_icon( HWND wnd,UINT callback_msg, LPCTSTR szTip, HMODULE module, UINT icon_id, UINT uID, LPCTSTR icons_file ); 

	LRESULT Create( HWND wnd, UINT callback_msg, LPCTSTR szTip, HMODULE module, UINT icon_id, UINT uID, LPCTSTR image_file = NULL); 

	virtual ~tray_icon();

public:
	LRESULT release_icons(); 
	BOOL Enable() { return m_bEnabled;}
	BOOL Visible() { return m_bHidden;}

	LRESULT Create(HWND hWnd,UINT uCallbackMessage,LPCTSTR szTip, HICON hicon,UINT uID );

	LRESULT SetToolTipText(LPCTSTR pszTip);
	LRESULT SetToolTipText(UINT nID);
	LPCTSTR tray_icon::GetToolTipText() const; 

	LRESULT SetIcon(HICON hIcon);
	LRESULT SetIcon(LPCTSTR lpszIconName);
	LRESULT SetIcon(UINT uIDResource);
	LRESULT SetStandardIcon(LPCTSTR lpIconName);
	LRESULT SetStandardIcon(UINT uIDResource);
	HICON GetIcon() const;
	LRESULT DeleteIcon();
	LRESULT HideIcon();
	LRESULT ShowIcon();

	LRESULT SetNotificationWnd(CWindowWnd *pWnd);
	LRESULT SetNotificationWnd(HWND hWnd);
	HWND GetNotificationWnd() const;

	LRESULT balloon_icon( HWND wnd, UINT callback_msg, LPCTSTR tip, LPCTSTR info_title, LPCTSTR info, HICON hicon, UINT id ); 
	LRESULT balloon_icon( HWND wnd, UINT callback_msg, LPCTSTR tip, LPCTSTR info_title, LPCTSTR info, HMODULE module, UINT icon_id, UINT animate_icon, UINT id, LPCTSTR icons_file ); 

	LRESULT set_cur_icon_index( ULONG index ); 
	LRESULT show_next_icon(); 

	LRESULT restore_main_icon(); 

protected:
	BOOL m_bEnabled;
	BOOL m_bDisabled;
	BOOL m_bHidden;

	NOTIFYICONDATA m_tnd; 
	HICON main_icon; 
#define MAX_ANIMATE_ICON_COUNT 5
	HICON animate_icons[ MAX_ANIMATE_ICON_COUNT ]; 
	ULONG animate_icon_num; 

	ULONG cur_icon_index; 
};

#endif //__TRAY_ICON_H__




	
	
