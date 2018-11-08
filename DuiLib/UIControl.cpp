#include "StdAfx.h"

namespace DuiLib {

CControlUI::CControlUI() : 
m_pManager(NULL), 
m_pParent(NULL), 
m_bUpdateNeeded(true),
m_bMenuUsed(false),
m_bVisible(true), 
m_bInternVisible(true),
m_bFocused(false),
m_bEnabled(true),
m_bMouseEnabled(true),
m_bFloat(false),
m_bSetPos(false),
m_chShortcut('\0'),
m_pTag(NULL),
m_dwBackColor(0),
m_dwBackColor2(0),
m_dwBackColor3(0),
m_dwBorderColor(0),
m_dwFocusBorderColor(0),
m_bColorHSL(false),
m_nBorderSize(1)
{
    m_cXY.cx = m_cXY.cy = 0;
    m_cxyFixed.cx = m_cxyFixed.cy = 0;
    m_cxyMin.cx = m_cxyMin.cy = 0;
    m_cxyMax.cx = m_cxyMax.cy = 9999;
    m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

    ::ZeroMemory(&m_rcPadding, sizeof(m_rcPadding));
    ::ZeroMemory(&m_rcItem, sizeof(RECT));
    ::ZeroMemory(&m_rcPaint, sizeof(RECT));
    ::ZeroMemory(&m_tRelativePos, sizeof(TRelativePosUI));
}

CControlUI::~CControlUI()
{
    if( OnDestroy ) OnDestroy(this);
    if( m_pManager != NULL ) m_pManager->ReapObjects(this);
}

CStdString CControlUI::GetName() const
{
    return m_sName;
}

void CControlUI::SetName(LPCTSTR pstrName)
{
    m_sName = pstrName;

	//if( 0 == GetName().CompareNoCase( _T( "trace_tab" ) ) )
	//{
	//	DBG_BP(); 
	//}

}

LPVOID CControlUI::GetInterface(LPCTSTR pstrName)
{

    if( _tcscmp(pstrName, UI_CONTROL_INTERFACE_NAME ) == 0 ) return this;
    return NULL;
}

LPCTSTR CControlUI::GetClass() const
{
    return UI_CONTROL_CLASS_NAME;
}

ULONG CControlUI::GetClassId() const
{
	return UI_CONTROL_CLASS_ID;
}


UINT CControlUI::GetControlFlags() const
{
    return 0;
}

bool CControlUI::Activate()
{
    if( !IsVisible() ) return false;
    if( !IsEnabled() ) return false;
    return true;
}

CPaintManagerUI* CControlUI::GetManager() const
{
    return m_pManager;
}

void CControlUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    m_pManager = pManager;
    m_pParent = pParent;
    if( bInit && m_pParent ) Init();
}

CControlUI* CControlUI::GetParent() const
{
    return m_pParent;
}

CStdString CControlUI::GetText() const
{
    return m_sText;
}

void CControlUI::SetText(LPCTSTR pstrText)
{
    if( m_sText == pstrText ) return;

    m_sText = pstrText;
    Invalidate();
}

DWORD CControlUI::GetBkColor() const
{
    return m_dwBackColor;
}

