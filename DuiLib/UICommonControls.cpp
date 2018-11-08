#include "StdAfx.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

CLabelUI::CLabelUI() : m_uTextStyle(DT_VCENTER), m_dwTextColor(0), 
    m_dwDisabledTextColor(0), m_iFont(-1), m_bShowHtml(false)
{
    ::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
}

LPCTSTR CLabelUI::GetClass() const
{
    return UI_LABEL_CLASS_NAME;
}

ULONG CLabelUI::GetClassId() const
{
	return UI_LABEL_CLASS_ID;
}

LPVOID CLabelUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_LABEL_INTERFACE_NAME) == 0 ) return static_cast<CLabelUI*>(this);
    return CControlUI::GetInterface(pstrName);
}

void CLabelUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

UINT CLabelUI::GetTextStyle() const
{
	return m_uTextStyle;
}

void CLabelUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
}

DWORD CLabelUI::GetTextColor() const
{
	return m_dwTextColor;
}

void CLabelUI::SetDisabledTextColor(DWORD dwTextColor)
{
    m_dwDisabledTextColor = dwTextColor;
}

DWORD CLabelUI::GetDisabledTextColor() const
{
	return m_dwDisabledTextColor;
}

void CLabelUI::SetFont(int index)
{
    m_iFont = index;
}

int CLabelUI::GetFont() const
{
	return m_iFont;
}

RECT CLabelUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CLabelUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

bool CLabelUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CLabelUI::SetShowHtml(bool bShowHtml)
{
    if( m_bShowHtml == bShowHtml ) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

SIZE CLabelUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 4);
    return CControlUI::EstimateSize(szAvailable);
}

void CLabelUI::DoEvent(TEventUI& event)
{
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
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        return;
    }
    CControlUI::DoEvent(event);
}

void CLabelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("align")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_RIGHT;
        }
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_BOTTOM;
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
    else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledTextColor(clrColor);
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
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CLabelUI::PaintText(HDC hDC)
{
    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
            NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
            m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
    else {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
}

/*********************************************************************
button image by handle 
*********************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
//
//

CButtonExUI::CButtonExUI() : m_uButtonState(0), 
						m_dwHotTextColor(0), 
						m_dwPushedTextColor(0), 
						m_dwFocusedTextColor(0), 
						m_dwUIStyle( 0 )

{
    m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
	UIInfo.m_dwFocusedBkColor = 0; 
	UIInfo.m_dwPushedBkColor = 0; 
	UIInfo.m_dwHotBkColor = 0; 
	UIInfo.m_dwDisabledBkColor = 0; 
	UIInfo.m_dwBkColorFade = 0; 

	UIInfo.m_hDisabledImage = NULL; 
	UIInfo.m_hFocusedImage = NULL; 
	UIInfo.m_hHotImage = NULL; 
	UIInfo.m_hNormalImage = NULL; 
	UIInfo.m_hPushedImage = NULL; 
}

LPCTSTR CButtonExUI::GetClass() const
{
    return UI_BUTTON_EX_CLASS_NAME;
}

ULONG CButtonExUI::GetClassId() const
{
	return UI_BUTTON_EX_CLASS_ID;
}

LPVOID CButtonExUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_BUTTON_EX_INTERFACE_NAME) == 0 ) return static_cast<CButtonExUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CButtonExUI::GetControlFlags() const
{
    return UIFLAG_TABSTOP | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

void CButtonExUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
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
    if( event.Type == UIEVENT_KEYDOWN )
    {
        if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
            Activate();
            return;
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
            m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
            else m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();
            m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        if( IsContextMenuUsed() ) {
            m_pManager->SendNotify(this, _T("menu"), event.wParam, event.lParam);
        }
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
        if( IsEnabled() ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR ) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return;
    }
    CLabelUI::DoEvent(event);
}

bool CButtonExUI::Activate()
{
	if( !CControlUI::Activate() ) return false;
	if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("click"));
	return true;
}

void CButtonExUI::SetEnabled(bool bEnable)
{
	CControlUI::SetEnabled(bEnable);
	if( !IsEnabled() ) {
		m_uButtonState = 0;
	}
}

void CButtonExUI::SetHotTextColor(DWORD dwColor)
{
	m_dwHotTextColor = dwColor;
}

DWORD CButtonExUI::GetHotTextColor() const
{
	return m_dwHotTextColor;
}

void CButtonExUI::SetPushedTextColor(DWORD dwColor)
{
	m_dwPushedTextColor = dwColor;
}

DWORD CButtonExUI::GetPushedTextColor() const
{
	return m_dwPushedTextColor;
}

void CButtonExUI::SetFocusedTextColor(DWORD dwColor)
{
	m_dwFocusedTextColor = dwColor;
}

DWORD CButtonExUI::GetFocusedTextColor() const
{
	return m_dwFocusedTextColor;
}

HANDLE CButtonExUI::GetNormalImage()
{
	return UIInfo.m_hNormalImage;
}


//void CButtonExUI::SetNormalBkColor( DWORD nor_clr )
//{
//	return m_dwBackColor = nor_clr;
//}

void CButtonExUI::SetUIStyle( DWORD ui_style )
{
	m_dwUIStyle = ui_style; 
}

void CButtonExUI::SetPushedBkColor( DWORD push_clr )
{
	UIInfo.m_dwPushedBkColor = push_clr;
}

void CButtonExUI::SetHotBkColor( DWORD hot_clr )
{
	UIInfo.m_dwHotBkColor = hot_clr;
}

void CButtonExUI::SetBkFade( DWORD bk_fade )
{
	UIInfo.m_dwBkColorFade = bk_fade;
}

void CButtonExUI::SetDisabledBkColor( DWORD disabled_clr )
{
	UIInfo.m_dwDisabledBkColor = disabled_clr; 
}

void CButtonExUI::SetFocusedBkColor( DWORD focusd_clr )
{
	UIInfo.m_dwFocusedBkColor = focusd_clr;
}

DWORD CButtonExUI::GetFocusedBkColor()
{
	return UIInfo.m_dwFocusedBkColor;
}

DWORD CButtonExUI::GetNormalBkColor()
{
	return m_dwBackColor;
}

DWORD CButtonExUI::GetPushedBkColor()
{
	return UIInfo.m_dwPushedBkColor;
}

DWORD CButtonExUI::GetHotBkColor()
{
	return UIInfo.m_dwHotBkColor;
}

DWORD CButtonExUI::GetBkFade()
{
	return UIInfo.m_dwBkColorFade;
}

DWORD CButtonExUI::GetUIStyle()
{
	return m_dwUIStyle; 
}

void CButtonExUI::SetNormalImage( HANDLE Image )
{
	UIInfo.m_hNormalImage = Image;
	Invalidate();
}

HANDLE CButtonExUI::GetHotImage()
{
	return UIInfo.m_hHotImage;
}

void CButtonExUI::SetHotImage(HANDLE Image )
{
	UIInfo.m_hHotImage = Image;
	Invalidate();
}

HANDLE CButtonExUI::GetPushedImage()
{
	return UIInfo.m_hPushedImage;
}

void CButtonExUI::SetPushedImage( HANDLE Image )
{
	UIInfo.m_hPushedImage = Image;
	Invalidate();
}

HANDLE CButtonExUI::GetFocusedImage()
{
	return UIInfo.m_hFocusedImage;
}

void CButtonExUI::SetFocusedImage( HANDLE Image )
{
	UIInfo.m_hFocusedImage = Image;
	Invalidate();
}

HANDLE CButtonExUI::GetDisabledImage()
{
	return UIInfo.m_hDisabledImage;
}

void CButtonExUI::SetDisabledImage( HANDLE Image )
{
	UIInfo.m_hDisabledImage = Image;
	Invalidate();
}

SIZE CButtonExUI::EstimateSize(SIZE szAvailable)
{
	if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8);
	return CControlUI::EstimateSize(szAvailable);
}

// background color setting: hotbkclr pushedbkclr focusedbkclr disabledbkclr
void CButtonExUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	LPTSTR pstr; 
	DWORD clrColor; 

	//if( _tcscmp(pstrName, _T("normalimage")) == 0 ) 
	//{ 
	//	SetNormalImage(pstrValue);
	//}
	//else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) 
	//{
	//	SetHotImage(pstrValue);
	//}
	//else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) 
	//{
	//	SetPushedImage(pstrValue);
	//}
	//else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) 
	//{
	//	SetFocusedImage(pstrValue);
	//}
	//else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) 
	//{
	//	SetDisabledImage(pstrValue);
	//}
	if( _tcscmp( pstrName, _T( "bkfade" ) ) == 0 )
	{
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetBkFade(  clrColor ); 
	}
	//else if( _tcscmp(pstrName, _T("normalbkclr")) == 0 ) 
	//{ 
	//	SetNormalBkColor(pstrValue);
	//}
	else if( _tcscmp(pstrName, _T("bkstyle")) == 0 )
	{
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetUIStyle( clrColor ); 
	}
	else if( _tcscmp(pstrName, _T("hotbkclr")) == 0 ) 
	{
		if( *pstrValue == _T( '#' ) )
		{
			pstrValue = ::CharNext(pstrValue);
		}

		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("pushedbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetPushedBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("focusedbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetFocusedBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("disabledbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledBkColor( clrColor);
	}
	else if( _tcscmp( pstrName, _T( "hottextcolor" ) ) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 ) 
	{
		if( *pstrValue == _T( '#' ) ) 
			pstrValue = ::CharNext( pstrValue );

		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);

		SetPushedTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);

		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);

		SetFocusedTextColor(clrColor);
	}
	else 
	{
		CLabelUI::SetAttribute(pstrName, pstrValue);
	}
}

void CButtonExUI::PaintText(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
	if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

	if( m_sText.IsEmpty() ) return;
	int nLinks = 0;
	RECT rc = m_rcItem;
	rc.left += m_rcTextPadding.left;
	rc.right -= m_rcTextPadding.right;
	rc.top += m_rcTextPadding.top;
	rc.bottom -= m_rcTextPadding.bottom;

	DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

	if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
		clrColor = GetPushedTextColor();
	else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
		clrColor = GetHotTextColor();
	else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
		clrColor = GetFocusedTextColor();

	if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
		NULL, NULL, nLinks, m_uTextStyle);
	else
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
		m_iFont, m_uTextStyle);
}

#define UI_IMAGE_STYLE 0x00000001
#define UI_BK_COLOR_STYLE 0x00000002
#define UI_BK_HOLLOW_ROUND_STYLE 0x00000004

VOID CButtonExUI::PaintStatusImage( HDC hDC )
{
	if( m_dwUIStyle == UI_BK_COLOR_STYLE )
	{
		PaintStatusBkColor( hDC ); 
	}

#ifdef _DEBUG
	if( 0 != ( button_draw_debug & DEBUG_IMAGE_LIST_BUTTON_IMAGE ) )
	{
		DBG_BP(); 
	}
#endif //_DEBUG


	_PaintStatusImage( hDC ); 

	return; 
}

#define BUTTON_CONER_WIDTH 2
#define SHADOW_COLOR 0x00bdbdbd
#define BUTTON_ROUND_COLOR 0x00cfcfcf 
VOID CButtonExUI::PaintStatusHolRound( HDC hDC )
{
	RECT rc; 

	rc = m_rcItem; 

	rc.bottom = (rc.bottom + rc.top) / 2; 

	CRenderEngine::_DrawColor( hDC, rc, 0x80ffffff, 46 ); 

	rc.top = rc.bottom; 
	rc.bottom = m_rcItem.bottom; 
	CRenderEngine::_DrawColor( hDC, rc, 0x80ffffff, 18 ); 

	//CRenderEngine::DrawAlphaGradient(hDC, rc, GetAdjustColor(0x80ffffff), true, 8, 90 );

	//CRenderEngine::DrawRoundRect( hDC, 
	//	m_rcItem, 
	//	BUTTON_CONER_WIDTH, 
	//	BUTTON_CONER_WIDTH, 
	//	1, 
	//	GetAdjustColor( BUTTON_ROUND_COLOR ) ); 

	//CRenderEngine::DrawLine( hDC, 
	//	m_rcItem.left - 1, 
	//	m_rcItem.top + BUTTON_CONER_WIDTH, 
	//	m_rcItem.left - 1, 
	//	m_rcItem.bottom - BUTTON_CONER_WIDTH, 
	//	1, 
	//	GetAdjustColor( SHADOW_COLOR ) ); 

	//CRenderEngine::DrawLine( hDC, 
	//	m_rcItem.left + BUTTON_CONER_WIDTH, 
	//	m_rcItem.bottom, 
	//	m_rcItem.right - BUTTON_CONER_WIDTH, 
	//	m_rcItem.bottom, 
	//	1, 
	//	GetAdjustColor( SHADOW_COLOR ) ); 

//_return:
	return; 
}

#define is_valid_bk_color( clr ) ( ( clr & 0xff000000 ) == 0x0f000000 )
VOID CButtonExUI::PaintStatusBkColor( HDC hDC )
{
	if( IsFocused() ) 
	{
		m_uButtonState |= UISTATE_FOCUSED;
	}
	else 
	{
		m_uButtonState &= ~UISTATE_FOCUSED;
	}

	if( !IsEnabled() ) 
	{
		m_uButtonState |= UISTATE_DISABLED;
	}
	else 
	{
		m_uButtonState &= ~UISTATE_DISABLED;
	}

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwDisabledBkColor ) ) 
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwDisabledBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwPushedBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwPushedBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwHotBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwHotBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwFocusedBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwFocusedBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	//else
	//{
	//	if( TRUE == is_valid_bk_color( m_dwBackColor ) )
	//	{
	//		CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, m_dwBackColor, UIInfo.m_dwBkColorFade ); 
	//	}
	//}

	return; 
}

void CButtonExUI::_PaintStatusImage(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	//if( (m_uButtonState & UISTATE_DISABLED) != 0 ) 
	//{
	//	if( !UIInfo.m_hDisabledImage != NULL ) 
	//	{
	//		//if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_hDisabledImage) ) 
	//		{
	//			UIInfo.m_hDisabledImage = NULL;
	//		}
	//		else 
	//		{
	//			return;
	//		}
	//	}
	//}
	//else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
	//{
	//	if( !UIInfo.m_hPushedImage != NULL ) 
	//	{
	//		if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sPushedImage) ) 
	//		{
	//			UIInfo.m_sPushedImage.Empty();
	//		}
	//		else 
	//		{
	//			return;
	//		}
	//	}
	//}
	//else if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	//{
	//	if( !UIInfo.m_sHotImage.IsEmpty() ) 
	//	{
	//		if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sHotImage) ) 
	//		{
	//			UIInfo.m_sHotImage.Empty();
	//		}
	//		else 
	//		{
	//			return;
	//		}
	//	}
	//}
	//else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) 
	//{
	//	if( !UIInfo.m_sFocusedImage.IsEmpty() ) 
	//	{
	//		if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sFocusedImage) ) 
	//		{
	//			UIInfo.m_sFocusedImage.Empty();
	//		}
	//		else 
	//		{
	//			return;
	//		}
	//	}
	//}

	//if( !UIInfo.m_sNormalImage.IsEmpty() ) 
	//{
	//	if( !DrawImage( hDC, (LPCTSTR)UIInfo.m_sNormalImage) ) 
	//	{
	//		UIInfo.m_sNormalImage.Empty();
	//	}
	//	else 
	//	{
	//		return;
	//	}
	//}

	return; 
}
/**********************************************************************
normal button
**********************************************************************/
/////////////////////////////////////////////////////////////////////////////////////
//
//

CButtonUI::CButtonUI() : m_uButtonState(0), 
m_dwHotTextColor(0), 
m_dwPushedTextColor(0), 
m_dwFocusedTextColor(0), 
m_dwUIStyle( 0 )

{
	m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
	UIInfo.m_dwFocusedBkColor = 0; 
	UIInfo.m_dwPushedBkColor = 0; 
	UIInfo.m_dwHotBkColor = 0; 
	UIInfo.m_dwDisabledBkColor = 0; 
	UIInfo.m_dwBkColorFade = 0; 
}

LPCTSTR CButtonUI::GetClass() const
{
	return UI_BUTTON_CLASS_NAME;
}

ULONG CButtonUI::GetClassId() const
{
	return UI_BUTTON_CLASS_ID;
}

LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, UI_BUTTON_INTERFACE_NAME) == 0 ) return static_cast<CButtonUI*>(this);
	return CLabelUI::GetInterface(pstrName);
}

