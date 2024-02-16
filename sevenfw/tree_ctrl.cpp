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
 */

#include "stdafx.h"
#include "StaticTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticTreeCtrl

CStaticTreeCtrl::CStaticTreeCtrl()
{
	m_pFirstNode			= NULL;
	m_pLastNode				= NULL;
	m_iIndent				= 4;//16;				// Indentation for tree branches
	m_iPadding				= 4;				// Padding between tree and the control border

	m_bShowLines			= TRUE;				// Show lines by default
	m_bScrollBarMessage		= FALSE;			// Only relevant when calculating the scrollbar

	m_iDocHeight			= 0;				// A polite yet meaningless default

	m_crDefaultTextColor	= RGB(0,64,128);//RGB(58,58,58);	// Some default
	m_crDefaultBkColor		= RGB(230,230,230);
	m_crDefaultBkHLColor	= RGB(90, 80, 30);
	m_crDefaultTextHLColor	= RGB(235,240,240);
	m_crConnectingLines		= RGB(128,128,128);	// Some default

	// Safeguards
	SetTextFont( 8, FALSE, FALSE, "Arial Unicode MS" );
	m_pSelected				= NULL;	

	InitializeCriticalSection( &Lock );
}

CStaticTreeCtrl::~CStaticTreeCtrl()
{
	DeleteLinkedNodes( m_pFirstNode );	// Delete all childs if there are any		// Delete top node
	m_pFirstNode = NULL;
	
	m_Font.DeleteObject();

	if( m_bmpBackground.GetSafeHandle() != NULL )
		m_bmpBackground.DeleteObject();
}


BEGIN_MESSAGE_MAP(CStaticTreeCtrl, CStatic)
	//{{AFX_MSG_MAP(CStaticTreeCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	//ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//	PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////

CStaticTreeCtrl& CStaticTreeCtrl::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const CString& csFaceName )
{
	HDC hDC = ::GetDC(NULL);
	m_lgFont.lfHeight			= -MulDiv( nHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );
	m_lgFont.lfWidth			= 0;
	m_lgFont.lfEscapement		= 0;
	m_lgFont.lfOrientation		= 0;
	m_lgFont.lfWeight			= ( bBold )? FW_BOLD:FW_DONTCARE;
	m_lgFont.lfItalic			= (BYTE)( ( bItalic )? TRUE:FALSE );
	m_lgFont.lfUnderline		= FALSE;
	m_lgFont.lfStrikeOut		= FALSE;
	m_lgFont.lfCharSet			= DEFAULT_CHARSET;
	m_lgFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	m_lgFont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	m_lgFont.lfQuality			= DEFAULT_QUALITY;
	m_lgFont.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;

	STRCPY( m_lgFont.lfFaceName, csFaceName );

	if( m_Font.GetSafeHandle() != NULL )
		m_Font.DeleteObject();
	
	m_Font.CreateFontIndirect( &m_lgFont );

	// Calculate node height for this font
	int		iSaved		= ::SaveDC(hDC);
	HANDLE	pOldFont	= SelectObject(hDC, m_Font.GetSafeHandle() );

	// Calculate the height of this font with a character likely to be 'big'
	// and don't forget to add a little padding
	SIZE size_char;
	::GetTextExtentPoint32(hDC, _T("T"), 1, &size_char);
	m_iLineHeight	= size_char.cy + 2;

	SelectObject(hDC, pOldFont );
	RestoreDC(hDC, iSaved );
	::ReleaseDC(NULL, hDC);

	return *this;
}

CStaticTreeCtrl& CStaticTreeCtrl::SetDefaultTextColor( COLORREF crText, COLORREF crTextHL, COLORREF crBk, COLORREF crBkHL )
{
	m_crDefaultTextColor = crText;
	m_crDefaultBkColor = crBk;
	m_crDefaultTextHLColor = crTextHL;
	m_crDefaultBkHLColor = crBkHL;
	return *this;
}

