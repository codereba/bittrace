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
#include "UIListEx.h"
#include <math.h>

namespace DuiLib
{

#define SCROLL_ITEM_COUNT_DIVIDE 4

	CListExUI::CListExUI() : CListBaseUI(), 
		button_pressed( FALSE ), 
		previous_cursor( NULL )


	{
		m_pList = new CListBodyExUI(this);
		m_ListInfo.dwBkColor = 0xFFEEEEEE; 

		do 
		{
			if( m_pList == NULL )
			{
				break; 
			}
			CVerticalLayoutUI::AddItem(m_pList);
		}while( FALSE );
	}

	LPCTSTR CListExUI::GetClass() const
	{
		return UI_LIST_EX_CLASS_NAME; 
	}

	ULONG CListExUI::GetClassId() const
	{
		return UI_LIST_EX_CLASS_ID; 
	}

	UINT CListExUI::GetControlFlags() const
	{
		return CListBaseUI::GetControlFlags(); 
	}

	LPVOID CListExUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp( pstrName, UI_LIST_INTERFACE_EX_NAME ) == 0 )
		{
			return ( IListUI* )this; 
		}
		else if( _tcscmp( pstrName, UI_LIST_OWNER_INTERFACE_NAME ) == 0 )
		{
			return ( IListOwnerUI* )this; 
		}
		
		return CListBaseUI::GetInterface( pstrName ); 
	}

	TListInfoUI* CListExUI::GetListInfo()
	{
		return CListBaseUI::GetListInfo(); 
	}
	
	//int CListExUI::GetCurSel() const
	//{
	//	
	//}
	
	//bool CListExUI::SelectListItem(int iIndex)
	//{
	//	return false; 
	//}

	BOOLEAN CListExUI::ColumnWidthChanged()
	{
		return FALSE; 
	}

	int CListExUI::GetCurSel() const
	{
		return m_iCurSel; 
	}

	bool CListExUI::SelectListItem(int iIndex)
	{
		if( iIndex == m_iCurSel )
		{
			return true;
		}

		int iOldSel = m_iCurSel;
		// We should first unselect the currently selected item

		if( m_iCurSel >= 0 )
		{
			CControlUI* pControl = GetSubItemAt(m_iCurSel, UI_LIST_ELEMENT_CLASS_ID );

			if( pControl != NULL) 
			{
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
				if( pListItem != NULL ) 
				{
					if( pListItem->GetIndex() != m_iCurSel )
					{
						DBG_BP(); 
						return false; 
					}

					pListItem->Select(false);
				}
				else
				{
					DBG_BP(); 
				}
			}

			m_iCurSel = -1;
		}

		if( iIndex < 0 ) 
		{
			return false;
		}

		CControlUI* pControl = GetSubItemAt( iIndex, UI_LIST_ELEMENT_CLASS_ID );
		if( pControl == NULL ) 
		{
			DBG_BP(); 
			return false;
		}

		//if( ( ( CListContainerElementExUI* )pControl )->GetIndex() != iIndex )
		//{
		//	DBG_BP(); 
		//	return false; 
		//}

		if( !pControl->IsVisible() ) 
		{
			return false;
		}

		if( !pControl->IsEnabled() ) 
		{
			return false;
		}

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
		
		if( pListItem == NULL ) 
		{
			DBG_BP(); 
			return false;
		}

		if( pListItem->GetIndex() != iIndex )
		{
			DBG_BP(); 
			return false; 
		}

		m_iCurSel = iIndex;
		
		if( !pListItem->Select(true) ) 
		{
			m_iCurSel = -1;
			return false;
		}

		//EnsureVisible( m_iCurSel ); 

		//pControl->SetFocus();
		if( m_pManager != NULL ) 
		{
			m_pManager->SendNotify(this, _T("itemselect"), m_iCurSel, iOldSel);
		}

		return true;
	}

	//CListHeaderUI* CListExUI::GetHeader() const
	//{

	//}

	CListBodyExUI* CListExUI::GetListEx() const
	{
		return m_pList; 
	}

	//TListInfoUI* CListExUI::GetListInfo()
	//{

	//}

	CControlUI* CListExUI::GetItemAt(int iIndex) const
	{
		return __super::GetItemAt( iIndex ); 
	}

	//INT32 CListExUI::GetItemIndex(CControlUI* pControl) const
	//{

	//}

	//bool CListExUI::SetItemIndex(CControlUI* pControl, int iIndex)
	//{
	//}
	//
	INT32 CListExUI::GetItemCount() const
	{
		return __super::GetItemCount(); 
	}

	//bool CListExUI::AddItem(CControlUI* pControl)
	//{

	//}

	//bool CListExUI::AddItemAt(CControlUI* pControl, int iIndex)
	//{

	//}

	//bool CListExUI::RemoveItem(CControlUI* pControl)
	//{

	//}

	//bool CListExUI::RemoveItemAt(int iIndex)
	//{

	//}

	//void CListExUI::RemoveAllItem()
	//{

	//}

	double CalculateDelay( double state )
	{
		return pow( state, 2 );
	}


	//bool CListExUI::AddItem(CControlUI* pControl)
	//{
	//	bool ret; 
	//	ret = AddSubItem( pControl, pControl->GetClassId() ); 

	//	return ret; 
	//	//return __super::AddItem( pControl ); 
	//}

	//bool CListExUI::AddItemAt(CControlUI* pControl, int iIndex)
	//{
	//	bool ret; 
	//	ret = AddSubItemAt( pControl, iIndex, pControl->GetClassId() ); 

	//	return ret; 
	//	//return __super::AddItem( pControl ); 
	//}

	bool CListExUI::AddItem(CControlUI* pControl)
	{
		bool ret = true; 

		do 
		{
			// Override the Add() method so we can add items specifically to
			// the intended widgets. Headers are assumed to be
			// answer the correct interface so we can add multiple list headers.
			if( pControl->GetInterface(_T("ListHeader")) != NULL ) {
				if( m_pHeader != pControl && m_pHeader->GetItemCount() == 0 ) {
					CVerticalLayoutUI::RemoveItem(m_pHeader);
					m_pHeader = static_cast<CListHeaderUI*>(pControl);
				}
				m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
				ret = CVerticalLayoutUI::AddItemAt(pControl, 0);
				break; 
			}

			// We also need to recognize header sub-items
			if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) {
				ret = m_pHeader->AddItem(pControl);
				m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
				break; 
			}
			// The list items should know about us
			//IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
			//if( pListItem != NULL ) {
			//	pListItem->SetOwner(this);
			//	pListItem->SetIndex(GetItemCount());
			//}
			//return m_pList->AddItem(pControl);

			DBG_BP(); 
			ret = false; 
		}while( FALSE );

		return ret; 
	}

	bool CListExUI::AddItemAt(CControlUI* pControl, int iIndex)
	{
		bool ret = true; 

		do 
		{
			// Override the AddAt() method so we can add items specifically to
			// the intended widgets. Headers and are assumed to be
			// answer the correct interface so we can add multiple list headers.
			if( pControl->GetInterface(_T("ListHeader")) != NULL ) {
				if( m_pHeader != pControl && m_pHeader->GetItemCount() == 0 ) {
					CVerticalLayoutUI::RemoveItem(m_pHeader);
					m_pHeader = static_cast<CListHeaderUI*>(pControl);
				}
				m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
				ret = CVerticalLayoutUI::AddItemAt(pControl, 0);
				break; 
			}
			// We also need to recognize header sub-items
			if( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ) {
				ret = m_pHeader->AddItemAt(pControl, iIndex);
				m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
				break; 
			}

			ret = false; 
		}while( FALSE );

		DBG_BP(); 

		return ret; 
		//if (!m_pList->AddItemAt(pControl, iIndex)) return false;

		//// The list items should know about us
		//IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
		//if( pListItem != NULL ) {
		//	pListItem->SetOwner(this);
		//	pListItem->SetIndex(iIndex);
		//}

		//for(int i = iIndex + 1; i < m_pList->GetItemCount(); ++i) {
		//	CControlUI* p = m_pList->GetItemAt(i);
		//	pListItem = static_cast<IListItemUI*>(p->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
		//	if( pListItem != NULL ) {
		//		pListItem->SetItemIndex(i);
		//	}
		//}
		//if( m_iCurSel >= iIndex ) m_iCurSel += 1;
		//return true;
	}

	CControlUI* CListExUI::GetSubItemAt(int iIndex, ULONG ClassId ) const
	{
		LRESULT ret; 
		CControlUI *Control = NULL; 
		//ULONG ItemRealIndex; 
		INT32 i; 
		//ULONG ItemRealIndexEnd; 
		
		do 
		{
			//ItemRealIndex = m_pList->GetBeginDisplayItemRealIndex(); 
			//ItemRealIndexEnd = ItemRealIndex + m_pList->GetDisplayItemCount(); 

			//if( ( ULONG )iIndex < ItemRealIndex 
			//	|| ( ULONG )iIndex > ItemRealIndexEnd )
			//{
			//	DBG_BP(); 

			//	__Trace( _T( "the index (%u (%u-%u) ) of the sub item in cached list is overflow\n" ), iIndex, 
			//		ItemRealIndex, 
			//		ItemRealIndexEnd ); 

			//	break; 
			//}

			switch( ClassId )
			{
			case UI_LIST_ELEMENT_CLASS_ID:
			case UI_LIST_CONTAINER_ELEMENT_CLASS_ID:
				//ret = m_pList->GetDisplayItemAt( iIndex - ItemRealIndex, &Control ); 
				//if( ret != ERROR_SUCCESS )
				//{
				//}

				for( i = 0; ( ULONG )i < m_pList->GetItemShowCount(); i ++ )
				{
					ret = m_pList->GetDisplayItemAt( i, &Control ); 
					if( ret == ERROR_SUCCESS )
					{
						if( ( ( CListContainerElementExUI* )Control )->GetIndex() == iIndex )
						{
							break; 
						}
					}
					else
					{
						if( ret == ERROR_NOT_FOUND )
						{
							break; 
						}

						//ASSERT( FALSE ); 
					}
				}

				if( ( ULONG )i == m_pList->GetItemShowCount() )
				{
					Control = NULL; 
				}
				
				break; 
			default:
				break; 
			}
		}while( FALSE ); 

		return Control; 
	}
	
	INT32 CListExUI::GetSubItemIndex(CControlUI* pControl, ULONG ClassId ) const
	{
		switch( ClassId )
		{
		case UI_LIST_ELEMENT_CLASS_ID:
			//m_pList->GetCachedItemIndex( pControl ); 
			break; 
		default:
			break; 
		}

		return INVALID_ITEM_INDEX; 
	}

	bool CListExUI::SetSubItemIndex(CControlUI* pControl, int iIndex, ULONG ClassId  )
	{
		return false; 
	}

	int CListExUI::GetSubItemCount( ULONG ClassId ) const
	{
		INT32 ItemCount = -1; 

		switch( ClassId )
		{
		case UI_LIST_ELEMENT_CLASS_ID:
			//ItemCount = m_pList->Get
			break; 
		default:
			break; 
		}

		DBG_BP(); 
		return -1; 
	}

	bool CListExUI::AddSubItem(CControlUI* pControl, ULONG ClassId )
	{
		DBG_BP(); 
		return false; 
	}

	bool CListExUI::AddSubItemAt(CControlUI* pControl, int iIndex, ULONG ClassId )
	{
		DBG_BP(); 
		return false; 
	}

	bool CListExUI::RemoveSubItem(CControlUI* pControl, ULONG ClassId )
	{
		DBG_BP(); 
		return false; 
	}
	
	bool CListExUI::RemoveSubItemAt(int iIndex, ULONG ClassId )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG DisplayItemBegin; 
		ULONG DisplayItemEnd; 

		DBG_BP(); 

		do 
		{
			DisplayItemBegin = m_pList->GetBeginDisplayItemRealIndex(); 
			DisplayItemEnd = DisplayItemBegin + m_pList->GetItemShowCount(); 

			if( ( ULONG )iIndex < DisplayItemBegin 
				|| ( ULONG )iIndex > DisplayItemEnd )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			ret = m_pList->RemoveDisplayItem( DisplayItemBegin + iIndex ); 
		} while ( FALSE ); 

		return ( bool )( ret == ERROR_SUCCESS ); 
	}

	void CListExUI::RemoveSubAllItem( ULONG ClassId )
	{
		switch( ClassId )
		{
		case UI_LIST_HEADER_CLASS_ID:
		case UI_LIST_HEADER_ITEM_CLASS_ID:
			break; 
		default:
			{
				ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 

				m_iCurSel = -1;
				m_iExpandedItem = -1;
				m_pList->RemoveAllCachedItems(); 
			}
			break; 
		}
	}


	void CListExUI::EnsureVisible(int iIndex)
	{
		LRESULT ret; 
		SIZE ItemSize; 

		
		do 
		{
			if( m_iCurSel < 0 ) 
			{
				break; 
			}

			RECT rcItem; // = m_pList->GetItemAt(iIndex)->GetPos();
			RECT rcList = m_pList->GetPos();
			RECT rcListInset = m_pList->GetInset();

			ret = m_pList->GetItemDisplaySize( &ItemSize ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			rcList.left += rcListInset.left;
			rcList.top += rcListInset.top;
			rcList.right -= rcListInset.right;
			rcList.bottom -= rcListInset.bottom;

			rcItem.left = rcList.left; 
			rcItem.top = iIndex * ItemSize.cy; 
			rcItem.right = rcList.right; 
			rcItem.bottom = rcItem.top + ItemSize.cy; 

			CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
			if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) 
			{
				rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
			}

			//int iPos = m_pList->GetScrollPos().cy;

			if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) 
			{
				break; 
			}

			int dy = 0;
			if( rcItem.top < rcList.top ) 
			{
				dy = rcItem.top - rcList.top;
			}

			if( rcItem.bottom > rcList.bottom ) 
			{
				dy = rcItem.bottom - rcList.bottom;
			}

			Scroll( 0, dy );
		}while( FALSE );

		return; 
	}

	void CListExUI::Scroll(int dx, int dy)
	{
		do 
		{
			if( dx == 0 && dy == 0 )
			{
				break;
			}

			SIZE sz = m_pList->GetScrollPos();

			m_pList->SetScrollPos( CSize( sz.cx + dx, sz.cy + dy ) );
		}while( FALSE );
	}

	int CListExUI::GetChildPadding() const
	{
		return m_pList->GetChildPadding(); 
	}

	void CListExUI::SetChildPadding(int iPadding)
	{
		m_pList->SetChildPadding(iPadding);
	}

	//void CListExUI::SetItemFont(int index)
	//{

	//}

	//void CListExUI::SetItemTextStyle(UINT uStyle)
	//{

	//}

	//void CListExUI::SetItemTextPadding(RECT rc)
	//{

	//}

	//void CListExUI::SetItemTextColor(DWORD dwTextColor)
	//{

	//}

	//void CListExUI::SetItemBkColor(DWORD dwBkColor)
	//{

	//}

	//void CListExUI::SetItemBkImage(LPCTSTR pStrImage)
	//{

	//}

	//void CListExUI::SetAlternateBk(bool bAlternateBk)
	//{

	//}

	//void CListExUI::SetSelectedItemTextColor(DWORD dwTextColor)
	//{

	//}

	//void CListExUI::SetSelectedItemBkColor(DWORD dwBkColor)
	//{

	//}

	//void CListExUI::SetSelectedItemImage(LPCTSTR pStrImage)
	//{

	//}

	//void CListExUI::SetHotItemTextColor(DWORD dwTextColor)
	//{

	//}

	//void CListExUI::SetHotItemBkColor(DWORD dwBkColor)
	//{

	//}

	//void CListExUI::SetHotItemImage(LPCTSTR pStrImage)
	//{

	//}

	//void CListExUI::SetDisabledItemTextColor(DWORD dwTextColor)
	//{

	//}

	//void CListExUI::SetDisabledItemBkColor(DWORD dwBkColor)
	//{

	//}

	//void CListExUI::SetDisabledItemImage(LPCTSTR pStrImage)
	//{

	//}

	//void CListExUI::SetItemLineColor(DWORD dwLineColor)
	//{

	//}

	//bool CListExUI::IsItemShowHtml()
	//{

	//}

	//void CListExUI::SetItemShowHtml(bool bShowHtml = true)
	//{

	//}

	//RECT CListExUI::GetItemTextPadding() const
	//{

	//}

	//DWORD CListExUI::GetItemTextColor() const
	//{

	//}

	//DWORD CListExUI::GetItemBkColor() const
	//{

	//}
	//
	//LPCTSTR CListExUI::GetItemBkImage() const
	//{

	//}

	//bool CListExUI::IsAlternateBk() const
	//{

	//}

	//DWORD CListExUI::GetSelectedItemTextColor() const
	//{

	//}

	//DWORD CListExUI::GetSelectedItemBkColor() const
	//{

	//}

	//LPCTSTR CListExUI::GetSelectedItemImage() const
	//{

	//}

	//DWORD CListExUI::GetHotItemTextColor() const
	//{

	//}

	//DWORD CListExUI::GetHotItemBkColor() const
	//{

	//}

	//LPCTSTR CListExUI::GetHotItemImage() const
	//{

	//}

	//DWORD CListExUI::GetDisabledItemTextColor() const
	//{

	//}

	//DWORD CListExUI::GetDisabledItemBkColor() const
	//{

	//}

	//LPCTSTR CListExUI::GetDisabledItemImage() const
	//{

	//}

	//DWORD CListExUI::GetItemLineColor() const
	//{

	//}

	//void CListExUI::SetMultiExpanding(bool bMultiExpandable)
	//{

	//}

	//int CListExUI::GetExpandedItem() const
	//{
	//}

	bool CListExUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
	{
		return false; 
	}

	void CListExUI::SetPos(RECT rc)
	{
		INT32 HeaderHeight = 0; 
		//INT32 HeaderWidth; 

		//ASSERT( HeaderHeight != 0 ); 
		CControlUI::SetPos(rc);
		
		if( m_pHeader != NULL )
		{
			HeaderHeight = m_pHeader->GetFixedHeight(); 
			//HeaderWidth = m_pHeader->GetFixedWidth(); 
		}

		ASSERT( m_pList != NULL ); 

		m_pList->SetPos( CRect( rc.left, rc.top + HeaderHeight, rc.right, rc.bottom ) ); 

		do 
		{
			if( m_pHeader == NULL ) 
			{
				DBG_BP(); 
				//ASSERT( FALSE && "list header is null" ); 

				break; 
			}

			// Determine general list information and the size of header columns
			m_ListInfo.nColumns = MIN( m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS ); 
			// The header/columns may or may not be visible at runtime. In either case
			// we should determine the correct dimensions...

			if( !m_pHeader->IsVisible() ) 
			{
				for( int it = 0; it < m_pHeader->GetItemCount(); it++ ) 
				{
					static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
				}

				m_pHeader->SetPos(CRect(rc.left, 0, rc.right, 0));
			}
			else
			{
				m_pHeader->SetPos(CRect(rc.left, rc.top, rc.right, rc.top + HeaderHeight ) ); 
			}

		}while( FALSE );

		int iOffset = m_pList->GetScrollPos().cx;
		for( int i = 0; i < m_ListInfo.nColumns; i++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
			if( !pControl->IsVisible() ) 
			{
				continue;
			}

			if( pControl->IsFloat() ) 
			{
				continue;
			}

			RECT rcPos = pControl->GetPos();
			if( iOffset > 0 ) 
			{
				rcPos.left -= iOffset;
				rcPos.right -= iOffset;
				pControl->SetPos(rcPos);
			}

			m_ListInfo.rcColumn[i] = pControl->GetPos();
		}

		/*************************************************************
		list drawing performance factor:
		1.all item drawing.

		current direct ui library bug:
		1.must traverse all item in the container to set it space.when drawing container.
		don't care that item will not display.

		2.must traverse all item in the container to check whether it will 
		display.
		*************************************************************/
		if( !m_pHeader->IsVisible() ) 
		{
			for( int it = 0; it < m_pHeader->GetItemCount(); it++ ) 
			{
				static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
			}

			//m_pHeader->SetPos(CRect(rc.left, 0, rc.right, 0));
		}
		//else
		//{
		//	m_pHeader->SetPos(CRect(rc.left, rc.top, rc.right, rc.top + HeaderHeight ) ); 
		//}

		//if( m_bUpdateNeeded == true )
		//{
		//	m_bUpdateNeeded = false; 
		//}

		//CListBaseUI::SetPos( rc ); 
	}

	LRESULT CListExUI::ChangeSelectListItem( LONG Index ) 
	{ 
		LRESULT ret = ERROR_SUCCESS; 
		LONG DisplayItemIndex; 
		LONG DisplayItemLine; 
		SIZE ItemSize; 
		SIZE pos = { 0 }; 

		do 
		{
			ret = m_pList->GetItemDisplaySize( &ItemSize ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( ItemSize.cy <= 0 )
			{
				ret = ERROR_NOT_READY; 
				break; 
			}

			if( Index < 0 )
			{
				Index = 0; 
			}

			if( Index > m_pList->ItemRealCount )
			{
				Index = m_pList->ItemRealCount; 
			}

			DisplayItemLine = ( LONG )m_iCurSel - m_pList->GetBeginDisplayItemRealIndex(); 

			if( DisplayItemLine < 0 )
			{
				//DBG_BP(); 
				DisplayItemLine = ( m_pList->GetItemShowCount() / 2 ); 
			}

			DisplayItemIndex = Index - DisplayItemLine; 

			if( DisplayItemIndex < 0 )
			{
				DisplayItemIndex = 0; 
			}

			pos.cy = DisplayItemIndex * ItemSize.cy; 

			SetScrollPos( pos ); 

			m_pList->SetDisplayIndexFromRealIndex( DisplayItemIndex ); 

			SelectListItem( Index ); 
		}while( FALSE );

		return ret; 
	}

	void CListExUI::DoEvent(TEventUI& event)
	{
		BOOLEAN handled = FALSE; 

		do 
		{
			if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND )
			{
				if( m_pParent != NULL ) 
				{
					m_pParent->DoEvent( event ); 
				}
				else 
				{
					CListBaseUI::DoEvent( event ); 
				}

				break; 
			}

			if( event.Type == UIEVENT_SETFOCUS ) 
			{
				m_bFocused = true; 
				break; 
			}
			if( event.Type == UIEVENT_KILLFOCUS ) 
			{
				m_bFocused = false; 
				break; 
			}

			switch( event.Type )
			{
			case UIEVENT_KEYDOWN: 
				{
					LRESULT ret; 
					SIZE ItemSize; 

					ret = m_pList->GetItemDisplaySize( &ItemSize ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					if( ItemSize.cy <= 0 )
					{
						break; 
					}

					switch( event.chKey )
					{
					case 'W': 
					case 'w':
					case VK_UP: 
						handled = TRUE; 
						ChangeSelectListItem( m_iCurSel - 1 ); 
						break; 
					case 'S':
					case 's':
					case VK_DOWN: 
						handled = TRUE; 
						ChangeSelectListItem( m_iCurSel + 1 ); 
						break; 
					case 'A':
					case 'a':
					case VK_LEFT: 
						{
							SIZE sz = GetScrollPos(); 

#define LIST_ITEM_HORIZONTAL_MOVE_FACTOR 4
							handled = TRUE; 
							sz.cx += ( -ItemSize.cy * LIST_ITEM_HORIZONTAL_MOVE_FACTOR ); 

							if( sz.cx < 0 ) 
							{
								sz.cx = 0; 
							}
							else if( sz.cx > GetScrollRange().cx )
							{
								ASSERT( FALSE ); 
								sz.cx = GetScrollRange().cx; 
							}

							SetScrollPos( sz ); 
						}
						break; 
					case 'D':
					case 'd': 
					case VK_RIGHT: 
						{
							SIZE sz = GetScrollPos(); 

							handled = TRUE; 
							sz.cx += ( ItemSize.cy * LIST_ITEM_HORIZONTAL_MOVE_FACTOR ); 

							if( sz.cx < 0 ) 
							{
								ASSERT( FALSE ); 
								sz.cx = 0; 
							}
							else if( sz.cx > GetScrollRange().cx )
							{
								sz.cx = GetScrollRange().cx; 
							}

							SetScrollPos( sz ); 
						}
						break; 
					case VK_HOME: 
						{
							SIZE sz = GetScrollPos(); 

							handled = TRUE; 

							sz.cy = 0; 

							SetScrollPos( sz ); 
						}
						break; 
					case VK_END:
						{
							LONG DisplayCount; 
							SIZE sz = GetScrollPos(); 

							handled = TRUE; 

							DisplayCount = m_pList->GetItemShowCount(); 
							sz.cy = GetScrollRange().cy - ( ItemSize.cy * 1 ); 

							SetScrollPos( sz ); 
						}
						break; 
					case VK_RETURN:
						if( m_iCurSel != -1 ) 
						{
							CControlUI *Item; 

							Item = GetSubItemAt( m_iCurSel, UI_LIST_ELEMENT_CLASS_ID ); 

							if( Item != NULL )
							{
								Item->Activate(); 
							}
							else
							{
								DBG_BP(); 
							}
						}
						break; 
					case VK_PRIOR: 
						{
							LONG DisplayCount; 
							SIZE sz = GetScrollPos(); 

							handled = TRUE; 
							DisplayCount = m_pList->GetItemShowCount(); 

							sz.cy += -( ItemSize.cy * DisplayCount ); 
							if( sz.cx < 0 ) 
							{
								ASSERT( FALSE ); 
								sz.cx = 0; 
							}
							else if( sz.cx > GetScrollRange().cx )
							{
								sz.cx = GetScrollRange().cx; 
							}

							SetScrollPos( sz ); 
						}
						break; 
					case VK_NEXT:
						{
							LONG DisplayCount; 
							SIZE sz = GetScrollPos(); 

							handled = TRUE; 
							DisplayCount = m_pList->GetItemShowCount(); 

							sz.cy += ( ItemSize.cy * DisplayCount ); 
							if( sz.cx < 0 ) 
							{
								ASSERT( FALSE ); 
								sz.cx = 0; 
							}
							else if( sz.cx > GetScrollRange().cx )
							{
								sz.cx = GetScrollRange().cx; 
							}

							SetScrollPos( sz ); 
						}
						break; 
					default:
						{

						}
						break; 
					}
				}
				break; 
			case UIEVENT_SCROLLWHEEL: 
				{
					LRESULT ret; 
					SIZE ItemSize; 

					ret = m_pList->GetItemDisplaySize( &ItemSize ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					if( ItemSize.cy <= 0 )
					{
						break; 
					}

					/****************************************************
					BUG:当前滚动不流畅

					处理滚动不流畅的问题的方法：
					1.只有滚动的长度达到一定的度之后才加载数据，不利之处是加载需要时间（100多条需3秒，会有停顿感）
					2.使用专有的加载工作线程进行加载，每次滚动时判断是否正在加载数据，如果是在
					加载，则跳过，不进行加载.
					1.或者，如果正在加载，中断当前加载过程，开始加载新的数据。
					不利之处是细节处理复杂。

					滚动加速处理包括以下几个方面:
					1.数据文件格式要有直接定位功能，通过ID直接算出偏移。
					但同时也支持SQL访问，需要改善SQLITE实现机制。
					2.对UI加载，不使用中间层缓存区，不然会引入缓存处理的
					延迟。
					3.在写UI时加锁，在读UI时试加锁，如果加锁失败，马上做其它工作，这样界面有最好的灵活度。
					4.在读UI时，如果读的不完全，记录下来。在UI写完成后，通知读线程，读线程读（绘）出其余UI。

					以上WORK方式，处理机制复杂，WORKER之间没有独立性，同时需要在WM_PAINT后手动PAINT。

					再次分析，把最花时间的工作（读取数据库）抽出，由WORKER来做，将从数据库读取出的所有的缓存区，一次性的打给界面。
					这种方式，独立性高，处理机制简单。

					加载事件信息的效率可以从3个方面入手处理：
					1.sqlite的加载机制的效率，分析SQLITE STEP函数的细节，从文件结构入手，初步考虑FLAT文件直接偏移计算法。
					2.加载时机，方式的速度，使用WORKER来进行加载，需要时给WORKER发一个任务。
					3.将信息加载到界面的效率，数据与输出接口或存储区直接定位输出，去掉所有的中间环节。
					4.处理显示过程的效率，如不使用FINDCTRL NEED_UPDATE来定位重绘控制，而是将其放在链表中，直接取出。
					****************************************************/

					LONG DisplayCount; 
					BOOLEAN scrolled = FALSE; 
					LONG ScrollItemCount; 
					SIZE sz = GetScrollPos(); 

					DisplayCount = m_pList->GetItemShowCount(); 

					ASSERT( DisplayCount >= 0 ); 

					if( DisplayCount == 0 )
					{
						break; 
					}

					switch (LOWORD(event.wParam))
					{
					case SB_LINEUP: 
						{
							scrolled = TRUE; 
							ScrollItemCount = ( DisplayCount / SCROLL_ITEM_COUNT_DIVIDE ); 

							if( ScrollItemCount == 0 )
							{
								ScrollItemCount = 1; 
							}

							sz.cy += ( -ItemSize.cy * ScrollItemCount ); 
						}
						break;
					case SB_LINEDOWN: 
						//FIXED!!!
						scrolled = TRUE; 
						ScrollItemCount = ( DisplayCount / SCROLL_ITEM_COUNT_DIVIDE ); 

						if( ScrollItemCount == 0 )
						{
							ScrollItemCount = 1; 
						}
						sz.cy += ( ItemSize.cy * ScrollItemCount ); 
						break; 
					case SB_PAGEUP: 
						scrolled = TRUE; 
						sz.cy += ( -ItemSize.cy * DisplayCount ); 
						break; 
					case SB_PAGEDOWN: 
						scrolled = TRUE; 
						sz.cy += ( ItemSize.cy * DisplayCount ); 
						break; 
						//case SB_TOP: 
						//	scrolled = TRUE; 
						//	sz.cy = 0; 
						//	break; 
						//case SB_BOTTOM:
						//	scrolled = TRUE; 
						//	sz.cy = GetScrollRange().cy - ( LIST_ITEM_NORMAL_HEIGHT * DisplayCount ); 
						//	break; 
						//#define SB_LINEUP           0
						//#define SB_LINELEFT         0
						//#define SB_LINEDOWN         1
						//#define SB_LINERIGHT        1
						//#define SB_PAGEUP           2
						//#define SB_PAGELEFT         2
						//#define SB_PAGEDOWN         3
						//#define SB_PAGERIGHT        3
						//#define SB_THUMBPOSITION    4
						//#define SB_THUMBTRACK       5
						//#define SB_TOP              6
						//#define SB_LEFT             6
						//#define SB_BOTTOM           7
						//#define SB_RIGHT            7
						//#define SB_ENDSCROLL        8
					}

					if( sz.cy < 0 ) 
					{
						sz.cy = 0; 
					}
					else if( sz.cy > GetScrollRange().cy ) //- ItemSize.cy * DisplayCount )
					{
						sz.cy = GetScrollRange().cy; //- ItemSize.cy * DisplayCount; 
					}

					SetScrollPos( sz ); 
				}
				break; 
			case UIEVENT_BUTTONDOWN: 
				{
					HCURSOR cursor_hand; 

					pressed_pos.x = event.ptMouse.x; 
					pressed_pos.y = event.ptMouse.y; 

					pressed_scroll_pos = GetScrollPos(); 

					SetCapture( m_pManager->GetPaintWindow() ); 
					button_pressed = TRUE; 

					cursor_hand = ::LoadCursor( NULL, IDC_HAND ); 
					if( NULL == cursor_hand )
					{
						ASSERT( FALSE ); 
						break; 
					}

					previous_cursor = ::SetCursor( cursor_hand ); 				
				}
				break; 

			case UIEVENT_MOUSEMOVE:
				{
					LRESULT ret; 
					POINT released_pos; 
					SIZE moved_pos; 
					LONG DisplayCount; 
					SIZE ItemSize; 

					if( button_pressed == FALSE )
					{
						break; 
					}

					DisplayCount = m_pList->GetItemShowCount(); 

					ASSERT( DisplayCount >= 0 ); 

					if( DisplayCount == 0 )
					{
						break; 
					}

					ret = m_pList->GetItemDisplaySize( &ItemSize ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					if( ItemSize.cy <= 0 )
					{
						break; 
					}

					released_pos.x = event.ptMouse.x; 
					released_pos.y = event.ptMouse.y; 

					moved_pos = pressed_scroll_pos; 

					moved_pos.cx += -( released_pos.x - pressed_pos.x ); 
					moved_pos.cy += -( released_pos.y - pressed_pos.y ); 

					if( moved_pos.cy < 0 ) 
					{
						moved_pos.cy = 0; 
					}
					else if( moved_pos.cy > GetScrollRange().cy ) //- ItemSize.cy * DisplayCount )
					{
						moved_pos.cy = GetScrollRange().cy; // - ItemSize.cy * DisplayCount; 
					}

					if( moved_pos.cx < 0 ) 
					{
						moved_pos.cx = 0; 
					}
					else if( moved_pos.cx > GetScrollRange().cx )
					{
						moved_pos.cx = GetScrollRange().cx; 
					}

					SetScrollPos( moved_pos ); 
					handled = TRUE; 
				}
				break; 
			case UIEVENT_BUTTONUP:
				{
					ReleaseCapture(); 
					button_pressed = FALSE; 

					if( previous_cursor != NULL )
					{
						::SetCursor( previous_cursor ); 
					}
				}
				break; 
			default: 
				break; 
			}

			if( handled == TRUE )
			{
				break; 
			}

			CListBaseUI::DoEvent(event);
		}while( FALSE ); 
	}

	//IListCallbackUI* CListExUI::GetListCallback() const
	//{

	//}

	//void CListExUI::SetListCallback(IListCallbackUI* pCallback)
	//{

	//}

	//void CListExUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	//{
	//}

	//virtual IListHeaderCallbackUI *CListExUI::GetListHeaderCallback() const
	//{

	//}

	//virtual void CListExUI::SetListHeaderCallback( IListHeaderCallbackUI *pCallback )
	//{

	//}

	SIZE CListExUI::GetScrollPos() const
	{
		return m_pList->GetScrollPos();
	}

	SIZE CListExUI::GetScrollRange() const
	{
		return m_pList->GetScrollRange();
	}

	void CListExUI::SetScrollPos(SIZE szPos)
	{
		m_pList->SetScrollPos(szPos);
	}

	void CListExUI::LineUp()
	{
		m_pList->LineUp();
	}

	void CListExUI::LineDown()
	{
		m_pList->LineDown();
	}

	void CListExUI::PageUp()
	{
		m_pList->PageUp();
	}

	void CListExUI::PageDown()
	{
		m_pList->PageDown();
	}

	void CListExUI::HomeUp()
	{
		m_pList->HomeUp();
	}

	void CListExUI::EndDown()
	{
		m_pList->EndDown();
	}

	void CListExUI::LineLeft()
	{
		m_pList->LineLeft();
	}

	void CListExUI::LineRight()
	{
		m_pList->LineRight();
	}

	void CListExUI::PageLeft()
	{
		m_pList->PageLeft();
	}

	void CListExUI::PageRight()
	{
		m_pList->PageRight();
	}

	void CListExUI::HomeLeft()
	{
		m_pList->HomeLeft();
	}

	void CListExUI::EndRight()
	{
		m_pList->EndRight();
	}

	void CListExUI::EnableScrollBar(bool bEnableVertical/* = true*/, bool bEnableHorizontal /*= false*/)
	{
		m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
	}

	CScrollBarUI* CListExUI::GetVerticalScrollBar() const
	{
		return m_pList->GetVerticalScrollBar();
	}

	CScrollBarUI* CListExUI::GetHorizontalScrollBar() const
	{
		return m_pList->GetHorizontalScrollBar();
	}

	//BOOLEAN CListExUI::ColumnWidthChanged()
	//{

	//}
}

