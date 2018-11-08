#ifndef __UICOMMONCONTROLS_H__
#define __UICOMMONCONTROLS_H__

#pragma once

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CLabelUI : public CControlUI
{
public:
    CLabelUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetTextStyle(UINT uStyle);
	UINT GetTextStyle() const;
    void SetTextColor(DWORD dwTextColor);
	DWORD GetTextColor() const;
    void SetDisabledTextColor(DWORD dwTextColor);
	DWORD GetDisabledTextColor() const;
    void SetFont(int index);
	int GetFont() const;
    RECT GetTextPadding() const;
    void SetTextPadding(RECT rc);
    bool IsShowHtml();
    void SetShowHtml(bool bShowHtml = true);

    SIZE EstimateSize(SIZE szAvailable);
    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void PaintText(HDC hDC);

protected:
    DWORD m_dwTextColor;
    DWORD m_dwDisabledTextColor;
    int m_iFont;
    UINT m_uTextStyle;
    RECT m_rcTextPadding;
    bool m_bShowHtml;
};

/////////////////////////////////////////////////////////////////////////////////////
//

typedef struct _BUTTON_UI_INFO
{
	DWORD m_dwFocusedBkColor; 
	DWORD m_dwPushedBkColor; 
	DWORD m_dwHotBkColor; 
	DWORD m_dwDisabledBkColor; 

	DWORD m_dwBkColorFade; 

	CStdString m_sNormalImage;
	CStdString m_sHotImage;
	CStdString m_sPushedImage;
	CStdString m_sFocusedImage;
	CStdString m_sDisabledImage;
} BUTTON_UI_INFO, *PBUTTON_UI_INFO; 

class UILIB_API CButtonUI : public CLabelUI
{
public:
    CButtonUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;

    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    bool Activate();
    void SetEnabled(bool bEnable = true);
    void DoEvent(TEventUI& event);

    LPCTSTR GetNormalImage();
    void SetNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetHotImage();
    void SetHotImage(LPCTSTR pStrImage);
    LPCTSTR GetPushedImage();
    void SetPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetFocusedImage();
    void SetFocusedImage(LPCTSTR pStrImage);
    LPCTSTR GetDisabledImage();
    void SetDisabledImage(LPCTSTR pStrImage);
	DWORD GetFocusedBkColor(); 
	DWORD GetNormalBkColor(); 
	DWORD GetPushedBkColor(); 
	DWORD GetHotBkColor(); 
	DWORD GetBkFade(); 
	DWORD GetUIStyle(); 
	void SetUIStyle( DWORD ui_style ); 
	//DWORD SetNormalBkColor( DWORD nor_clr ); 
	void SetPushedBkColor( DWORD push_clr ); 
	void SetHotBkColor( DWORD hot_clr ); 
	void SetBkFade( DWORD bk_fade ); 
	void SetFocusedBkColor( DWORD focusd_clr ); 
	void SetDisabledBkColor( DWORD disabled_clr ); 

	void SetHotTextColor(DWORD dwColor);
	DWORD GetHotTextColor() const;
    void SetPushedTextColor(DWORD dwColor);
	DWORD GetPushedTextColor() const;
    void SetFocusedTextColor(DWORD dwColor);
	DWORD GetFocusedTextColor() const;
    SIZE EstimateSize(SIZE szAvailable);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void PaintText(HDC hDC);
    void PaintStatusImage(HDC hDC);
	//void OnPainted( HDC hDC ); 
	void _PaintStatusImage(HDC hDC);

#define is_valid_bk_color( clr ) ( ( clr & 0xff000000 ) == 0x0f000000 )

	VOID PaintStatusHolRound( HDC hDC ); 
	VOID PaintStatusBkColor( HDC hDC ); 

protected:
    UINT m_uButtonState;

	DWORD m_dwHotTextColor;
	DWORD m_dwPushedTextColor;
	DWORD m_dwFocusedTextColor;

	DWORD m_dwUIStyle; 

	BUTTON_UI_INFO UIInfo; 
};


