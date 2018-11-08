#include "StdAfx.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListBaseUI::CListBaseUI() : m_pCallback(NULL), 
					m_pListHeaderCallback( NULL ), 
					m_bScrollSelect(false), 
					m_iCurSel(-1), 
					m_iExpandedItem(-1), 
					m_bColumnWidthChanged( FALSE )
{
    m_pHeader = new CListHeaderUI;

    AddItem(m_pHeader);

    m_ListInfo.nColumns = 0;
    m_ListInfo.nFont = -1;
    m_ListInfo.uTextStyle = DT_VCENTER; // m_uTextStyle(DT_VCENTER | DT_END_ELLIPSIS)
    m_ListInfo.dwTextColor = 0xFF000000;
    m_ListInfo.dwBkColor = 0;
    m_ListInfo.bAlternateBk = false;
    m_ListInfo.dwSelectedTextColor = 0xFF000000;
    m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
    m_ListInfo.dwHotTextColor = 0xFF000000;
    m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
    m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
    m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
    m_ListInfo.dwLineColor = 0;
    m_ListInfo.bShowHtml = false;
    m_ListInfo.bMultiExpandable = false;
    ::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
    ::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));
}

CListUI::CListUI() : button_pressed( FALSE ), 
					previous_cursor( NULL )
{
	m_pList = new CListBodyUI(this);

	do 
	{
		if( m_pList == NULL )
		{
			break; 
		}

		__super::AddItem(m_pList);
	}while( FALSE );
}

CListUI::~CListUI() 
{
}

LPCTSTR CListUI::GetClass() const
{
    return UI_LIST_CLASS_NAME;
}

ULONG CListUI::GetClassId() const
{
	return UI_LIST_CLASS_ID;
}

UINT CListBaseUI::GetControlFlags() const
{
    return UIFLAG_TABSTOP;
}

LPVOID CListUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, UI_LIST_INTERFACE_NAME ) == 0 ) return static_cast<CListUI*>(this);
    if( _tcscmp(pstrName, _T("IList")) == 0 ) return static_cast<IListUI*>(this);
    if( _tcscmp(pstrName, UI_LIST_OWNER_INTERFACE_NAME) == 0 ) return static_cast<IListOwnerUI*>(this);
    return CVerticalLayoutUI::GetInterface(pstrName);
}

bool CListBaseUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	return __super::SetItemIndex( pControl, iIndex ); 
}

INT32 CListBaseUI::GetItemCount() const
{
	return __super::GetItemCount(); 
}

bool CListBaseUI::AddItem(CControlUI* pControl)
{
	return __super::AddItem( pControl ); 
}

CControlUI* CListBaseUI::GetItemAt(int iIndex) const
{
	return __super::GetItemAt( iIndex ); 
}

INT32 CListBaseUI::GetItemIndex(CControlUI* pControl) const
{
	return __super::GetItemIndex( pControl ); 
}

//bool CListBaseUI::SetItemIndex(CControlUI* pControl, int iIndex)
//{
//	return __super::SetItemIndex( pControl, iIndex ); 
//}

//INT32 CListBaseUI::GetItemCount() const
//{
//	return __super::GetItemCount(); 
//}

bool CListUI::AddItem(CControlUI* pControl)
{
	return AddSubItem( pControl, pControl->GetClassId() ); 
}

//bool CListUI::AddItemAt(CControlUI* pControl)
//{
//	return AddSubItemAt( pControl, pControl->GetClassId() ); 
//}

bool CListBaseUI::AddItemAt(CControlUI* pControl, int iIndex)
{
	return __super::AddItemAt( pControl, iIndex ); 
}

bool CListBaseUI::RemoveItem(CControlUI* pControl)
{
	return __super::RemoveItem( pControl ); 
}

bool CListBaseUI::RemoveItemAt(int iIndex)
{
	return __super::RemoveItemAt( iIndex ); 
}

void CListBaseUI::RemoveAllItem()
{
	return __super::RemoveAllItem(); 
}

bool CListUI::SetSubItemIndex(CControlUI* pControl, int iIndex, ULONG ClassId )
{
	switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		ASSERT( pControl->GetInterface(_T("ListHeader")) != NULL ); 		
		return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		{
			// We also need to recognize header sub-items
			ASSERT( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ); 
			return m_pHeader->SetItemIndex(pControl, iIndex);
		}
		break; 
	default:
		{
			int iOrginIndex = m_pList->GetItemIndex(pControl);
			if( iOrginIndex == -1 ) return false;
			if( iOrginIndex == iIndex ) return true;

			IListItemUI* pSelectedListItem = NULL;
			if( m_iCurSel >= 0 ) pSelectedListItem = 
				static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
			if( !m_pList->SetItemIndex(pControl, iIndex) ) return false;
			int iMinIndex = min(iOrginIndex, iIndex);
			int iMaxIndex = max(iOrginIndex, iIndex);
			for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
				CControlUI* p = m_pList->GetItemAt(i);
				IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
				if( pListItem != NULL ) {
					pListItem->SetIndex(i);
				}
			}
			if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
			return true;
		}
		break; 
	}
}

CControlUI* CListUI::GetSubItemAt(int iIndex, ULONG ClassId  ) const
{
	register CControlUI *SubCtrl = NULL; 
	switch( ClassId )
	{
	case UI_CONTROL_CLASS_ID:
		SubCtrl = __super::GetItemAt(iIndex);
		break;
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		SubCtrl = m_pHeader->GetItemAt(iIndex);
		break;
	default:
		ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 
		SubCtrl = m_pList->GetItemAt(iIndex);
		break; 
	}

	return SubCtrl; 
}

int CListUI::GetSubItemIndex(CControlUI* pControl, ULONG ClassId) const
{
	switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		{
			ASSERT( pControl->GetInterface(_T("ListHeader")) != NULL ); 
			return CVerticalLayoutUI::GetItemIndex(pControl);
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		// We also need to recognize header sub-items
		ASSERT( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ); 
		return m_pHeader->GetItemIndex(pControl);
		break; 
	default:
		ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 
		return m_pList->GetItemIndex(pControl);
		break; 
	}
}

INT32 CListUI::GetSubItemCount( ULONG ClassId ) const
{
	INT32 item_count; 
	switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		{
			item_count = 0; 
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		{
			item_count = m_pHeader->GetItemCount(); 
		}
		break; 
	default:
		{
			if( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) )
			{
				item_count = m_pList->GetItemCount();
			}
			else
			{
				item_count = 0; 
				ASSERT( FALSE ); 
			}
		}
		break; 
	}

	return item_count; 
}

