#include "StdAfx.h"

namespace DuiLib
{
	CCachedContainerUI::CCachedContainerUI() : m_pHorizontalScrollBar( NULL ), 
		m_pVerticalScrollBar( NULL ), 
		m_iChildPadding( 0 ), 
		m_bMouseChildEnabled( FALSE ), 
		m_bScrollProcess( FALSE )
	{
		memset( &m_rcInset, 0, sizeof( m_rcInset ) ); 
	}

	CCachedContainerUI::~CCachedContainerUI()
	{
		if( m_pVerticalScrollBar != NULL )
		{
			delete m_pVerticalScrollBar; 
		}

		if( m_pHorizontalScrollBar != NULL )
		{
			delete m_pHorizontalScrollBar; 
		}
	}

	void CCachedContainerUI::SetPos(RECT rc)
	{
		//INT32 ret; 
		//LRESULT _ret; 
		//
		///**********************************************************************
		//the cached item containter layout functions need to do 3 process.
		//1.recalculate the real items size, and set the scroll region.
		//2.calculate the scroll pos and item display index.
		//3.load the display item to the cache for display.
		//**********************************************************************/

		//do 
		//{
		//	CControlUI::SetPos(rc);
		//	rc = m_rcItem;
		//	SIZE ScrollPos; 

		//	// Adjust for inset
		//	rc.left += m_rcInset.left;
		//	rc.top += m_rcInset.top;
		//	rc.right -= m_rcInset.right;
		//	rc.bottom -= m_rcInset.bottom;

		//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
		//	{
		//		ScrollPos.cx = m_pVerticalScrollBar->GetScrollPos(); 
		//	}

		//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		//	{
		//		rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		//		ScrollPos.cy = m_pHorizontalScrollBar->GetScrollPos(); 
		//	}

		//	/**********************************************************************
		//	real item count is used to set the scroll region,and calculates the 
		//	scroll position to the item index.which will being display.
		//	**********************************************************************/

		//	register INT32 RealItemCount; 

		//	ASSERT( CacheManager != NULL ); 
		//	RealItemCount = CacheManager->GetRealItemsCount(); 

		//	if( RealItemCount == 0 )
		//	{
		//		ProcessScrollBar(rc, 0, 0);
		//		break; 
		//	}

		//	// Determine the minimum size
		//	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

		//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
		//	{
		//		szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
		//	}

		//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
		//	{
		//		szAvailable.cx += m_pVerticalScrollBar->GetScrollPos();
		//	}

		//	SIZE AllRealItemSize; 
		//	_ret = CacheManager->GetRealContentSize( &AllRealItemSize ); 
		//	if( _ret != ERROR_SUCCESS )
		//	{
		//		break; 
		//	}

		//	//int nAdjustables = 0;
		//	//int cyFixed = 0;
		//	//int nEstimateNum = 0;
		//	//register CControlUI *pControl; 

		//	//for( int it1 = 0; it1 < ItemCount; it1++ ) {
		//	//	pControl = static_cast<CControlUI*>(GetItemAt( it1 ) ); 

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
		//	//	nEstimateNum++;
		//	//}
		//	//cyFixed += (nEstimateNum - 1) * m_iChildPadding;

		//	//// Place elements
		//	//int cyNeeded = 0;
		//	//int cyExpand = 0;
		//	//if( nAdjustables > 0 ) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		//	//// Position the elements
		//	//SIZE szRemaining = szAvailable;
		//	//int iPosY = rc.top;
		//	//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
		//	//	iPosY -= m_pVerticalScrollBar->GetScrollPos();
		//	//}
		//	//int iPosX = rc.left;
		//	//if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
		//	//	iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		//	//}
		//	//int iAdjustable = 0;
		//	//int cyFixedRemaining = cyFixed;

		//	//for( int it2 = 0; it2 < ItemCount; it2++ ) {
		//	//	pControl = static_cast<CControlUI*>(GetItemAt(it2));
		//	//	if( !pControl->IsVisible() ) continue;
		//	//	if( pControl->IsFloat() ) {
		//	//		SetFloatPos(it2);
		//	//		continue;
		//	//	}

		//	//	RECT rcPadding = pControl->GetPadding();
		//	//	szRemaining.cy -= rcPadding.top;
		//	//	SIZE sz = pControl->EstimateSize(szRemaining);
		//	//	if( sz.cy == 0 ) {
		//	//		iAdjustable++;
		//	//		sz.cy = cyExpand;
		//	//		// Distribute remaining to last element (usually round-off left-overs)
		//	//		if( iAdjustable == nAdjustables ) {
		//	//			sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
		//	//		} 
		//	//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
		//	//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
		//	//	}
		//	//	else {
		//	//		if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
		//	//		if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
		//	//		cyFixedRemaining -= sz.cy;
		//	//	}

		//	//	sz.cx = pControl->GetFixedWidth();
		//	//	if( sz.cx == 0 ) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
		//	//	if( sz.cx < 0 ) sz.cx = 0;
		//	//	if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
		//	//	if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

		//	//	RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
		//	//	pControl->SetPos(rcCtrl);

		//	//	iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
		//	//	cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
		//	//	szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
		//	//}
		//	//cyNeeded += (nEstimateNum - 1) * m_iChildPadding;


		//	ULONG cy; 
		//	cy = GetScrollRange().cy + GetFixedHeight(); 

		//	_ret = CacheManager->OnScrollPosChanged( ScrollPos.cx, cy, ScrollPos.cy ); 
		//	if( _ret != ERROR_SUCCESS )
		//	{
		//		ret = ( INT32 )_ret; 
		//		__Trace( _T( "set the scroll pos to cache manager error %u\n" ), ret ); 
		//	}

		//	//CacheManager->FillSubItemsContent( )
		//	
		//	// Process the scrollbar
		//	ProcessScrollBar(rc, 0, AllRealItemSize.cy );
		//}while( FALSE ); 
	}