typedef struct _BUTTON_UI_EX_INFO
{
	DWORD m_dwFocusedBkColor; 
	DWORD m_dwPushedBkColor; 
	DWORD m_dwHotBkColor; 
	DWORD m_dwDisabledBkColor; 

	DWORD m_dwBkColorFade; 

	HANDLE m_hNormalImage;
	HANDLE m_hHotImage;
	HANDLE m_hPushedImage;
	HANDLE m_hFocusedImage;
	HANDLE m_hDisabledImage;
} BUTTONEX_UI_INFO, *PBUTTONEX_UI_INFO; 

class UILIB_API CButtonExUI : public CLabelUI
{
public:
	CButtonExUI();

	LPCTSTR GetClass() const;
	ULONG GetClassId() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;

	bool Activate();
	void SetEnabled(bool bEnable = true);
	void DoEvent(TEventUI& event);

	HANDLE GetNormalImage();
	void SetNormalImage(HANDLE Image);
	HANDLE GetHotImage();
	void SetHotImage(HANDLE Image);
	HANDLE GetPushedImage();
	void SetPushedImage(HANDLE Image);
	HANDLE GetFocusedImage();
	void SetFocusedImage(HANDLE Image);
	HANDLE GetDisabledImage();
	void SetDisabledImage(HANDLE Image);
	DWORD GetFocusedBkColor(); 
	DWORD GetNormalBkColor(); 
	DWORD GetPushedBkColor(); 
	DWORD GetHotBkColor(); 
	DWORD GetBkFade(); 
	DWORD GetUIStyle(); 
	void SetUIStyle( DWORD ui_style ); 
	//DWORD SetNormalBkColor( DWORD nor_clr ); 
	void SetPushedBkColor( DWORD push_clr ); 
	void SetHotBkColor( DWORD hot_clr ); 
	void SetBkFade( DWORD bk_fade ); 
	void SetFocusedBkColor( DWORD focusd_clr ); 
	void SetDisabledBkColor( DWORD disabled_clr ); 

	void SetHotTextColor(DWORD dwColor);
	DWORD GetHotTextColor() const;
	void SetPushedTextColor(DWORD dwColor);
	DWORD GetPushedTextColor() const;
	void SetFocusedTextColor(DWORD dwColor);
	DWORD GetFocusedTextColor() const;
	SIZE EstimateSize(SIZE szAvailable);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void PaintText(HDC hDC);
	void PaintStatusImage(HDC hDC);
	//void OnPainted( HDC hDC ); 
	void _PaintStatusImage(HDC hDC);

#define is_valid_bk_color( clr ) ( ( clr & 0xff000000 ) == 0x0f000000 )

	VOID PaintStatusHolRound( HDC hDC ); 
	VOID PaintStatusBkColor( HDC hDC ); 

protected:
	UINT m_uButtonState;

	DWORD m_dwHotTextColor;
	DWORD m_dwPushedTextColor;
	DWORD m_dwFocusedTextColor;

	DWORD m_dwUIStyle; 