bool CListUI::AddSubItem(CControlUI* pControl, ULONG ClassId )
{
	bool ret; 

    // Override the Add() method so we can add items specifically to
    // the intended widgets. Headers are assumed to be
    // answer the correct interface so we can add multiple list headers.
    switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		{
			ASSERT( pControl->GetInterface(_T("ListHeader")) != NULL ); 

			if( m_pHeader != pControl && m_pHeader->GetItemCount() == 0 ) 
			{
				CVerticalLayoutUI::RemoveItem(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}

			m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS); 
			ret = CVerticalLayoutUI::AddItemAt(pControl, 0);
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		// We also need to recognize header sub-items
		{
			ASSERT( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ); 
			
			ret = m_pHeader->AddItem(pControl);
			CListHeaderItemUI *pHeaderItem; 
			
			pHeaderItem = static_cast<CListHeaderItemUI*>(pControl); 
			pHeaderItem->SetOwner( this ); 
			m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS); 
		}
		break; 
	default:
		{
			if( FALSE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) )
			{
				ASSERT( FALSE ); 
				break; 
			}

			// The list items should know about us

			/************************************************************************
			add list item time:
			1.when parse xml get the item of the list header.
			2.manual add the item of the list body.

			list have 2 sub container, use one contianer interface to handel it.

			the method to fix the sub item locating bug:
			1.add the new ui item class to the xml.
			2.list use one interface to handle 3 type items.but some function need get the type tip of the tiem.
			3.add the item when parse xml use different interface.
			************************************************************************/
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface( UI_LIST_ITEM_INTERFACE_NAME));
			
			if( pListItem != NULL ) 
			{
				pListItem->SetOwner(this);
				pListItem->SetIndex(m_pList->GetItemCount());
			}
			else
			{
				ASSERT( FALSE && "why adding list item that's not the control of the list" ); 
			}

			ret = m_pList->AddItem(pControl);
		}	
		break; 
	}

	return ret; 
}

bool CListUI::AddSubItemAt(CControlUI* pControl, INT32 iIndex, ULONG ClassId )
{
    // Override the AddAt() method so we can add items specifically to
    // the intended widgets. Headers and are assumed to be
    // answer the correct interface so we can add multiple list headers.
	switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		{
			ASSERT( pControl->GetInterface( _T("ListHeader") ) != NULL ); 

			if( m_pHeader != pControl && m_pHeader->GetItemCount() == 0 ) 
			{
				CVerticalLayoutUI::RemoveItem(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}

			m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddItemAt(pControl, 0);
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		{
			// We also need to recognize header sub-items
			ASSERT( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ); 

			bool ret = m_pHeader->AddItemAt(pControl, iIndex);
			CListHeaderItemUI *pHeaderItem; 
			pHeaderItem = static_cast<CListHeaderItemUI*>(pControl); 
			pHeaderItem->SetOwner( this ); 

			m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		break; 

	default:
		{
			ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 

			if (!m_pList->AddItemAt(pControl, iIndex)) 
			{
				return false;
			}

			// The list items should know about us
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
			if( pListItem != NULL ) {
				pListItem->SetOwner(this);
				pListItem->SetIndex(iIndex);
			}

			for(int i = iIndex + 1; i < m_pList->GetItemCount(); ++i) {
				CControlUI* p = m_pList->GetItemAt(i);
				pListItem = static_cast<IListItemUI*>(p->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
				if( pListItem != NULL ) {
					pListItem->SetIndex(i);
				}
			}

			if( m_iCurSel >= iIndex ) m_iCurSel += 1; 
		}
		break; 
	}
    return true;
}

bool CListUI::RemoveSubItem(CControlUI* pControl, ULONG ClassId )
{
	switch( ClassId )
	{
	case UI_LIST_HEADER_CLASS_ID:
		{
			ASSERT( pControl->GetInterface(_T("ListHeader")) != NULL ); 
			return CVerticalLayoutUI::RemoveItem(pControl);
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		{
			// We also need to recognize header sub-items
			ASSERT( _tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL ); 
			return m_pHeader->RemoveItem(pControl);
		}
		break; 
	default:
		{
			ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 

			int iIndex = m_pList->GetItemIndex(pControl);
			if (iIndex == -1) return false;

			//if (!m_pList->RemoveItemAt(iIndex)) return false;

			for(int i = iIndex; i < m_pList->GetItemCount(); ++i) {
				CControlUI* p = m_pList->GetItemAt(i);
				IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
				if( pListItem != NULL ) {
					pListItem->SetIndex(i);
				}
			}

			if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
				int iSel = m_iCurSel;
				m_iCurSel = -1;
				SelectListItem(FindSelectable(iSel, false));
			}
			else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
			return true;
		}
		break; 
	}
}

bool CListUI::RemoveSubItemAt(int iIndex, ULONG ClassId )
{
	switch( ClassId ) 
	{
	case UI_LIST_HEADER_CLASS_ID: 
		{
			return CVerticalLayoutUI::RemoveItemAt( iIndex );
		}
		break; 
	case UI_LIST_HEADER_ITEM_CLASS_ID:
		{
			// We also need to recognize header sub-items
			return m_pHeader->RemoveItemAt( iIndex );
		}
		break; 
	default:
		{
			ASSERT( TRUE == IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) ); 

			//if (!m_pList->RemoveItemAt(iIndex)) return false;

			for(int i = iIndex; i < m_pList->GetItemCount(); ++i) {
				CControlUI* p = m_pList->GetItemAt(i);
				IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
				if( pListItem != NULL ) pListItem->SetIndex(i);
			}

			if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
				int iSel = m_iCurSel;
				m_iCurSel = -1;
				SelectListItem(FindSelectable(iSel, false));
			}
			else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
			return true;
		}
		break; 
	}
}

void CListUI::RemoveSubAllItem( ULONG ClassId )
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
			m_pList->RemoveAllItem();
		}
		break; 
	}
}

