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