UINT CButtonUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

void CButtonUI::DoEvent(TEventUI& event)
{
	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
		if( m_pParent != NULL ) m_pParent->DoEvent(event);
		else CLabelUI::DoEvent(event);
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
	if( event.Type == UIEVENT_KEYDOWN )
	{
		if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
			Activate();
			return;
		}
	}
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
	{
		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
			else m_uButtonState &= ~UISTATE_PUSHED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_CONTEXTMENU )
	{
		if( IsContextMenuUsed() ) {
			m_pManager->SendNotify(this, _T("menu"), event.wParam, event.lParam);
		}
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
		if( IsEnabled() ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_SETCURSOR ) {
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
		return;
	}
	CLabelUI::DoEvent(event);
}

bool CButtonUI::Activate()
{
    if( !CControlUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("click"));
    return true;
}

void CButtonUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

void CButtonUI::SetHotTextColor(DWORD dwColor)
{
	m_dwHotTextColor = dwColor;
}

DWORD CButtonUI::GetHotTextColor() const
{
	return m_dwHotTextColor;
}

void CButtonUI::SetPushedTextColor(DWORD dwColor)
{
	m_dwPushedTextColor = dwColor;
}

DWORD CButtonUI::GetPushedTextColor() const
{
	return m_dwPushedTextColor;
}

void CButtonUI::SetFocusedTextColor(DWORD dwColor)
{
	m_dwFocusedTextColor = dwColor;
}

DWORD CButtonUI::GetFocusedTextColor() const
{
	return m_dwFocusedTextColor;
}

LPCTSTR CButtonUI::GetNormalImage()
{
    return UIInfo.m_sNormalImage;
}


//void CButtonUI::SetNormalBkColor( DWORD nor_clr )
//{
//	return m_dwBackColor = nor_clr;
//}

void CButtonUI::SetUIStyle( DWORD ui_style )
{
	m_dwUIStyle = ui_style; 
}

void CButtonUI::SetPushedBkColor( DWORD push_clr )
{
	UIInfo.m_dwPushedBkColor = push_clr;
}

void CButtonUI::SetHotBkColor( DWORD hot_clr )
{
	UIInfo.m_dwHotBkColor = hot_clr;
}

void CButtonUI::SetBkFade( DWORD bk_fade )
{
	UIInfo.m_dwBkColorFade = bk_fade;
}

void CButtonUI::SetDisabledBkColor( DWORD disabled_clr )
{
	UIInfo.m_dwDisabledBkColor = disabled_clr; 
}

void CButtonUI::SetFocusedBkColor( DWORD focusd_clr )
{
	UIInfo.m_dwFocusedBkColor = focusd_clr;
}

DWORD CButtonUI::GetFocusedBkColor()
{
	return UIInfo.m_dwFocusedBkColor;
}

DWORD CButtonUI::GetNormalBkColor()
{
	return m_dwBackColor;
}

DWORD CButtonUI::GetPushedBkColor()
{
	return UIInfo.m_dwPushedBkColor;
}

DWORD CButtonUI::GetHotBkColor()
{
	return UIInfo.m_dwHotBkColor;
}

DWORD CButtonUI::GetBkFade()
{
	return UIInfo.m_dwBkColorFade;
}

DWORD CButtonUI::GetUIStyle()
{
	return m_dwUIStyle; 
}