//void CListUI::SetPos(RECT rc)
//{
//    CVerticalLayoutUI::SetPos(rc);
//    if( m_pHeader == NULL ) return;
//    // Determine general list information and the size of header columns
//    m_ListInfo.nColumns = MIN(m_pHeader->GetItemCount(), UILIST_MAX_COLUMNS);
//    // The header/columns may or may not be visible at runtime. In either case
//    // we should determine the correct dimensions...
//
//    if( !m_pHeader->IsVisible() ) {
//        for( int it = 0; it < m_pHeader->GetItemCount(); it++ ) {
//            static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
//        }
//        m_pHeader->SetPos(CRect(rc.left, 0, rc.right, 0));
//    }
//    int iOffset = m_pList->GetScrollPos().cx;
//    for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
//        CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
//        if( !pControl->IsVisible() ) continue;
//        if( pControl->IsFloat() ) continue;
//
//        RECT rcPos = pControl->GetPos();
//        if( iOffset > 0 ) {
//            rcPos.left -= iOffset;
//            rcPos.right -= iOffset;
//            pControl->SetPos(rcPos);
//        }
//        m_ListInfo.rcColumn[i] = pControl->GetPos();
//    }
//
//	/*************************************************************
//	list drawing performance factor:
//	1.all item drawing.
//
//	current direct ui library bug:
//	1.must traverse all item in the container to set it space.when drawing container.
//	don't care that item will not display.
//
//	2.must traverse all item in the container to check whether it will 
//	display.
//	*************************************************************/
//    if( !m_pHeader->IsVisible() ) {
//        for( int it = 0; it < m_pHeader->GetItemCount(); it++ ) {
//            static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
//        }
//    }
//}
void CListUI::SetPos(RECT rc)
{
	INT32 HeaderHeight; 
	INT32 HeaderWidth; 

	//ASSERT( HeaderHeight != 0 ); 
	CControlUI::SetPos(rc);

	do 
	{
		if( m_pHeader == NULL ) 
		{
			DBG_BP(); 
			//ASSERT( FALSE && "list header is null" ); 

			break; 
		}

		if( !m_pHeader->IsVisible() )
		{
			HeaderHeight = 0; 
			HeaderWidth = 0; 
		}
		else
		{
			HeaderHeight = m_pHeader->GetFixedHeight(); 
			HeaderWidth = m_pHeader->GetFixedWidth(); 
		}

		rc = m_rcItem; 

		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

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


	ASSERT( m_pList != NULL ); 

	m_pList->SetPos( CRect( rc.left, rc.top + HeaderHeight, rc.right, rc.bottom ) ); 
}

void CListBaseUI::DoEvent(TEventUI& event)
{
    CVerticalLayoutUI::DoEvent(event);
}

void CListUI::DoEvent(TEventUI& event)
{
	do 
	{
		if( !IsMouseEnabled() 
			&& event.Type > UIEVENT__MOUSEBEGIN 
			&& event.Type < UIEVENT__MOUSEEND ) 
		{
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CListBaseUI::DoEvent(event); 
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

		{
			BOOLEAN handled = FALSE; 
			switch( event.Type ) 
			{
			case UIEVENT_KEYDOWN:
				switch( event.chKey )
				{
				case 'W': 
				case 'w':
				case VK_UP:
					SelectListItem( FindSelectable(m_iCurSel - 1, false ));
					handled = TRUE; 
					break; 
				case 'S':
				case 's':
				case VK_DOWN:
					SelectListItem( FindSelectable( m_iCurSel + 1, true ) );
					handled = TRUE; 
					break; 
				case VK_PRIOR:
					PageUp();
					handled = TRUE; 
					break; 
				case VK_NEXT:
					PageDown();
					handled = TRUE; 
					break; 
				case VK_HOME:
					SelectListItem( FindSelectable( 0, false ) );
					handled = TRUE; 
					break; 
				case VK_END:
					SelectListItem( FindSelectable( GetItemCount() - 1, true ) );
					handled = TRUE; 
					break; 
				case 'A':
				case 'a':
				case VK_LEFT: 
					{
						SIZE sz = GetScrollPos(); 

#define LIST_UI_ITEM_HORIZONTAL_MOVE_FACTOR ( LONG )( 4 * 16 )
						handled = TRUE; 
						sz.cx += ( -LIST_UI_ITEM_HORIZONTAL_MOVE_FACTOR ); 

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
						sz.cx += ( LIST_UI_ITEM_HORIZONTAL_MOVE_FACTOR ); 

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
				case VK_RETURN:
					if( m_iCurSel != -1 ) 
					{
						GetSubItemAt( m_iCurSel, UI_LIST_ELEMENT_CLASS_ID )->Activate();
					}
					handled = TRUE; 
					break; 
				}
				break;
			case UIEVENT_SCROLLWHEEL:
				{
					switch( LOWORD(event.wParam) ) 
					{
					case SB_LINEUP:
						if( m_bScrollSelect ) 
						{
							SelectListItem(FindSelectable(m_iCurSel - 1, false));
						}
						else 
						{
							LineUp(); 
						}
						
						handled = TRUE; 
						break; 
					case SB_LINEDOWN:
						if( m_bScrollSelect ) 
						{
							SelectListItem(FindSelectable(m_iCurSel + 1, true));
						}
						else 
						{
							LineDown();
						}
						
						handled = TRUE; 
						break; 
					case SB_PAGEUP: 
						PageUp(); 
						handled = TRUE; 
						break; 

					case SB_PAGEDOWN: 
						PageDown(); 
						handled = TRUE; 
						break; 
					}
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
					POINT released_pos; 
					SIZE moved_pos; 

					if( button_pressed == FALSE )
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
					else if( moved_pos.cy > GetScrollRange().cy )
					{
						moved_pos.cy = GetScrollRange().cy; 
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
			}

			if( handled == FALSE )
			{
				CListBaseUI::DoEvent(event); 
			}
		}
	}while( FALSE );

	return; 
}

CListHeaderUI* CListBaseUI::GetHeader() const
{
    return m_pHeader;
}

CListBodyUI* CListUI::GetList() const
{
    return m_pList;
}

bool CListBaseUI::GetScrollSelect()
{
    return m_bScrollSelect;
}

void CListBaseUI::SetScrollSelect(bool bScrollSelect)
{
    m_bScrollSelect = bScrollSelect;
}

int CListBaseUI::GetCurSel() const
{
    return m_iCurSel;
}

bool CListUI::SelectListItem(int iIndex)
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
        if( pControl != NULL) {
            IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
            if( pListItem != NULL ) pListItem->Select(false);
        }

        m_iCurSel = -1;
    }
    if( iIndex < 0 ) 
	{
		return false;
	}

    CControlUI* pControl = GetSubItemAt( iIndex, UI_LIST_ELEMENT_CLASS_ID );
    if( pControl == NULL ) return false;
    if( !pControl->IsVisible() ) return false;
    if( !pControl->IsEnabled() ) return false;

    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
    if( pListItem == NULL ) return false;
    m_iCurSel = iIndex;
    if( !pListItem->Select(true) ) {
        m_iCurSel = -1;
        return false;
    }
    EnsureVisible(m_iCurSel);
    //pControl->SetFocus();
    if( m_pManager != NULL ) {
        m_pManager->SendNotify(this, _T("itemselect"), m_iCurSel, iOldSel);
    }

    return true;
}

TListInfoUI* CListBaseUI::GetListInfo()
{
    return &m_ListInfo;
}

//int CListBaseUI::GetChildPadding() const
//{
//	return 0; 
//    //return m_pList->GetChildPadding();
//}

//void CListBaseUI::SetChildPadding(int iPadding)
//{
//    //m_pList->SetChildPadding(iPadding);
//}

int CListUI::GetChildPadding() const
{
	return m_pList->GetChildPadding();
}

void CListUI::SetChildPadding(int iPadding)
{
	m_pList->SetChildPadding(iPadding);
}

void CListBaseUI::SetItemFont(int index)
{
    m_ListInfo.nFont = index;
    NeedUpdate();
}

void CListBaseUI::SetItemTextStyle(UINT uStyle)
{
    m_ListInfo.uTextStyle = uStyle;
    NeedUpdate();
}

void CListBaseUI::SetItemTextPadding(RECT rc)
{
    m_ListInfo.rcTextPadding = rc;
    NeedUpdate();
}

RECT CListBaseUI::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CListBaseUI::SetItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwTextColor = dwTextColor;
    Invalidate();
}

void CListBaseUI::SetItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwBkColor = dwBkColor;
    Invalidate();
}

void CListBaseUI::SetItemBkImage(LPCTSTR pStrImage)
{
    m_ListInfo.sBkImage = pStrImage;
    Invalidate();
}

void CListBaseUI::SetAlternateBk(bool bAlternateBk)
{
    m_ListInfo.bAlternateBk = bAlternateBk;
    Invalidate();
}

DWORD CListBaseUI::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CListBaseUI::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CListBaseUI::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CListBaseUI::IsAlternateBk() const
{
    return m_ListInfo.bAlternateBk;
}

void CListBaseUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwSelectedTextColor = dwTextColor;
    Invalidate();
}

void CListBaseUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwSelectedBkColor = dwBkColor;
    Invalidate();
}

void CListBaseUI::SetSelectedItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sSelectedImage = pStrImage;
    Invalidate();
}

DWORD CListBaseUI::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CListBaseUI::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CListBaseUI::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CListBaseUI::SetHotItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwHotTextColor = dwTextColor;
    Invalidate();
}

void CListBaseUI::SetHotItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwHotBkColor = dwBkColor;
    Invalidate();
}

void CListBaseUI::SetHotItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sHotImage = pStrImage;
    Invalidate();
}

DWORD CListBaseUI::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CListBaseUI::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CListBaseUI::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CListBaseUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwDisabledTextColor = dwTextColor;
    Invalidate();
}

void CListBaseUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwDisabledBkColor = dwBkColor;
    Invalidate();
}

void CListBaseUI::SetDisabledItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sDisabledImage = pStrImage;
    Invalidate();
}

DWORD CListBaseUI::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CListBaseUI::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CListBaseUI::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CListBaseUI::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CListBaseUI::SetItemLineColor(DWORD dwLineColor)
{
    m_ListInfo.dwLineColor = dwLineColor;
    Invalidate();
}

bool CListBaseUI::IsItemShowHtml()
{
    return m_ListInfo.bShowHtml;
}

void CListBaseUI::SetItemShowHtml(bool bShowHtml)
{
    if( m_ListInfo.bShowHtml == bShowHtml ) return;

    m_ListInfo.bShowHtml = bShowHtml;
    NeedUpdate();
}

void CListBaseUI::SetMultiExpanding(bool bMultiExpandable)
{
    m_ListInfo.bMultiExpandable = bMultiExpandable;
}

bool CListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
{
    if( m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
        CControlUI* pControl = GetSubItemAt(m_iExpandedItem);
        if( pControl != NULL ) {
            IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
            if( pItem != NULL ) pItem->Expand(false);
        }
        m_iExpandedItem = -1;
    }
    if( bExpand ) {
        CControlUI* pControl = GetSubItemAt(iIndex);
        if( pControl == NULL ) return false;
        if( !pControl->IsVisible() ) return false;
        IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(UI_LIST_ITEM_INTERFACE_NAME));
        if( pItem == NULL ) return false;
        m_iExpandedItem = iIndex;
        if( !pItem->Expand(true) ) {
            m_iExpandedItem = -1;
            return false;
        }
    }
    NeedUpdate();
    return true;
}

int CListBaseUI::GetExpandedItem() const
{
    return m_iExpandedItem;
}

void CListUI::EnsureVisible(int iIndex)
{
    if( m_iCurSel < 0 ) return;
    RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pList->GetPos();
    RECT rcListInset = m_pList->GetInset();

    rcList.left += rcListInset.left;
    rcList.top += rcListInset.top;
    rcList.right -= rcListInset.right;
    rcList.bottom -= rcListInset.bottom;

    CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
    if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

    //int iPos = m_pList->GetScrollPos().cy;
    if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) 
	{
		return;
	}
	
	int dx = 0;
    if( rcItem.top < rcList.top ) 
	{
		dx = rcItem.top - rcList.top;
	}

	if( rcItem.bottom > rcList.bottom ) 
	{
		dx = rcItem.bottom - rcList.bottom;
	}
	
	Scroll( 0, dx );
}

void CListUI::Scroll(int dx, int dy)
{
    if( dx == 0 && dy == 0 ) return;
    SIZE sz = m_pList->GetScrollPos();
    m_pList->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

void CListBaseUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
    else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
    else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
    else if( _tcscmp(pstrName, _T("itemalign")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_ListInfo.uTextStyle |= DT_RIGHT;
        }
    }
    else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
        else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
    }    
    if( _tcscmp(pstrName, _T("itemtextpadding")) == 0 ) {
        RECT rcTextPadding = { 0 };
        LPTSTR pstr = NULL;
        rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetItemTextPadding(rcTextPadding);
    }
    else if( _tcscmp(pstrName, _T("itemtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itembkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itembkimage")) == 0 ) SetItemBkImage(pstrValue);
    else if( _tcscmp(pstrName, _T("itemaltbk")) == 0 ) SetAlternateBk(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("itemselectedtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemselectedbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetSelectedItemBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemselectedimage")) == 0 ) SetSelectedItemImage(pstrValue);
    else if( _tcscmp(pstrName, _T("itemhottextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotItemTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemhotbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotItemBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemhotimage")) == 0 ) SetHotItemImage(pstrValue);
    else if( _tcscmp(pstrName, _T("itemdisabledtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledItemTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledItemBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemdisabledimage")) == 0 ) SetDisabledItemImage(pstrValue);
    else if( _tcscmp(pstrName, _T("itemlinecolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemLineColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("itemshowhtml")) == 0 ) SetItemShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
    else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

IListCallbackUI* CListBaseUI::GetListCallback() const
{
    return m_pCallback;
}

IListHeaderCallbackUI* CListBaseUI::GetListHeaderCallback() const
{
	return m_pListHeaderCallback;
}

BOOLEAN CListBaseUI::ColumnWidthChanged()
{
	return m_bColumnWidthChanged; 
}

void CListBaseUI::SetListHeaderCallback(IListHeaderCallbackUI* pCallback)
{
	m_pListHeaderCallback = pCallback;
}

void CListBaseUI::SetListCallback(IListCallbackUI* pCallback)
{
    m_pCallback = pCallback;
}

SIZE CListUI::GetScrollPos() const
{
    return m_pList->GetScrollPos();
}

SIZE CListUI::GetScrollRange() const
{
    return m_pList->GetScrollRange();
}

void CListUI::SetScrollPos(SIZE szPos)
{
    m_pList->SetScrollPos(szPos);
}

void CListUI::LineUp()
{
    m_pList->LineUp();
}

void CListUI::LineDown()
{
    m_pList->LineDown();
}

void CListUI::PageUp()
{
    m_pList->PageUp();
}

void CListUI::PageDown()
{
    m_pList->PageDown();
}

void CListUI::HomeUp()
{
    m_pList->HomeUp();
}

void CListUI::EndDown()
{
    m_pList->EndDown();
}

void CListUI::LineLeft()
{
    m_pList->LineLeft();
}

void CListUI::LineRight()
{
    m_pList->LineRight();
}

void CListUI::PageLeft()
{
    m_pList->PageLeft();
}

void CListUI::PageRight()
{
    m_pList->PageRight();
}

void CListUI::HomeLeft()
{
    m_pList->HomeLeft();
}

void CListUI::EndRight()
{
    m_pList->EndRight();
}

void CListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
    m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

CScrollBarUI* CListUI::GetVerticalScrollBar() const
{
    return m_pList->GetVerticalScrollBar();
}

CScrollBarUI* CListUI::GetHorizontalScrollBar() const
{
    return m_pList->GetHorizontalScrollBar();
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CListHeaderUI::CListHeaderUI()
{
}

LPCTSTR CListHeaderUI::GetClass() const
{
    return UI_LIST_HEADER_CLASS_NAME;
}

void CListHeaderUI::SetPos(RECT rc)
{
	return __super::SetPos( rc ); 
}

ULONG CListHeaderUI::GetClassId() const
{
	return UI_LIST_HEADER_CLASS_ID;
}

LPVOID CListHeaderUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListHeader")) == 0 ) return this;
    return CHorizontalLayoutUI::GetInterface(pstrName);
}

SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
{
    SIZE cXY = {0, m_cxyFixed.cy};
	if( cXY.cy == 0 && m_pManager != NULL ) {
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			cXY.cy = MAX(cXY.cy,static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
		}
		int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6;
		cXY.cy = MAX(cXY.cy,nMin);
	}

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        cXY.cx +=  static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
    }

    return cXY;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListHeaderItemUI::CListHeaderItemUI() 
							: m_pOwner(NULL), 
							m_bDragable(true), 
							m_uButtonState(0), 
							m_iSepWidth(4),
							m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), 
							m_dwTextColor(0), 
							m_iFont(-1), 
							m_bShowHtml(false)
{
	SetTextPadding(CRect(2, 0, 2, 0));
    ptLastMouse.x = ptLastMouse.y = 0;
    SetMinWidth(16);
}

LPCTSTR CListHeaderItemUI::GetClass() const
{
    return UI_LIST_HEADER_ITEM_CLASS_NAME;
}

ULONG CListHeaderItemUI::GetClassId() const
{
	return UI_LIST_HEADER_ITEM_CLASS_ID;
}

IListUI* CListHeaderItemUI::GetOwner()
{
	return m_pOwner;
}

void CListHeaderItemUI::SetOwner(CControlUI* pOwner)
{
	m_pOwner = static_cast<IListUI*>(pOwner->GetInterface(_T("IList")));
}

LPVOID CListHeaderItemUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("ListHeaderItem")) == 0 ) return this;
    return CControlUI::GetInterface(pstrName);
}

