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

#ifndef __TRAY_ICON_H__
#define __TRAY_ICON_H__

#include <SHELLAPI.H>
//
///////////////////////////////////////////////////////////////
//tray_icon Window

class tray_icon
{
public:
	tray_icon();
	tray_icon(HWND wnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID); 
	virtual ~tray_icon();

//operations
public:
	BOOL Enable() { return m_bEnabled;}
	BOOL Visible() { return m_bHidden;}

//Get the system icons
	BOOL Create(CWindowWnd* pWnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID);
	BOOL Create(HWND hWnd,UINT uCallbackMessage,LPCTSTR szTip,HICON hicon,UINT uID);
//About the tip text
	BOOL SetToolTipText(LPCTSTR pszTip);
	BOOL SetToolTipText(UINT nID);
	LPCTSTR tray_icon::GetToolTipText() const; 
//About the icon
	BOOL SetIcon(HICON hIcon);
	BOOL SetIcon(LPCTSTR lpszIconName);
	BOOL SetIcon(UINT uIDResource);
	BOOL SetStandardIcon(LPCTSTR lpIconName);
	BOOL SetStandardIcon(UINT uIDResource);
	HICON GetIcon() const;
	void DeleteIcon();
	void HideIcon();
	void ShowIcon();
//About notifications the window with this icon
	BOOL SetNotificationWnd(CWindowWnd *pWnd);
	BOOL SetNotificationWnd(HWND hWnd);
	HWND GetNotificationWnd() const;
//Declares the transaction functions by the user defined message.
	virtual LRESULT OnIconNotification(WPARAM wID,LPARAM lEvent);
	void balloon_icon( HWND wnd, ULONG uCallbackMessage, INT32 add_icon ); 

protected:
	BOOL m_bEnabled;
	BOOL m_bDisabled;
	BOOL m_bHidden;

	NOTIFYICONDATA m_tnd;
};

#endif //__TRAY_ICON_H__




	
	
