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

namespace DuiLib
{

	/**********************************************************************************
	use cache to manager the all items in the container, must to do these things:
	1.all process sub control function need to modify,beause don't need procss the all 
	real items, and process the all display items is not needed.
	2.this way may lose some freadture of hat control the items feature detail perciesly.
	**********************************************************************************/

	//ULONG get_ring_item_index( ULONG index, ULONG item_count )
	//{
	//	return ( index % item_count ); 
	//}

	CListBodyUI::CListBodyUI(CListUI* pOwner) : m_pOwner(pOwner)
	{
		ASSERT(m_pOwner);
		SetInset(CRect(0,0,0,0));
	}

	void CListBodyUI::SetScrollPos(SIZE szPos)
	{
		int cx = 0;
		int cy = 0;

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
		{
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();

			m_pVerticalScrollBar->SetScrollPos(szPos.cy);

			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		{
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();

			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);

			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( cx == 0 && cy == 0 ) 
		{
			return;
		}

		RECT rcPos;

		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);

			if( !pControl->IsVisible() ) 
			{
				continue;
			}
			
			if( pControl->IsFloat() ) 
			{
				continue;
			}

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;

			pControl->SetPos(rcPos);
		}

		Invalidate();

		if( cx != 0 && m_pOwner ) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if( pHeader == NULL ) return;
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			pInfo->nColumns = MIN(pHeader->GetItemCount(), UILIST_MAX_COLUMNS);

			/*************************************************************************
			so slow performance feature.
			*************************************************************************/
			