	BUTTONEX_UI_INFO UIInfo; 
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API COptionUI : public CButtonUI
{
public:
    COptionUI();
    ~COptionUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

    bool Activate();
    void SetEnabled(bool bEnable = true);

    LPCTSTR GetSelectedImage();
	LPCTSTR GetSelectedHotImage();
    void SetSelectedImage(LPCTSTR pStrImage);
	void SetSelectedHotImage( LPCTSTR pStrImage ); 
	
	void SetSelectedTextColor(DWORD dwTextColor);
	DWORD GetSelectedTextColor();
		
	LPCTSTR GetForeImage();
	void SetForeImage(LPCTSTR pStrImage);

    LPCTSTR GetGroup() const;
    void SetGroup(LPCTSTR pStrGroupName = NULL);
    bool IsSelected() const;
    void Selected(bool bSelected);

    SIZE EstimateSize(SIZE szAvailable);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void PaintStatusImage(HDC hDC);
	void PaintText(HDC hDC);

protected:
    bool m_bSelected;
    CStdString m_sGroupName;

	DWORD m_dwSelectedTextColor;

    CStdString m_sSelectedImage;
	CStdString m_sSelectedHotImage; 
	CStdString m_sForeImage;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CTextUI : public CLabelUI
{
public:
    CTextUI();
    ~CTextUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    CStdString* GetLinkContent(int iIndex);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);

    void PaintText(HDC hDC);

protected:
    enum { MAX_LINK = 8 };
    int m_nLinks;
    RECT m_rcLinks[MAX_LINK];
    CStdString m_sLinks[MAX_LINK];
    int m_nHoverLink;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CProgressUI : public CLabelUI
{
public:
    CProgressUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 

    LPVOID GetInterface(LPCTSTR pstrName);

    bool IsHorizontal();
    void SetHorizontal(bool bHorizontal = true);
    int GetMinValue() const;
    void SetMinValue(int nMin);
    int GetMaxValue() const;
    void SetMaxValue(int nMax);
    int GetValue() const;
    void SetValue(int nValue);
    LPCTSTR GetFgImage() const;
    void SetFgImage(LPCTSTR pStrImage);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void PaintStatusImage(HDC hDC);

protected:
    bool m_bHorizontal;
    int m_nMax;
    int m_nMin;
    int m_nValue;

    CStdString m_sFgImage;
    CStdString m_sFgImageModify;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CSliderUI : public CProgressUI
{
public:
    CSliderUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
	UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetEnabled(bool bEnable = true);

    int GetChangeStep();
    void SetChangeStep(int step);
    void SetThumbSize(SIZE szXY);
    RECT GetThumbRect() const;
    LPCTSTR GetThumbImage() const;
    void SetThumbImage(LPCTSTR pStrImage);
    LPCTSTR GetThumbHotImage() const;
    void SetThumbHotImage(LPCTSTR pStrImage);
    LPCTSTR GetThumbPushedImage() const;
    void SetThumbPushedImage(LPCTSTR pStrImage);

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void PaintStatusImage(HDC hDC);

protected:
    SIZE m_szThumb;
    UINT m_uButtonState;
    int m_nStep;

    CStdString m_sThumbImage;
    CStdString m_sThumbHotImage;
    CStdString m_sThumbPushedImage;

    CStdString m_sImageModify;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CEditWnd;

class UILIB_API CEditUI : public CLabelUI
{
    friend CEditWnd;
public:
    CEditUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    void SetEnabled(bool bEnable = true);
    void SetText(LPCTSTR pstrText);
    void SetMaxChar(UINT uMax);
    UINT GetMaxChar();
    void SetReadOnly(bool bReadOnly);
    bool IsReadOnly() const;
    void SetPasswordMode(bool bPasswordMode);
    bool IsPasswordMode() const;
    void SetPasswordChar(TCHAR cPasswordChar);
    TCHAR GetPasswordChar() const;

	void SetMultiline( bool is_multiline )
	{
		m_multiline = is_multiline; 
	}

	bool IsMultiline()
	{
		return m_multiline; 
	}

	LPCTSTR GetNormalImage();
    void SetNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetHotImage();
    void SetHotImage(LPCTSTR pStrImage);
    LPCTSTR GetFocusedImage();
    void SetFocusedImage(LPCTSTR pStrImage);
    LPCTSTR GetDisabledImage();
    void SetDisabledImage(LPCTSTR pStrImage);
    void SetNativeEditBkColor(DWORD dwBkColor);
    DWORD GetNativeEditBkColor() const;

    void SetPos(RECT rc);
    void SetVisible(bool bVisible = true);
    void SetInternVisible(bool bVisible = true);
    SIZE EstimateSize(SIZE szAvailable);
    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void PaintStatusImage(HDC hDC);
    void PaintText(HDC hDC);

protected:
    CEditWnd* m_pWindow;

    UINT m_uMaxChar;
    bool m_bReadOnly;
    bool m_bPasswordMode;
	bool m_multiline; 
    TCHAR m_cPasswordChar;
    UINT m_uButtonState;
    CStdString m_sNormalImage;
    CStdString m_sHotImage;
    CStdString m_sFocusedImage;
    CStdString m_sDisabledImage;
    DWORD m_dwEditbkColor;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CScrollBarUI : public CControlUI
{
public:
    CScrollBarUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);

    CBaseContainerUI* GetOwner() const;
    void SetOwner(CBaseContainerUI* pOwner);

    void SetEnabled(bool bEnable = true);
    void SetFocus();

    bool IsHorizontal();
    void SetHorizontal(bool bHorizontal = true);
    int GetScrollRange() const;
    void SetScrollRange(int nRange);
    int GetScrollPos() const;
    void SetScrollPos(int nPos);
    int GetLineSize() const;
    void SetLineSize(int nSize);

    bool GetShowButton1();
    void SetShowButton1(bool bShow);
    LPCTSTR GetButton1NormalImage();
    void SetButton1NormalImage(LPCTSTR pStrImage);
    LPCTSTR GetButton1HotImage();
    void SetButton1HotImage(LPCTSTR pStrImage);
    LPCTSTR GetButton1PushedImage();
    void SetButton1PushedImage(LPCTSTR pStrImage);
    LPCTSTR GetButton1DisabledImage();
    void SetButton1DisabledImage(LPCTSTR pStrImage);

    bool GetShowButton2();
    void SetShowButton2(bool bShow);
    LPCTSTR GetButton2NormalImage();
    void SetButton2NormalImage(LPCTSTR pStrImage);
    LPCTSTR GetButton2HotImage();
    void SetButton2HotImage(LPCTSTR pStrImage);
    LPCTSTR GetButton2PushedImage();
    void SetButton2PushedImage(LPCTSTR pStrImage);
    LPCTSTR GetButton2DisabledImage();
    void SetButton2DisabledImage(LPCTSTR pStrImage);

    LPCTSTR GetThumbNormalImage();
    void SetThumbNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetThumbHotImage();
    void SetThumbHotImage(LPCTSTR pStrImage);
    LPCTSTR GetThumbPushedImage();
    void SetThumbPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetThumbDisabledImage();
    void SetThumbDisabledImage(LPCTSTR pStrImage);

    LPCTSTR GetRailNormalImage();
    void SetRailNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetRailHotImage();
    void SetRailHotImage(LPCTSTR pStrImage);
    LPCTSTR GetRailPushedImage();
    void SetRailPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetRailDisabledImage();
    void SetRailDisabledImage(LPCTSTR pStrImage);

    LPCTSTR GetBkNormalImage();
    void SetBkNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetBkHotImage();
    void SetBkHotImage(LPCTSTR pStrImage);
    LPCTSTR GetBkPushedImage();
    void SetBkPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetBkDisabledImage();
    void SetBkDisabledImage(LPCTSTR pStrImage);

    void SetPos(RECT rc);
    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void DoPaint(HDC hDC, const RECT& rcPaint);

    void PaintBk(HDC hDC);
    void PaintButton1(HDC hDC);
    void PaintButton2(HDC hDC);
    void PaintThumb(HDC hDC);
    void PaintRail(HDC hDC);

protected:

    enum { 
        DEFAULT_SCROLLBAR_SIZE = 16,
        DEFAULT_TIMERID = 10,
    };

    bool m_bHorizontal;
    LONGLONG m_nRange;
    LONGLONG m_nScrollPos;
    LONGLONG m_nLineSize;
    CBaseContainerUI* m_pOwner;
    POINT ptLastMouse;
    LONGLONG m_nLastScrollPos;
    LONGLONG m_nLastScrollOffset;
    int m_nScrollRepeatDelay;

    CStdString m_sBkNormalImage;
    CStdString m_sBkHotImage;
    CStdString m_sBkPushedImage;
    CStdString m_sBkDisabledImage;

    bool m_bShowButton1;
    RECT m_rcButton1;
    UINT m_uButton1State;
    CStdString m_sButton1NormalImage;
    CStdString m_sButton1HotImage;
    CStdString m_sButton1PushedImage;
    CStdString m_sButton1DisabledImage;

    bool m_bShowButton2;
    RECT m_rcButton2;
    UINT m_uButton2State;
    CStdString m_sButton2NormalImage;
    CStdString m_sButton2HotImage;
    CStdString m_sButton2PushedImage;
    CStdString m_sButton2DisabledImage;

    RECT m_rcThumb;
    UINT m_uThumbState;
    CStdString m_sThumbNormalImage;
    CStdString m_sThumbHotImage;
    CStdString m_sThumbPushedImage;
    CStdString m_sThumbDisabledImage;

    CStdString m_sRailNormalImage;
    CStdString m_sRailHotImage;
    CStdString m_sRailPushedImage;
    CStdString m_sRailDisabledImage;

    CStdString m_sImageModify;
};

} // namespace DuiLib

#endif // __UICOMMONCONTROLS_H__