void CButtonUI::SetNormalImage(LPCTSTR pStrImage)
{
    UIInfo.m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetHotImage()
{
    return UIInfo.m_sHotImage;
}

void CButtonUI::SetHotImage(LPCTSTR pStrImage)
{
    UIInfo.m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetPushedImage()
{
    return UIInfo.m_sPushedImage;
}

void CButtonUI::SetPushedImage(LPCTSTR pStrImage)
{
    UIInfo.m_sPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetFocusedImage()
{
    return UIInfo.m_sFocusedImage;
}

void CButtonUI::SetFocusedImage(LPCTSTR pStrImage)
{
    UIInfo.m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetDisabledImage()
{
    return UIInfo.m_sDisabledImage;
}

void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
{
    UIInfo.m_sDisabledImage = pStrImage;
    Invalidate();
}

SIZE CButtonUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8);
    return CControlUI::EstimateSize(szAvailable);
}

// background color setting: hotbkclr pushedbkclr focusedbkclr disabledbkclr
void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	LPTSTR pstr; 
	DWORD clrColor; 

    if( _tcscmp(pstrName, _T("normalimage")) == 0 ) 
	{ 
		SetNormalImage(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) 
	{
		SetHotImage(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) 
	{
		SetPushedImage(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) 
	{
		SetFocusedImage(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) 
	{
		SetDisabledImage(pstrValue);
	}
	else if( _tcscmp( pstrName, _T( "bkfade" ) ) == 0 )
	{
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetBkFade(  clrColor ); 
	}
	//else if( _tcscmp(pstrName, _T("normalbkclr")) == 0 ) 
	//{ 
	//	SetNormalBkColor(pstrValue);
	//}
	else if( _tcscmp(pstrName, _T("bkstyle")) == 0 )
	{
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetUIStyle( clrColor ); 
	}
	else if( _tcscmp(pstrName, _T("hotbkclr")) == 0 ) 
	{
		if( *pstrValue == _T( '#' ) )
		{
			pstrValue = ::CharNext(pstrValue);
		}
		
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetHotBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("pushedbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetPushedBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("focusedbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetFocusedBkColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("disabledbkclr")) == 0 ) 
	{
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		pstr = NULL;
		clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetDisabledBkColor( clrColor);
	}
    else if( _tcscmp( pstrName, _T( "hottextcolor" ) ) == 0 ) 
	{
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        pstr = NULL;
        clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 ) 
	{
        if( *pstrValue == _T( '#' ) ) 
			pstrValue = ::CharNext( pstrValue );
        
		pstr = NULL;
        clrColor = _tcstoul(pstrValue, &pstr, 16);
        
		SetPushedTextColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 ) 
	{
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        
		pstr = NULL;
        clrColor = _tcstoul(pstrValue, &pstr, 16);
        
		SetFocusedTextColor(clrColor);
    }
    else 
	{
		CLabelUI::SetAttribute(pstrName, pstrValue);
	}
}

void CButtonUI::PaintText(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

	DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

	if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
		clrColor = GetPushedTextColor();
	else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
		clrColor = GetHotTextColor();
	else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
		clrColor = GetFocusedTextColor();

    if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
        NULL, NULL, nLinks, m_uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
        m_iFont, m_uTextStyle);
}

#define UI_IMAGE_STYLE 0x00000001
#define UI_BK_COLOR_STYLE 0x00000002
#define UI_BK_HOLLOW_ROUND_STYLE 0x00000004

VOID CButtonUI::PaintStatusImage( HDC hDC )
{
	if( m_dwUIStyle == UI_BK_COLOR_STYLE )
	{
		PaintStatusBkColor( hDC ); 
	}

#ifdef _DEBUG
	if( 0 != ( button_draw_debug & DEBUG_IMAGE_LIST_BUTTON_IMAGE ) )
	{
		DBG_BP(); 
	}
#endif //_DEBUG

	
	_PaintStatusImage( hDC ); 
	
	return; 
}

#define BUTTON_CONER_WIDTH 2
#define SHADOW_COLOR 0x00bdbdbd
#define BUTTON_ROUND_COLOR 0x00cfcfcf 
VOID CButtonUI::PaintStatusHolRound( HDC hDC )
{
	RECT rc; 

	rc = m_rcItem; 

	rc.bottom = (rc.bottom + rc.top) / 2; 

	CRenderEngine::_DrawColor( hDC, rc, 0x20202020, 255 ); 

	rc.top = rc.bottom; 
	rc.bottom = m_rcItem.bottom; 
	CRenderEngine::_DrawColor( hDC, rc, 0x08080808, 255 ); 

	//CRenderEngine::DrawAlphaGradient(hDC, rc, GetAdjustColor(0x80ffffff), true, 8, 90 );

	//CRenderEngine::DrawRoundRect( hDC, 
	//	m_rcItem, 
	//	BUTTON_CONER_WIDTH, 
	//	BUTTON_CONER_WIDTH, 
	//	1, 
	//	GetAdjustColor( BUTTON_ROUND_COLOR ) ); 

	//CRenderEngine::DrawLine( hDC, 
	//	m_rcItem.left - 1, 
	//	m_rcItem.top + BUTTON_CONER_WIDTH, 
	//	m_rcItem.left - 1, 
	//	m_rcItem.bottom - BUTTON_CONER_WIDTH, 
	//	1, 
	//	GetAdjustColor( SHADOW_COLOR ) ); 

	//CRenderEngine::DrawLine( hDC, 
	//	m_rcItem.left + BUTTON_CONER_WIDTH, 
	//	m_rcItem.bottom, 
	//	m_rcItem.right - BUTTON_CONER_WIDTH, 
	//	m_rcItem.bottom, 
	//	1, 
	//	GetAdjustColor( SHADOW_COLOR ) ); 

//_return:
	return; 
}

#define is_valid_bk_color( clr ) ( ( clr & 0xff000000 ) == 0x0f000000 )
VOID CButtonUI::PaintStatusBkColor( HDC hDC )
{
	if( IsFocused() ) 
	{
		m_uButtonState |= UISTATE_FOCUSED;
	}
	else 
	{
		m_uButtonState &= ~UISTATE_FOCUSED;
	}
		
	if( !IsEnabled() ) 
	{
		m_uButtonState |= UISTATE_DISABLED;
	}
	else 
	{
		m_uButtonState &= ~UISTATE_DISABLED;
	}

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwDisabledBkColor ) ) 
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwDisabledBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwPushedBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwPushedBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwHotBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwHotBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) 
	{
		if( TRUE == is_valid_bk_color( UIInfo.m_dwFocusedBkColor ) )
		{
			CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, GetAdjustColor( UIInfo.m_dwFocusedBkColor ), UIInfo.m_dwBkColorFade ); 
		}
	}
	//else
	//{
	//	if( TRUE == is_valid_bk_color( m_dwBackColor ) )
	//	{
	//		CRenderEngine::DrawFadeBkColor( hDC, m_rcItem, m_dwBackColor, UIInfo.m_dwBkColorFade ); 
	//	}
	//}

	return; 
}

void CButtonUI::_PaintStatusImage(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~ UISTATE_DISABLED;

    if( (m_uButtonState & UISTATE_DISABLED) != 0 ) 
	{
        if( !UIInfo.m_sDisabledImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sDisabledImage) ) 
			{
				UIInfo.m_sDisabledImage.Empty();
			}
			else 
			{
				return;
			}
        }
    }
    else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
	{
        if( !UIInfo.m_sPushedImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sPushedImage) ) 
			{
				UIInfo.m_sPushedImage.Empty();
			}
            else 
			{
				return;
			}
        }
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) 
	{
        if( !UIInfo.m_sHotImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sHotImage) ) 
			{
				UIInfo.m_sHotImage.Empty();
			}
			else 
			{
				return;
			}
        }
    }
    else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) 
	{
        if( !UIInfo.m_sFocusedImage.IsEmpty() ) 
		{
            if( !DrawImage(hDC, (LPCTSTR)UIInfo.m_sFocusedImage) ) 
			{
				UIInfo.m_sFocusedImage.Empty();
			}
            else 
			{
				return;
			}
        }
    }

    if( !UIInfo.m_sNormalImage.IsEmpty() ) 
	{
        if( !DrawImage( hDC, (LPCTSTR)UIInfo.m_sNormalImage) ) 
		{
			UIInfo.m_sNormalImage.Empty();
		}
        else 
		{
			return;
		}
    }

	return; 
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

COptionUI::COptionUI() : m_bSelected(false), m_dwSelectedTextColor(0)
{
}

COptionUI::~COptionUI()
{
    if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
}

LPCTSTR COptionUI::GetClass() const
{
    return UI_OPTION_CLASS_NAME;
}

ULONG COptionUI::GetClassId() const
{
	return UI_OPTION_CLASS_ID;
}

LPVOID COptionUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_OPTION_INTERFACE_NAME) == 0 ) 
	{
		return static_cast<COptionUI*>(this);
	}
	
	return CButtonUI::GetInterface(pstrName);
}

void COptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CControlUI::SetManager(pManager, pParent, bInit);
    if( bInit && !m_sGroupName.IsEmpty() ) {
        if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
    }
}

LPCTSTR COptionUI::GetGroup() const
{
    return m_sGroupName;
}

void COptionUI::SetGroup(LPCTSTR pStrGroupName)
{
    if( pStrGroupName == NULL ) 
	{
        if( m_sGroupName.IsEmpty() ) 
		{
			return;
		}
        m_sGroupName.Empty();
    }
    else 
	{
        if( m_sGroupName == pStrGroupName ) 
		{
			return;
		}
        if (!m_sGroupName.IsEmpty() && m_pManager) 
		{
			m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}
		
		m_sGroupName = pStrGroupName;
    }

    if( !m_sGroupName.IsEmpty() ) {
        if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
    }
    else {
        if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
    }

    Selected(m_bSelected);
}

bool COptionUI::IsSelected() const
{
    return m_bSelected;
}

