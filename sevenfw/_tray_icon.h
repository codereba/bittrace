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




	
	
