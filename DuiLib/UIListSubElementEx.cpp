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