void COptionUI::Selected(bool bSelected)
{
    if( m_bSelected == bSelected ) return;
    m_bSelected = bSelected;
    if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
    else m_uButtonState &= ~UISTATE_SELECTED;

    if( m_pManager != NULL ) {
        if( !m_sGroupName.IsEmpty() ) {
            if( m_bSelected ) {
                CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
                for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
                    COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
                    if( pControl != this ) {
                        pControl->Selected(false);
                    }
                }
                m_pManager->SendNotify(this, _T("selectchanged"));
            }
        }
        else {
            m_pManager->SendNotify(this, _T("selectchanged"));
        }
    }

    Invalidate();
}

bool COptionUI::Activate()
{
    if( !CButtonUI::Activate() ) 
	{
		return false;
	}
    
	if( !m_sGroupName.IsEmpty() ) 
	{
		Selected(true);
	}
    else 
	{
		Selected(!m_bSelected);
	}

    return true;
}

void COptionUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        if( m_bSelected ) m_uButtonState = UISTATE_SELECTED;
        else m_uButtonState = 0;
    }
}

LPCTSTR COptionUI::GetSelectedImage()
{
    return m_sSelectedImage;
}

LPCTSTR COptionUI::GetSelectedHotImage()
{
	return m_sSelectedHotImage;
}

void COptionUI::SetSelectedImage(LPCTSTR pStrImage)
{
    m_sSelectedImage = pStrImage;
    Invalidate();
}

void COptionUI::SetSelectedHotImage( LPCTSTR pStrImage )
{
	m_sSelectedHotImage = pStrImage; 
	Invalidate(); 
}

void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
{
	m_dwSelectedTextColor = dwTextColor;
}

DWORD COptionUI::GetSelectedTextColor()
{
	if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
	return m_dwSelectedTextColor;
}

LPCTSTR COptionUI::GetForeImage()
{
	return m_sForeImage;
}

void COptionUI::SetForeImage(LPCTSTR pStrImage)
{
	m_sForeImage = pStrImage;
	Invalidate();
}

SIZE COptionUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) 
	{
		return CSize( m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8 );
	}
	
	return CControlUI::EstimateSize( szAvailable ); 
}

void COptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("group")) == 0 ) SetGroup(pstrValue);
    else if( _tcscmp(pstrName, _T("selected")) == 0 ) Selected(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("selectedimage")) == 0 ) SetSelectedImage(pstrValue);
	else if( _tcscmp( pstrName, _T( "selectedhotimage" ) ) == 0 ) SetSelectedHotImage( pstrValue ); 
	else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
	else if( _tcscmp(pstrName, _T("selectedtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedTextColor(clrColor);
	}
    else CButtonUI::SetAttribute(pstrName, pstrValue);
}

void COptionUI::PaintStatusImage(HDC hDC)
{
    m_uButtonState &= ~UISTATE_PUSHED;

    if( (m_uButtonState & UISTATE_SELECTED) != 0 ) 
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 )
		{
			if( !m_sSelectedHotImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage) ) m_sSelectedHotImage.Empty();
				else goto Label_ForeImage;
			}
		}
		else
		{
			if( !m_sSelectedImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sSelectedImage) ) m_sSelectedImage.Empty();
				else goto Label_ForeImage;
			}
		}
    }

    CButtonUI::PaintStatusImage(hDC);

Label_ForeImage:
	if( !m_sForeImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
	}

	if( m_dwUIStyle == UI_BK_HOLLOW_ROUND_STYLE )
	{
		if( m_uButtonState & ( UISTATE_SELECTED | UISTATE_HOT ) )
		{
			PaintStatusHolRound( hDC ); 
		}
	}
}

void COptionUI::PaintText(HDC hDC)
{
	if( (m_uButtonState & UISTATE_SELECTED) != 0 )
	{
		DWORD oldTextColor = m_dwTextColor;
		if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			m_iFont, m_uTextStyle);

		m_dwTextColor = oldTextColor;
	}
	else
		CButtonUI::PaintText(hDC);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CTextUI::CTextUI() : m_nLinks(0), m_nHoverLink(-1)
{
    m_uTextStyle = DT_WORDBREAK;
    m_rcTextPadding.left = 2;
    m_rcTextPadding.right = 2;
    ::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
}

CTextUI::~CTextUI()
{
}

LPCTSTR CTextUI::GetClass() const
{
    return UI_TEXT_CLASS_NAME;
}

ULONG CTextUI::GetClassId() const
{
	return UI_TEXT_CLASS_ID;
}

LPVOID CTextUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_TEXT_INTERFACE_NAME) == 0 ) return static_cast<CTextUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CTextUI::GetControlFlags() const
{
    if( IsEnabled() && m_nLinks > 0 ) return UIFLAG_SETCURSOR;
    else return 0;
}

CStdString* CTextUI::GetLinkContent(int iIndex)
{
    if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
    return NULL;
}

void CTextUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                Invalidate();
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
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    // When you move over a link
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink) {
            m_nHoverLink = nHoverLink;
            Invalidate();
            return;
        }      
    }
    if( event.Type == UIEVENT_MOUSELEAVE ) {
        if( m_nLinks > 0 && IsEnabled() ) {
            if(m_nHoverLink != -1) {
                m_nHoverLink = -1;
                Invalidate();
                return;
            }
        }
    }

    CLabelUI::DoEvent(event);
}

SIZE CTextUI::EstimateSize(SIZE szAvailable)
{
    RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
    rcText.left += m_rcTextPadding.left;
    rcText.right -= m_rcTextPadding.right;
    if( m_bShowHtml ) {   
        int nLinks = 0;
        CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, NULL, NULL, nLinks, DT_CALCRECT | m_uTextStyle);
    }
    else {
        CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
    }
    SIZE cXY = {rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right,
		( m_cxyFixed.cy != 0 ) ? m_cxyFixed.cy : ( rcText.bottom - rcText.top + m_rcTextPadding.top + m_rcTextPadding.bottom ) };

    //if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
    return cXY;
}

void CTextUI::PaintText(HDC hDC)
{
    if( m_sText.IsEmpty() ) {
        m_nLinks = 0;
        return;
    }

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;

    m_nLinks = lengthof(m_rcLinks);
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
            m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
            m_iFont, m_uTextStyle);
    }
    else {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
        else
            CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            m_iFont, m_uTextStyle);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CProgressUI::CProgressUI() : m_bHorizontal(true), m_nMin(0), m_nMax(100), m_nValue(0)
{
    m_uTextStyle = DT_SINGLELINE | DT_CENTER;
    SetFixedHeight(12);
}

LPCTSTR CProgressUI::GetClass() const
{
    return UI_PROGRESS_CLASS_NAME;
}

ULONG CProgressUI::GetClassId() const
{
	return UI_PROGRESS_CLASS_ID;
}

LPVOID CProgressUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_PROGRESS_INTERFACE_NAME) == 0 ) return static_cast<CProgressUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

bool CProgressUI::IsHorizontal()
{
    return m_bHorizontal;
}

void CProgressUI::SetHorizontal(bool bHorizontal)
{
    if( m_bHorizontal == bHorizontal ) return;

    m_bHorizontal = bHorizontal;
    Invalidate();
}

int CProgressUI::GetMinValue() const
{
    return m_nMin;
}

void CProgressUI::SetMinValue(int nMin)
{
    m_nMin = nMin;
    Invalidate();
}

int CProgressUI::GetMaxValue() const
{
    return m_nMax;
}

void CProgressUI::SetMaxValue(int nMax)
{
    m_nMax = nMax;
    Invalidate();
}

int CProgressUI::GetValue() const
{
    return m_nValue;
}

void CProgressUI::SetValue(int nValue)
{
    m_nValue = nValue;
    Invalidate();
}

LPCTSTR CProgressUI::GetFgImage() const
{
    return m_sFgImage;
}

void CProgressUI::SetFgImage(LPCTSTR pStrImage)
{
    if( m_sFgImage == pStrImage ) return;

    m_sFgImage = pStrImage;
    Invalidate();
}

void CProgressUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("fgimage")) == 0 ) SetFgImage(pstrValue);
    else if( _tcscmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("min")) == 0 ) SetMinValue(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("max")) == 0 ) SetMaxValue(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("value")) == 0 ) SetValue(_ttoi(pstrValue));
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CProgressUI::PaintStatusImage(HDC hDC)
{
    if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
    if( m_nValue > m_nMax ) m_nValue = m_nMax;
    if( m_nValue < m_nMin ) m_nValue = m_nMin;

    RECT rc = {0};
    if( m_bHorizontal ) {
        rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
        rc.bottom = m_rcItem.bottom - m_rcItem.top;
    }
    else {
        rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
        rc.right = m_rcItem.right - m_rcItem.left;
        rc.bottom = m_rcItem.bottom - m_rcItem.top;
    }

    if( !m_sFgImage.IsEmpty() ) {
        m_sFgImageModify.Empty();
        m_sFgImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);

        if( !DrawImage(hDC, (LPCTSTR)m_sFgImage, (LPCTSTR)m_sFgImageModify) ) m_sFgImage.Empty();
        else return;
    }
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CSliderUI::CSliderUI() : m_uButtonState(0), m_nStep(1)
{
    m_uTextStyle = DT_SINGLELINE | DT_CENTER;
    m_szThumb.cx = m_szThumb.cy = 10;
}

LPCTSTR CSliderUI::GetClass() const
{
    return UI_SLIDER_CLASS_NAME;
}

ULONG CSliderUI::GetClassId() const
{
	return UI_SLIDER_CLASS_ID;
}

UINT CSliderUI::GetControlFlags() const
{
    if( IsEnabled() ) return UIFLAG_SETCURSOR;
    else return 0;
}

LPVOID CSliderUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_SLIDER_INTERFACE_NAME) == 0 ) return static_cast<CSliderUI*>(this);
    return CProgressUI::GetInterface(pstrName);
}

void CSliderUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CSliderUI::GetChangeStep()
{
    return m_nStep;
}

void CSliderUI::SetChangeStep(int step)
{
    m_nStep = step;
}

void CSliderUI::SetThumbSize(SIZE szXY)
{
    m_szThumb = szXY;
}