void CControlUI::SetBkColor(DWORD dwBackColor)
{
    if( m_dwBackColor == dwBackColor ) return;

    m_dwBackColor = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor2() const
{
    return m_dwBackColor2;
}

void CControlUI::SetBkColor2(DWORD dwBackColor)
{
    if( m_dwBackColor2 == dwBackColor ) return;

    m_dwBackColor2 = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor3() const
{
    return m_dwBackColor3;
}

void CControlUI::SetBkColor3(DWORD dwBackColor)
{
    if( m_dwBackColor3 == dwBackColor ) return;

    m_dwBackColor3 = dwBackColor;
    Invalidate();
}

LPCTSTR CControlUI::GetBkImage()
{
    return m_sBkImage;
}

void CControlUI::SetBkImage(LPCTSTR pStrImage)
{
    if( m_sBkImage == pStrImage ) return;

    m_sBkImage = pStrImage;
    Invalidate();
}

DWORD CControlUI::GetBorderColor() const
{
    return m_dwBorderColor;
}

void CControlUI::SetBorderColor(DWORD dwBorderColor)
{
    if( m_dwBorderColor == dwBorderColor ) return;

    m_dwBorderColor = dwBorderColor;
    Invalidate();
}

DWORD CControlUI::GetFocusBorderColor() const
{
    return m_dwFocusBorderColor;
}

void CControlUI::SetFocusBorderColor(DWORD dwBorderColor)
{
    if( m_dwFocusBorderColor == dwBorderColor ) return;

    m_dwFocusBorderColor = dwBorderColor;
    Invalidate();
}

bool CControlUI::IsColorHSL() const
{
    return m_bColorHSL;
}

void CControlUI::SetColorHSL(bool bColorHSL)
{
    if( m_bColorHSL == bColorHSL ) return;

    m_bColorHSL = bColorHSL;
    Invalidate();
}

int CControlUI::GetBorderSize() const
{
    return m_nBorderSize;
}

void CControlUI::SetBorderSize(int nSize)
{
    if( m_nBorderSize == nSize ) return;

    m_nBorderSize = nSize;
    Invalidate();
}

SIZE CControlUI::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void CControlUI::SetBorderRound(SIZE cxyRound)
{
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

bool CControlUI::DrawImage(HDC hDC, LPCTSTR pStrImage, LPCTSTR pStrModify )
{
	bool ret = false; 

	//switch( Type )
	//{
	//case IMAGE_DATA_CONFIG_STRING: 
		ret = CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, ( LPCTSTR )pStrImage, pStrModify);
		//break; 
	//case IMAGE_DATA_HICON:
	//	break; 
	
	//default:
	//	break; 
	//}

	return ret; 

}

const RECT& CControlUI::GetPos()
{
    return m_rcItem;
}

/*****************************************************************************************
配置控制位置大小的过程

配置新的控制位置:

如果位置需要计算(如相对位置)，进行位置计算。
如果位置直接使用，直接配置。

对控制内容进行重绘

控制的相互包含关系:
1.控制的大小，在父控制中的位置完全不确定。需要遍历计算父控制的位置区间，最后得到
控制窗体显示位置区间。
2.但可以通过对在父控制中的位置的确定方法，减小遍历计算的范围。
*****************************************************************************************/

void CControlUI::SetPos(RECT rc)
{
	CRect invalidateRc; 

	//if( 0 == GetName().CompareNoCase( _T( "trace_tab" ) ) )
	//{
	//	DBG_BP(); 
	//}

    if( rc.right < rc.left ) 
	{
		rc.right = rc.left;
	}

    if( rc.bottom < rc.top )
	{
		rc.bottom = rc.top;
	}

	//if( rc.left == m_rcItem.left 
	//	&& rc.right == m_rcItem.right 
	//	&& rc.bottom == m_rcItem.bottom 
	//	&& rc.top == m_rcItem.top )
	if( EqualRect( &rc, &m_rcItem ) == TRUE 
		&& rc.left == 0 
		&& rc.top == 0 
		&& rc.bottom == 0 
		&& rc.right == 0 )
	{
		//DBG_BP(); 
		m_bUpdateNeeded = false; 

		return; 
	}

	if( m_pManager == NULL ) 
	{
		//DBG_BP(); 
		return;
	}
	
	invalidateRc = m_rcItem;

    m_rcItem = rc;

    if( m_bFloat 
		&& m_tRelativePos.bRelative == false )
	{
        CControlUI* pParent = GetParent();

        if( pParent != NULL )
		{
            RECT rcParentPos = pParent->GetPos(); 

            if( m_cXY.cx >= 0 ) 
			{
				m_cXY.cx = m_rcItem.left - rcParentPos.left;
				ASSERT( m_cXY.cx >= 0 ); 
			}
			else 
			{
				m_cXY.cx = m_rcItem.right - rcParentPos.right;
				ASSERT( m_cXY.cx < 0 ); 
			}

            if( m_cXY.cy >= 0 ) 
			{
				m_cXY.cy = m_rcItem.top - rcParentPos.top;
				ASSERT( m_cXY.cy >= 0 ); 
			}
			else 
			{
				m_cXY.cy = m_rcItem.bottom - rcParentPos.bottom;
				ASSERT( m_cXY.cy < 0 ); 
			}
		}
    }

	//this is to make the synchronize when access the event handler of sizing.use the critical section or some lock mechanism may be greater.

	/********************************************************************************************************************
	对控制的大小事件处理过程规定:
	1.控制内部的OnSize事件不可以调用SetPos
	2.或控制内部的OnSize事件调用了SetPos，那么调用OnSize的SetPos函数需要退出。
	不再SetPos重复多次配置控制位置区间的效果叠加结果是不完善的处理方法。
	********************************************************************************************************************/
    if( false == m_bSetPos ) 
	{
        m_bSetPos = true;

        if( OnSize != NULL ) 
		{
			OnSize(this);
		}
		
		m_bSetPos = false;
    }
	else
	{
		ASSERT( "don't call setpos in onsize" ); 
	}

    m_bUpdateNeeded = false;

    // NOTE: SetPos() is usually called during the WM_PAINT cycle where all controls are
    //       being laid out. Calling UpdateLayout() again would be wrong. config the
    //       paint region of the the window that is good.(if we're already inside WM_PAINT we'll just validate it out).

	if( ::IsRectEmpty( &invalidateRc ) ) 
	{
		invalidateRc = rc;
	}
	else
	{
		invalidateRc.Join( m_rcItem ); 
	}

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;

	/***************************************************
	advance performance tip:
	don't use more container if it's useless.
	***************************************************/

	while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc; 
    
		rcParent = pParent->GetPos(); 

        if( !::IntersectRect( &invalidateRc, &rcTemp, &rcParent ) ) 
        {
            return;
        }
    }

    m_pManager->Invalidate( invalidateRc ); 
}

int CControlUI::GetWidth() const
{
    return m_rcItem.right - m_rcItem.left;
}

int CControlUI::GetHeight() const
{
    return m_rcItem.bottom - m_rcItem.top;
}

int CControlUI::GetX() const
{
    return m_rcItem.left;
}

int CControlUI::GetY() const
{
    return m_rcItem.top;
}

RECT CControlUI::GetPadding() const
{
    return m_rcPadding;
}

void CControlUI::SetPadding(RECT rcPadding)
{
    m_rcPadding = rcPadding;
    NeedParentUpdate();
}

SIZE CControlUI::GetFixedXY() const
{
    return m_cXY;
}

void CControlUI::SetFixedXY(SIZE szXY)
{
	if( 0 == GetName().CompareNoCase( _T( "trace_tab" ) ) )
	{
		DBG_BP(); 
	}

	m_cXY.cx = szXY.cx;
	m_cXY.cy = szXY.cy;

	if( !m_bFloat ) 
	{
		NeedParentUpdate();
	}
	else 
	{
		NeedUpdate();
	}
}

int CControlUI::GetFixedWidth() const
{
	if( 0 == GetName().CompareNoCase( _T( "trace_tab" ) ) )
	{
		DBG_BP(); 
	}

    return m_cxyFixed.cx;
}

void CControlUI::SetFixedWidth(int cx)
{
    if( cx < 0 ) return; 

	if( 0 == GetName().CompareNoCase( _T( "trace_tab" ) ) )
	{
		DBG_BP(); 
	}

    m_cxyFixed.cx = cx;
    if( !m_bFloat ) 
	{
		NeedParentUpdate();
	}
	else 
	{
		NeedUpdate();
	}
}

int CControlUI::GetFixedHeight() const
{
    return m_cxyFixed.cy;
}

void CControlUI::SetFixedHeight(int cy)
{
    if( cy < 0 ) return; 
    m_cxyFixed.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinWidth() const
{
    return m_cxyMin.cx;
}

void CControlUI::SetMinWidth(int cx)
{
    if( m_cxyMin.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMin.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxWidth() const
{
    return m_cxyMax.cx;
}

void CControlUI::SetMaxWidth(int cx)
{
    if( m_cxyMax.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMax.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinHeight() const
{
    return m_cxyMin.cy;
}

void CControlUI::SetMinHeight(int cy)
{
    if( m_cxyMin.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMin.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxHeight() const
{
    return m_cxyMax.cy;
}

void CControlUI::SetMaxHeight(int cy)
{
    if( m_cxyMax.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMax.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

void CControlUI::SetRelativePos(SIZE szMove,SIZE szZoom)
{
    m_tRelativePos.bRelative = TRUE;
    m_tRelativePos.nMoveXPercent = szMove.cx;
    m_tRelativePos.nMoveYPercent = szMove.cy;
    m_tRelativePos.nZoomXPercent = szZoom.cx;
    m_tRelativePos.nZoomYPercent = szZoom.cy;
}

void CControlUI::SetRelativeParentSize(SIZE sz)
{
    m_tRelativePos.szParent = sz;
}

TRelativePosUI CControlUI::GetRelativePos() const
{
    return m_tRelativePos;
}

bool CControlUI::IsRelativePos() const
{
    return m_tRelativePos.bRelative;
}

CStdString CControlUI::GetToolTip() const
{
    return m_sToolTip;
}

void CControlUI::SetToolTip(LPCTSTR pstrText)
{
    m_sToolTip = pstrText;
}


TCHAR CControlUI::GetShortcut() const
{
    return m_chShortcut;
}

void CControlUI::SetShortcut(TCHAR ch)
{
    m_chShortcut = ch;
}

bool CControlUI::IsContextMenuUsed() const
{
    return m_bMenuUsed;
}

void CControlUI::SetContextMenuUsed(bool bMenuUsed)
{
    m_bMenuUsed = bMenuUsed;
}

const CStdString& CControlUI::GetUserData()
{
    return m_sUserData;
}

void CControlUI::SetUserData(LPCTSTR pstrText)
{
    m_sUserData = pstrText;
}

UINT_PTR CControlUI::GetTag() const
{
    return m_pTag;
}

void CControlUI::SetTag(UINT_PTR pTag)
{
    m_pTag = pTag;
}

bool CControlUI::IsVisible() const
{
    return m_bVisible && m_bInternVisible;
}

void CControlUI::SetVisible(bool bVisible)
{
    if( m_bVisible == bVisible ) return;

    bool v = IsVisible();
    m_bVisible = bVisible;
    if( m_bFocused ) m_bFocused = false;
    if( IsVisible() != v ) {
        NeedParentUpdate();
    }
}

void CControlUI::SetInternVisible(bool bVisible)
{
    m_bInternVisible = bVisible;
}

bool CControlUI::IsEnabled() const
{
    return m_bEnabled;
}

void CControlUI::SetEnabled(bool bEnabled)
{
    if( m_bEnabled == bEnabled ) return;

    m_bEnabled = bEnabled;
    Invalidate();
}

bool CControlUI::IsMouseEnabled() const
{
    return m_bMouseEnabled;
}

void CControlUI::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

bool CControlUI::IsFocused() const
{
    return m_bFocused;
}

void CControlUI::SetFocus()
{
    if( m_pManager != NULL ) m_pManager->SetFocus(this);
}

bool CControlUI::IsFloat() const
{
    return m_bFloat;
}

void CControlUI::SetFloat( bool bFloat )
{
    if( m_bFloat == bFloat )
	{
		return;
	}

    m_bFloat = bFloat;

    NeedParentUpdate();
}

CControlUI* CControlUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
    if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
    return Proc(this, pData);
}

void CControlUI::Invalidate()
{
    if( !IsVisible() ) 
	{
		return;
	}

    RECT invalidateRc = m_rcItem;

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;

    while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if( !::IntersectRect( &invalidateRc, &rcTemp, &rcParent ) ) 
        {
            return;
        }
    }

    if( m_pManager != NULL ) 
	{
		m_pManager->Invalidate( invalidateRc );
	}
}

bool CControlUI::IsUpdateNeeded() const
{
    return m_bUpdateNeeded;
}

void CControlUI::NeedUpdate()
{
	do 
	{
		if( !IsVisible() ) 
		{
			break; 
		}

		//if( m_bUpdateNeeded == true )
		//{
		//	break; 
		//}

		m_bUpdateNeeded = true;

		Invalidate();

		if( m_pManager != NULL ) 
		{
			m_pManager->NeedUpdate();
		}

	} while ( FALSE );
}

void CControlUI::NeedUpdateNoInvalidate()
{
	if( !IsVisible() ) 
	{
		return;
	}
	
	m_bUpdateNeeded = true;
	//Invalidate();

	if( m_pManager != NULL ) 
	{
		m_pManager->NeedUpdate();
	}
}

void CControlUI::NeedParentUpdate()
{
    if( GetParent() ) 
	{
		/*********************************************
		performance tip:
		1.if this control is visible,then it will set update
		flags in the need update function.don't need update again.
		2.if this control is visible,then paint manager update
		flags is set too.
		*********************************************/
        GetParent()->NeedUpdate();
        GetParent()->Invalidate();
    }
    else 
	{
        NeedUpdate();
    }

    if( m_pManager != NULL ) 
	{
		m_pManager->NeedUpdate();
	}

}

DWORD CControlUI::GetAdjustColor(DWORD dwColor)
{
    if( !m_bColorHSL ) return dwColor;
    short H, S, L;
    CPaintManagerUI::GetHSL(&H, &S, &L);
    return CRenderEngine::AdjustColor(dwColor, H, S, L);
}

void CControlUI::Init()
{
    DoInit();
    if( OnInit ) OnInit(this);
}

void CControlUI::DoInit()
{

}

void CControlUI::Event(TEventUI& event)
{
    if( OnEvent(&event) ) DoEvent(event);
}

void CControlUI::DoEvent(TEventUI& event)
{
    if( event.Type == UIEVENT_SETCURSOR )
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        return;
    }
    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        m_bFocused = true;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        m_bFocused = false;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_TIMER )
    {
        m_pManager->SendNotify(this, _T("timer"), event.wParam, event.lParam);
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        if( IsContextMenuUsed() ) {
            m_pManager->SendNotify(this, _T("menu"), event.wParam, event.lParam);
            return;
        }
    }
    if( m_pParent != NULL ) 
	{
		m_pParent->DoEvent(event);
	}
}

void CControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("pos")) == 0 ) {
        RECT rcPos = { 0 };
        LPTSTR pstr = NULL;
        rcPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SIZE szXY = {rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom};
        SetFixedXY(szXY);
        SetFixedWidth(rcPos.right - rcPos.left);
        SetFixedHeight(rcPos.bottom - rcPos.top);
    }
    else if( _tcscmp(pstrName, _T("relativepos")) == 0 ) {
        SIZE szMove,szZoom;
        LPTSTR pstr = NULL;
        szMove.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        szMove.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        szZoom.cx = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        szZoom.cy = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr); 
        SetRelativePos(szMove,szZoom);
    }
    else if( _tcscmp(pstrName, _T("padding")) == 0 ) {
        RECT rcPadding = { 0 };
        LPTSTR pstr = NULL;
        rcPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetPadding(rcPadding);
    }
    else if( _tcscmp(pstrName, _T("bkcolor")) == 0 || _tcscmp(pstrName, _T("bkcolor1")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bkcolor2")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor2(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bkcolor3")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor3(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bordercolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBorderColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("focusbordercolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetFocusBorderColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("colorhsl")) == 0 ) SetColorHSL(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("bordersize")) == 0 ) SetBorderSize(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("borderround")) == 0 ) {
        SIZE cxyRound = { 0 };
        LPTSTR pstr = NULL;
        cxyRound.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
        SetBorderRound(cxyRound);
    }
    else if( _tcscmp(pstrName, _T("bkimage")) == 0 ) 
	{
		SetBkImage(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("width")) == 0 ) SetFixedWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("height")) == 0 ) SetFixedHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("minwidth")) == 0 ) SetMinWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("minheight")) == 0 ) SetMinHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("maxwidth")) == 0 ) SetMaxWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("maxheight")) == 0 ) SetMaxHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("name")) == 0 ) 
	{
//		if( _tcscmp( pstrValue, _T( "icon_menu" ) ) == 0 )
//		{
//#define DBG_BP() __asm int 3; 
//
//			DBG_BP(); 
//		}
		SetName(pstrValue);
	}
    else if( _tcscmp(pstrName, _T("text")) == 0 ) SetText(pstrValue);
    else if( _tcscmp(pstrName, _T("tooltip")) == 0 ) SetToolTip(pstrValue);
    else if( _tcscmp(pstrName, _T("userdata")) == 0 ) SetUserData(pstrValue);
    else if( _tcscmp(pstrName, _T("enabled")) == 0 ) SetEnabled(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("mouse")) == 0 ) SetMouseEnabled(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("visible")) == 0 ) SetVisible(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("float")) == 0 ) SetFloat(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("shortcut")) == 0 ) SetShortcut(pstrValue[0]);
    else if( _tcscmp(pstrName, _T("menu")) == 0 ) SetContextMenuUsed(_tcscmp(pstrValue, _T("true")) == 0);
}

