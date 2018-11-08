#ifndef __UICONTAINER_H__
#define __UICONTAINER_H__

#pragma once

#define CALC_SCROLL_RANGE( _size_, _page_size_ ) ( ( _size_ ) - ( _page_size_ ) )

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

/////////////////////////////////////////////////////////////////////////////////////
//
class CScrollBarUI;

class UILIB_API CBaseContainerUI : public CControlUI
{
public:
    CBaseContainerUI();
    virtual ~CBaseContainerUI();

public:
    void DoEvent(TEventUI& event);
    //void SetVisible(bool bVisible = true);
    //void SetInternVisible(bool bVisible = true);
    void SetMouseEnabled(bool bEnable = true);

    virtual RECT GetInset() const;
    virtual void SetInset(RECT rcInset); // 设置内边距，相当于设置客户区
    virtual int GetChildPadding() const;
    virtual void SetChildPadding(int iPadding);

    virtual bool IsMouseChildEnabled() const;
    virtual void SetMouseChildEnabled(bool bEnable = true);

    //void SetPos(RECT rc);
    //void DoPaint(HDC hDC, const RECT& rcPaint);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	LRESULT SetRelativeBase( SIZE &Size ); 
	SIZE GetRelativeBase() const; 

	//void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
    CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

    virtual SIZE GetScrollPos() const;
    virtual SIZE GetScrollRange() const;
    virtual void SetScrollPos(SIZE szPos);
    virtual void LineUp();
    virtual void LineDown();
    virtual void PageUp();
    virtual void PageDown();
    virtual void HomeUp();
    virtual void EndDown();
#define LINE_CX_FIXED 8
    virtual void LineLeft();
    virtual void LineRight();
    virtual void PageLeft();
    virtual void PageRight();
    virtual void HomeLeft();
    virtual void EndRight();
    virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
    virtual CScrollBarUI* GetVerticalScrollBar() const;
    virtual CScrollBarUI* GetHorizontalScrollBar() const;

protected:
    //virtual void SetFloatPos(int iIndex);

    virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);

protected:
    RECT m_rcInset;
    int m_iChildPadding;
    bool m_bMouseChildEnabled;
    bool m_bScrollProcess; // 防止SetPos循环调用
	SIZE RelativeBase; 

    CScrollBarUI* m_pVerticalScrollBar;
    CScrollBarUI* m_pHorizontalScrollBar;
};

class UILIB_API CContainerUI : public CBaseContainerUI, public IItemManager
{
public:
	CContainerUI();
	virtual ~CContainerUI();

	LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
	LPVOID GetInterface(LPCTSTR pstrName);
	void SetVisible(bool bVisible = true);
	void SetInternVisible(bool bVisible = true);

	CControlUI* GetItemAt(int iIndex) const; 
	INT32 GetItemIndex(CControlUI* pControl) const;
	bool SetItemIndex(CControlUI* pControl, int iIndex);
	INT32 GetItemCount() const; 

	bool AddItem(CControlUI* pControl);
	bool AddItemAt(CControlUI* pControl, int iIndex);
	bool RemoveItem(CControlUI* pControl);
	bool RemoveItemAt(int iIndex);
	void RemoveAllItem(); 

	bool IsAutoDestroy() const; 
	void SetAutoDestroy(bool bAuto); 
	bool IsDelayedDestroy() const; 
	void SetDelayedDestroy(bool bDelayed); 

	void SetPos(RECT rc);
	LRESULT DumpSubItemPos( LPCTSTR Name ); 

	virtual void SetScrollPos(SIZE szPos);
	virtual int FindSelectable(int iIndex, bool bForward = true) const;
	void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

protected:
	virtual void SetFloatPos(int iIndex);

public: 

	CStdPtrArray m_items;
	bool m_bAutoDestroy;
	bool m_bDelayedDestroy;

}; 

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CVerticalLayoutUI : public CContainerUI
{
public:
    CVerticalLayoutUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    void SetSepHeight(int iHeight);
    int GetSepHeight() const;
    void SetSepImmMode(bool bImmediately);
    bool IsSepImmMode() const;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void DoEvent(TEventUI& event);

    void SetPos(RECT rc);
    void DoPostPaint(HDC hDC, const RECT& rcPaint);

    RECT GetThumbRect(bool bUseNew = false) const;

protected:
    int m_iSepHeight;
    UINT m_uButtonState;
    POINT ptLastMouse;
    RECT m_rcNewPos;
    bool m_bImmMode;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CHorizontalLayoutUI : public CContainerUI
{
public:
    CHorizontalLayoutUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;
    
    void SetSepWidth(int iWidth);
	int GetSepWidth() const;
    void SetSepImmMode(bool bImmediately);
	bool IsSepImmMode() const;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void DoEvent(TEventUI& event);

    void SetPos(RECT rc);
    void DoPostPaint(HDC hDC, const RECT& rcPaint);

    RECT GetThumbRect(bool bUseNew = false) const;

protected:
    int m_iSepWidth;
    UINT m_uButtonState;
    POINT ptLastMouse;
    RECT m_rcNewPos;
    bool m_bImmMode;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CTileLayoutUI : public CContainerUI
{
public:
    CTileLayoutUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetPos(RECT rc);

    int GetColumns() const;
    void SetColumns(int nCols);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
    int m_nColumns;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CDialogLayoutUI : public CContainerUI
{
public:
    CDialogLayoutUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetStretchMode(CControlUI* pControl, UINT uMode);

    void SetPos(RECT rc);
    SIZE EstimateSize(SIZE szAvailable);
    RECT RecalcArea();   

protected:  
    typedef struct 
    {
        CControlUI* pControl;
        UINT uMode;
        RECT rcItem;
    } STRETCHMODE;

    CStdValArray m_aModes;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CTabLayoutUI : public CContainerUI
{
public:
    CTabLayoutUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);

    bool AddItem(CControlUI* pControl);
    bool AddItemAt(CControlUI* pControl, int iIndex);
    bool RemoveItem(CControlUI* pControl);
    void RemoveAllItem();
    int GetCurSel() const;
    bool SelectItem(int iIndex);

    void SetPos(RECT rc);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
    int m_iCurSel;
};

} // namespace DuiLib

#endif // __UICONTAINER_H__