RECT CSliderUI::GetThumbRect() const
{
    if( m_bHorizontal ) {
        int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
        int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
        return CRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
    }
    else {
        int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
        int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
        return CRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
    }
}

LPCTSTR CSliderUI::GetThumbImage() const
{
    return m_sThumbImage;
}

void CSliderUI::SetThumbImage(LPCTSTR pStrImage)
{
    m_sThumbImage = pStrImage;
    Invalidate();
}

LPCTSTR CSliderUI::GetThumbHotImage() const
{
    return m_sThumbHotImage;
}

void CSliderUI::SetThumbHotImage(LPCTSTR pStrImage)
{
    m_sThumbHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CSliderUI::GetThumbPushedImage() const
{
    return m_sThumbPushedImage;
}

void CSliderUI::SetThumbPushedImage(LPCTSTR pStrImage)
{
    m_sThumbPushedImage = pStrImage;
    Invalidate();
}

void CSliderUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CProgressUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            RECT rcThumb = GetThumbRect();
            if( ::PtInRect(&rcThumb, event.ptMouse) ) {
                m_uButtonState |= UISTATE_CAPTURED;
            }
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            m_uButtonState &= ~UISTATE_CAPTURED;
        }
        if( m_bHorizontal ) {
            if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
            else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
            else m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
        }
        else {
            if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
            else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
            else m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
        }
        m_pManager->SendNotify(this, _T("valuechanged"));
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_SCROLLWHEEL ) 
    {
        switch( LOWORD(event.wParam) ) {
        case SB_LINEUP:
            SetValue(GetValue() + GetChangeStep());
            m_pManager->SendNotify(this, _T("valuechanged"));
            return;
        case SB_LINEDOWN:
            SetValue(GetValue() - GetChangeStep());
            m_pManager->SendNotify(this, _T("valuechanged"));
            return;
        }
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( m_bHorizontal ) {
                if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
                else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
                else m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
            }
            else {
                if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
                else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
                else m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
            }
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        RECT rcThumb = GetThumbRect();
        if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
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


void CSliderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("thumbimage")) == 0 ) SetThumbImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbsize")) == 0 ) {
        SIZE szXY = {0};
        LPTSTR pstr = NULL;
        szXY.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        szXY.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
        SetThumbSize(szXY);
    }
    else if( _tcscmp(pstrName, _T("step")) == 0 ) {
        SetChangeStep(_ttoi(pstrValue));
    }
    else CProgressUI::SetAttribute(pstrName, pstrValue);
}

void CSliderUI::PaintStatusImage(HDC hDC)
{
    CProgressUI::PaintStatusImage(hDC);

    RECT rcThumb = GetThumbRect();
    rcThumb.left -= m_rcItem.left;
    rcThumb.top -= m_rcItem.top;
    rcThumb.right -= m_rcItem.left;
    rcThumb.bottom -= m_rcItem.top;
    if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
        if( !m_sThumbPushedImage.IsEmpty() ) {
            m_sImageModify.Empty();
            m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
            if( !DrawImage(hDC, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) ) m_sThumbPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( !m_sThumbHotImage.IsEmpty() ) {
            m_sImageModify.Empty();
            m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
            if( !DrawImage(hDC, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) ) m_sThumbHotImage.Empty();
            else return;
        }
    }

    if( !m_sThumbImage.IsEmpty() ) {
        m_sImageModify.Empty();
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if( !DrawImage(hDC, (LPCTSTR)m_sThumbImage, (LPCTSTR)m_sImageModify) ) m_sThumbImage.Empty();
        else return;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

class CEditWnd : public CWindowWnd
{
public:
    CEditWnd();

    void Init(CEditUI* pOwner);
    RECT CalPos();

    LPCTSTR GetWindowClassName() const;
    LPCTSTR GetSuperClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    CEditUI* m_pOwner;
    HBRUSH m_hBkBrush;
};


CEditWnd::CEditWnd() : m_pOwner(NULL), m_hBkBrush(NULL)
{
}

void CEditWnd::Init(CEditUI* pOwner)
{
    m_pOwner = pOwner;
    RECT rcPos = CalPos();
    UINT uStyle = WS_CHILD | ES_AUTOHSCROLL;  

	if( pOwner->IsMultiline() == true ) 
	{
		uStyle |= ES_MULTILINE; 
	}

    if( m_pOwner->IsPasswordMode() ) uStyle |= ES_PASSWORD;
    Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
    SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetDefaultFontInfo()->hFont, TRUE);
    Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
    if( m_pOwner->IsPasswordMode() ) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());
    Edit_SetText(m_hWnd, m_pOwner->GetText());
    Edit_SetModify(m_hWnd, FALSE);
    SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
    Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
    Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);
    ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
    ::SetFocus(m_hWnd);
}

RECT CEditWnd::CalPos()
{
    CRect rcPos = m_pOwner->GetPos();
    RECT rcInset = m_pOwner->GetTextPadding();
    rcPos.left += rcInset.left;
    rcPos.top += rcInset.top;
    rcPos.right -= rcInset.right;
    rcPos.bottom -= rcInset.bottom;
    LONG lEditHeight = m_pOwner->GetManager()->GetDefaultFontInfo()->tm.tmHeight;
    if( lEditHeight < rcPos.GetHeight() ) {
        rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
        rcPos.bottom = rcPos.top + lEditHeight;
    }
    return rcPos;
}

LPCTSTR CEditWnd::GetWindowClassName() const
{
    return _T("EditWnd");
}

LPCTSTR CEditWnd::GetSuperClassName() const
{
    return WC_EDIT;
}

void CEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
    // Clear reference and die
    if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
    m_pOwner->m_pWindow = NULL;
    delete this;
}

LRESULT CEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    BOOL bHandled = TRUE;

	__Trace( _T( "enter %s message %u wparam %u lparam %u\n" ), __TFUNCTION__, uMsg, wParam, lParam ); 

    if( uMsg == WM_KILLFOCUS ) 
	{
		//OnEditChanged( 0, 0, 0, bHandled ); 
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
	}
	else if( uMsg == OCM_COMMAND ) 
	{
        if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) 
		{
			lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
		}
		else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) 
		{
            RECT rcClient;
            ::GetClientRect(m_hWnd, &rcClient);
            ::InvalidateRect(m_hWnd, &rcClient, FALSE);
        }
    }
    else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ) 
	{
        m_pOwner->GetManager()->SendNotify(m_pOwner, _T("return"));
    }
	//else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_ESCAPE ) 
	//{
	//	m_pOwner->GetManager()->SendNotify(m_pOwner, _T("escape"));
	//}
    else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) 
	{
        if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF ) 
		{
			return NULL;
		}

        ::SetBkMode((HDC)wParam, TRANSPARENT); 

		DWORD dwTextColor = m_pOwner->GetTextColor();
        ::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
        
		if( m_hBkBrush == NULL ) 
		{
            DWORD clrColor = m_pOwner->GetNativeEditBkColor();
            m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
        }

		return (LRESULT)m_hBkBrush;
    }
    else 
	{
		bHandled = FALSE;
	}

    if( !bHandled ) 
	{
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
	
	return lRes;
}

LRESULT CEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    PostMessage(WM_CLOSE);
    return lRes;
}

LRESULT CEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if( m_pOwner == NULL ) 
	{
		return 0;
	}

    // Copy text back
    int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
    LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
    
	ASSERT(pstr);
    if( pstr == NULL ) 
	{
		return 0;
	}

    ::GetWindowText(m_hWnd, pstr, cchLen);
    
	m_pOwner->m_sText = pstr;
    m_pOwner->GetManager()->SendNotify(m_pOwner, _T("textchanged"));
    
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CEditUI::CEditUI() : m_pWindow(NULL), m_uMaxChar(255), m_bReadOnly(false), 
m_bPasswordMode(false), m_multiline( false ), m_cPasswordChar(_T('*')), m_uButtonState(0), m_dwEditbkColor(0xFFFFFFFF)
{
    SetTextPadding(CRect(4, 3, 4, 3));
    SetBkColor(0xFFFFFFFF);
}

LPCTSTR CEditUI::GetClass() const
{
    return UI_EDIT_CLASS_NAME;
}

ULONG CEditUI::GetClassId() const
{
	return UI_EDIT_CLASS_ID;
}

LPVOID CEditUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_EDIT_INTERFACE_NAME ) == 0 ) return static_cast<CEditUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CEditUI::GetControlFlags() const
{
    if( !IsEnabled() ) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CEditUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) 
	{
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
        return;
    }
    if( event.Type == UIEVENT_WINDOWSIZE )
    {
        if( m_pWindow != NULL ) 
		{
			m_pManager->SetFocusNeeded(this); 
		}
    }
    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
        if( m_pWindow != NULL ) 
		{
			return;
		}
    }
    if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
    {
        if( m_pWindow ) 
		{
			return;
		}

        m_pWindow = new CEditWnd();
        ASSERT(m_pWindow);
        m_pWindow->Init(this);
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
    {
        if( IsEnabled() ) 
		{
            GetManager()->ReleaseCapture();

            if( IsFocused() && m_pWindow == NULL )
            {
                m_pWindow = new CEditWnd();
                ASSERT(m_pWindow);
                m_pWindow->Init(this);

				if( PtInRect(&m_rcItem, event.ptMouse) )
				{
					int nSize = GetWindowTextLength(*m_pWindow);
					if( nSize == 0 )
						nSize = 1;

					Edit_SetSel(*m_pWindow, 0, nSize);
				}
            }
            else if( m_pWindow != NULL )
            {
#if 1
				int nSize = GetWindowTextLength(*m_pWindow);
				if( nSize == 0 )
					nSize = 1;

				Edit_SetSel(*m_pWindow, 0, nSize);
#else
                POINT pt = event.ptMouse;
                pt.x -= m_rcItem.left + m_rcTextPadding.left;
                pt.y -= m_rcItem.top + m_rcTextPadding.top;
                ::SendMessage(*m_pWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
#endif
            }
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
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) 
		{
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }

        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) 
		{
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }

        return;
    }
    CLabelUI::DoEvent(event);
}

void CEditUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

void CEditUI::SetText(LPCTSTR pstrText)
{
    m_sText = pstrText;
    if( m_pWindow != NULL ) Edit_SetText(*m_pWindow, m_sText);
    Invalidate();
}

void CEditUI::SetMaxChar(UINT uMax)
{
    m_uMaxChar = uMax;
    if( m_pWindow != NULL ) Edit_LimitText(*m_pWindow, m_uMaxChar);
}

UINT CEditUI::GetMaxChar()
{
    return m_uMaxChar;
}

void CEditUI::SetReadOnly(bool bReadOnly)
{
    if( m_bReadOnly == bReadOnly ) return;

    m_bReadOnly = bReadOnly;
    if( m_pWindow != NULL ) Edit_SetReadOnly(*m_pWindow, m_bReadOnly);
    Invalidate();
}

bool CEditUI::IsReadOnly() const
{
    return m_bReadOnly;
}

void CEditUI::SetPasswordMode(bool bPasswordMode)
{
    if( m_bPasswordMode == bPasswordMode ) return;
    m_bPasswordMode = bPasswordMode;
    Invalidate();
}

bool CEditUI::IsPasswordMode() const
{
    return m_bPasswordMode;
}

void CEditUI::SetPasswordChar(TCHAR cPasswordChar)
{
    if( m_cPasswordChar == cPasswordChar ) return;
    m_cPasswordChar = cPasswordChar;
    if( m_pWindow != NULL ) Edit_SetPasswordChar(*m_pWindow, m_cPasswordChar);
    Invalidate();
}

TCHAR CEditUI::GetPasswordChar() const
{
    return m_cPasswordChar;
}

LPCTSTR CEditUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CEditUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetHotImage()
{
    return m_sHotImage;
}

void CEditUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CEditUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CEditUI::SetDisabledImage(LPCTSTR pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

void CEditUI::SetNativeEditBkColor(DWORD dwBkColor)
{
    m_dwEditbkColor = dwBkColor;
}

DWORD CEditUI::GetNativeEditBkColor() const
{
    return m_dwEditbkColor;
}

void CEditUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);
    if( m_pWindow != NULL ) {
        RECT rcPos = m_pWindow->CalPos();
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
            rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
    }
}

void CEditUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
}

void CEditUI::SetInternVisible(bool bVisible)
{
    if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
}

SIZE CEditUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 6);
    return CControlUI::EstimateSize(szAvailable);
}

void CEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("readonly")) == 0 ) SetReadOnly(_tcscmp(pstrValue, _T("true")) == 0);
	else if( _tcscmp( pstrName, _T( "multiline" ) ) == 0 ) 
	{
		SetMultiline( _tcscmp(pstrValue, _T("true")) == 0 ); 
	}

	else if( _tcscmp(pstrName, _T("password")) == 0 ) SetPasswordMode(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("maxchar")) == 0 ) SetMaxChar(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("nativebkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetNativeEditBkColor(clrColor);
    }
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CEditUI::PaintStatusImage(HDC hDC)
{
    if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~ UISTATE_DISABLED;

    if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
        if( !m_sDisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
        if( !m_sFocusedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( !m_sHotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
            else return;
        }
    }

    if( !m_sNormalImage.IsEmpty() ) {
        if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
        else return;
    }
}

void CEditUI::PaintText(HDC hDC)
{
    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;

    CStdString sText = m_sText;
    if( m_bPasswordMode ) {
        sText.Empty();
        LPCTSTR p = m_sText.GetData();
        while( *p != _T('\0') ) {
            sText += m_cPasswordChar;
            p = ::CharNext(p);
        }
    }

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwTextColor, \
            m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
    else {
        CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, \
            m_iFont, DT_SINGLELINE | m_uTextStyle);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

#define DEFFAULT_SCROLL_RANGE_VALUE 100

CScrollBarUI::CScrollBarUI() : m_bHorizontal(false), m_nRange( DEFFAULT_SCROLL_RANGE_VALUE ), m_nScrollPos(0), m_nLineSize(8), 
m_pOwner(NULL), m_nLastScrollPos(0), m_nLastScrollOffset(0), m_nScrollRepeatDelay(0), m_uButton1State(0), \
m_uButton2State(0), m_uThumbState(0), m_bShowButton1(true), m_bShowButton2(true)
{
    m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
    ptLastMouse.x = ptLastMouse.y = 0;
    ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
    ::ZeroMemory(&m_rcButton1, sizeof(m_rcButton1));
    ::ZeroMemory(&m_rcButton2, sizeof(m_rcButton2));
}

LPCTSTR CScrollBarUI::GetClass() const
{
    return UI_SCROLL_BAR_CLASS_NAME;
}

ULONG CScrollBarUI::GetClassId() const
{
	return UI_SCROLL_BAR_CLASS_ID;
}

LPVOID CScrollBarUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, UI_SCROLL_BAR_INTERFACE_NAME) == 0 ) return static_cast<CScrollBarUI*>(this);
    return CControlUI::GetInterface(pstrName);
}

CBaseContainerUI* CScrollBarUI::GetOwner() const
{
    return m_pOwner;
}

void CScrollBarUI::SetOwner(CBaseContainerUI* pOwner)
{
    m_pOwner = pOwner;
}

void CScrollBarUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButton1State = 0;
        m_uButton2State = 0;
        m_uThumbState = 0;
    }
}

void CScrollBarUI::SetFocus()
{
    if( m_pOwner != NULL ) m_pOwner->SetFocus();
    else CControlUI::SetFocus();
}

bool CScrollBarUI::IsHorizontal()
{
    return m_bHorizontal;
}

void CScrollBarUI::SetHorizontal(bool bHorizontal)
{
#ifdef _DEBUG
	if( bHorizontal == true )
	{
		//DBG_BP(); 
	}
#endif //_DEBUG

    if( m_bHorizontal == bHorizontal ) 
	{
		return;
	}

    m_bHorizontal = bHorizontal;
    if( m_bHorizontal ) 
	{
        if( m_cxyFixed.cy == 0 ) 
		{
            m_cxyFixed.cx = 0;
            m_cxyFixed.cy = DEFAULT_SCROLLBAR_SIZE;
        }
    }
    else 
	{
        if( m_cxyFixed.cx == 0 ) 
		{
            m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
            m_cxyFixed.cy = 0;
        }
    }

    if( m_pOwner != NULL ) 
	{
		m_pOwner->NeedUpdate(); 
	}
	else 
	{
		NeedParentUpdate();
	}
}


int CScrollBarUI::GetScrollRange() const
{
    return m_nRange;
}

void CScrollBarUI::SetScrollRange(int nRange)
{
//#ifdef _DEBUG
//	if( m_bHorizontal == true )
//	{
//		DBG_BP(); 
//	}
//#endif //_DEBUG

    if( m_nRange == nRange ) 
	{
		return;
	}

    m_nRange = nRange;

    if( m_nRange < 0 ) 
	{
		m_nRange = 0;
	}

    if( m_nScrollPos > m_nRange ) 
	{
		m_nScrollPos = m_nRange;
	}

    SetPos( m_rcItem );
}

int CScrollBarUI::GetScrollPos() const
{
    return m_nScrollPos;
}

void CScrollBarUI::SetScrollPos(int nPos)
{
    if( m_nScrollPos == nPos ) 
	{
		return;
	}

    m_nScrollPos = nPos;
    if( m_nScrollPos < 0 ) 
	{
		m_nScrollPos = 0;
	}
	
	if( m_nScrollPos > m_nRange ) 
	{
		m_nScrollPos = m_nRange;
	}
	
	//do invalidate of the scroll bar.

	SetPos( m_rcItem );
}

int CScrollBarUI::GetLineSize() const
{
    return m_nLineSize;
}

void CScrollBarUI::SetLineSize(int nSize)
{
    m_nLineSize = nSize;
}

bool CScrollBarUI::GetShowButton1()
{
    return m_bShowButton1;
}

void CScrollBarUI::SetShowButton1(bool bShow)
{
    m_bShowButton1 = bShow;
    SetPos(m_rcItem);
}

LPCTSTR CScrollBarUI::GetButton1NormalImage()
{
    return m_sButton1NormalImage;
}

