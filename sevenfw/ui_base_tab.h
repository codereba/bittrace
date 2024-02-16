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