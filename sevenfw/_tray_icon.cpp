/*
 * Copyright 2010 JiJie Shi
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

#include "stdafx.h"
#include "tray_icon.h"

//////////////////////////////////////////////////////////////////
tray_icon::tray_icon()
{
	memset(&m_tnd,0,sizeof(m_tnd));
	m_bEnabled=FALSE;
	m_bHidden=FALSE;
}

tray_icon::tray_icon(HWND wnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID)
{
	Create(wnd,uCallbackMessage,szTip,hicon,uID);
	m_bHidden=FALSE;
}

BOOL tray_icon::Create(CWindowWnd *pWnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID)
{
	//Functions only used in windows 95 later version.
	ASSERT(m_bEnabled=(GetVersion() & 0xff)>=4);
	if(!m_bEnabled) return FALSE;

	ASSERT(m_bEnabled=(pWnd && ::IsWindow(pWnd->GetHWND())));
	if(!m_bEnabled) return FALSE;

	//Confirms to the user defined message greater than WM_USER
	ASSERT(uCallbackMessage >= WM_USER);
	//Confirms to the length of the tips text lower than 64
	ASSERT(_tcslen(szTip)<=64);

	//Defines the items,that's in NOTIFYICONDATA structure.
	m_tnd.cbSize=sizeof(NOTIFYICONDATA);
	m_tnd.hWnd=pWnd->GetHWND();
	m_tnd.uID=uID;
	m_tnd.hIcon=hicon;
	m_tnd.uFlags=NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tnd.uCallbackMessage=uCallbackMessage;
	_tcscpy(m_tnd.szTip,szTip);

	//Set icon
	ASSERT(m_bEnabled=Shell_NotifyIcon(NIM_ADD,&m_tnd));
	return m_bEnabled;
}

BOOL tray_icon::Create(HWND hWnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID)
{
	//Functions only used in windows 95 later version.
	ASSERT(m_bEnabled=(GetVersion() & 0xff)>=4);
	if(!m_bEnabled) return FALSE;

	ASSERT(m_bEnabled=(hWnd && ::IsWindow(hWnd)));
	if(!m_bEnabled) return FALSE;

	//Confirms to the user defined message greater than WM_USER
	ASSERT(uCallbackMessage >= WM_USER);
	//Confirms to the length of the tips text lower than 64
	ASSERT(_tcslen(szTip)<=64);

	//Defines the items,that's in NOTIFYICONDATA structure.
	m_tnd.cbSize=sizeof(NOTIFYICONDATA);
	m_tnd.hWnd=hWnd;
	m_tnd.uID=uID;
	m_tnd.hIcon=hicon;
	m_tnd.uFlags=NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tnd.uCallbackMessage=uCallbackMessage;
	_tcscpy(m_tnd.szTip,szTip);

	//Set icon
	ASSERT(m_bEnabled=Shell_NotifyIcon(NIM_ADD,&m_tnd));
	return m_bEnabled;
}

tray_icon::~tray_icon()
{
	DeleteIcon();
}

void tray_icon::DeleteIcon()
{
	if(!m_bEnabled) return;
	m_tnd.uFlags=0;
	Shell_NotifyIcon(NIM_DELETE,&m_tnd);
	m_bEnabled=FALSE;
}

void tray_icon::HideIcon()
{
	if(m_bEnabled && !m_bHidden)
	{
		m_tnd.uFlags=NIF_ICON;
		Shell_NotifyIcon(NIM_DELETE,&m_tnd);
		m_bHidden=TRUE;
	}
}

void tray_icon::ShowIcon()
{
	if(m_bEnabled && m_bHidden)
	{
		m_tnd.uFlags=NIF_MESSAGE | NIF_ICON | NIF_TIP;
		Shell_NotifyIcon(NIM_ADD,&m_tnd);
		m_bHidden=FALSE;
	}
}

BOOL tray_icon::SetIcon(HICON hIcon)
{
	if(!m_bEnabled) return FALSE;
	m_tnd.uFlags=NIF_ICON;
	m_tnd.hIcon=hIcon;
	return Shell_NotifyIcon(NIM_MODIFY,&m_tnd);
}

BOOL tray_icon::SetIcon(LPCTSTR lpszIconName)
{
	HICON hIcon=::LoadIcon(NULL, lpszIconName);
	return SetIcon(hIcon);
}

BOOL tray_icon::SetIcon(UINT uIDResource)
{
	HICON hIcon=::LoadIcon(NULL, MAKEINTRESOURCE(  uIDResource ) );
	return SetIcon(hIcon);
}

BOOL tray_icon::SetStandardIcon(LPCTSTR lpIconName)
{
	HICON hIcon=LoadIcon(NULL,lpIconName);
	return SetIcon(hIcon);
}

BOOL tray_icon::SetStandardIcon(UINT uIDResource)
{
	HICON hIcon=LoadIcon(NULL,MAKEINTRESOURCE(uIDResource));
	return SetIcon(hIcon);
}

HICON tray_icon::GetIcon() const
{
	HICON hIcon=NULL;
	if(m_bEnabled)
		hIcon=m_tnd.hIcon;
	return hIcon;
}

BOOL tray_icon::SetToolTipText(LPCTSTR pszTip)
{
	if(!m_bEnabled) return FALSE;
	m_tnd.uFlags=NIF_TIP;
	_tcscpy(m_tnd.szTip,pszTip);
	return Shell_NotifyIcon(NIM_MODIFY,&m_tnd);
}

#define TOOL_TIP_LEN 128
BOOL tray_icon::SetToolTipText(UINT uID)
{
	TCHAR tool_tip_text[ TOOL_TIP_LEN ]; 
	::LoadString( NULL, uID, tool_tip_text, TOOL_TIP_LEN ); 

	return SetToolTipText( tool_tip_text );
}

LPCTSTR tray_icon::GetToolTipText() const
{
	LPCTSTR tool_tip_text = NULL; 
	
	if(m_bEnabled)
		tool_tip_text = m_tnd.szTip;
	
	return tool_tip_text;
}

BOOL tray_icon::SetNotificationWnd(CWindowWnd *pWnd)
{
	if(!m_bEnabled) return FALSE;
	ASSERT(pWnd && IsWindow(pWnd->GetHWND()));
	m_tnd.hWnd=pWnd->GetHWND();
	m_tnd.uFlags=0;
	return Shell_NotifyIcon(NIM_MODIFY,&m_tnd);
}

BOOL tray_icon::SetNotificationWnd(HWND hWnd)
{
	if(!m_bEnabled) return FALSE;
	ASSERT(hWnd && IsWindow(hWnd));
	m_tnd.hWnd=hWnd;
	m_tnd.uFlags=0;
	return Shell_NotifyIcon(NIM_MODIFY,&m_tnd);
}

HWND tray_icon::GetNotificationWnd() const
{
	return m_tnd.hWnd;
}

LRESULT tray_icon::OnIconNotification(WPARAM wID,LPARAM lEvent)
{
	if(wID!=m_tnd.uID)
		return 0L;
	//CMenu menu; 
	//CMenuHandle pSubMenu;

	//if(lEvent==WM_RBUTTONUP)
	//{
	//	if(!menu.LoadMenu(m_tnd.uID)) return 0L;
	//	if(!(pSubMenu=menu.GetSubMenu(0))) return 0L;
	//	::SetMenuDefaultItem(menu.m_hMenu,0,TRUE);
	//	CPoint pos;
	//	GetCursorPos(&pos);
	//	::SetForegroundWindow(m_tnd.hWnd);
	//	::TrackPopupMenu(pSubMenu.m_hMenu,0,pos.x,pos.y,0,m_tnd.hWnd,NULL);
	//	::PostMessage(m_tnd.hWnd,WM_NULL,0,0);
	//	menu.DestroyMenu();
	//}
	//else if(lEvent == WM_LBUTTONDBLCLK) 
	//{ 

	//	if (!menu.LoadMenu(m_tnd.uID)) return 0; 
	//	if (!(pSubMenu = menu.GetSubMenu(0))) return 0; 
	//	////双击左键起动缺省菜单 
	//	//::SetForegroundWindow(m_tnd.hWnd); 
	//	::SendMessage(m_tnd.hWnd,WM_COMMAND,pSubMenu.GetMenuItemID(2), 0); 
	//	menu.DestroyMenu(); 
	//} 

	return 1; 
} 

void tray_icon::balloon_icon( HWND wnd, ULONG uCallbackMessage, INT32 add_icon ) 
{
	m_tnd.cbSize = sizeof(NOTIFYICONDATA);
	m_tnd.hWnd = wnd;
	m_tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_INFO;
	m_tnd.uCallbackMessage=uCallbackMessage;

	_tcscpy(m_tnd.szTip, _T("生成一个图标！") );

	_tcscpy(m_tnd.szInfoTitle, _T("提示"));

	_tcscpy(m_tnd.szInfo,_T("程序的图标已经生成成功！" ) );

	//m_tnd.uID=IDR_MAINFRAME;

	//HICON hIcon;
	//hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_tnd.hIcon=hIcon;
	//m_tnd.dwState=NIS_SHAREDICON;
	//m_tnd.uTimeout = 100;
	//m_tnd.dwInfoFlags = NIIF_INFO;

	DWORD mode; 
	if( add_icon )
	{
		mode = NIM_ADD; 
	}
	else
	{
		mode = NIM_DELETE; 
	}

	::Shell_NotifyIcon(mode,&m_tnd);

}