void CScrollBarUI::SetButton1NormalImage(LPCTSTR pStrImage)
{
    m_sButton1NormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1HotImage()
{
    return m_sButton1HotImage;
}

void CScrollBarUI::SetButton1HotImage(LPCTSTR pStrImage)
{
    m_sButton1HotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1PushedImage()
{
    return m_sButton1PushedImage;
}

void CScrollBarUI::SetButton1PushedImage(LPCTSTR pStrImage)
{
    m_sButton1PushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1DisabledImage()
{
    return m_sButton1DisabledImage;
}

void CScrollBarUI::SetButton1DisabledImage(LPCTSTR pStrImage)
{
    m_sButton1DisabledImage = pStrImage;
    Invalidate();
}

bool CScrollBarUI::GetShowButton2()
{
    return m_bShowButton2;
}

void CScrollBarUI::SetShowButton2(bool bShow)
{
    m_bShowButton2 = bShow;
    SetPos(m_rcItem);
}

LPCTSTR CScrollBarUI::GetButton2NormalImage()
{
    return m_sButton2NormalImage;
}

void CScrollBarUI::SetButton2NormalImage(LPCTSTR pStrImage)
{
    m_sButton2NormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2HotImage()
{
    return m_sButton2HotImage;
}

void CScrollBarUI::SetButton2HotImage(LPCTSTR pStrImage)
{
    m_sButton2HotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2PushedImage()
{
    return m_sButton2PushedImage;
}

void CScrollBarUI::SetButton2PushedImage(LPCTSTR pStrImage)
{
    m_sButton2PushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2DisabledImage()
{
    return m_sButton2DisabledImage;
}

void CScrollBarUI::SetButton2DisabledImage(LPCTSTR pStrImage)
{
    m_sButton2DisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbNormalImage()
{
    return m_sThumbNormalImage;
}

void CScrollBarUI::SetThumbNormalImage(LPCTSTR pStrImage)
{
    m_sThumbNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbHotImage()
{
    return m_sThumbHotImage;
}

void CScrollBarUI::SetThumbHotImage(LPCTSTR pStrImage)
{
    m_sThumbHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbPushedImage()
{
    return m_sThumbPushedImage;
}

void CScrollBarUI::SetThumbPushedImage(LPCTSTR pStrImage)
{
    m_sThumbPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbDisabledImage()
{
    return m_sThumbDisabledImage;
}

void CScrollBarUI::SetThumbDisabledImage(LPCTSTR pStrImage)
{
    m_sThumbDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailNormalImage()
{
    return m_sRailNormalImage;
}

void CScrollBarUI::SetRailNormalImage(LPCTSTR pStrImage)
{
    m_sRailNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailHotImage()
{
    return m_sRailHotImage;
}

void CScrollBarUI::SetRailHotImage(LPCTSTR pStrImage)
{
    m_sRailHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailPushedImage()
{
    return m_sRailPushedImage;
}

void CScrollBarUI::SetRailPushedImage(LPCTSTR pStrImage)
{
    m_sRailPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailDisabledImage()
{
    return m_sRailDisabledImage;
}

void CScrollBarUI::SetRailDisabledImage(LPCTSTR pStrImage)
{
    m_sRailDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkNormalImage()
{
    return m_sBkNormalImage;
}

void CScrollBarUI::SetBkNormalImage(LPCTSTR pStrImage)
{
    m_sBkNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkHotImage()
{
    return m_sBkHotImage;
}

void CScrollBarUI::SetBkHotImage(LPCTSTR pStrImage)
{
    m_sBkHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkPushedImage()
{
    return m_sBkPushedImage;
}

void CScrollBarUI::SetBkPushedImage(LPCTSTR pStrImage)
{
    m_sBkPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkDisabledImage()
{
    return m_sBkDisabledImage;
}

void CScrollBarUI::SetBkDisabledImage(LPCTSTR pStrImage)
{
    m_sBkDisabledImage = pStrImage;
    Invalidate();
}

void CScrollBarUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);
    rc = m_rcItem;

    if( m_bHorizontal ) {
        int cx = rc.right - rc.left;
        if( m_bShowButton1 ) cx -= m_cxyFixed.cy;
        if( m_bShowButton2 ) cx -= m_cxyFixed.cy;
        if( cx > m_cxyFixed.cy ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cy;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_bShowButton2 ) {
                m_rcButton2.left = rc.right - m_cxyFixed.cy;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }

            m_rcThumb.top = rc.top;
            m_rcThumb.bottom = rc.top + m_cxyFixed.cy;
            if( m_nRange > 0 ) {
                int cxThumb = cx * (rc.right - rc.left) / (m_nRange + rc.right - rc.left);
                if( cxThumb < m_cxyFixed.cy ) 
				{
					cxThumb = m_cxyFixed.cy;
				}

                m_rcThumb.left = m_nScrollPos * (cx - cxThumb) / m_nRange + m_rcButton1.right;
                m_rcThumb.right = m_rcThumb.left + cxThumb;
                if( m_rcThumb.right > m_rcButton2.left ) {
                    m_rcThumb.left = m_rcButton2.left - cxThumb;
                    m_rcThumb.right = m_rcButton2.left;
                }
            }
            else {
                m_rcThumb.left = m_rcButton1.right;
                m_rcThumb.right = m_rcButton2.left;
            }
        }
        else {
            int cxButton = (rc.right - rc.left) / 2;
            if( cxButton > m_cxyFixed.cy ) 
			{
				cxButton = m_cxyFixed.cy;
			}
			
			m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + cxButton;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_bShowButton2 ) {
                m_rcButton2.left = rc.right - cxButton;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }

            ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
        }
    }
    else {
        int cy = rc.bottom - rc.top;
        if( m_bShowButton1 ) 
		{
			cy -= m_cxyFixed.cx;
		}

		if( m_bShowButton2 ) 
		{
			cy -= m_cxyFixed.cx;
		}
		
		if( cy > m_cxyFixed.cx ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cx;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_bShowButton2 ) {
                m_rcButton2.top = rc.bottom - m_cxyFixed.cx;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }

            m_rcThumb.left = rc.left;
            m_rcThumb.right = rc.left + m_cxyFixed.cx;
            if( m_nRange > 0 ) {
                int cyThumb = cy * (rc.bottom - rc.top) / (m_nRange + rc.bottom - rc.top);
                if( cyThumb < m_cxyFixed.cx ) 
				{
					cyThumb = m_cxyFixed.cx;
				}

				//m_rcThumb.top = m_nScrollPos * ( DOUBLE )( ( DOUBLE )( cy - cyThumb ) / ( DOUBLE )m_nRange ) + m_rcButton1.bottom;
				m_rcThumb.top = ( ( LONGLONG )m_nScrollPos * ( cy - cyThumb ) ) / m_nRange + m_rcButton1.bottom;
				__Trace( _T( "m_rcThumb.top %u, m_nScrollPos %u, cy - cyThumb %u, m_nRange %u, m_rcButton1.bottom %u\n" ), 
					m_rcThumb.top, 
					m_nScrollPos, 
					cy - cyThumb, 
					m_nRange, 
					m_rcButton1.bottom ); 

				m_rcThumb.bottom = m_rcThumb.top + cyThumb;
                if( m_rcThumb.bottom > m_rcButton2.top ) 
				{
                    m_rcThumb.top = m_rcButton2.top - cyThumb;
                    m_rcThumb.bottom = m_rcButton2.top;
                }
            }
            else {
                m_rcThumb.top = m_rcButton1.bottom;
                m_rcThumb.bottom = m_rcButton2.top;
            }
        }
        else {
            int cyButton = (rc.bottom - rc.top) / 2;
            if( cyButton > m_cxyFixed.cx ) 
			{
				cyButton = m_cxyFixed.cx;
			}
			
			m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + cyButton;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_bShowButton2 ) {
                m_rcButton2.top = rc.bottom - cyButton;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }

            ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
        }
    }
}

void CScrollBarUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;

        m_nLastScrollOffset = 0;
        m_nScrollRepeatDelay = 0;
        m_pManager->SetTimer(this, DEFAULT_TIMERID, 50U);

        if( ::PtInRect(&m_rcButton1, event.ptMouse) ) {
            m_uButton1State |= UISTATE_PUSHED;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineUp(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
        }
        else if( ::PtInRect(&m_rcButton2, event.ptMouse) ) {
            m_uButton2State |= UISTATE_PUSHED;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineDown(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineRight(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
        }
        else if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
            m_uThumbState |= UISTATE_CAPTURED | UISTATE_PUSHED;
            ptLastMouse = event.ptMouse;
            m_nLastScrollPos = m_nScrollPos;
        }
        else {
            if( !m_bHorizontal ) {
                if( event.ptMouse.y < m_rcThumb.top ) {
                    if( m_pOwner != NULL ) m_pOwner->PageUp(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( event.ptMouse.y > m_rcThumb.bottom ){
                    if( m_pOwner != NULL ) m_pOwner->PageDown(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
                }
            }
            else {
                if( event.ptMouse.x < m_rcThumb.left ) {
                    if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( event.ptMouse.x > m_rcThumb.right ){
                    if( m_pOwner != NULL ) m_pOwner->PageRight(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
                }
            }
        }
        if( m_pManager != NULL && m_pOwner == NULL ) m_pManager->SendNotify(this, _T("scroll"));
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        m_nScrollRepeatDelay = 0;
        m_nLastScrollOffset = 0;
        m_pManager->KillTimer(this, DEFAULT_TIMERID);

        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            m_uThumbState &= ~( UISTATE_CAPTURED | UISTATE_PUSHED );
            Invalidate();
        }
        else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
            m_uButton1State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
            m_uButton2State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            do 
			{
				if( event.ptMouse.x < 0
					|| event.ptMouse.y < 0)
				{
					ASSERT( event.ptMouse.x == -1 &&
						event.ptMouse.y == -1 ); 
					break; 
				}

				if( !m_bHorizontal ) 
				{
					//if( event.ptMouse.y < 0 )
					//{
					//	break; 
					//}

					//ASSERT( ( event.ptMouse.y - ptLastMouse.y ) > 0 ); 
					
					m_nLastScrollOffset = ( LONGLONG )(( LONGLONG )event.ptMouse.y - ptLastMouse.y) * m_nRange / \
						( LONGLONG )(m_rcItem.bottom - m_rcItem.top - m_rcThumb.bottom + m_rcThumb.top - 2 * m_cxyFixed.cx);

					//ASSERT( m_nLastScrollOffset >= 0 ); 
				}
				else 
				{
					m_nLastScrollOffset = ( LONGLONG )(event.ptMouse.x - ptLastMouse.x) * m_nRange / \
						( LONGLONG )(m_rcItem.right - m_rcItem.left - m_rcThumb.right + m_rcThumb.left - 2 * m_cxyFixed.cy);
				}
			}while( FALSE ); 

			//ASSERT( m_nLastScrollOffset >= 0 ); 
        }
        else {
            if( (m_uThumbState & UISTATE_HOT) != 0 ) {
                if( !::PtInRect(&m_rcThumb, event.ptMouse) ) {
                    m_uThumbState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            else {
                if( !IsEnabled() ) return;
                if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
                    m_uThumbState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_TIMER && event.wParam == DEFAULT_TIMERID )
    {
        ++m_nScrollRepeatDelay;
        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CSize(m_pOwner->GetScrollPos().cx, \
                    m_nLastScrollPos + m_nLastScrollOffset)); 
                else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CSize(m_nLastScrollPos + m_nLastScrollOffset, \
                    m_pOwner->GetScrollPos().cy)); 
                else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
            }
            Invalidate();
        }
        else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
            if( m_nScrollRepeatDelay <= 5 ) return;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineUp(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
        }
        else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
            if( m_nScrollRepeatDelay <= 5 ) return;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineDown(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineRight(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
        }
        else {
            if( m_nScrollRepeatDelay <= 5 ) return;
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_pManager->GetPaintWindow(), &pt);
            if( !m_bHorizontal ) {
                if( pt.y < m_rcThumb.top ) {
                    if( m_pOwner != NULL ) m_pOwner->PageUp(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( pt.y > m_rcThumb.bottom ){
                    if( m_pOwner != NULL ) m_pOwner->PageDown(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
                }
            }
            else {
                if( pt.x < m_rcThumb.left ) {
                    if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( pt.x > m_rcThumb.right ){
                    if( m_pOwner != NULL ) m_pOwner->PageRight(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
                }
            }
        }
        if( m_pManager != NULL && m_pOwner == NULL ) m_pManager->SendNotify(this, _T("scroll"));
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButton1State |= UISTATE_HOT;
            m_uButton2State |= UISTATE_HOT;
            if( ::PtInRect(&m_rcThumb, event.ptMouse) ) m_uThumbState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButton1State &= ~UISTATE_HOT;
            m_uButton2State &= ~UISTATE_HOT;
            m_uThumbState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }

    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CScrollBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("button1normalimage")) == 0 ) SetButton1NormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button1hotimage")) == 0 ) SetButton1HotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button1pushedimage")) == 0 ) SetButton1PushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button1disabledimage")) == 0 ) SetButton1DisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button2normalimage")) == 0 ) SetButton2NormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button2hotimage")) == 0 ) SetButton2HotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button2pushedimage")) == 0 ) SetButton2PushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("button2disabledimage")) == 0 ) SetButton2DisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbnormalimage")) == 0 ) SetThumbNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("thumbdisabledimage")) == 0 ) SetThumbDisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("railnormalimage")) == 0 ) SetRailNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("railhotimage")) == 0 ) SetRailHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("railpushedimage")) == 0 ) SetRailPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("raildisabledimage")) == 0 ) SetRailDisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("bknormalimage")) == 0 ) SetBkNormalImage(pstrValue);
    else if( _tcscmp(pstrName, _T("bkhotimage")) == 0 ) SetBkHotImage(pstrValue);
    else if( _tcscmp(pstrName, _T("bkpushedimage")) == 0 ) SetBkPushedImage(pstrValue);
    else if( _tcscmp(pstrName, _T("bkdisabledimage")) == 0 ) SetBkDisabledImage(pstrValue);
    else if( _tcscmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("linesize")) == 0 ) SetLineSize(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("range")) == 0 ) SetScrollRange(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("value")) == 0 ) SetScrollPos(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("showbutton1")) == 0 ) SetShowButton1(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("showbutton2")) == 0 ) SetShowButton2(_tcscmp(pstrValue, _T("true")) == 0);
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CScrollBarUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    PaintBk(hDC);
    PaintButton1(hDC);
    PaintButton2(hDC);
    PaintThumb(hDC);
    PaintRail(hDC);
}

void CScrollBarUI::PaintBk(HDC hDC)
{
    if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
    else m_uThumbState &= ~ UISTATE_DISABLED;

    if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
        if( !m_sBkDisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkDisabledImage) ) m_sBkDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
        if( !m_sBkPushedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkPushedImage) ) m_sBkPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
        if( !m_sBkHotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sBkHotImage) ) m_sBkHotImage.Empty();
            else return;
        }
    }

    if( !m_sBkNormalImage.IsEmpty() ) {
        if( !DrawImage(hDC, (LPCTSTR)m_sBkNormalImage) ) m_sBkNormalImage.Empty();
        else return;
    }
}