			if( !pHeader->IsVisible() ) 
			{
				for( int it = 0; it < pHeader->GetItemCount(); it++ ) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
				}
			}

			for( int i = 0; i < pInfo->nColumns; i++ ) {
				CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
				if( !pControl->IsVisible() ) 
				{
					continue;
				}

				if( pControl->IsFloat() ) 
				{
					continue;
				}

				RECT rcPos = pControl->GetPos();
				rcPos.left -= cx;
				rcPos.right -= cx;
				pControl->SetPos(rcPos);
				
				pInfo->rcColumn[i] = pControl->GetPos();
			}

			if( !pHeader->IsVisible() ) 
			{
				for( int it = 0; it < pHeader->GetItemCount(); it++ ) 
				{
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
				}
			}
		}
	}

	void CListBodyUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cy == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

			RECT rcPadding = pControl->GetPadding();
			sz.cx = MAX(sz.cx, 0);
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX(cxNeeded, sz.cx);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * m_iChildPadding;

		if( m_pOwner ) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if( pHeader != NULL && pHeader->GetItemCount() > 0 ) {
				cxNeeded = MAX(0, pHeader->EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
			}
		}

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if( nAdjustables > 0 ) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		int iPosX = rc.left;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it2);
				continue;
			}

			RECT rcPadding = pControl->GetPadding();
			szRemaining.cy -= rcPadding.top;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if( sz.cy == 0 ) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if( iAdjustable == nAdjustables ) {
					sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
				} 
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

			RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
			pControl->SetPos(rcCtrl);

			iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
			cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
		}
		cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

		if( m_pHorizontalScrollBar != NULL ) {
			if( cxNeeded > rc.right - rc.left ) {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CListBodyUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	CControlUI* CListBodyExUI::GetItemAt(int iIndex) const
	{
		//return __super::GetItemAt( iIndex ); 

		//INT32 ItemIndex; 
		//INT32 ItemEndIndex; 

		//INT32 ScrollPos; 
		//INT32 ScrollRange; 
		//INT32 DisplayHeight; 
		//INT32 ItemDisplayCount; 

		//if( m_pVerticalScrollBar != NULL )
		//{
		//	ScrollPos = m_pVerticalScrollBar->GetScrollPos(); 
		//	ScrollRange = m_pVerticalScrollBar->GetScrollRange(); 
		//}
		//else
		//{
		//	ScrollPos = 0; 
		//	ScrollRange = 0; 
		//}

		//DisplayHeight = GetFixedHeight(); 
		//ScrollRange += DisplayHeight; 

		//if( ScrollRange == 0 )
		//{
		//	return __super::GetItemAt( iIndex ); 
		//}

		//ItemDisplayCount = DisplayHeight / kFriendListItemNormalHeight; 
		//ItemIndex = ( INT32 )( ( ( DOUBLE )ScrollPos / ( DOUBLE )ScrollRange ) * m_items.GetSize() ); 

		//ItemEndIndex = ItemIndex + ItemDisplayCount; 

		//if( ItemIndex <= iIndex && iIndex <= ItemEndIndex )
		//{
		//	return __super::GetItemAt( iIndex ); 
		//}

		//return ModalCtrl; 
		return NULL; 
	}

	int CListBodyExUI::GetItemIndex(CControlUI* pControl) const
	{
		//return __super::GetItemIndex( pControl ); 
		return 0; 
	}

	bool CListBodyExUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		return 0; 
		//return __super::SetItemIndex( pControl, iIndex ); 
	}

	int CListBodyExUI::GetSubItemCount( ULONG ClassId ) const
	{
		if( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) )
		{
			DBG_BP(); 
			//return 0; 
		}

		return 0; 
		//return __super::GetItemCount();
	}

	int CListBodyExUI::GetItemCount() const
	{
		return 0; 
		//return __super::GetItemCount();
	}


	//LRESULT CListBodyExUI::InitItemCache(void)
	//{
	//	return CCachedItemManager::InitItemCache(); 
	//}

	//long WINAPI CListBodyExUI::UninitItemCache(void)
	//{
	//	return CCachedItemManager::UninitItemCache(); 
	//}

	void CListBodyExUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	//INT32 CListBodyExUI::FillSubItemsContent(HDC hDC, const RECT& rcPaint)
	//{
	//	INT32 Ret = 0; 
	//	ULONG ItemCount; 
	//	ULONG RealItemIndex; 
	//	//ULONG i; 

	//	do 
	//	{
	//		if( m_pOwner == NULL )
	//		{
	//			Ret = ERROR_ERRORS_ENCOUNTERED; 
	//			break; 
	//		}

	//		ItemCount = m_pOwner->GetRealItemsCount(); 
	//		RealItemIndex = 0; 

	//		//for( i = 0; i < ItemCount; i ++ )
	//		{
	//			m_pOwner->FillSubItemsContent( RealItemIndex, ItemCount ); 
	//		}

	//	}while( FALSE );

	//	return Ret; 
	//}

	//BOOLEAN LoadItemCached = FALSE; 

	//void CListBodyExUI::SetPos(RECT rc)
	//{
	//	CControlUI::SetPos(rc);

	//	rc = m_rcItem;

	//	// Adjust for inset
	//	rc.left += m_rcInset.left;
	//	rc.top += m_rcInset.top;
	//	rc.right -= m_rcInset.right;
	//	rc.bottom -= m_rcInset.bottom;
	//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

	//	// Determine the minimum size
	//	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	//		szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

	//	int cxNeeded = 0;
	//	int nAdjustables = 0;
	//	int cyFixed = 0;
	//	int nEstimateNum = 0;
	//	//for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
	//	//	CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
	//	//	if( !pControl->IsVisible() ) continue;
	//	//	if( pControl->IsFloat() ) continue;
	//	//	SIZE sz = pControl->EstimateSize(szAvailable);
	//	//	if( sz.cy == 0 ) {
	//	//		nAdjustables++;
	//	//	}
	//	//	else {
	//	//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
	//	//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
	//	//	}
	//	//	cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

	//	//	//RECT rcPadding = pControl->GetPadding();
	//	//	sz.cx = MAX(sz.cx, 0);
	//	//	if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
	//	//	if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
	//	//	cxNeeded = MAX(cxNeeded, sz.cx);
	//	//	nEstimateNum++;
	//	//}
	//	cyFixed += (nEstimateNum - 1) * m_iChildPadding;

	//	if( m_pOwner ) {
	//		CListHeaderUI* pHeader = m_pOwner->GetHeader();
	//		if( pHeader != NULL && pHeader->GetItemCount() > 0 ) {
	//			cxNeeded = MAX(0, pHeader->EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
	//		}
	//	}

	//	// Place elements
	//	int cyNeeded = 0;
	//	int cyExpand = 0; 
	//	LRESULT ret; 

	//	SIZE ContentSize; 

	//	if( CacheManager ) 
	//	{
	//		ret = GetRealContentSize( &ContentSize ); 
	//	}
	//	else
	//	{
	//		ASSERT( FALSE && "the cache manager for the list is null" ); 
	//		ret = ERROR_ERRORS_ENCOUNTERED; 
	//	}

	//	if( ret != ERROR_SUCCESS )
	//	{
	//		if( nAdjustables > 0 ) 
	//		{
	//			cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
	//		}

	//		// Position the elements
	//		SIZE szRemaining = szAvailable;
	//		int iPosY = rc.top;
	//		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
	//			iPosY -= m_pVerticalScrollBar->GetScrollPos();
	//		}
	//		int iPosX = rc.left;
	//		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
	//			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
	//		}
	//		int iAdjustable = 0;
	//		int cyFixedRemaining = cyFixed;
	//		//for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
	//		//	CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
	//		//	if( !pControl->IsVisible() ) continue;
	//		//	if( pControl->IsFloat() ) {
	//		//		SetFloatPos(it2);
	//		//		continue;
	//		//	}

	//		//	RECT rcPadding = pControl->GetPadding();
	//		//	szRemaining.cy -= rcPadding.top;
	//		//	SIZE sz = pControl->EstimateSize(szRemaining);
	//		//	if( sz.cy == 0 ) {
	//		//		iAdjustable++;
	//		//		sz.cy = cyExpand;
	//		//		// Distribute remaining to last element (usually round-off left-overs)
	//		//		if( iAdjustable == nAdjustables ) {
	//		//			sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
	//		//		} 
	//		//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
	//		//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
	//		//	}
	//		//	else {
	//		//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
	//		//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
	//		//		cyFixedRemaining -= sz.cy;
	//		//	}

	//		//	sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

	//		//	if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
	//		//	if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

	//		//	RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
	//		//	pControl->SetPos(rcCtrl);

	//		//	iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
	//		//	cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
	//		//	szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
	//		//}
	//		cyNeeded += (nEstimateNum - 1) * m_iChildPadding;
	//	}
	//	else
	//	{
	//		//ULONG VertPos; 
	//		cyNeeded = ContentSize.cy; 
	//	}

	//	if( m_pHorizontalScrollBar != NULL ) {
	//		if( cxNeeded > rc.right - rc.left ) {
	//			if( m_pHorizontalScrollBar->IsVisible() ) {
	//				m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
	//			}
	//			else {
	//				m_pHorizontalScrollBar->SetVisible(true);
	//				m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
	//				m_pHorizontalScrollBar->SetScrollPos(0);
	//				rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
	//			}
	//		}
	//		else {
	//			if( m_pHorizontalScrollBar->IsVisible() ) {
	//				m_pHorizontalScrollBar->SetVisible(false);
	//				m_pHorizontalScrollBar->SetScrollRange(0);
	//				m_pHorizontalScrollBar->SetScrollPos(0);
	//				rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
	//			}
	//		}
	//	}

	//	// Process the scrollbar
	//	ProcessScrollBar(rc, cxNeeded, cyNeeded);
	//}