UINT CListHeaderItemUI::GetControlFlags() const
{
    if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
    else return 0;
}

void CListHeaderItemUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

bool CListHeaderItemUI::IsDragable() const
{
	return m_bDragable;
}

void CListHeaderItemUI::SetDragable(bool bDragable)
{
    m_bDragable = bDragable;
    if ( !m_bDragable ) m_uButtonState &= ~UISTATE_CAPTURED;
}

DWORD CListHeaderItemUI::GetSepWidth() const
{
	return m_iSepWidth;
}

void CListHeaderItemUI::SetSepWidth(int iWidth)
{
    m_iSepWidth = iWidth;
}

DWORD CListHeaderItemUI::GetTextStyle() const
{
	return m_uTextStyle;
}

void CListHeaderItemUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

DWORD CListHeaderItemUI::GetTextColor() const
{
	return m_dwTextColor;
}


void CListHeaderItemUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
}

RECT CListHeaderItemUI::GetTextPadding() const
{
	return m_rcTextPadding;
}

void CListHeaderItemUI::SetTextPadding(RECT rc)
{
	m_rcTextPadding = rc;
	Invalidate();
}

void CListHeaderItemUI::SetFont(int index)
{
    m_iFont = index;
}

bool CListHeaderItemUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CListHeaderItemUI::SetShowHtml(bool bShowHtml)
{
    if( m_bShowHtml == bShowHtml ) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetNormalImage() const
{
	return m_sNormalImage;
}

void CListHeaderItemUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetHotImage() const
{
    return m_sHotImage;
}

void CListHeaderItemUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetPushedImage() const
{
    return m_sPushedImage;
}

void CListHeaderItemUI::SetPushedImage(LPCTSTR pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetFocusedImage() const
{
    return m_sFocusedImage;
}

void CListHeaderItemUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CListHeaderItemUI::GetSepImage() const
{
    return m_sSepImage;
}

void CListHeaderItemUI::SetSepImage(LPCTSTR pStrImage)
{
    m_sSepImage = pStrImage;
    Invalidate();
}

void CListHeaderItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("dragable")) == 0 ) SetDragable(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("sepwidth")) == 0 ) SetSepWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("align")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_uTextStyle |= DT_RIGHT;
        }
    }
    else if( _tcscmp(pstrName, _T("endellipsis")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
        else m_uTextStyle &= ~DT_END_ELLIPSIS;
    }    
    else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
    }
	else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
		RECT rcTextPadding = { 0 };
		LPTSTR pstr = NULL;
		rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetTextPadding(rcTextPadding);
	}
    else if( _tcscmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("sepimage")) == 0 ) SetSepImage(pstrValue);
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CListHeaderItemUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;
        RECT rcSeparator = GetThumbRect();
        if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
            if( m_bDragable ) {
                m_uButtonState |= UISTATE_CAPTURED;
                ptLastMouse = event.ptMouse;
            }
        }
        else {
            m_uButtonState |= UISTATE_PUSHED;
            m_pManager->SendNotify(this, _T("headerclick"));
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            m_uButtonState &= ~UISTATE_CAPTURED;
            if( GetParent() ) 
                GetParent()->NeedParentUpdate();
        }
        else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
            m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
		{
            RECT rc = m_rcItem;
            if( m_iSepWidth >= 0 ) 
			{
                rc.right -= ptLastMouse.x - event.ptMouse.x;
            }
            else 
			{
                rc.left -= ptLastMouse.x - event.ptMouse.x;
            }
            
            if( rc.right - rc.left > GetMinWidth() )
			{
				//ASSERT( m_pOwner != NULL ); 

				//IListHeaderCallbackUI* pCallback = m_pOwner->GetListHeaderCallback();
				if( 0 == GetName().CompareNoCase( _T( "event_list_layout" ) ) )
				{
					DBG_BP(); 
				}

                m_cxyFixed.cx = rc.right - rc.left;

				//if( pCallback != NULL )
				//{
				//	pCallback->NotifyHeaderDragged( this ); 
				//}

                ptLastMouse = event.ptMouse;
                if( GetParent() ) 
                    GetParent()->NeedParentUpdate();
            }
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        RECT rcSeparator = GetThumbRect();
        if( IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse) ) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
            return;
        }
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CControlUI::DoEvent(event);
}

SIZE CListHeaderItemUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14);
    return CControlUI::EstimateSize(szAvailable);
}

RECT CListHeaderItemUI::GetThumbRect() const
{
    if( m_iSepWidth >= 0 ) return CRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
    else return CRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
}

void CListHeaderItemUI::PaintStatusImage(HDC hDC)
{
    if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~ UISTATE_FOCUSED;

    if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
        if( m_sPushedImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage);
        if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ) m_sPushedImage.Empty();
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( m_sHotImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage);
        if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
    }
    else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
        if( m_sFocusedImage.IsEmpty() && !m_sNormalImage.IsEmpty() ) DrawImage(hDC, (LPCTSTR)m_sNormalImage);
        if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
    }
    else {
        if( !m_sNormalImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
        }
    }

    if( !m_sSepImage.IsEmpty() ) {
        RECT rcThumb = GetThumbRect();
        rcThumb.left -= m_rcItem.left;
        rcThumb.top -= m_rcItem.top;
        rcThumb.right -= m_rcItem.left;
        rcThumb.bottom -= m_rcItem.top;

        m_sSepImageModify.Empty();
        m_sSepImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if( !DrawImage(hDC, (LPCTSTR)m_sSepImage, (LPCTSTR)m_sSepImageModify) ) m_sSepImage.Empty();
    }
}