HTREENODE CStaticTreeCtrl::InsertChild( HTREENODE pPrev, const CString csLabel,
										COLORREF crText /* = 0 */, COLORREF crBk, 
										COLORREF crTextHL,			COLORREF crBkHL,
										BOOL bUseDefaultTextColor /* = TRUE */,
										BOOL bInvalidate /* = FALSE  */)
{
	HTREENODE pNewNode = NULL;

	EnterCriticalSection( &Lock ); 

	//CString DebugOut; 
	if(m_pFirstNode == NULL)
	{
		m_pFirstNode = new CTreeNode();

		//DebugOut.Format( "new node 0x%0.8x \n", m_pFirstNode ); 
		//OutputDebugString( DebugOut.GetBuffer() ); 
		ASSERT( m_pFirstNode != NULL );
		m_pFirstNode->pPrev = NULL;
		m_pFirstNode->pNext = NULL;
		m_pFirstNode->m_uNo = 1;
		m_pLastNode = m_pFirstNode;
		pNewNode = m_pLastNode;
	}
	else
	{
		if( pPrev == NULL )	// Check for top node
			pPrev = m_pLastNode;

		pNewNode = new CTreeNode();
		//DebugOut.Format( "new node 0x%0.8x \n", pNewNode ); 
		//OutputDebugString( DebugOut.GetBuffer() ); 
		ASSERT( pNewNode != NULL );
		pNewNode->pPrev	= pPrev;	// New node's parent
		
		if(pPrev->pNext != NULL)
		{
			pNewNode->pNext	= pPrev->pNext;
			for(HTREENODE pTmpNode = pNewNode; pTmpNode; pTmpNode = pTmpNode->pNext)
			{
				pTmpNode->m_uNo = pTmpNode->pPrev->m_uNo + 1;
			}
			pPrev->pNext = pNewNode;
		}
		else
		{
			pPrev->pNext = pNewNode;
			pNewNode->pNext = NULL;
			pNewNode->m_uNo = pPrev->m_uNo + 1;
		}
		m_pLastNode = pNewNode;
	}

	LeaveCriticalSection( &Lock ); 

	// Basic node information
	pNewNode->csLabel	= csLabel;	// New node's label

	if( bUseDefaultTextColor )
		pNewNode->bUseDefaultTextColor = TRUE;		// Use the default text color
	else
	{
		pNewNode->bUseDefaultTextColor = FALSE;
		pNewNode->crText = crText;					// New node's text color
		pNewNode->crBk = crBk;
		pNewNode->crTextHL = crTextHL;
		pNewNode->crBkHL = crBkHL;
	}

	
	// Repaint the control if so desired
	if( bInvalidate )
		Invalidate();
	
	return pNewNode;
}

void CStaticTreeCtrl::DeleteNode( HTREENODE pNode, BOOL bInvalidate /* = FALSE  */)
{
	ASSERT( pNode != NULL );	// Make sure the node exists

	// If this node is not the top node, fix pointers in sibling list

	EnterCriticalSection( &Lock );
	if( pNode->pPrev != NULL )
	{
		pNode->pPrev->pNext = pNode->pNext;
	}
	if( pNode->pNext != NULL )
	{
		pNode->pNext->pPrev = pNode->pPrev;
	}
	LeaveCriticalSection( &Lock ); 

	delete pNode;
	pNode = NULL;

	// Repaint the control if so desired
	if( bInvalidate )
		Invalidate();
}

void CStaticTreeCtrl::SetNodeColor( HTREENODE pNode, COLORREF crText, COLORREF crBk, COLORREF crBkHL, 
								   COLORREF crTextHL, BOOL bInvalidate /* = FALSE  */)
{
	ASSERT( pNode != NULL );

	pNode->bUseDefaultTextColor	= FALSE;
	pNode->crText				= crText;
	pNode->crBkHL				= crBkHL;
	pNode->crTextHL				= crTextHL;
	pNode->crBk					= crBk;

	if( bInvalidate )
		Invalidate();
}