CControlUI* CControlUI::ApplyAttributeList(LPCTSTR pstrList)
{
    CStdString sItem;
    CStdString sValue;
    while( *pstrList != _T('\0') ) {
        sItem.Empty();
        sValue.Empty();
        while( *pstrList != _T('\0') && *pstrList != _T('=') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('=') );
        if( *pstrList++ != _T('=') ) return this;
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        while( *pstrList != _T('\0') && *pstrList != _T('\"') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        SetAttribute(sItem, sValue);
        if( *pstrList++ != _T(' ') ) return this;
    }
    return this;
}

SIZE CControlUI::EstimateSize(SIZE szAvailable)
{
    return m_cxyFixed;
}

void CControlUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect( &m_rcPaint, &rcPaint, &m_rcItem ) ) 
	{
		return;
	}

	//if( m_sName == _T( "filter_btn" ) )
	//{
	//	__asm int 3; 
	//	//DBG_BP(); 
	//	//SetBkColor( 0xffffffff ); 
	//}

    // 绘制循序：背景颜色->背景图->状态图->文本->边框
    if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )
	{
		//this is the second clip on one process of painting.

        CRenderClip roundClip;
        CRenderClip::GenerateRoundClip(hDC, m_rcPaint,  m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
        //PaintBkColor(hDC);
        //PaintBkImage(hDC);
        //PaintStatusImage(hDC);
        //PaintText(hDC);
        //PaintBorder(hDC);
    }
    
	//else
	{
        PaintBkColor(hDC);
        PaintBkImage(hDC);
        PaintStatusImage(hDC);
        PaintText(hDC);
        PaintBorder(hDC);
    }
}

