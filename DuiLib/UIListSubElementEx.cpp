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

#include "StdAfx.h"
#include "UIListSubElementEx.h"

namespace DuiLib
{
	CListContainerElementExUI::CListContainerElementExUI() : user_data( NULL )
	{

	}

	CListContainerElementExUI::~CListContainerElementExUI()
	{

	}

	LPCTSTR CListContainerElementExUI::GetClass() const
	{
		return UI_LIST_CONTAINER_ELEMENT_EX_CLASS_NAME; 
	}

	void CListContainerElementExUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		INT32 i; 
		IListOwnerUI *List; 
		TListInfoUI *ListInfo; 
		CContainerUI *EventInfoLayout; 
		CControlUI *Control; 
		CListSubElementExUI *ListSubItem; 

		List = GetOwner(); 
		ASSERT( List != NULL ); 

		do 
		{
			ListInfo = List->GetListInfo(); 
			ASSERT( ListInfo != NULL ); 

			EventInfoLayout = ( CContainerUI* )m_pManager->FindSubControlByName( this, _T( "event_info_layout" ) ); 
			ASSERT( EventInfoLayout != NULL ); 

			for( i = 0; i < EventInfoLayout->GetItemCount(); i ++ )
			{
				Control = EventInfoLayout->GetItemAt( i ); 
				if( Control == NULL )
				{
					break; 
				}

				if( 0 != _tcscmp( Control->GetClass(), _T( "ListSubElementExUI" ) ) )
				{
					continue; 
				}

				ListSubItem = ( CListSubElementExUI* )Control; 

				{
					ListSubItem->set_x( ListInfo->rcColumn[ i ] ); 
				}
			}
		}while( FALSE );

		CListContainerElementUI::DoPaint( hDC, rcPaint ); 
	}

} //namespace DuiLib


