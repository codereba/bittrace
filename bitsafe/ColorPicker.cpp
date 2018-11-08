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
#include <windows.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif

#include "win_impl_base.hpp"
#include "ColorPicker.hpp"
#include "chat_dialog.hpp"

static DWORD Colors[5][8] = 
{
	{
		0xFF000000,0xFFA52A00,0xFF004040,0xFF005500,0xFF00005E,0xFF00008B,0xFF4B0082,0xFF282828
	},
	{
	0xFF8B0000,0xFFFF6820,0xFF8B8B00,0xFF009300,0xFF388E8E,0xFF0000FF,0xFF7B7BC0,0xFF666666
	},
	{
		0xFFFF0000,0xFFFFAD5B,0xFF32CD32,0xFF3CB371,0xFF7FFFD4,0xFF7D9EC0,0xFF800080,0xFF7F7F7F
	},
	{
		0xFFFFC0CB,0xFFFFD700,0xFFFFFF00,0xFF00FF00,0xFF40E0D0,0xFFC0FFFF,0xFF480048,0xFFC0C0C0
	},
	{
		0xFFFFE4E1,0xFFD2B48C,0xFFFFFFE0,0xFF98FB98,0xFFAFEEEE,0xFF68838B,0xFFE6E6FA,0xFFA020F0
	}
};

CColorPicker::CColorPicker(ChatDialog* chat_dialog, POINT ptMouse)
: based_point_(ptMouse)
, chat_dialog_(chat_dialog)
{
	Create(NULL, _T("color"), WS_POPUP, WS_EX_TOOLWINDOW, 0, 0);
	ShowWindow(true);
}

LPCTSTR CColorPicker::GetWindowClassName() const 
{ 
	return _T("ColorWindow");
}

void CColorPicker::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

void CColorPicker::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, _T("click")) == 0)
	{
		CControlUI* pOne = static_cast<CControlUI*>(paint_manager_.FindControl(msg.ptMouse));
		if (_tcsicmp(pOne->GetClass(), _T("ButtonUI")) == 0)
		{
			DWORD nColor = pOne->GetBkColor();
			CVerticalLayoutUI* pColorContiner = static_cast<CVerticalLayoutUI*>(paint_manager_.FindControl(_T("color")));
			pColorContiner->SetBkColor(nColor);
			UpdateWindow(m_hWnd);
			Sleep(500);
			chat_dialog_->SetTextColor(nColor);
		}
	}
}

void CColorPicker::Init()
{
	CVerticalLayoutUI* pColorContiner = static_cast<CVerticalLayoutUI*>(paint_manager_.FindControl(_T("color")));
	for (int i = 0; (i < 5) && (pColorContiner != NULL); i ++)
	{
		CHorizontalLayoutUI* pLine = new CHorizontalLayoutUI();
		pLine->SetFixedHeight(12);
		pColorContiner->Add(pLine);
		for (int j = 0; j < 8; j++)
		{
			CButtonUI* pOne = new CButtonUI();
			pOne->ApplyAttributeList(_T("bordersize=\"1\" bordercolor=\"#FF000000\" width=\"10\" height=\"10\""));
			pOne->SetBkColor(Colors[i][j]);
			pLine->Add(pOne);
			if (i < 7)
			{
				CControlUI* pMargin = new CControlUI();
				pMargin->SetFixedWidth(2);
				pLine->Add(pMargin);
			}
		}
	}

	SIZE size = paint_manager_.GetInitSize();
	MoveWindow(m_hWnd, based_point_.x - static_cast<LONG>(size.cx / 2), based_point_.y - size.cy, size.cx, size.cy, FALSE);
}

tString CColorPicker::GetSkinFile()
{
	return _T("color.xml");
}

tString CColorPicker::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath()) + _T("skin\\");
}

LRESULT CColorPicker::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	chat_dialog_->SetTextColor(0);
	Close();
	return 0;
}
