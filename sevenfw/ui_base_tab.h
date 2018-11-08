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

 #pragma once

#define DLG_BK_CLR RGB( 255, 255, 255 )
class ui_base_tab : public CWindow
{
public:
	ui_base_tab(HWND hWnd = NULL) throw() : 
		CWindow(hWnd)
	{
		//bk_br.CreateSolidBrush( DLG_BK_CLR );
	}

	ui_base_tab& operator=(HWND hWnd) throw()
	{
		m_hWnd = hWnd;
		return *this;
	}
	
	~ui_base_tab()
	{
		//bk_br.DeleteObject();
	}

public:
	LRESULT OnCtlColor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
	{
		bHandled = FALSE;
		return (LRESULT)NULL;
	}
protected:
	//CBrush bk_br;
};