void CListHeaderItemUI::PaintText(HDC hDC)
{
    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();

	RECT rcText = m_rcItem;
	rcText.left += m_rcTextPadding.left;
	rcText.top += m_rcTextPadding.top;
	rcText.right -= m_rcTextPadding.right;
	rcText.bottom -= m_rcTextPadding.bottom;

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    if( m_bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, m_dwTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, m_dwTextColor, \
        m_iFont, DT_SINGLELINE | m_uTextStyle);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListElementUI::CListElementUI() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_bSelected(false),
m_uButtonState(0)
{
}

LPCTSTR CListElementUI::GetClass() const
{
    return UI_LIST_ELEMENT_CLASS_NAME;
}

ULONG CListElementUI::GetClassId() const
{
	return UI_LIST_ELEMENT_CLASS_ID;
}

UINT CListElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

LPVOID CListElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_LIST_ITEM_INTERFACE_NAME ) == 0 ) 
	{
		return static_cast<IListItemUI*>(this);
	}
	
	if( _tcscmp(pstrName, UI_LIST_ELEMENT_INTERFACE_NAME) == 0 ) 
	{
		return static_cast<CListElementUI*>(this);
	}
	
	return CControlUI::GetInterface(pstrName);
}

IListOwnerUI* CListElementUI::GetOwner()
{
    return m_pOwner;
}

void CListElementUI::SetOwner(CControlUI* pOwner)
{
    m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(UI_LIST_OWNER_INTERFACE_NAME));
	ASSERT( m_pOwner != NULL ); 
}

void CListElementUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if( !IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if( m_pOwner != NULL ) m_pOwner->SelectListItem(-1);
    }
}

void CListElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CListElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListElementUI::Invalidate()
{
    if( !IsVisible() ) return;

    if( GetParent() ) {
        CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
        if( pParentContainer ) {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
            CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if( !::IntersectRect(&invalidateRc, &m_rcItem, &rc) ) 
            {
                return;
            }

            CControlUI* pParent = GetParent();
            RECT rcTemp;
            RECT rcParent;
            while( pParent = pParent->GetParent() )
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
                {
                    return;
                }
            }

            if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
        }
        else {
            CControlUI::Invalidate();
        }
    }
    else {
        CControlUI::Invalidate();
    }
}

bool CListElementUI::Activate()
{
    if( !CControlUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"));
    return true;
}

bool CListElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListElementUI::Select(bool bSelect)
{
    if( !IsEnabled() ) return false;
    if( bSelect == m_bSelected ) return true;
    m_bSelected = bSelect;
    if( bSelect && m_pOwner != NULL ) m_pOwner->SelectListItem(m_iIndex);
    Invalidate();

    return true;
}

bool CListElementUI::IsExpanded() const
{
    return false;
}

bool CListElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            Activate();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_KEYDOWN && IsEnabled() )
    {
        if( event.chKey == VK_RETURN ) {
            Activate();
            Invalidate();
            return;
        }
    }
    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CListElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
    ASSERT(m_pOwner);
    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iBackColor = 0;
    if( m_iIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;
    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        iBackColor = pInfo->dwHotBkColor;
    }
    if( IsSelected() ) {
        iBackColor = pInfo->dwSelectedBkColor;
    }
    if( !IsEnabled() ) {
        iBackColor = pInfo->dwDisabledBkColor;
    }

    if ( iBackColor != 0 ) {
        CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
    }

    if( !IsEnabled() ) {
        if( !pInfo->sDisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage) ) pInfo->sDisabledImage.Empty();
            else return;
        }
    }
    if( IsSelected() ) {
        if( !pInfo->sSelectedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage) ) pInfo->sSelectedImage.Empty();
            else return;
        }
    }
    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( !pInfo->sHotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
            else return;
        }
    }

    if( !m_sBkImage.IsEmpty() ) {
        if( m_iIndex % 2 == 0 ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
        }
    }

    if( m_sBkImage.IsEmpty() ) {
        if( !pInfo->sBkImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
            else return;
        }
    }

    if ( pInfo->dwLineColor != 0 ) {
        RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
        CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListLabelElementUI::CListLabelElementUI()
{
}

LPCTSTR CListLabelElementUI::GetClass() const
{
    return UI_LIST_LABEL_ELEMENT_CLASS_NAME;
}

ULONG CListLabelElementUI::GetClassId() const
{
	return UI_LIST_LABEL_ELEMENT_CLASS_ID;
}

LPVOID CListLabelElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_LIST_LABEL_ELEMENT_INTERFACE_NAME) == 0 ) return static_cast<CListLabelElementUI*>(this);
    return CListElementUI::GetInterface(pstrName);
}

void CListLabelElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CListElementUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_RBUTTONDOWN )
    {
        if( IsEnabled() ) {
            m_pManager->SendNotify(this, _T("itemclick"));
            Select();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE ) 
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( (m_uButtonState & UISTATE_HOT) != 0 ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CListElementUI::DoEvent(event);
}

SIZE CListLabelElementUI::EstimateSize(SIZE szAvailable)
{
    if( m_pOwner == NULL ) return CSize(0, 0);

    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    SIZE cXY = m_cxyFixed;
    if( cXY.cy == 0 && m_pManager != NULL ) {
        cXY.cy = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
        cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    if( cXY.cx == 0 && m_pManager != NULL ) {
        RECT rcText = { 0, 0, 9999, cXY.cy };
        if( pInfo->bShowHtml ) {
            int nLinks = 0;
            CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, 0, NULL, NULL, nLinks, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
        }
        else {
            CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, 0, pInfo->nFont, DT_SINGLELINE | DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
        }
        cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right;        
    }

    return cXY;
}

void CListLabelElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    DrawItemBk(hDC, m_rcItem);
    DrawItemText(hDC, m_rcItem);
}

VOID dump_list_info( TListInfoUI *pInfo )
{
	TRACE( _T( "enter %s \n" ), __TFUNCTION__ ); 
	TRACE( _T( "column count is %d \n" ), pInfo->nColumns ); 
	TRACE( _T( "font number is %d \n" ), pInfo->nFont ); 
	TRACE( _T( "item text color is 0x%0.8x \n" ), pInfo->uTextStyle ); 
	TRACE( _T( "item text padding is ( %d, %d, %d, %d) \n" ), 
		pInfo->rcTextPadding.left, 
		pInfo->rcTextPadding.top,
		pInfo->rcTextPadding.right,
		pInfo->rcTextPadding.bottom ); 
	TRACE( _T( "item text color is 0x%0.8x \n" ), pInfo->dwTextColor ); 
	TRACE( _T( "item bk color is 0x%0.8x \n" ), pInfo->dwBkColor ); 
	TRACE( _T( "item bk image is %s \n" ), pInfo->sBkImage.GetData() ); 
	TRACE( _T( "alternate bk is %d \n" ), pInfo->bAlternateBk ); 
	TRACE( _T( "item selected text color is 0x%0.8x \n" ), pInfo->dwSelectedTextColor ); 
	TRACE( _T( "item selected bk color is 0x%0.8x \n" ), pInfo->dwSelectedBkColor ); 
	TRACE( _T( "item hot text color is 0x%0.8x \n" ), pInfo->dwHotTextColor ); 
	TRACE( _T( "item hot bk color is 0x%0.8x \n" ), pInfo->dwHotBkColor ); 
	TRACE( _T( "item hot image is %s \n" ), pInfo->sHotImage.GetData() ); 
	TRACE( _T( "item disabled text color is 0x%0.8x \n" ), pInfo->dwDisabledTextColor ); 
	TRACE( _T( "item disabled bk color is 0x%0.8x \n" ), pInfo->dwDisabledBkColor ); 
	TRACE( _T( "item disabled image is %s \n" ), pInfo->sDisabledImage.GetData() ); 
	TRACE( _T( "item line color is 0x%0.8x \n" ), pInfo->dwLineColor ); 
	TRACE( _T( "item text color is %d \n" ), pInfo->bShowHtml ); 
	TRACE( _T( "item text color is %d \n" ), pInfo->bMultiExpandable ); 

	TRACE( _T( "leave %s \n" ), __TFUNCTION__ ); 
//_retunr:
	return; 
}

void CListLabelElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
	TListInfoUI* pInfo; 
	DWORD iTextColor; 
	int nLinks = 0;
	RECT rcText = rcItem;

	TRACE( _T( "enter %s \n" ), __TFUNCTION__ ); 

    if( m_sText.IsEmpty() ) 
	{
		goto _return;
	}

    if( m_pOwner == NULL ) 
	{
		goto _return; 
	}
	pInfo = m_pOwner->GetListInfo();
    iTextColor = pInfo->dwTextColor;
    
	dump_list_info( pInfo ); ; 

	if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
        iTextColor = pInfo->dwHotTextColor;
    }

    if( IsSelected() ) 
	{
        iTextColor = pInfo->dwSelectedTextColor;
    }

    if( !IsEnabled() ) 
	{
        iTextColor = pInfo->dwDisabledTextColor;
    }
    
	rcText.left += pInfo->rcTextPadding.left;
    rcText.right -= pInfo->rcTextPadding.right;
    rcText.top += pInfo->rcTextPadding.top;
    rcText.bottom -= pInfo->rcTextPadding.bottom;

    if( pInfo->bShowHtml )
	{
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
	}
	else
	{
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
	}

_return:
	TRACE( _T( "leave %s \n" ), __TFUNCTION__ ); 
	return;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CListTextElementUI::CListTextElementUI() : m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL)
{
    ::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
}

CListTextElementUI::~CListTextElementUI()
{
    CStdString* pText;
    for( int it = 0; it < m_aTexts.GetSize(); it++ ) {
        pText = static_cast<CStdString*>(m_aTexts[it]);
        if( pText ) delete pText;
    }
    m_aTexts.Empty();
}

LPCTSTR CListTextElementUI::GetClass() const
{
    return UI_LIST_TEXT_ELEMENT_CLASS_NAME;
}

ULONG CListTextElementUI::GetClassId() const
{
	return UI_LIST_TEXT_ELEMENT_CLASS_ID;
}

LPVOID CListTextElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_LIST_TEXT_ELEMENT_INTERFACE_NAME) == 0 ) return static_cast<CListTextElementUI*>(this);
    return CListLabelElementUI::GetInterface(pstrName);
}

UINT CListTextElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN | ( (IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
}

LPCTSTR CListTextElementUI::GetText(int iIndex) const
{
    CStdString* pText = static_cast<CStdString*>(m_aTexts.GetAt(iIndex));
    if( pText ) return pText->GetData();
    return NULL;
}

void CListTextElementUI::SetText(int iIndex, LPCTSTR pstrText)
{
    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
    while( m_aTexts.GetSize() < pInfo->nColumns ) { m_aTexts.Add(NULL); }

    CStdString* pText = static_cast<CStdString*>(m_aTexts[iIndex]);
    if( (pText == NULL && pstrText == NULL) || (pText && *pText == pstrText) ) return;

    if( pText ) delete pText;
    m_aTexts.SetAt(iIndex, new CStdString(pstrText));
    Invalidate();
}

void CListTextElementUI::SetOwner(CControlUI* pOwner)
{
    CListElementUI::SetOwner(pOwner);
    m_pOwner = static_cast<IListUI*>(pOwner->GetInterface(_T("IList")));
}

CStdString* CListTextElementUI::GetLinkContent(int iIndex)
{
    if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
    return NULL;
}

void CListTextElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CListLabelElementUI::DoEvent(event);
        return;
    }

    // When you hover over a link
    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }      
    }
    if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                m_pManager->SendNotify(this, _T("link"), i);
                return;
            }
        }
    }
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink) {
            Invalidate();
            m_nHoverLink = nHoverLink;
        }
    }
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE ) {
        if(m_nHoverLink != -1) {
            Invalidate();
            m_nHoverLink = -1;
        }
    }
    CListLabelElementUI::DoEvent(event);
}

SIZE CListTextElementUI::EstimateSize(SIZE szAvailable)
{
    TListInfoUI* pInfo = NULL;
    if( m_pOwner ) pInfo = m_pOwner->GetListInfo();

    SIZE cXY = m_cxyFixed;
    if( cXY.cy == 0 && m_pManager != NULL ) {
        cXY.cy = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
        if( pInfo ) cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
    }

    return cXY;
}

void CListTextElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;

    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    IListCallbackUI* pCallback = m_pOwner->GetListCallback();
    //ASSERT(pCallback);
    //if( pCallback == NULL ) return;

    m_nLinks = 0;
    int nLinks = lengthof(m_rcLinks);
    for( int i = 0; i < pInfo->nColumns; i++ )
    {
        RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
        rcItem.left += pInfo->rcTextPadding.left;
        rcItem.right -= pInfo->rcTextPadding.right;
        rcItem.top += pInfo->rcTextPadding.top;
        rcItem.bottom -= pInfo->rcTextPadding.bottom;

        LPCTSTR pstrText = NULL;
        if( pCallback ) pstrText = pCallback->GetItemText(this, m_iIndex, i);
        else pstrText = GetText(i);
		//pstrText = GetText(i); 

		//if( ( *pstrText == _T( '\0' ) || pstrText == NULL ) 
		//	&& pCallback ) 
		//{
		//	pstrText = pCallback->GetItemText(this, m_iIndex, i);
		//}
		if( pInfo->bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rcItem, pstrText, iTextColor, \
                &m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, DT_SINGLELINE | pInfo->uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rcItem, pstrText, iTextColor, \
            pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);

        m_nLinks += nLinks;
        nLinks = lengthof(m_rcLinks) - m_nLinks; 
    }
    for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
        ::ZeroMemory(m_rcLinks + i, sizeof(RECT));
        ((CStdString*)(m_sLinks + i))->Empty();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CListContainerElementUI::CListContainerElementUI() : 
m_iIndex(-1),
m_pOwner(NULL), 
m_bSelected(false),
m_uButtonState(0)
{
}