//	void CListBodyExUI::DoPaint(HDC hDC, const RECT& rcPaint)
//	{
//		RECT rcTemp = { 0 };
//		LRESULT ret; 
//
//		do 
//		{
//			if(   !::IntersectRect( &rcTemp, &rcPaint, &m_rcItem ) )
//			{
//				break; 
//			}
//
//			CRenderClip clip;
//			CRenderClip::GenerateClip( hDC, rcTemp, clip );
//
//			TRACE( _T( "enter %s" ), __TFUNCTION__ ); 
//
//			//dump_contro_ui_info( this ); 
//
//			//TRACE( _T( "begin paint container %s" ), __TFUNCTION__ ); 
//
//			CControlUI::DoPaint(hDC, rcPaint);
//
//			register INT32 ItemCount; 
//			register CControlUI *pControl; 
//			SIZE ItemSize; 
//
//			ret = GetDisplayItemSize( &ItemSize ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			ItemCount = GetDisplayCount(); 
//
//			if( ItemCount == INVALID_ITEM_COUNT )
//			{
//				break; 
//			}
//
//			if( ItemCount > 0 ) 
//			{
//				RECT rc = m_rcItem;
//				rc.left += m_rcInset.left;
//				rc.top += m_rcInset.top;
//				rc.right -= m_rcInset.right;
//				rc.bottom -= m_rcInset.bottom;
//
//				if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
//				{
//					rc.right -= m_pVerticalScrollBar->GetFixedWidth();
//				}
//
//				if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
//				{
//					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
//				}
//
//				if( !::IntersectRect( &rcTemp, &rcPaint, &rc ) ) 
//				{
//					break; 
//
//					//				for( int it = 0; it < ItemCount; it++ ) 
//					//				{
//					//					pControl = static_cast< CControlUI* >( GetItemAt( it ) ); 
//					//
//					//					if( pControl == NULL )
//					//					{
//					//						__Trace( _T("why sub control %u is null??" ), it ); 
//					//					}
//					//
//					//					if( pControl->IsFloat() ) 
//					//					{
//					//						continue; 
//					//					}
//					//
//					//#ifdef _DEBUG
//					//					DebugControlPaint( pControl ); 
//					//#endif //_DEBUG
//					//
//					//					//if( !pControl->IsVisible() ) continue;
//					//					
//					//					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) 
//					//					{
//					//						continue;
//					//					}
//					//
//					//		
//					//					{
//					//						if( !::IntersectRect( &rcTemp, &m_rcItem, &pControl->GetPos()) ) 
//					//						{
//					//							continue;
//					//						}
//					//
//					//						pControl->DoPaint (hDC, rcPaint );
//					//					}
//					//				}
//				}
//
//
//				RECT rcDisplay; 
//				CRenderClip childClip;
//				CRenderClip::GenerateClip(hDC, rcTemp, childClip);
//				for( int it = 0; it < ItemCount; it++ ) 
//				{
//					ret = GetDisplayItem( it, &pControl ); 
//					if( ret != ERROR_SUCCESS )
//					{
//						__Trace( _T( "get the display item %u error\n" ), it ); 
//						continue; 
//					}
//
//					if( pControl == NULL )
//					{
//						__Trace( _T("why sub control %u is null??" ), it ); 
//					}
//
//					//CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
//#ifdef _DEBUG
//					DebugControlPaint( pControl ); 
//#endif //_DEBUG
//
//					//if( !pControl->IsVisible() ) continue;
//					//if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
//					//if( pControl ->IsFloat() )
//					//{
//					//	if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
//					//	CRenderClip::UseOldClipBegin(hDC, childClip);
//					//	pControl->DoPaint(hDC, rcPaint);
//					//	CRenderClip::UseOldClipEnd(hDC, childClip);
//					//}
//					//else {
//
//					//if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) 
//					//{
//					//	continue; 
//					//}
//
//					/************************************************************************
//					drawing the dislay control process:
//					1.calculate teh display item region, load it from the database.
//					2.raverse all display item from the display cache.
//					3.drawing it from the container top to the container bottom.
//					************************************************************************/
//
//					rcDisplay.top = rcTemp.top + ItemSize.cy * it; 
//					rcDisplay.bottom = rcDisplay.top + ItemSize.cy; 
//
//					rcDisplay.left = m_rcItem.left; 
//					rcDisplay.right = rcDisplay.left + m_rcItem.right - m_rcItem.left; 
//
//					ret = SetDisplayPos( pControl, &rcDisplay ); 
//					if( ret != ERROR_SUCCESS )
//					{
//						__Trace( _T("") ); 
//						continue; 
//					}
//
//					pControl->DoPaint(hDC, rcPaint);
//				}
//			}
//
//			if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) 
//			{
//				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) 
//				{
//					m_pVerticalScrollBar->DoPaint(hDC, rcPaint);
//				}
//			}
//
//			if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) 
//			{
//				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) 
//				{
//					m_pHorizontalScrollBar->DoPaint(hDC, rcPaint);
//				}
//			}
//		}while( FALSE ); 
//	
//		return; 
//	}

	void CListBodyExUI::SetScrollPos(SIZE szPos)
	{
		LRESULT ret = ERROR_SUCCESS; 
		INT32 cx = 0;
		INT32 cy = 0;
		INT32 OldPosX; 
		INT32 OldPosY; 

		do
		{
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			{
				OldPosY = m_pVerticalScrollBar->GetScrollPos(); 
				m_pVerticalScrollBar->SetScrollPos( szPos.cy ); 
				cy = m_pVerticalScrollBar->GetScrollPos() - OldPosY; 
			}

			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			{
				OldPosX = m_pHorizontalScrollBar->GetScrollPos(); 
				m_pHorizontalScrollBar->SetScrollPos(szPos.cx); 
				cx = m_pHorizontalScrollBar->GetScrollPos() - OldPosX; 
			}

			//if( CacheManager != NULL )
			if( cy != 0 )
			{
				ULONG Height; 
				Height = GetScrollRange().cy + GetFixedHeight(); 

				OnScrollPosChanged( szPos.cx, szPos.cy, Height ); 
			}
			//else
			//{
			//	ASSERT( FALSE && "cache manager is null" ); 
			//}

			//if( cx == 0 && cy == 0 ) 
			//{
			//	return;
			//}

			if( cx == 0 ) 
			{
				break; 
			}

			do 
			{
				RECT rcPos;

				//if( cx == 0 )
				//{
				//	break; 
				//}

				//for( int it2 = 0; it2 < ItemDisplayCount; it2++ ) 
				//{
				//	CControlUI* pControl; 

				//	ret = GetDisplayItemAt( it2, &pControl ); 
				//	if( ret != ERROR_SUCCESS )
				//	{
				//		continue; 
				//	}

				//	ASSERT( pControl != NULL ); 

				//	if( !pControl->IsVisible() ) 
				//	{
				//		continue; 
				//	}

				//	if( pControl->IsFloat() ) 
				//	{
				//		continue; 
				//	}

				//	rcPos = pControl->GetPos();

				//	rcPos.left -= cx;
				//	rcPos.right -= cx;
				//	//rcPos.top -= cy;
				//	//rcPos.bottom -= cy;

				//	pControl->SetPos( rcPos );
				//}

				Invalidate();

				if( NULL == m_pOwner ) 
				{
					break; 
				}

				CListHeaderUI* pHeader = m_pOwner->GetHeader();
				if( pHeader == NULL ) 
				{
					DBG_BP(); 
					break; 
				}

				TListInfoUI* pInfo = m_pOwner->GetListInfo();
				pInfo->nColumns = MIN( pHeader->GetItemCount(), UILIST_MAX_COLUMNS );

				if( !pHeader->IsVisible() ) 
				{
					for( int it = 0; it < pHeader->GetItemCount(); it++ ) 
					{
						static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
					}
				}

				for( int i = 0; i < pInfo->nColumns; i++ ) 
				{
					CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
					if( !pControl->IsVisible() ) 
					{
						continue;
					}

					if( pControl->IsFloat() ) 
					{
						continue;
					}

					rcPos = pControl->GetPos();
					rcPos.left -= cx;
					rcPos.right -= cx;
					pControl->SetPos( rcPos ); 
					pInfo->rcColumn[ i ] = pControl->GetPos();
				}

				if( !pHeader->IsVisible() ) 
				{
					for( int it = 0; it < pHeader->GetItemCount(); it++ ) 
					{
						static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
					}
				}

			}while( FALSE ); 
		}while( FALSE ); 

	}

	void CListBodyExUI::SetPos(RECT rc)
	{
		//INT32 ret; 
		LRESULT _ret; 
		ULONG DisplayItemCount; 
		RECT ItemRc; 
		RECT rcTemp; 
		SIZE ItemSize; 
		
		/**********************************************************************
		the cached item containter layout functions need to do 3 process.
		1.recalculate the real items size, and set the scroll region.
		2.calculate the scroll pos and item display index.
		3.load the display item to the cache for display.
		**********************************************************************/

		do 
		{
			//SIZE ScrollPos = { 0 }; 

			CControlUI::SetPos(rc); 

			register INT32 RealItemCount; 

			//ASSERT( CacheManager != NULL ); 
			RealItemCount = GetRealItemsCount(); 

			if( RealItemCount == 0 )
			{
				ProcessScrollBar(rc, 0, 0);
				break; 
			}

			//ret = GetItemDisplaySize( &display_size ); 
			_ret = GetItemDisplaySize( &ItemSize ); 
			if( _ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( ItemSize.cy <= 0 
				|| ItemSize.cx <= 0 )
			{
				DBG_BP(); 
				break; 
			}

			rc = m_rcItem; 

			//ScrollPos = GetScrollPos(); 
			
			// Adjust for inset
			rc.left += m_rcInset.left;
			rc.top += m_rcInset.top;
			rc.right -= m_rcInset.right;
			rc.bottom -= m_rcInset.bottom;

			//是否在这个函数进行滚动相关的处理
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			{
				rc.right -= m_pVerticalScrollBar->GetFixedWidth();
				//ScrollPos.cy = m_pVerticalScrollBar->GetScrollPos(); 
			}

			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			{
				rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				//ScrollPos.cx = m_pHorizontalScrollBar->GetScrollPos(); 
			}

			{
				LONG list_width; 
				LONG list_height; 
				LONG item_height; 
				//RECT list_body_rc; 
				//CControlUI *ctrl; 
				//CScrollBarUI *scroll_bar; 
				//SIZE display_size; 

				do 
				{
					list_width = rc.right - rc.left; 

#define LIST_BODY_SMALL_WIDTH 760
					if( list_width < LIST_BODY_SMALL_WIDTH )
					{
						list_width = LIST_BODY_SMALL_WIDTH; 
					}

					//scroll_bar = GetHorizontalScrollBar(); 
					//if( scroll_bar != NULL )
					//{
					//	scroll_bar->SetScrollRange( display_size.cx - list_width ); 
					//}

					//#ifdef _UI_DEBUG
					//				ctrl = m_pManager->FindControl( _T( "friends" ) ); 
					//#else
					//				ctrl = m_pManager->FindControl( _T( "event_list_layout" ) ); 
					//#endif //_UI_DEBUG
					//
					list_height = rc.bottom - rc.top; 

#define LIST_BODY_SMALL_HEIGHT 350 
					if( list_height < LIST_BODY_SMALL_HEIGHT )
					{
						list_height = LIST_BODY_SMALL_HEIGHT; 
					}

					item_height = ItemSize.cy; 

					ItemDisplayCount = list_height / item_height; 
					if( ItemDisplayCount == 0 )
					{
						DBG_BP(); 
						//ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}
				}while( FALSE ); 
			}

			/**********************************************************************
			real item count is used to set the scroll region,and calculates the 
			scroll position to the item index.which will being display.
			**********************************************************************/

			// Determine the minimum size
			//SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

			//if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			//{
			//	szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
			//}

			//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			//{
			//	szAvailable.cx += m_pVerticalScrollBar->GetScrollPos();
			//}

			SIZE AllRealItemSize; 
			_ret = GetRealContentSize( &AllRealItemSize ); 
			if( _ret != ERROR_SUCCESS )
			{
				break; 
			}

			//int nAdjustables = 0;
			//int cyFixed = 0;
			//int nEstimateNum = 0;
			//register CControlUI *pControl; 

			//for( int it1 = 0; it1 < ItemCount; it1++ ) {
			//	pControl = static_cast<CControlUI*>(GetItemAt( it1 ) ); 

			//	if( !pControl->IsVisible() ) continue;
			//	if( pControl->IsFloat() ) continue;
			//	SIZE sz = pControl->EstimateSize(szAvailable);
			//	if( sz.cy == 0 ) {
			//		nAdjustables++;
			//	}
			//	else {
			//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			//	}
			//	cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;
			//	nEstimateNum++;
			//}
			//cyFixed += (nEstimateNum - 1) * m_iChildPadding;

			//// Place elements
			//int cyNeeded = 0;
			//int cyExpand = 0;
			//if( nAdjustables > 0 ) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
			//// Position the elements
			//SIZE szRemaining = szAvailable;
			//int iPosY = rc.top;
			//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			//	iPosY -= m_pVerticalScrollBar->GetScrollPos();
			//}
			//int iPosX = rc.left;
			//if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			//	iPosX -= m_pHorizontalScrollBar->GetScrollPos();
			//}
			//int iAdjustable = 0;
			//int cyFixedRemaining = cyFixed;

			//for( int it2 = 0; it2 < ItemCount; it2++ ) {
			//	pControl = static_cast<CControlUI*>(GetItemAt(it2));
			//	if( !pControl->IsVisible() ) continue;
			//	if( pControl->IsFloat() ) {
			//		SetFloatPos(it2);
			//		continue;
			//	}

			//	RECT rcPadding = pControl->GetPadding();
			//	szRemaining.cy -= rcPadding.top;
			//	SIZE sz = pControl->EstimateSize(szRemaining);
			//	if( sz.cy == 0 ) {
			//		iAdjustable++;
			//		sz.cy = cyExpand;
			//		// Distribute remaining to last element (usually round-off left-overs)
			//		if( iAdjustable == nAdjustables ) {
			//			sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
			//		} 
			//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			//	}
			//	else {
			//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			//		cyFixedRemaining -= sz.cy;
			//	}

			//	sz.cx = pControl->GetFixedWidth();
			//	if( sz.cx == 0 ) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
			//	if( sz.cx < 0 ) sz.cx = 0;
			//	if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			//	if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

			//	RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
			//	pControl->SetPos(rcCtrl);

			//	iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
			//	cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			//	szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
			//}
			//cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

//			{
//				LONG list_width; 
//				LONG list_height; 
//				LONG item_height; 
//				//RECT list_body_rc; 
//				//CControlUI *ctrl; 
//				//CScrollBarUI *scroll_bar; 
//				//SIZE display_size; 
//
//				do 
//				{
//					//ret = GetItemDisplaySize( &display_size ); 
//					ret = GetItemDisplaySize( &ItemSize ); 
//					if( ret != ERROR_SUCCESS )
//					{
//						break; 
//					}
//#ifdef _DEBUG
//					if( ItemSize.cy <= 0 
//						|| ItemSize.cx <= 0 )
//					{
//						DBG_BP(); 
//						break; 
//					}
//#endif //_DEBUG
//					list_width = rc.right - rc.left; 
//
//#define LIST_BODY_SMALL_WIDTH 760
//					if( list_width < LIST_BODY_SMALL_WIDTH )
//					{
//						list_width = LIST_BODY_SMALL_WIDTH; 
//					}
//
//					//scroll_bar = GetHorizontalScrollBar(); 
//					//if( scroll_bar != NULL )
//					//{
//					//	scroll_bar->SetScrollRange( display_size.cx - list_width ); 
//					//}
//
//					//#ifdef _UI_DEBUG
//					//				ctrl = m_pManager->FindControl( _T( "friends" ) ); 
//					//#else
//					//				ctrl = m_pManager->FindControl( _T( "event_list_layout" ) ); 
//					//#endif //_UI_DEBUG
//					//
//					list_height = rc.bottom - rc.top; 
//
//#define LIST_BODY_SMALL_HEIGHT 350 
//					if( list_height < LIST_BODY_SMALL_HEIGHT )
//					{
//						list_height = LIST_BODY_SMALL_HEIGHT; 
//					}
//
//					item_height = ItemSize.cy; 
//
//					ItemDisplayCount = list_height / item_height; 
//					if( ItemDisplayCount == 0 )
//					{
//						DBG_BP(); 
//						//ret = ERROR_ERRORS_ENCOUNTERED; 
//						break; 
//					}
//				}while( FALSE ); 
//			}

			DisplayItemCount = GetItemShowCount(); 

			//ASSERT( DisplayItemCount > 0 ); 

			for( ULONG it = 0; it < DisplayItemCount; it++ ) 
			{
				CControlUI* pControl = NULL; 

				_ret = GetDisplayItemAt( it, &pControl ); 
				if( _ret == ERROR_SUCCESS )
				{
					//#ifdef _DEBUG
					//					DebugControlPaint( pControl ); 
					//#endif //_DEBUG

					ASSERT( pControl != NULL ); 

					if( !pControl->IsVisible() ) 
					{
						continue;
					}

					//ItemSize.cy = 12;
					ItemRc.left = this->GetPadding().left + rc.left; 
					ItemRc.right = ItemRc.left + ItemSize.cx; 
					ItemRc.top = it * ItemSize.cy + rc.top; 
					ItemRc.bottom = ItemRc.top + ItemSize.cy; 

					if( !::IntersectRect(&rcTemp, &m_rcItem, &ItemRc ) ) 
					{
						continue;
					}

					if( pControl->IsFloat() ) 
					{
					}
					else 
					{
						//if( !::IntersectRect(&rcTemp, &rc, &ItemRc ) ) 
						//{
						//	continue;
						//}

						//pControl->SetManager( NULL, NULL, false ); 

						pControl->SetPos( rcTemp ); 

						//__Trace( _T( "set the sub control position result:\n" ) ); 

						//( ( CContainerUI* )pControl )->DumpSubItemPos( _T( "time" ) ); 
						//( ( CContainerUI* )pControl )->DumpSubItemPos( _T( "event_desc" ) ); 

						//pControl->SetPosNoLayout( rcTemp ); 

						//( ( CListContainerElementUI* )pControl )->SetIndex( it ); 

						//pControl->DoPaint(hDC, rcTemp);
					}
				}
			}

			//if( m_pOwner == NULL )
			//{
			//	break; 
			//}

			//	CListHeaderUI *pHeader; 
			//	TListInfoUI *pInfo; 

			//	pHeader = m_pOwner->GetHeader(); 
			//	pInfo = m_pOwner->GetListInfo(); 

			//	ASSERT( pHeader != NULL ); 
			//	ASSERT( pInfo != NULL ); 

			//	if( !pHeader->IsVisible() ) 
			//	{
			//		for( int it = 0; it < pHeader->GetItemCount(); it++ ) {
			//			static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
			//		}
			//	}

			//	for( int i = 0; i < pInfo->nColumns; i++ ) {
			//		CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
			//		if( !pControl->IsVisible() ) 
			//		{
			//			continue;
			//		}

			//		if( pControl->IsFloat() ) 
			//		{
			//			continue;
			//		}

			//		RECT rcPos = pControl->GetPos();
			//		rcPos.left -= cx;
			//		rcPos.right -= cx;
			//		pControl->SetPos(rcPos);

			//		pInfo->rcColumn[i] = pControl->GetPos();
			//	}

			//	if( !pHeader->IsVisible() ) 
			//	{
			//		for( int it = 0; it < pHeader->GetItemCount(); it++ ) 
			//		{
			//			static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
			//		}
			//	}
			//}

			//ULONG cy; 
			//cy = GetScrollRange().cy + GetFixedHeight(); 

			//_ret = OnScrollPosChanged( ScrollPos.cx, cy, ScrollPos.cy ); 
			//if( _ret != ERROR_SUCCESS )
			//{
			//	ret = ( INT32 )_ret; 
			//	__Trace( _T( "set the scroll pos to cache manager error %u\n" ), ret ); 
			//}

			//FillSubItemsContent( )

			// Process the scrollbar
			ProcessScrollBar(rc, AllRealItemSize.cx, AllRealItemSize.cy );
		}while( FALSE ); 
	}

	//bool CListBodyExUI::RemoveItem(CControlUI* pControl)
	//{
	//	if( pControl == NULL) return false;

	//	for( int it = 0; it < m_items.GetSize(); it++ ) {
	//		if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
	//			NeedUpdateNoInvalidate();
	//			if( m_bAutoDestroy ) {
	//				if( m_bDelayedDestroy && m_pManager ) m_pManager->AddDelayedCleanup(pControl);             
	//				else delete pControl;
	//			}
	//			return m_items.Remove(it);
	//		}
	//	}
	//	return false;
	//}

	//bool CListBodyExUI::AddItem(CControlUI* pControl)
	//{
	//	if( pControl == NULL) return false;

	//	if( m_pManager != NULL ) m_pManager->InitControls(pControl, this);
	//	if( IsVisible() ) NeedUpdateNoInvalidate();
	//	else pControl->SetInternVisible(false);
	//	return m_items.Add(pControl);   
	//}

	//bool CListBodyExUI::AddAtNoUpdate(CControlUI* pControl, int iIndex)
	//{
	//	if( pControl == NULL) return false;

	//	if( m_pManager != NULL ) m_pManager->InitControls(pControl, this);
	//	if( IsVisible() ) NeedUpdateNoInvalidate();
	//	else pControl->SetInternVisible(false);
	//	return m_items.InsertAt(iIndex, pControl);
	//}

	//bool CListBodyExUI::RemoveAtNoUpdate(int iIndex)
	//{
	//	CControlUI* pControl = GetItemAt(iIndex);
	//	if (pControl != NULL) {
	//		//return CContainerUI::RemoveNoUpdate(pControl);
	//		ASSERT( static_cast<CControlUI*>( m_items[ iIndex ] ) == pControl ); 
	//		NeedUpdateNoInvalidate();
	//		if( m_bAutoDestroy ) {
	//			if( m_bDelayedDestroy && m_pManager ) m_pManager->AddDelayedCleanup(pControl);             
	//			else delete pControl;
	//		}
	//		return m_items.Remove(iIndex);
	//	}

	//	return false;
	//}

	//void CVerticalLayoutUI::SetPos(RECT rc)
	//{
	//	CControlUI::SetPos(rc);
	//	rc = m_rcItem;

	//	// Adjust for inset, sub items display region
	//	rc.left += m_rcInset.left;
	//	rc.top += m_rcInset.top;
	//	rc.right -= m_rcInset.right;
	//	rc.bottom -= m_rcInset.bottom;

	//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
	//	{
	//		rc.right -= m_pVerticalScrollBar->GetFixedWidth();
	//	}
	//	
	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	//	{
	//		rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
	//	}

	//	register INT32 ItemCount; 

	//	ItemCount = GetItemCount(); 

	//	if( ItemCount == 0) {
	//		ProcessScrollBar( rc, 0, 0 ); 
	//		return;
	//	}

	//	// Determine the minimum size
	//	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top }; 

	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	//	{
	//		szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
	//	}

	//	int nAdjustables = 0;
	//	int cyFixed = 0;
	//	int nEstimateNum = 0;
	//	register CControlUI *pControl; 

	//	for( int it1 = 0; it1 < ItemCount; it1++ ) {
	//		pControl = static_cast< CControlUI* >( GetItemAt( it1 ) ); 

	//		if( !pControl->IsVisible() ) 
	//		{
	//			continue;
	//		}

	//		if( pControl->IsFloat() ) 
	//		{
	//			continue;
	//		}

	//		SIZE sz = pControl->EstimateSize(szAvailable);
	//		if( sz.cy == 0 )
	//		{
	//			nAdjustables++;
	//		}
	//		else 
	//		{
	//			if( sz.cy < pControl->GetMinHeight() ) 
	//			{
	//				sz.cy = pControl->GetMinHeight();
	//			}

	//			if( sz.cy > pControl->GetMaxHeight() ) 
	//			{
	//				sz.cy = pControl->GetMaxHeight();
	//			}
	//		}

	//		cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;
	//		nEstimateNum++; 
	//	}

	//	cyFixed += ( nEstimateNum - 1 ) * m_iChildPadding;

	//	// Place elements
	//	int cyNeeded = 0;
	//	int cyExpand = 0;

	//	if( nAdjustables > 0 ) 
	//	{
	//		cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
	//	}
	//	
	//	// Position the elements
	//	SIZE szRemaining = szAvailable;
	//	int iPosY = rc.top;

	//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
	//	{
	//		iPosY -= m_pVerticalScrollBar->GetScrollPos();
	//	}

	//	int iPosX = rc.left;

	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	//	{
	//		iPosX -= m_pHorizontalScrollBar->GetScrollPos();
	//	}

	//	int iAdjustable = 0;
	//	int cyFixedRemaining = cyFixed;

	//	for( INT32 it2 = 0; it2 < ItemCount; it2 ++ ) 
	//	{
	//		pControl = static_cast< CControlUI* >( GetItemAt( it2 ) );
	//	
	//		if( !pControl->IsVisible() ) 
	//		{
	//			continue;
	//		}

	//		if( pControl->IsFloat() ) 
	//		{
	//			SetFloatPos( it2 );
	//			continue;
	//		}

	//		RECT rcPadding = pControl->GetPadding();

	//		szRemaining.cy -= rcPadding.top;

	//		SIZE sz = pControl->EstimateSize( szRemaining );

	//		if( sz.cy == 0 )
	//		{
	//			iAdjustable++;

	//			sz.cy = cyExpand;

	//			// Distribute remaining to last element (usually round-off left-overs)
	//			if( iAdjustable == nAdjustables ) 
	//			{
	//				sz.cy = MAX( 0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining );
	//			} 

	//			if( sz.cy < pControl->GetMinHeight() ) 
	//			{
	//				sz.cy = pControl->GetMinHeight();
	//			}
	//				
	//			if( sz.cy > pControl->GetMaxHeight() ) 
	//			{
	//				sz.cy = pControl->GetMaxHeight();
	//			}
	//		}
	//		else 
	//		{
	//			if( sz.cy < pControl->GetMinHeight() ) 
	//			{
	//				sz.cy = pControl->GetMinHeight();
	//			}
	//			
	//			if( sz.cy > pControl->GetMaxHeight() ) 
	//			{
	//				sz.cy = pControl->GetMaxHeight();
	//			}

	//			cyFixedRemaining -= sz.cy;
	//		}

	//		sz.cx = pControl->GetFixedWidth();

	//		if( sz.cx == 0 ) 
	//		{
	//			sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
	//		}

	//		if( sz.cx < 0 ) 
	//		{
	//			sz.cx = 0;
	//		}

	//		if( sz.cx < pControl->GetMinWidth() ) 
	//		{
	//			sz.cx = pControl->GetMinWidth();
	//		}
	//		
	//		if( sz.cx > pControl->GetMaxWidth() ) 
	//		{
	//			sz.cx = pControl->GetMaxWidth();
	//		}

	//		/*****************************************************************
	//		core of the vertical layout function:
	//		1.item is placed one by one on vertical direction.
	//		*****************************************************************/
	//		RECT rcCtrl = { iPosX + rcPadding.left, 
	//			iPosY + rcPadding.top, 
	//			iPosX + rcPadding.left + sz.cx, 
	//			iPosY + sz.cy + rcPadding.top + rcPadding.bottom }; 

	//		pControl->SetPos( rcCtrl );

	//		iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
	//		cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
	//		szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
	//	}

	//	cyNeeded += ( nEstimateNum - 1 ) * m_iChildPadding; 

	//	// Process the scrollbar
	//	ProcessScrollBar( rc, 0, cyNeeded );
	//}

} // namespace DuiLib