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
 *
 *  2007.6.10	Ji Jie Shi added this file based on the duilib. 
 */

#ifndef __LIST_SUB_ITEM_EX_H__
#define __LIST_SUB_ITEM_EX_H__

namespace DuiLib
{
	class CListSubElementExUI : public CLabelUI
	{
	public: 
		CListSubElementExUI()
		{}

		CListSubElementExUI( CControlUI *control )
		{}

		~CListSubElementExUI()
		{

		}

	public:
		void set_x( RECT &rc )
		{
			ASSERT( rc.right >= rc.left ); 

			m_rcItem.left = rc.left; 
			m_rcItem.right = rc.right; 
		}

		void set_x_locate_ctrl( CControlUI *control )
		{
			//x_locate_ctrl = control; 
		}

		LPCTSTR GetClass() const
		{
			return _T( "ListSubElementExUI" ); 
		}
	};

	class CListContainerElementExUI : public CListContainerElementUI
	{
	public: 
		CListContainerElementExUI(); 
		~CListContainerElementExUI(); 

		LPCTSTR GetClass() const;
		
		void DoEvent(TEventUI& event);
		void DoPaint(HDC hDC, const RECT& rcPaint);

		PVOID get_user_data()
		{
			return user_data; 
		}

		VOID set_user_data( PVOID data )
		{
			user_data = data; 
		}

	protected:
		PVOID user_data; 
	};
} //namespace DuiLib

#endif //__LIST_SUB_ITEM_EX_H__