	CCachedItemManager::CCachedItemManager() : CachedControlUI( NULL ), 
		ItemCacheBeginIndex( 0 ), 
		ItemCacheCount( 0 ), 
		ItemRealCount( 0 ), 
		ItemRingBegin( 0 ), 
		MaxItemCacheCount( 0 ), 
		ItemDisplayBegin( 0 ), 
		//ItemDisplayOffset( 0 ), 
		ItemDisplayCount( 0 ), 
		//ItemFilteredBeginIndex( 0 ), 
		m_pContainerCallback( NULL ), 
		ItemDisplayWidth( 0 ), 
		ItemDisplayHeight( 0 ) 
	{
		//init_cs_lock( cache_lock ); 
	}

	CCachedItemManager::~CCachedItemManager()
	{
		UninitItemCache(); 
		//uninit_cs_lock( cache_lock ); 
	}

	LRESULT CCachedItemManager::SetDisplayPos( CControlUI *pControl, RECT *rcDisplay )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{

		}while( FALSE );

		return ret; 
	}

	LRESULT CCachedItemManager::OnScrollPosChanged( ULONG PosX, ULONG Height, ULONG PosY )
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG DisplayBegin; 
		ULONG DisplayOffset; 
		ULONG DisplayCount; 
		LONG ItemLoaded; 

		do 
		{
			DisplayBegin = ( ULONG )( ( DOUBLE )PosY / ( DOUBLE )Height ) * ItemCacheCount; 
			DisplayOffset = PosY / ItemDisplayHeight; 

			DisplayCount = GetItemShowCount(); 
			if( DisplayCount == INVALID_ITEM_COUNT )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			//FillSubItemsContent
			ret = FillSubItemsContent( DisplayBegin, DisplayCount, &ItemLoaded ); 
			//}
			//else
			//{
			//	ASSERT( FALSE && "cache manager is null" ); 
			//}

		} while ( FALSE );

		return ret; 
	}

	ULONG CCachedItemManager::GetDisplayItemCount()
	{
		return ItemDisplayCount; 
	}

	ULONG CCachedItemManager::GetItemShowCount()
	{
		ULONG ShowCount; 
		//ULONG ItemCacheCount; 
		//ULONG ItemDisplayCount; 

		//ItemCacheCount = GetRealItemsCount(); 
		//ItemDisplayCount = GetDisplayCount(); 

		if( ItemCacheCount > ItemDisplayCount )
		{
			ShowCount = ItemDisplayCount; 
		}
		else
		{
			ShowCount = ItemCacheCount; 
		}

		return ShowCount; 
	}

	LRESULT CCachedItemManager::GetDisplayItemAt( ULONG Index, CControlUI **UIItem )
	{
		LRESULT ret = ERROR_CALL_NOT_IMPLEMENTED; 
		//LRESULT ret = ERROR_SUCCESS; 
		//ULONG ItemIndex; 
		////ULONG ItemCacheCount; 
		////ULONG ItemDisplayBegin; 
		//CControlUI *CachedItem; 

		//do 
		//{

		//	if( Index == INVALID_ITEM_INDEX )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 			
		//	}

		//	if( UIItem == NULL )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 			
		//	}

		//	if( Index > ( ItemCacheCount - ItemDisplayBegin ) )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}

		//	//ItemDisplayBegin = 0; 

		//	ItemIndex = GetCachedItemIndex( ItemDisplayOffset + Index ); 
		//	if( ItemIndex == INVALID_ITEM_INDEX )
		//	{
		//		break; 
		//	}

		//	CachedItem = ControlUICache[ ItemIndex ]; 

		//	if( CachedItem == NULL )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		DBG_BP(); 
		//		break; 
		//	}

		//	*UIItem = CachedItem; 
		//}while( FALSE );

		return ret; 
	}

	//LRESULT CCachedItemManager::GetDisplayItemSize( SIZE *Size )
	//{
	//	LRESULT ret = ERROR_NOT_SUPPORTED; 

	//	do 
	//	{
	//		Size->cx = 0; 
	//		Size->cy = 0; 

	//	}while( FALSE );

	//	return ret; 
	//}

	LRESULT CCachedItemManager::UninitItemCache()
	{
		LRESULT ret = ERROR_SUCCESS; 
		//INT32 i; 

		do 
		{
			//for( i = 0; i < MaxItemCacheCount; i ++ )
			//{
			//	if( ControlUICache[ i ] != NULL )
			//	{
			//		delete ControlUICache[ i ]; 
			//		ControlUICache[ i ] = NULL; 
			//	}
			//}

			ret = RemoveAllCachedItems(); 
			if( ret != ERROR_SUCCESS )
			{

			}

			if( NULL != CachedControlUI )
			{
				free( CachedControlUI ); 
				CachedControlUI = NULL; 
			}

			MaxItemCacheCount = 0; 
			ItemCacheCount = 0; 

		}while( FALSE );

		return ret; 
	}

	ULONG CCachedItemManager::GetRealItemsCount(void)
	{
		return ItemRealCount; 
	}

	IListContainerCallback *CCachedItemManager::GetListContainerCallback() const
	{
		return m_pContainerCallback; 
	}

	void CCachedItemManager::SetListContainerCallback( IListContainerCallback *pCallback )
	{
		do 
		{
			if( pCallback == NULL )
			{
				break; 
			}

			m_pContainerCallback = pCallback; 
		}while( FALSE );

		return; 
	}

	//LRESULT CCachedItemManager::FillSubItemsContent( ULONG ItemBeginIndex, ULONG ItemCount )
	//{
	//	LRESULT ret = ERROR_SUCCESS; 
	//	//ui_cache_context context; 
	//	ULONG RealIndexEnd; 
	//	LONG NeedLoadItemCount; 
	//	ULONG ItemRingIndex; 
	//	ULONG i; 

	//	do 
	//	{

	//		if( m_pContainerCallback == NULL )
	//		{
	//			ret = ERROR_NOT_READY; 
	//			break; 
	//		}

	//		ret = m_pContainerCallback->LoadSubItemsContent( ItemBeginIndex, ItemCount ); 

	//	//	if( ItemCount > MaxItemCacheCount )
	//	//	{
	//	//		ret = ERROR_BUFFER_OVERFLOW; 
	//	//		break; 
	//	//	}

	//	//	RealIndexEnd = ItemBeginIndex + ItemCount; 

	//	//	//#ifdef _DEBUG
	//	//	//		if( ( RealIndexEnd > ( ItemCacheBegin + ItemCacheCount ) )
	//	//	//
	//	//	//			&& ( RealIndex < ItemCacheBegin ) )
	//	//	//		{
	//	//	//			ASSERT( FALSE && "item to load too more" ); 
	//	//	//			ret = ERROR_INVALID_PARAMETER; 
	//	//	//			break; 
	//	//	//		}
	//	//	//#endif //_DEBUG

	//	//	if( ItemBeginIndex < ItemCacheBegin )
	//	//	{
	//	//		NeedLoadItemCount = ( ItemCacheBegin - ItemBeginIndex ); 

	//	//		context.manager = m_pManager; 
	//	//		context.row_index = 0; 
	//	//		context.ring_load_index = ItemRingBegin; 
	//	//		context.max_count = ItemCacheCount; 
	//	//		context.ui_cache = ControlUICache; 

	//	//		ret = load_action_log( ItemBeginIndex, NeedLoadItemCount, &context ); 
	//	//		if( context.row_index != NeedLoadItemCount )
	//	//		{
	//	//			log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
	//	//		}

	//	//		ItemCacheCount += context.row_index; 
	//	//		if( ItemCacheCount > MaxItemCacheCount )
	//	//		{
	//	//			ItemCacheCount = MaxItemCacheCount; 
	//	//		}

	//	//		ItemRingBegin -= context.row_index; 

	//	//		if( ItemRingBegin < 0 )
	//	//		{
	//	//			ItemRingBegin = ( MaxItemCacheCount + ItemRingIndex ); 
	//	//		}

	//	//	}

	//	//	if( RealIndexEnd > ( ItemCacheBegin + ItemCacheCount ) )
	//	//	{
	//	//		NeedLoadItemCount = RealIndexEnd - ( ItemCacheBegin + ItemCacheCount ); 

	//	//		ASSERT( NeedLoadItemCount > 0 ); 

	//	//		context.manager = m_pManager; 
	//	//		context.row_index = 0; 
	//	//		context.ring_load_index = ItemRingBegin + ItemCacheCount; 
	//	//		context.max_count = ItemCacheCount; 
	//	//		context.ui_cache = ControlUICache; 

	//	//		ret = load_action_log( ItemBeginIndex, NeedLoadItemCount, &context ); 

	//	//		if( context.row_index != NeedLoadItemCount )
	//	//		{
	//	//			log_trace( ( MSG_ERROR, "load the event log to ui is not completely successful" ) ); 
	//	//		}

	//	//		ItemCacheCount += context.row_index; 

	//	//		if( ItemCacheCount > MaxItemCacheCount )
	//	//		{
	//	//			ItemCacheCount = MaxItemCacheCount; 
	//	//		}

	//	//		if( ( ItemRingBegin + ItemCacheCount + context.row_index ) > MaxItemCacheCount )
	//	//		{
	//	//			ItemRingBegin += ( ( ItemRingBegin + ItemCacheCount + context.row_index ) % MaxItemCacheCount ); 
	//	//		}

	//	//	}

	//	}while( FALSE ); 

	//	return ret; 
	//}

	LRESULT CCachedItemManager::GetRealContentSize( SIZE *Size )
	{
		LRESULT ret = ERROR_SUCCESS; 
	
		do 
		{
#ifdef _DEBUG
			if( ItemDisplayHeight == 0 
				|| ItemDisplayWidth == 0 )
			{
				DBG_BP(); 
			}
#endif //_DEBUG

			Size->cx = ItemDisplayWidth; 
			Size->cy = ItemRealCount * ItemDisplayHeight; 
		}while( FALSE ); 
	
		return ret; 
	}

	ULONG CCachedItemManager::GetRingIndexFromDisplayIndex( ULONG ItemIndex )
	{
		return INVALID_ITEM_INDEX; 
	}
	
	LRESULT CCachedItemManager::OnItemAdded( ULONG item_count )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
			ItemRealCount += item_count; 
		}while( FALSE ); 

		return ret; 
	}

	//LRESULT CCachedItemManager::LoadItemCache( ULONG item_begin )
	//{
	//	LRESULT ret = ERROR_NOT_SUPPORTED; 

	//	DBG_BP(); 
	//	//do 
	//	//{

	//	//}while( FALSE );

	//	return ret; 
	//}

	LRESULT CCachedItemManager::RemoveAllCachedItems()
	{
		LRESULT ret = ERROR_SUCCESS; 
		ULONG ItemCachedIndex; 
		INT32 i; 

		do 
		{
			if( CachedControlUI == NULL 
				|| ItemCacheCount == 0 )
			{
				DBG_BP(); 
				ret = ERROR_NOT_READY; 
				break; 
			}

			for( i = 0; ( LONG )i < ItemCacheCount; i ++ )
			{
				ItemCachedIndex = GetRingItemIndex( i + ItemCacheBeginIndex, ItemCacheCount ); 
				ASSERT( ItemCachedIndex < ItemCacheCount ); 

				if( ItemCachedIndex == INVALID_ITEM_INDEX )
				{
					ASSERT( FALSE && "fatal error: item ring index caculated is invalid" ); 
					//continue; 
				}

				if( CachedControlUI[ i ] != NULL )
				{
					delete CachedControlUI[ i ]; 
					CachedControlUI[ i ] = NULL; 
				}
				else
				{
					ASSERT( FALSE ); 
				}
			}

			ItemCacheCount = 0; 
			ItemCacheBeginIndex = 0; 

		}while( FALSE );
	
		return ret; 
	}

	LRESULT CCachedItemManager::RemoveDisplayItem( INT32 ItemIndex )
	{
		LRESULT ret = ERROR_SUCCESS; 

		do 
		{
		} while ( FALSE );
	
		return ret; 
	}

	void CCachedContainerUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			return;
		}

		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsEnabled() )
		{
			if( event.Type == UIEVENT_KEYDOWN ) 
			{
				switch( event.chKey ) {
			case VK_DOWN:
				LineDown();
				return;
			case VK_UP:
				LineUp();
				return;
			case VK_NEXT:
				PageDown();
				return;
			case VK_PRIOR:
				PageUp();
				return;
			case VK_HOME:
				HomeUp();
				return;
			case VK_END:
				EndDown();
				return;
				}
			}
			else if( event.Type == UIEVENT_SCROLLWHEEL )
			{
				switch( LOWORD(event.wParam) ) 
				{
				case SB_LINEUP:
					LineUp();
					return;
				case SB_LINEDOWN:
					LineDown();
					return;
				}
			}
		}
		else if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled() ) 
		{
			if( event.Type == UIEVENT_KEYDOWN ) 
			{
				switch( event.chKey ) 
				{
				case VK_DOWN:
					LineRight();
					return;
				case VK_UP:
					LineLeft();
					return;
				case VK_NEXT:
					PageRight();
					return;
				case VK_PRIOR:
					PageLeft();
					return;
				case VK_HOME:
					HomeLeft();
					return;
				case VK_END:
					EndRight();
					return;
				}
			}
			else if( event.Type == UIEVENT_SCROLLWHEEL )
			{
				switch( LOWORD(event.wParam) ) 
				{
				case SB_LINEUP:
					LineLeft();
					return;
				case SB_LINEDOWN:
					LineRight();
					return;
				}
			}
		}
		CControlUI::DoEvent(event);
	}

	void CCachedContainerUI::SetVisible(bool bVisible)
	{
		//register INT32 ItemCount; 
		//register CControlUI *SubCtrl; 

		if( m_bVisible == bVisible ) 
		{
			return;
		}
			
		CControlUI::SetVisible(bVisible);

		//INT32 i; 
		//ItemCount = CacheManager->GetRealItemsCount(); 

		//for( i = 0; i < ItemCount; i ++ )
		//{
		//		CacheManager->
		//}

		//for( int it = 0; it < ItemCount; it++ ) {
		//	SubCtrl = static_cast<CControlUI*>(GetItemAt( it ) ); 
		//	if( SubCtrl == NULL )
		//	{
		//		__Trace( _T("why sub control %u is null??" ), it ); 
		//		continue; 
		//	}

		//	SubCtrl->SetInternVisible(IsVisible());
		//}
	}

	void CCachedContainerUI::SetInternVisible(bool bVisible)
	{
		//register INT32 ItemCount; 
		//register CControlUI *SubCtrl; 

		CControlUI::SetInternVisible(bVisible);

		//ItemCount = GetItemCount(); 

		//if( ItemCount == 0 ) return;


		//for( int it = 0; it < ItemCount; it++ ) {
		//	SubCtrl = static_cast<CControlUI*>(GetItemAt( it ) ); 
		//	if( SubCtrl == NULL )
		//	{
		//		__Trace( _T("why sub control %u is null??" ), it ); 
		//		continue; 
		//	}

		//	SubCtrl->SetInternVisible(IsVisible());
		//}
	}

	void CCachedContainerUI::SetMouseEnabled(bool bEnabled)
	{
		if( m_pVerticalScrollBar != NULL ) 
		{
			m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
		}

		if( m_pHorizontalScrollBar != NULL ) 
		{
			m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
		}
			
		CControlUI::SetMouseEnabled(bEnabled);
	}

	SIZE CCachedContainerUI::GetScrollPos() const
	{
		SIZE sz = {0, 0};

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
		{
			sz.cy = m_pVerticalScrollBar->GetScrollPos();
		}
		
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		{
			sz.cx = m_pHorizontalScrollBar->GetScrollPos();
		}
		
		return sz;
	}

	SIZE CCachedContainerUI::GetScrollRange() const
	{
		SIZE sz = {0, 0};
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
		{
			sz.cy = m_pVerticalScrollBar->GetScrollRange();
		}

		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
		{
			sz.cx = m_pHorizontalScrollBar->GetScrollRange();
		}
		
		return sz;
	}

	void CCachedContainerUI::SetScrollPos(SIZE szPos)
	{
		int cx = 0;
		int cy = 0;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( cx == 0 && cy == 0 ) return;

		RECT rcPos;
		register INT32 ItemCount; 
		register CControlUI *pControl; 

		//ItemCount = GetItemCount(); 

		//for( int it2 = 0; it2 < ItemCount; it2++ ) {
		//	pControl = static_cast<CControlUI*>(GetItemAt( it2 ) ); 
		//	if( pControl == NULL )
		//	{
		//		__Trace( _T("why sub control %u is null??" ), it2 ); 
		//		continue; 
		//	}

		//	if( !pControl->IsVisible() ) continue;
		//	if( pControl->IsFloat() ) continue;

		//	rcPos = pControl->GetPos();
		//	rcPos.left -= cx;
		//	rcPos.right -= cx;
		//	rcPos.top -= cy;
		//	rcPos.bottom -= cy;
		//	pControl->SetPos(rcPos);
		//}

		Invalidate();
	}

	void CCachedContainerUI::LineUp()
	{
		int cyLine = LINE_CX_FIXED;
		if( m_pManager ) 
		{
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + LINE_CX_FIXED;
		}

		SIZE sz = GetScrollPos();
		sz.cy -= cyLine;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::LineDown()
	{
		int cyLine = LINE_CX_FIXED;
		if( m_pManager ) 
		{
			cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + LINE_CX_FIXED;
		}

		SIZE sz = GetScrollPos();
		sz.cy += cyLine;
		SetScrollPos(sz);
	}


	void CCachedContainerUI::PageUp()
	{
		SIZE sz = GetScrollPos();
		int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy -= iOffset;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::PageDown()
	{
		SIZE sz = GetScrollPos();
		int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy += iOffset;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::HomeUp()
	{
		SIZE sz = GetScrollPos();
		sz.cy = 0;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::EndDown()
	{
		SIZE sz = GetScrollPos();
		sz.cy = GetScrollRange().cy;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::LineLeft()
	{
		SIZE sz = GetScrollPos();
		sz.cx -= LINE_CX_FIXED;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::LineRight()
	{
		SIZE sz = GetScrollPos();
		sz.cx += LINE_CX_FIXED;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::PageLeft()
	{
		SIZE sz = GetScrollPos();
		int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
		sz.cx -= iOffset;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::PageRight()
	{
		SIZE sz = GetScrollPos();
		int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
		sz.cx += iOffset;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::HomeLeft()
	{
		SIZE sz = GetScrollPos();
		sz.cx = 0;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::EndRight()
	{
		SIZE sz = GetScrollPos();
		sz.cx = GetScrollRange().cx;
		SetScrollPos(sz);
	}

	void CCachedContainerUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
	{
		if( bEnableVertical && !m_pVerticalScrollBar ) 
		{
			m_pVerticalScrollBar = new CScrollBarUI;
			m_pVerticalScrollBar->SetOwner(this);
			m_pVerticalScrollBar->SetManager(m_pManager, NULL, false);
		
			if ( m_pManager ) 
			{
				LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("VScrollBar"));
				if( pDefaultAttributes ) 
				{
					m_pVerticalScrollBar->ApplyAttributeList(pDefaultAttributes);
				}
			}
		}
		else if( !bEnableVertical && m_pVerticalScrollBar ) 
		{
			delete m_pVerticalScrollBar;
			m_pVerticalScrollBar = NULL;
		}

		if( bEnableHorizontal && !m_pHorizontalScrollBar ) 
		{
			m_pHorizontalScrollBar = new CScrollBarUI;
			m_pHorizontalScrollBar->SetHorizontal(true);
			m_pHorizontalScrollBar->SetOwner(this);
			m_pHorizontalScrollBar->SetManager(m_pManager, NULL, false);
			if ( m_pManager ) 
			{
				LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("HScrollBar"));
				
				if( pDefaultAttributes ) 
				{
					m_pHorizontalScrollBar->ApplyAttributeList(pDefaultAttributes);
				}
			}
		}
		else if( !bEnableHorizontal && m_pHorizontalScrollBar ) 
		{
			delete m_pHorizontalScrollBar;
			m_pHorizontalScrollBar = NULL;
		}

		NeedUpdate();
	}

	CScrollBarUI* CCachedContainerUI::GetVerticalScrollBar() const
	{
		return m_pVerticalScrollBar;
	}

	CScrollBarUI* CCachedContainerUI::GetHorizontalScrollBar() const
	{
		return m_pHorizontalScrollBar;
	}

	void CCachedContainerUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		
	}

	int CCachedContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const
	{
		// NOTE: This is actually a helper-function for the list/combo/ect controls
		//       that allow them to find the next enabled/available selectable item
		//if( GetItemCount() == 0 ) return -1;
		//iIndex = CLAMP(iIndex, 0, GetItemCount() - 1);
		//if( bForward ) {
		//	for( int i = iIndex; i < GetItemCount(); i++ ) {
		//		if( GetItemAt(i)->GetInterface(UI_LIST_ITEM_INTERFACE_NAME) != NULL 
		//			&& GetItemAt(i)->IsVisible()
		//			&& GetItemAt(i)->IsEnabled() ) return i;
		//	}
		//	return -1;
		//}
		//else {
		//	for( int i = iIndex; i >= 0; --i ) {
		//		if( GetItemAt(i)->GetInterface(UI_LIST_ITEM_INTERFACE_NAME) != NULL 
		//			&& GetItemAt(i)->IsVisible()
		//			&& GetItemAt(i)->IsEnabled() ) return i;
		//	}
		//	return FindSelectable(0, true);
		//}
		return NULL; 
	}

	void CCachedContainerUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		//INT32 ItemCount; 
		//CControlUI *SubCtrl; 

		//ItemCount = GetItemCount(); 

		//for( int it = 0; it < ItemCount; it++ )
		//{
		//	SubCtrl = static_cast<CControlUI*>( GetItemAt( it ) ); 
		//	if( SubCtrl == NULL )
		//	{
		//		continue; 
		//	}

		//	SubCtrl->SetManager(pManager, this, bInit);
		//}

		if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetManager(pManager, this, bInit);
		if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetManager(pManager, this, bInit);
		CControlUI::SetManager(pManager, pParent, bInit);
	}

	//CControlUI* CCachedContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	//{
	//	// Check if this guy is valid
	//	if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
	//	if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
	//	if( (uFlags & UIFIND_HITTEST) != 0 ) {
	//		if( !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))) ) return NULL;
	//		if( !m_bMouseChildEnabled ) {
	//			CControlUI* pResult = NULL;
	//			if( m_pVerticalScrollBar != NULL ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
	//			if( pResult == NULL && m_pHorizontalScrollBar != NULL ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
	//			if( pResult == NULL ) pResult = CControlUI::FindControl(Proc, pData, uFlags);
	//			return pResult;
	//		}
	//	}

	//	CControlUI* pResult = NULL;
	//	if( m_pVerticalScrollBar != NULL ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
	//	if( pResult == NULL && m_pHorizontalScrollBar != NULL ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
	//	if( pResult != NULL ) return pResult;

	//	if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
	//		CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
	//		if( pControl != NULL ) return pControl;
	//	}
	//	RECT rc = m_rcItem;
	//	rc.left += m_rcInset.left;
	//	rc.top += m_rcInset.top;
	//	rc.right -= m_rcInset.right;
	//	rc.bottom -= m_rcInset.bottom;
	//	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
	//	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

	//	register INT32 ItemCount; 
	//	register CControlUI *pSubContainer; 

	//	ItemCount = GetItemCount(); 

	//	if( (uFlags & UIFIND_TOP_FIRST) != 0 ) {
	//		for( int it = ItemCount - 1; it >= 0; it-- ) {
	//			pSubContainer = static_cast<CControlUI*>(GetItemAt( it ) ); 
	//			if( pSubContainer == NULL )
	//			{
	//				__Trace( _T("why sub control %u is null??" ), it ); 
	//				continue; 
	//			}

	//			CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
	//			if( pControl != NULL ) {
	//				if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
	//					continue;
	//				else 
	//					return pControl;
	//			}            
	//		}
	//	}
	//	else {
	//		for( int it = 0; it < ItemCount; it++ ) {
	//			pSubContainer = static_cast<CControlUI*>(GetItemAt( it ) ); 
	//			if( pSubContainer == NULL )
	//			{
	//				__Trace( _T("why sub control %u is null??" ), it ); 
	//				continue; 
	//			}
	//			CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
	//			if( pControl != NULL ) {
	//				if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
	//					continue;
	//				else 
	//					return pControl;
	//			} 
	//		}
	//	}

	//	if( pResult == NULL ) pResult = CControlUI::FindControl(Proc, pData, uFlags);
	//	return pResult;
	//}

	void CCachedContainerUI::SetPos2(RECT rc)
	{
		register INT32 ItemCount; 
		register CControlUI *pControl; 

		CControlUI::SetPos(rc);

		//ItemCount = GetItemCount(); 

		//if( ItemCount == 0 ) 
		//{
		//	return;
		//}

		//rc.left += m_rcInset.left;
		//rc.top += m_rcInset.top;
		//rc.right -= m_rcInset.right;
		//rc.bottom -= m_rcInset.bottom;

		//for( int it = 0; it < ItemCount; it++ )
		//{
		//	pControl = static_cast< CControlUI* >( GetItemAt( it ) ); 

		//	if( pControl == NULL )
		//	{
		//		__Trace( _T( "why sub control %u is null??" ), it ); 
		//		continue; 
		//	}

		//	if( !pControl->IsVisible() ) 
		//	{
		//		continue;
		//	}

		//	if( pControl->IsFloat() ) 
		//	{
		//		SetFloatPos( it );
		//	}

		//	else 
		//	{
		//		pControl->SetPos( rc ); // 所有非float子控件放大到整个客户区
		//	}
		//}
	}

	void CCachedContainerUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
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
//			ret = CacheManager->GetDisplayItemSize( &ItemSize ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			ItemCount = CacheManager->GetDisplayCount(); 
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
//					ret = CacheManager->GetDisplayItem( it, &pControl ); 
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
//					ret = CacheManager->SetDisplayPos( pControl, &rcDisplay ); 
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
		//}while( FALSE ); 
	
		return; 
	}

	void CCachedContainerUI::SetFloatPos(int iIndex)
	{
		ASSERT( FALSE ); 
		// 因为CControlUI::SetPos对float的操作影响，这里不能对float组件添加滚动条的影响
		//register INT32 ItemCount; 
		//if( iIndex < 0 || iIndex >= GetItemCount() ) return;

		//CControlUI* pControl = static_cast<CControlUI*>(GetItemAt( iIndex ) );

		//if( !pControl->IsVisible() ) return;
		//if( !pControl->IsFloat() ) return;

		//SIZE szXY = pControl->GetFixedXY();
		//SIZE sz = {pControl->GetFixedWidth(), pControl->GetFixedHeight()};
		//RECT rcCtrl = { 0 };
		//if( szXY.cx >= 0 ) {
		//	rcCtrl.left = m_rcItem.left + szXY.cx;
		//	rcCtrl.right = m_rcItem.left + szXY.cx + sz.cx;
		//}
		//else {
		//	rcCtrl.left = m_rcItem.right + szXY.cx - sz.cx;
		//	rcCtrl.right = m_rcItem.right + szXY.cx;
		//}
		//if( szXY.cy >= 0 ) {
		//	rcCtrl.top = m_rcItem.top + szXY.cy;
		//	rcCtrl.bottom = m_rcItem.top + szXY.cy + sz.cy;
		//}
		//else {
		//	rcCtrl.top = m_rcItem.bottom + szXY.cy - sz.cy;
		//	rcCtrl.bottom = m_rcItem.bottom + szXY.cy;
		//}
		//if( pControl->IsRelativePos() )
		//{
		//	TRelativePosUI tRelativePos = pControl->GetRelativePos();
		//	SIZE szParent = {m_rcItem.right-m_rcItem.left,m_rcItem.bottom-m_rcItem.top};
		//	if(tRelativePos.szParent.cx != 0)
		//	{
		//		int nIncrementX = szParent.cx-tRelativePos.szParent.cx;
		//		int nIncrementY = szParent.cy-tRelativePos.szParent.cy;
		//		rcCtrl.left += (nIncrementX*tRelativePos.nMoveXPercent/100);
		//		rcCtrl.top += (nIncrementY*tRelativePos.nMoveYPercent/100);
		//		rcCtrl.right = rcCtrl.left+sz.cx+(nIncrementX*tRelativePos.nZoomXPercent/100);
		//		rcCtrl.bottom = rcCtrl.top+sz.cy+(nIncrementY*tRelativePos.nZoomYPercent/100);
		//	}
		//	pControl->SetRelativeParentSize(szParent);
		//}
		//pControl->SetPos(rcCtrl);
	}

	CControlUI* CCachedContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		CControlUI *pResult = NULL;
		//// Check if this guy is valid
		//if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
		//if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
		//if( (uFlags & UIFIND_HITTEST) != 0 ) {
		//	if( !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))) ) return NULL;
		//	if( !m_bMouseChildEnabled ) {
		//		CControlUI* pResult = NULL;
		//		if( m_pVerticalScrollBar != NULL ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
		//		if( pResult == NULL && m_pHorizontalScrollBar != NULL ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
		//		if( pResult == NULL ) pResult = CControlUI::FindControl(Proc, pData, uFlags);
		//		return pResult;
		//	}
		//}

		//CControlUI* pResult = NULL;
		//if( m_pVerticalScrollBar != NULL ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
		//if( pResult == NULL && m_pHorizontalScrollBar != NULL ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
		//if( pResult != NULL ) return pResult;

		//if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
		//	CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
		//	if( pControl != NULL ) return pControl;
		//}
		//RECT rc = m_rcItem;
		//rc.left += m_rcInset.left;
		//rc.top += m_rcInset.top;
		//rc.right -= m_rcInset.right;
		//rc.bottom -= m_rcInset.bottom;
		//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		//if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		//register INT32 ItemCount; 
		//register CControlUI *pSubContainer; 

		//ItemCount = GetItemCount(); 

		//if( (uFlags & UIFIND_TOP_FIRST) != 0 ) {
		//	for( int it = ItemCount - 1; it >= 0; it-- ) {
		//		pSubContainer = static_cast<CControlUI*>(GetItemAt( it ) ); 
		//		if( pSubContainer == NULL )
		//		{
		//			__Trace( _T("why sub control %u is null??" ), it ); 
		//			continue; 
		//		}

		//		CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
		//		if( pControl != NULL ) {
		//			if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
		//				continue;
		//			else 
		//				return pControl;
		//		}            
		//	}
		//}
		//else {
		//	for( int it = 0; it < ItemCount; it++ ) {
		//		pSubContainer = static_cast<CControlUI*>(GetItemAt( it ) ); 
		//		if( pSubContainer == NULL )
		//		{
		//			__Trace( _T("why sub control %u is null??" ), it ); 
		//			continue; 
		//		}
		//		CControlUI* pControl = pSubContainer->FindControl(Proc, pData, uFlags);
		//		if( pControl != NULL ) {
		//			if( (uFlags & UIFIND_HITTEST) != 0 && !pControl->IsFloat() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
		//				continue;
		//			else 
		//				return pControl;
		//		} 
		//	}
		//}

		//if( pResult == NULL ) pResult = CControlUI::FindControl(Proc, pData, uFlags);
		pResult = CControlUI::FindControl(Proc, pData, uFlags);

		return pResult;
	}

	void CCachedContainerUI::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired)
	{
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) 
		{
			//RECT old_pos; 
			//INT32 old_scroll_pos; 

			RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight() };
			m_pHorizontalScrollBar->SetPos( rcScrollBarPos ); 

			//old_pos = m_pHorizontalScrollBar->GetPos(); 
			//old_scroll_pos = m_pHorizontalScrollBar->GetScrollPos(); 

			//ASSERT( old_pos.right >= old_pos.left ); 
			//old_scroll_pos += ( old_pos.right - old_pos.left ); 

			//if( old_scroll_pos + ( rc.right - rc.left ) > cxRequired )
			//{
			//	m_pHorizontalScrollBar->SetScrollPos( cxRequired - ( rc.right - rc.left ) ); 
			//}

			m_pHorizontalScrollBar->SetScrollRange( cxRequired - ( rc.right - rc.left ) ); 
		}

		if( m_pVerticalScrollBar == NULL ) 
		{
			return;
		}

		if( cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible() ) 
		{
			m_pVerticalScrollBar->SetVisible( true ); 

			m_pVerticalScrollBar->SetScrollRange( cyRequired - ( rc.bottom - rc.top ) ); 

			m_pVerticalScrollBar->SetScrollPos(0);
			
			m_bScrollProcess = true;
			
			SetPos( m_rcItem );
			
			m_bScrollProcess = false;
			return;
		}

		// No scrollbar required
		if( !m_pVerticalScrollBar->IsVisible() ) 
		{
			return;
		}

		// Scroll not needed anymore?
		int cyScroll = cyRequired - ( rc.bottom - rc.top ); 

		if( cyScroll <= 0 && !m_bScrollProcess) 
		{
			m_pVerticalScrollBar->SetVisible( false );
			m_pVerticalScrollBar->SetScrollPos( 0 );
			m_pVerticalScrollBar->SetScrollRange( 0 );
			SetPos( m_rcItem );
		}
		else
		{
			RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };

			m_pVerticalScrollBar->SetPos( rcScrollBarPos );

			if( m_pVerticalScrollBar->GetScrollRange() != cyScroll ) 
			{
				int iScrollPos = m_pVerticalScrollBar->GetScrollPos();

				m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
				
				if( m_pVerticalScrollBar->GetScrollRange() == 0 ) 
				{
					m_pVerticalScrollBar->SetVisible(false);
					m_pVerticalScrollBar->SetScrollPos(0);
				}

				/********************************************************************
				if scroll region shorter, then the scroll position must is to change to 
				the bottom of the new region.
				********************************************************************/
				
				if( iScrollPos > m_pVerticalScrollBar->GetScrollPos() ) 
				{
					SetPos(m_rcItem);
				}
			}
		}
	}

} //namespace DuiLib