void CStaticTreeCtrl::SetBackgroundBitmap( BOOL bInvalidate /* = FALSE  */)
{
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER | OFN_FILEMUSTEXIST, _T("Bitmap Files (*.bmp)|*.bmp||"), this );

	// If the user clicked 'ok'
	if( fd.DoModal() == IDOK )
	{
		// If there is a bitmap already loaded, delete it
	    if( m_bmpBackground.GetSafeHandle() != NULL )
			m_bmpBackground.DeleteObject();
		
		// Load the bitmap from the file selected
		HBITMAP hBitmap =	(HBITMAP)LoadImage( NULL, fd.GetPathName(), IMAGE_BITMAP, 
												0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE );

		// Attach it to the CBitmap object
		m_bmpBackground.Attach( hBitmap );

		// Repaint if so desired
		if( bInvalidate )
			Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
//	PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////

void CStaticTreeCtrl::DeleteAllNode()
{
	EnterCriticalSection( &Lock ); 
	DeleteLinkedNodes(m_pFirstNode);
	m_pLastNode = m_pFirstNode = 0;
	LeaveCriticalSection( &Lock ); 
	Invalidate();
}

void CStaticTreeCtrl::DeleteLinkedNodes( HTREENODE pNode )
{
	HTREENODE NextNode; 

	if( pNode == NULL )
		return;

	for( ; ; )
	{
		NextNode = pNode->pNext; 

		if( pNode->pPrev != NULL )
		{
			pNode->pPrev->pNext = pNode->pNext;
		}

		if( pNode->pNext != NULL )
		{
			pNode->pNext->pPrev = pNode->pPrev;
		}

		//CString DebugOut; 
		//DebugOut.Format( "CStaticTreeCtrl::DeleteLinkedNodes detete node 0x%0.8x \n",pNode ); 
		//OutputDebugString( DebugOut.GetBuffer() ); 

		delete pNode;

		ASSERT( NextNode != pNode ); 

		pNode = NextNode; 

		if( NextNode == NULL )
		{
			break; 
		}
	}
}

int CStaticTreeCtrl::HowMuchTextFits( CDC* pDC, int iAvailableWidth, CString csText )
{
	int iValidSoFar = csText.GetLength() - 1;					// Assume the entire text fits

	// If the text's pixel width is larger than what's available
	if( pDC->GetTextExtent( csText ).cx > iAvailableWidth )
	{
		int iNextBlank	= 0;	// Position of the next blank in text
		int iPixelWidth	= 0;	// Text's pixel width

		// Loop until we can fit no more of the text
		while( iPixelWidth < iAvailableWidth )
		{
			iValidSoFar	= iNextBlank;							// Record the char pos so far
			iNextBlank	= csText.Find( ' ', iNextBlank + 1 );	// Advance one word at a time

			// Have reached the end of the string?
			if( iNextBlank == -1 )
				iNextBlank = csText.GetLength();

			// Calculate the new width
			iPixelWidth = pDC->GetTextExtent( csText.Left( iNextBlank ) ).cx;
		}
	}

	return iValidSoFar;
}

void CStaticTreeCtrl::DrawLinesRecursive( CDC* pDC, HTREENODE pNode )
{
	// Where is the elbow joint of this connecting line?
	int iJointX	= pNode->rNode.left - m_iIndent - 6;
	int iJointY	= pNode->rNode.top + ( m_iLineHeight / 2 );

	// If the parent is not the top node, throw a connecting line to it
	if( pNode->pPrev != m_pFirstNode )
	{
		// How far up from the joint is the parent?
		int iDispY = iJointY - pNode->pPrev->rNode.top - ( m_iLineHeight / 2 );
		
		// Use 1 pixel wide rectangles to draw lines
		pDC->FillSolidRect( iJointX, iJointY, m_iIndent, 1, m_crConnectingLines );	// Horizontal line
		pDC->FillSolidRect( iJointX, iJointY, 1, -iDispY, m_crConnectingLines );		// Vertical line
	}

	// Put a solid dot to mark a node
	pDC->FillSolidRect( iJointX + m_iIndent - 2, iJointY - 2, 5, 5, m_crConnectingLines );

	// Hollow out the dot if the node has no childs
	if( pNode->pNext == NULL )
		pDC->FillSolidRect( iJointX + m_iIndent - 1, iJointY - 1, 3, 3, RGB(255,255,255) );

	// Draw the next sibling if there are any
	if( pNode->pNext != NULL )
		DrawLinesRecursive( pDC, pNode->pNext );
}

void CStaticTreeCtrl::ResetScrollBar()
{
	// Flag to avoid a call from OnSize while resetting the scrollbar
	m_bScrollBarMessage = TRUE;

	CRect rFrame;

	GetClientRect( rFrame );

	// Need for scrollbars?
	if( rFrame.Height() > m_iDocHeight + 8 )
	{
		ShowScrollBar( SB_VERT, FALSE );	// Hide it
		SetScrollPos( SB_VERT, 0 );
	}
	else
	{
		SCROLLINFO	si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = rFrame.Height();
		si.nMax = m_iDocHeight + 8;
		si.nMin = 0 ;

		SetScrollInfo( SB_VERT, &si );
		EnableScrollBarCtrl( SB_VERT, TRUE );
	}

	m_bScrollBarMessage = FALSE;
}

HTREENODE CStaticTreeCtrl::FindNodeByPoint( const CPoint& point, HTREENODE pNode )
{
	// Found it?
	if( pNode->rNode.PtInRect( point ) )
		return pNode;

	if( pNode->pNext != NULL )
		return FindNodeByPoint( point, pNode->pNext );
	return NULL;
}

int CStaticTreeCtrl::DrawNode( CDC* pDC, HTREENODE pNode, int x, int y, CRect rFrame )
{
	int		iDocHeight = 0;		// Total document height
	CRect	rNode;

	// The node's location and dimensions on screen
	rNode.left		= x;
	rNode.top		= y;
	rNode.right		= rFrame.right - m_iPadding;
	rNode.bottom	= y + m_iLineHeight;

	pNode->rNode.CopyRect( rNode );		// Record the rectangle

	CBrush NodeBkBrh;
	COLORREF cr;	
	if( !pNode->bUseDefaultTextColor )
	{
		if(pNode->IsHighLight)
		{
			NodeBkBrh.CreateSolidBrush(pNode->crBkHL);
			cr = pNode->crTextHL;
		}
		else
		{

			NodeBkBrh.CreateSolidBrush(pNode->crBk);
			cr = pNode->crText;
		}
		
	}
	else
	{
		if(pNode->IsHighLight)
		{
			NodeBkBrh.CreateSolidBrush(m_crDefaultBkHLColor);
			cr = m_crDefaultTextHLColor;
		}
		else
		{

			NodeBkBrh.CreateSolidBrush(m_crDefaultBkColor);
			cr = m_crDefaultTextColor;
		}
	}
	pDC->FillRect(rNode, &NodeBkBrh);
	NodeBkBrh.DeleteObject();

	COLORREF crOldText	= pDC->SetTextColor( cr );

	// MULTILINE TEXT - begins
	CString	cs		= pNode->csLabel;
	CString tmp; 
	int		iPos	= 0;

	// Draw text until there is nothing left to draw
	while( cs.GetLength() > 0 )
	{
		// Height of a line of text
		rNode.bottom = rNode.top + m_iLineHeight;

		// Find out how much text fits in one line
		iPos = HowMuchTextFits( pDC, rFrame.right - m_iPadding - rNode.left, cs );

		// Draw only if the node is visible
		if( rNode.bottom > 0 && rNode.top < rFrame.bottom )
			pDC->DrawText( cs.Left( iPos + 1 ), rNode, DT_LEFT | DT_SINGLELINE | DT_VCENTER );

		// Eliminate the text that has been already drawn
		tmp = cs.Mid( iPos + 1 );
		cs = tmp; 

		// The node grows everytime another line of text is drawn
		pNode->rNode.UnionRect( pNode->rNode, rNode );

		// Move down the drawing rectangle for the next line of text
		rNode.top = rNode.bottom;
	}
	// MULTILINE TEXT - ends

	pDC->SetTextColor( crOldText );

	return pNode->rNode.Height();
}

/////////////////////////////////////////////////////////////////////////////
// CStaticTreeCtrl message handlers

void CStaticTreeCtrl::OnPaint() 
{
	CPaintDC dc(this); // Device context for painting
	
	// Double-buffering
	CDC*		pDCMem		= new CDC;
	CBitmap*	pOldBitmap	= NULL;
	CBitmap		bmpCanvas;
	CRect		rFrame;

	GetClientRect( rFrame );

	pDCMem->CreateCompatibleDC( &dc );

	bmpCanvas.CreateCompatibleBitmap( &dc, rFrame.Width(), rFrame.Height() );

	pOldBitmap = pDCMem->SelectObject( &bmpCanvas );

	// START DRAW -------------------------------------------------

	// If there is a bitmap loaded, use it
	// Otherwise, paint the background white
    if( m_bmpBackground.GetSafeHandle() != NULL )
    {
		CDC*	pDCTemp = new CDC;;
		BITMAP	bmp;

		pDCTemp->CreateCompatibleDC( &dc );

		m_bmpBackground.GetBitmap( &bmp );

		// Select the bitmap into the temp device context
		CBitmap* pOldBitmap = (CBitmap*) pDCTemp->SelectObject( &m_bmpBackground );

		// Stretch the bitmap to fill the entire control area
		pDCMem->StretchBlt( 0, 0, rFrame.Width(), rFrame.Height(), pDCTemp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY );

		pDCTemp->SelectObject( pOldBitmap ); 
		
		delete pDCTemp;
	}
	else
		pDCMem->FillSolidRect( rFrame, RGB(255,255,255) );

	UINT	nMode		= pDCMem->SetBkMode( TRANSPARENT );
	CFont*	pOldFont	= pDCMem->SelectObject( &m_Font );
	HTREENODE pNode; 

	int iLastNodePos = 0;

	EnterCriticalSection( &Lock ); 
	pNode = m_pFirstNode; 

	for( ; ; )
	{
		//CString DebugOut; 
		//DebugOut.Format( "CStaticTreeCtrl::OnPaint Draw node 0x%0.8x \n",pNode ); 
		//OutputDebugString( DebugOut.GetBuffer() ); 

		if( pNode == NULL )
		{
			break; 
		}

		iLastNodePos = iLastNodePos + DrawNode(	pDCMem, pNode,
			rFrame.left + m_iIndent,
			m_iPadding - GetScrollPos( SB_VERT ) + iLastNodePos,
			rFrame );

		pNode = pNode->pNext; 


		//if( m_bShowLines )
		//DrawLinesRecursive( pDCMem, m_pFirstNode->pNext );
	}

	LeaveCriticalSection( &Lock ); 

	pDCMem->SelectObject( pOldFont );
	pDCMem->SetBkMode( nMode );

	pDCMem->Draw3dRect( rFrame, RGB(0,0,0), RGB(0,0,0) );	// Border
	
	// END DRAW   -------------------------------------------------

	dc.BitBlt( 0, 0, rFrame.Width(), rFrame.Height(), pDCMem, 0, 0, SRCCOPY );

	pDCMem->SelectObject( pOldBitmap );

	delete pDCMem;

	// Has the total document height changed?
	if( iLastNodePos != m_iDocHeight )
	{
		BOOL bInvalidate = ( ( m_iDocHeight < rFrame.Height() ) != ( iLastNodePos < rFrame.Height() ) );
		
		m_iDocHeight = iLastNodePos;

		ResetScrollBar();

		// If the scrollbar has just been hidden/shown, repaint
		if( bInvalidate )
			Invalidate();
	}
}

void CStaticTreeCtrl::OnSize(UINT nType, int cx, int cy) 
{
	// Setting the scroll sends its own size message. Prevent it thus avoiding an ugly loop.
	// Other than that, resizing the control means that the tree height may change (word-wrap).
	if( !m_bScrollBarMessage )
		ResetScrollBar();
	
	CStatic::OnSize(nType, cx, cy);
}

void CStaticTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int iScrollBarPos = GetScrollPos( SB_VERT );

	CRect rFrame;

	GetClientRect( rFrame );

	switch( nSBCode )
	{
		case SB_LINEUP:
			iScrollBarPos = max( iScrollBarPos - m_iLineHeight, 0 );
		break;

		case SB_LINEDOWN:
			iScrollBarPos = min( iScrollBarPos + m_iLineHeight, GetScrollLimit( SB_VERT ) );
		break;

		case SB_PAGEUP:
			iScrollBarPos = max( iScrollBarPos - rFrame.Height(), 0 );
		break;

		case SB_PAGEDOWN:
			iScrollBarPos = min( iScrollBarPos + rFrame.Height(), GetScrollLimit( SB_VERT ) );
		break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
		{
			SCROLLINFO si;

			ZeroMemory( &si, sizeof(SCROLLINFO) );

			si.cbSize	= sizeof(SCROLLINFO);
			si.fMask	= SIF_TRACKPOS;

			if( GetScrollInfo( SB_VERT, &si, SIF_TRACKPOS ) )
				iScrollBarPos = si.nTrackPos;
			else
				iScrollBarPos = (UINT)nPos;
			break;
		}
	}		

	SetScrollPos( SB_VERT, iScrollBarPos );
	
	Invalidate();
	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CStaticTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if( message == WM_NCHITTEST || message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK )
		return ::DefWindowProc( m_hWnd, message, wParam, lParam );
	
	return CStatic::WindowProc(message, wParam, lParam);
}


void CStaticTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	HTREENODE pClickedOn = NULL;		// Assume no node was clicked on
	static HTREENODE pPrevClicked = NULL;

	EnterCriticalSection( &Lock ); 
	if( m_pFirstNode != NULL)		// If the tree is populated, search it
		pClickedOn = FindNodeByPoint( point, m_pFirstNode );
	LeaveCriticalSection( &Lock ); 

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	GetScrollInfo( SB_VERT, &si, SIF_TRACKPOS );
	CRect RedrawRc;

	if(pPrevClicked && pPrevClicked != pClickedOn)
	{
		pPrevClicked->IsHighLight = FALSE;
		RedrawRc = pPrevClicked->rNode;
		RedrawRc.top -= si.nPos;
		InvalidateRect(RedrawRc,0);
	}

	if(pClickedOn)
	{
		pClickedOn->IsHighLight = TRUE;
		RedrawRc = pClickedOn->rNode;
		RedrawRc.top -= si.nPos;
		InvalidateRect(RedrawRc,0);
		GetParent()->SendMessage(WM_GETPACKDETAIL, 0, pClickedOn->m_uNo);
		pPrevClicked = pClickedOn;
	}
	CStatic::OnLButtonUp(nFlags, point);
}

BOOL CStaticTreeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// zDelta greater than 0, means rotating away from the user, that is, scrolling up
	OnVScroll( ( zDelta > 0 )? SB_LINEUP:SB_LINEDOWN, 0, NULL );

	return CStatic::OnMouseWheel(nFlags, zDelta, pt);
}

void CStaticTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{

}

void CStaticTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ

	CStatic::OnMouseMove(nFlags, point);
}