LPCTSTR CListContainerElementUI::GetClass() const
{
    return UI_LIST_CONTAINER_ELEMENT_CLASS_NAME;
}

ULONG CListContainerElementUI::GetClassId() const
{
	return UI_LIST_CONTAINER_ELEMENT_CLASS_ID;
}

UINT CListContainerElementUI::GetControlFlags() const
{
    return UIFLAG_WANTRETURN;
}

LPVOID CListContainerElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_LIST_ITEM_INTERFACE_NAME) == 0 ) return static_cast<IListItemUI*>(this);
	if( _tcscmp(pstrName, UI_LIST_CONTAINER_ELEMENT_INTERFACE_NAME) == 0 ) return static_cast<CListContainerElementUI*>(this);
    return CContainerUI::GetInterface(pstrName);
}

IListOwnerUI* CListContainerElementUI::GetOwner()
{
    return m_pOwner;
}

void CListContainerElementUI::SetOwner(CControlUI* pOwner)
{
    m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface( UI_LIST_OWNER_INTERFACE_NAME ) );
}

void CListContainerElementUI::SetVisible(bool bVisible)
{
    CContainerUI::SetVisible(bVisible);
    if( !IsVisible() && m_bSelected)
    {
        m_bSelected = false;
        if( m_pOwner != NULL ) m_pOwner->SelectListItem(-1);
    }
}

void CListContainerElementUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CListContainerElementUI::GetIndex() const
{
    return m_iIndex;
}

void CListContainerElementUI::SetIndex(int iIndex)
{
    m_iIndex = iIndex;
}

void CListContainerElementUI::Invalidate()
{
    if( !IsVisible() ) return;

    if( GetParent() ) {
        CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
        if( pParentContainer ) {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
            CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if( !::IntersectRect(&invalidateRc, &m_rcItem, &rc) ) 
            {
                return;
            }

            CControlUI* pParent = GetParent();
            RECT rcTemp;
            RECT rcParent;
            while( pParent = pParent->GetParent() )
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
                {
                    return;
                }
            }

            if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
        }
        else {
            CContainerUI::Invalidate();
        }
    }
    else {
        CContainerUI::Invalidate();
    }
}

bool CListContainerElementUI::Activate()
{
    if( !CContainerUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("itemactivate"));
    return true;
}

bool CListContainerElementUI::IsSelected() const
{
    return m_bSelected;
}

bool CListContainerElementUI::Select(bool bSelect)
{
    if( !IsEnabled() ) 
	{
		return false;
	}

	if( bSelect == m_bSelected ) 
	{
		return true;
	}
	
	m_bSelected = bSelect;
    if( bSelect && m_pOwner != NULL ) 
	{
		m_pOwner->SelectListItem(m_iIndex);
	}
	
	Invalidate();

    return true;
}

bool CListContainerElementUI::IsExpanded() const
{
    return false;
}

bool CListContainerElementUI::Expand(bool /*bExpand = true*/)
{
    return false;
}

void CListContainerElementUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CContainerUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            Activate();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_KEYDOWN && IsEnabled() )
    {
        if( event.chKey == VK_RETURN ) {
            Activate();
            Invalidate();
            return;
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN 
		|| event.Type == UIEVENT_RBUTTONDOWN ) //item can be selected by r button. 
    {
        if( IsEnabled() ){
            m_pManager->SendNotify(this, _T("itemclick"));
            Select();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP ) 
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( (m_uButtonState & UISTATE_HOT) != 0 ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }

    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CListContainerElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("selected")) == 0 ) Select();
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CListContainerElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
   if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    DrawItemBk(hDC, m_rcItem);

#ifdef _DEBUG
	//button_draw_debug |= DEBUG_IMAGE_LIST_BUTTON_IMAGE; 
#endif //_DEBUG

    CContainerUI::DoPaint(hDC, rcPaint);

#ifdef _DEBUG
	//button_draw_debug &= ~DEBUG_IMAGE_LIST_BUTTON_IMAGE; 
#endif //_DEBUG

}

void CListContainerElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    return;
}

void CListContainerElementUI::DrawItemBk(HDC hDC, const RECT& rcItem)
{
    ASSERT(m_pOwner);

	TRACE( _T( "enter %s\n" ), __TFUNCTION__ ); 

    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iBackColor = 0;

	//dump_list_info( pInfo ); 

    if( m_iIndex % 2 == 0 ) 
	{
		iBackColor = pInfo->dwBkColor;
	}

    if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
        iBackColor = pInfo->dwHotBkColor;
    }
    
	if( IsSelected() ) 
	{
        iBackColor = pInfo->dwSelectedBkColor;
    }
    
	if( !IsEnabled() ) 
	{
        iBackColor = pInfo->dwDisabledBkColor;
    }

	//TRACE( _T( "back color is 0x%0.8x \n" ), iBackColor ); 

    if ( iBackColor != 0 ) 
	{
        CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor( iBackColor ) ); 
    }

	//TRACE( _T( "disabled image is %s" ), pInfo->sDisabledImage.GetData() ); 
    
	if( !IsEnabled() ) 
	{
		if( !pInfo->sDisabledImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sDisabledImage) ) pInfo->sDisabledImage.Empty();
            else return;
        }
    }

	//TRACE( _T( "selected image is %s" ), pInfo->sSelectedImage.GetData() ); 

    if( IsSelected() ) 
	{
        if( !pInfo->sSelectedImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sSelectedImage) ) pInfo->sSelectedImage.Empty();
            else return;
        }
    }

	//TRACE( _T( "hot image is %s" ), pInfo->sHotImage.GetData() ); 

    if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
        if( !pInfo->sHotImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sHotImage) ) pInfo->sHotImage.Empty();
            else return;
        }
    }

	//TRACE( _T( "bk image is %s" ), m_sBkImage.GetData() ); 
	//TRACE( _T( "info bk image is %s" ), pInfo->sBkImage.GetData() ); 

    if( !m_sBkImage.IsEmpty() ) 
	{
        if( m_iIndex % 2 == 0 ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
        }
    }

    if( m_sBkImage.IsEmpty() ) 
	{
        if( !pInfo->sBkImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)pInfo->sBkImage) ) pInfo->sBkImage.Empty();
            else return;
        }
    }

	//TRACE( _T( "line color is 0x%0.8x" ), pInfo->dwLineColor ); 
    if ( pInfo->dwLineColor != 0 ) 
	{
        RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
        CRenderEngine::DrawLine(hDC, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
    }

	TRACE( _T( "leave %s\n" ), __TFUNCTION__ ); 
}

HRESULT add_list_item( CListUI* list, ULONG index/*, ULONG flags */)
{
	HRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	ULONG item_count; 
	CListTextElementUI *new_element = NULL; 


	ASSERT( list != NULL ); 

	item_count = list->GetItemCount(); 

	if( index >= item_count )
	{
		new_element = new CListTextElementUI; 

		if( new_element == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		//if( new_element != NULL )
		{
			//new_element->GetIndex( index );

			_ret = list->AddItem( new_element ); 
			if( _ret == false )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}
		}
	}

_return:

	if( _ret == false )
	{
		if( new_element != NULL )
		{
			delete new_element; 
		}
	}

	return ret; 
}

} // namespace DuiLib