void CControlUI::PaintBkColor(HDC hDC)
{
    if( m_dwBackColor != 0 )
	{
        if( m_dwBackColor2 != 0 )
		{
            if( m_dwBackColor3 != 0 )
			{
                RECT rc = m_rcItem;
                rc.bottom = (rc.bottom + rc.top) / 2;
                CRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 8);
                rc.top = rc.bottom;
                rc.bottom = m_rcItem.bottom;
                CRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), true, 8);
            }
            else 
			{
                CRenderEngine::DrawGradient(hDC, m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 16);
			}
		}
        else if( m_dwBackColor >= 0xFF000000 ) 
		{
			CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwBackColor));
		}
		else 
		{
			CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(m_dwBackColor));
		}
	}
}

void CControlUI::PaintBkImage(HDC hDC)
{
    if( m_sBkImage.IsEmpty() ) return;
    if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
}

void CControlUI::PaintStatusImage(HDC hDC)
{
    return;
}

void CControlUI::PaintText(HDC hDC)
{
    return;
}

void CControlUI::PaintBorder(HDC hDC)
{
	if( m_nBorderSize > 0 )
	{
        if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )
		{
			if (IsFocused() && m_dwFocusBorderColor != 0)
				CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor));
			else
				CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor));
		}
		else
		{
			if (IsFocused() && m_dwFocusBorderColor != 0)
				CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwFocusBorderColor));
			else if (m_dwBorderColor != 0)
				CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwBorderColor));
		}
	}
}

void CControlUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
    return;
}

} // namespace DuiLib