void CScrollBarUI::PaintButton1(HDC hDC)
{
    if( !m_bShowButton1 ) return;
    
    if( !IsEnabled() ) m_uButton1State |= UISTATE_DISABLED;
    else m_uButton1State &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcButton1.left - m_rcItem.left, \
        m_rcButton1.top - m_rcItem.top, m_rcButton1.right - m_rcItem.left, m_rcButton1.bottom - m_rcItem.top);

    if( (m_uButton1State & UISTATE_DISABLED) != 0 ) {
        if( !m_sButton1DisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton1DisabledImage, (LPCTSTR)m_sImageModify) ) m_sButton1DisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
        if( !m_sButton1PushedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton1PushedImage, (LPCTSTR)m_sImageModify) ) m_sButton1PushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButton1State & UISTATE_HOT) != 0 ) {
        if( !m_sButton1HotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton1HotImage, (LPCTSTR)m_sImageModify) ) m_sButton1HotImage.Empty();
            else return;
        }
    }

    if( !m_sButton1NormalImage.IsEmpty() ) {
        if( !DrawImage(hDC, (LPCTSTR)m_sButton1NormalImage, (LPCTSTR)m_sImageModify) ) m_sButton1NormalImage.Empty();
        else return;
    }

    DWORD dwBorderColor = 0xFF85E4FF;
    int nBorderSize = 2;
    CRenderEngine::DrawRect(hDC, m_rcButton1, nBorderSize, dwBorderColor);
}

void CScrollBarUI::PaintButton2(HDC hDC)
{
    if( !m_bShowButton2 ) return;
    
    if( !IsEnabled() ) m_uButton2State |= UISTATE_DISABLED;
    else m_uButton2State &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcButton2.left - m_rcItem.left, \
        m_rcButton2.top - m_rcItem.top, m_rcButton2.right - m_rcItem.left, m_rcButton2.bottom - m_rcItem.top);

    if( (m_uButton2State & UISTATE_DISABLED) != 0 ) {
        if( !m_sButton2DisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton2DisabledImage, (LPCTSTR)m_sImageModify) ) m_sButton2DisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
        if( !m_sButton2PushedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton2PushedImage, (LPCTSTR)m_sImageModify) ) m_sButton2PushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButton2State & UISTATE_HOT) != 0 ) {
        if( !m_sButton2HotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sButton2HotImage, (LPCTSTR)m_sImageModify) ) m_sButton2HotImage.Empty();
            else return;
        }
    }

    if( !m_sButton2NormalImage.IsEmpty() ) {
        if( !DrawImage(hDC, (LPCTSTR)m_sButton2NormalImage, (LPCTSTR)m_sImageModify) ) m_sButton2NormalImage.Empty();
        else return;
    }

    DWORD dwBorderColor = 0xFF85E4FF;
    int nBorderSize = 2;
    CRenderEngine::DrawRect(hDC, m_rcButton2, nBorderSize, dwBorderColor);
}

void CScrollBarUI::PaintThumb(HDC hDC)
{
	do 
	{
		if( m_rcThumb.left == 0 
			&& m_rcThumb.top == 0 
			&& m_rcThumb.right == 0 
			&& m_rcThumb.bottom == 0 ) 
		{
			break; 
		}

		if( !IsEnabled() ) 
		{
			m_uThumbState |= UISTATE_DISABLED;
		}
		else 
		{
			m_uThumbState &= ~ UISTATE_DISABLED;
		}

		do 
		{
			m_sImageModify.Empty();
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcThumb.left - m_rcItem.left, \
				m_rcThumb.top - m_rcItem.top, m_rcThumb.right - m_rcItem.left, m_rcThumb.bottom - m_rcItem.top ); 

			if( (m_uThumbState & UISTATE_DISABLED) != 0 ) 
			{
				if( !m_sThumbDisabledImage.IsEmpty() ) 
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sThumbDisabledImage, (LPCTSTR)m_sImageModify) ) 
					{
						m_sThumbDisabledImage.Empty(); 
					}
					else 
					{
						break; 
					}
				}
			}
			else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) 
			{
				if( !m_sThumbPushedImage.IsEmpty() ) 
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) ) 
					{
						m_sThumbPushedImage.Empty();
					}
					else 
					{
						break; 
					}
				}
			}
			else if( (m_uThumbState & UISTATE_HOT) != 0 ) 
			{
				if( !m_sThumbHotImage.IsEmpty() ) 
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) ) 
					{
						m_sThumbHotImage.Empty();
					}
					else 
					{
						break; 
					}
				}
			}

			if( !m_sThumbNormalImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sThumbNormalImage, (LPCTSTR)m_sImageModify) ) 
				{
					m_sThumbNormalImage.Empty();
				}
				else 
				{
					break; 
				}
			}

		}while( FALSE );

		//DWORD dwBorderColor = 0xFF85E4FF;
		
		DWORD dwBorderColor = GetBorderColor(); 
		int nBorderSize = 1;
		CRenderEngine::DrawRect(hDC, m_rcThumb, nBorderSize, dwBorderColor); 
	}while( FALSE );

}

void CScrollBarUI::PaintRail(HDC hDC)
{
    if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
    if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
    else m_uThumbState &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    if( !m_bHorizontal ) {
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcThumb.left - m_rcItem.left, \
            (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top - m_cxyFixed.cx / 2, \
            m_rcThumb.right - m_rcItem.left, \
            (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top + m_cxyFixed.cx - m_cxyFixed.cx / 2);
    }
    else {
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), \
            (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left - m_cxyFixed.cy / 2, \
            m_rcThumb.top - m_rcItem.top, \
            (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left + m_cxyFixed.cy - m_cxyFixed.cy / 2, \
            m_rcThumb.bottom - m_rcItem.top);
    }

    if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
        if( !m_sRailDisabledImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sRailDisabledImage, (LPCTSTR)m_sImageModify) ) m_sRailDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
        if( !m_sRailPushedImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sRailPushedImage, (LPCTSTR)m_sImageModify) ) m_sRailPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
        if( !m_sRailHotImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sRailHotImage, (LPCTSTR)m_sImageModify) ) m_sRailHotImage.Empty();
            else return;
        }
    }

    if( !m_sRailNormalImage.IsEmpty() ) {
        if( !DrawImage(hDC, (LPCTSTR)m_sRailNormalImage, (LPCTSTR)m_sImageModify) ) m_sRailNormalImage.Empty();
        else return;
    }
}

} // namespace DuiLib
