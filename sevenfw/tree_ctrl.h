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

#ifndef __STATIC_TREE_CTRL_H__
#define __STATIC_TREE_CTRL_H__

// StaticTreeCtrl.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#define		CM_INSERTCHILD				WM_APP + 10000
#define		CM_INSERTSIBLING			WM_APP + 10001
#define		CM_DELETENODE				WM_APP + 10002
#define		CM_MODIFYNODETEXT			WM_APP + 10003
#define		CM_CHANGENODECOLOR			WM_APP + 10004
#define		CM_TOGGLECONNECTINGLINES	WM_APP + 10010
#define		CM_SETCONNECTINGLINESCOLOR	WM_APP + 10011
#define		CM_SETFONT					WM_APP + 10020
#define		CM_SETDEFAULTCOLOR			WM_APP + 10021
#define		CM_SETBACKGROUNDBITMAP		WM_APP + 10022
#define		CM_TOGGLEMENUSOUND			WM_APP + 10030

#if defined( _UNICODE )
	#define STRCPY(x,y)				wcscpy(x,y)
#else
	#define STRCPY(x,y)				strcpy(x,y)
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeNode class

class CTreeNode
{
public:
	CTreeNode() : csLabel( "" )
	{
		rNode.SetRectEmpty();

		bUseDefaultTextColor = TRUE;
		m_uNo			= 1;
		pPrev		= NULL;
		pNext		= NULL;
		IsHighLight = FALSE;
	}

	virtual ~CTreeNode()
	{
		csLabel.Empty();
	}

	CString		csLabel;
	CRect		rNode;
	UINT		m_uNo;
	BOOL		IsHighLight;
	
	COLORREF	crText;
	COLORREF	crBk;
	COLORREF	crBkHL;
	COLORREF	crTextHL;
	BOOL		bUseDefaultTextColor;

	CTreeNode*	pNext;;
	CTreeNode*	pPrev;
};

#define    HTREENODE   CTreeNode*
#define    HTOPNODE    ( (HTREENODE) -0x10000 )

/////////////////////////////////////////////////////////////////////////////
// CStaticTreeCtrl window
#define WM_GETPACKDETAIL (WM_USER + 0x561)
class CStaticTreeCtrl : public CStatic
{
// Construction
public:
	CStaticTreeCtrl();

// Attributes
protected:
	LOGFONT			m_lgFont;
	CFont			m_Font;
	COLORREF		m_crDefaultTextColor;
	COLORREF		m_crDefaultBkColor;
	COLORREF		m_crDefaultBkHLColor;
	COLORREF		m_crDefaultTextHLColor;
	COLORREF		m_crConnectingLines;

	BOOL			m_bShowLines;

	CBitmap			m_bmpBackground;
	
	int				m_iDocHeight;
	BOOL			m_bScrollBarMessage;
	
	int				m_iLineHeight;
	int				m_iIndent;
	int				m_iPadding;


	HTREENODE		m_pSelected;

	CRITICAL_SECTION Lock;

// Operations
public:
	HTREENODE		m_pFirstNode;
	HTREENODE		m_pLastNode;
	virtual CStaticTreeCtrl&	SetTextFont			( LONG nHeight, BOOL bBold, BOOL bItalic, const CString& csFaceName );
	virtual CStaticTreeCtrl&	SetDefaultTextColor( COLORREF crText, COLORREF crTextHL, COLORREF crBk, COLORREF crBkHL );

	HTREENODE InsertChild( HTREENODE pPrev, const CString csLabel,
										COLORREF crText  = 0, COLORREF crBk = 0, 
										COLORREF crTextHL = 0,			COLORREF crBkHL = 0,
										BOOL bUseDefaultTextColor = TRUE,
										BOOL bInvalidate = FALSE);
	void			DeleteNode		( HTREENODE pNode, BOOL bInvalidate = FALSE );
	void			SetNodeColor( HTREENODE pNode, COLORREF crText, COLORREF crBk, COLORREF crBkHL, 
								   COLORREF crTextHL, BOOL bInvalidate = FALSE);

	void			SetBackgroundBitmap	( BOOL bInvalidate = FALSE );
	void			DeleteLinkedNodes( HTREENODE pNode );		// Recursive delete
	void			DeleteAllNode();

protected:
	

	int				DrawNode( CDC* pDC, HTREENODE pNode, int x, int y, CRect rFrame );
	int				HowMuchTextFits		( CDC* pDC, int iAvailableWidth, CString csText );

	void			DrawLinesRecursive	( CDC* pDC, HTREENODE pNode );

	void			ResetScrollBar		();

	HTREENODE		FindNodeByPoint		( const CPoint& point, HTREENODE pNode );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticTreeCtrl)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticTreeCtrl)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //__STATIC_TREE_CTRL_H__
