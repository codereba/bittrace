#ifndef __UILIST_H__
#define __UILIST_H__

#pragma once

/**********************************************************************
how can achieve the best performance?
if you add one instruction to the original function to running,that will
slower.
how can implement the diverse function and best performance.
this will not add any instruction.so the tech is template or macro.
**********************************************************************/

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class CListHeaderUI;
class CListBodyUI; 
class CListBodyExUI; 

#define UILIST_MAX_COLUMNS 32

typedef struct tagTListInfoUI
{
    int nColumns;
    RECT rcColumn[UILIST_MAX_COLUMNS];
    int nFont;
    UINT uTextStyle;
    RECT rcTextPadding;
    DWORD dwTextColor;
    DWORD dwBkColor;
    CStdString sBkImage;
    bool bAlternateBk;
    DWORD dwSelectedTextColor;
    DWORD dwSelectedBkColor;
    CStdString sSelectedImage;
    DWORD dwHotTextColor;
    DWORD dwHotBkColor;
    CStdString sHotImage;
    DWORD dwDisabledTextColor;
    DWORD dwDisabledBkColor;
    CStdString sDisabledImage;
    DWORD dwLineColor;
    bool bShowHtml;
    bool bMultiExpandable;
} TListInfoUI;


/////////////////////////////////////////////////////////////////////////////////////
//

class IListCallbackUI
{
public:
    virtual LPCTSTR GetItemText(CControlUI* pList, int iItem, int iSubItem) = 0;
};

//class IListContainerCallback
//{
//public:
//	LRESULT LoadSubItemsContent( ULONG ItemBeginIndex, ULONG ItemCount ); 
//};

class IListHeaderCallbackUI
{
public:
	virtual LRESULT NotifyHeaderDragged(CControlUI* pList ) = 0;
};

class IListOwnerUI
{
public:
    virtual TListInfoUI* GetListInfo() = 0;
    virtual int GetCurSel() const = 0;
    virtual bool SelectListItem(int iIndex) = 0;
    virtual void DoEvent(TEventUI& event) = 0;
	virtual BOOLEAN ColumnWidthChanged()
	{
		return FALSE; 
	}
};

//class CCachedContainerUI;

class IListUI : public IListOwnerUI
{
public:
    virtual CListHeaderUI* GetHeader() const = 0;
    virtual IListCallbackUI* GetListCallback() const = 0;
    virtual void SetListCallback(IListCallbackUI* pCallback) = 0;

	virtual IListHeaderCallbackUI *GetListHeaderCallback() const = 0; 
	virtual void SetListHeaderCallback( IListHeaderCallbackUI *pCallback ) = 0; 

	virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
    virtual int GetExpandedItem() const = 0;
	virtual CListBodyExUI* GetListEx() const
	{
		return NULL; 
	}

	virtual CListBodyUI* GetList() const = 0; 

};

//class IListUI : public IListBaseUI 
//{
//public:
//	virtual CListBodyUI* GetList() const = 0; 
//
//};
//
//class IListExUI : public IListBaseUI 
//{
//public:
//	virtual CListBodyExUI* GetList() const = 0; 
//
//};





class IListItemUI
{
public:
    virtual int GetIndex() const = 0;
    virtual void SetIndex(int iIndex) = 0;
    virtual IListOwnerUI* GetOwner() = 0;
    virtual void SetOwner(CControlUI* pOwner) = 0;
    virtual bool IsSelected() const = 0;
    virtual bool Select(bool bSelect = true) = 0;
    virtual bool IsExpanded() const = 0;
    virtual bool Expand(bool bExpand = true) = 0;
    virtual void DrawItemText(HDC hDC, const RECT& rcItem) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CListBodyUI;
class CListBodyExUI; 
class CListHeaderUI; 

typedef enum _LIST_WORK_MODE
{
	NORMAL_LIST, 
	CACHED_LIST, 
} LIST_WORK_MODE, *PLIST_WORK_MODE; 

class UILIB_API CListBaseUI : public CVerticalLayoutUI, 
	public IListUI 
{
public:
    CListBaseUI();
	~CListBaseUI()
	{
		//DBG_BP(); 
	}

    UINT GetControlFlags() const;

    bool GetScrollSelect();
    void SetScrollSelect(bool bScrollSelect);
    int GetCurSel() const;

    CListHeaderUI* GetHeader() const;  
    TListInfoUI* GetListInfo();

	CControlUI* GetItemAt(int iIndex) const;
    INT32 GetItemIndex(CControlUI* pControl) const;
    bool SetItemIndex(CControlUI* pControl, int iIndex);
    INT32 GetItemCount() const;
    bool AddItem(CControlUI* pControl);
    bool AddItemAt(CControlUI* pControl, int iIndex);
    bool RemoveItem(CControlUI* pControl);
    bool RemoveItemAt(int iIndex);
	void RemoveAllItem(); 

    void EnsureVisible(int iIndex);
    //void Scroll(int dx, int dy);

    //int GetChildPadding() const;
    //void SetChildPadding(int iPadding);

    void SetItemFont(int index);
    void SetItemTextStyle(UINT uStyle);
    void SetItemTextPadding(RECT rc);
    void SetItemTextColor(DWORD dwTextColor);
    void SetItemBkColor(DWORD dwBkColor);
    void SetItemBkImage(LPCTSTR pStrImage);
    void SetAlternateBk(bool bAlternateBk);
    void SetSelectedItemTextColor(DWORD dwTextColor);
    void SetSelectedItemBkColor(DWORD dwBkColor);
    void SetSelectedItemImage(LPCTSTR pStrImage); 
    void SetHotItemTextColor(DWORD dwTextColor);
    void SetHotItemBkColor(DWORD dwBkColor);
    void SetHotItemImage(LPCTSTR pStrImage);
    void SetDisabledItemTextColor(DWORD dwTextColor);
    void SetDisabledItemBkColor(DWORD dwBkColor);
    void SetDisabledItemImage(LPCTSTR pStrImage);
    void SetItemLineColor(DWORD dwLineColor);
    bool IsItemShowHtml();
    void SetItemShowHtml(bool bShowHtml = true);
	RECT GetItemTextPadding() const;
	DWORD GetItemTextColor() const;
	DWORD GetItemBkColor() const;
	LPCTSTR GetItemBkImage() const;
    bool IsAlternateBk() const;
	DWORD GetSelectedItemTextColor() const;
	DWORD GetSelectedItemBkColor() const;
	LPCTSTR GetSelectedItemImage() const;
	DWORD GetHotItemTextColor() const;
	DWORD GetHotItemBkColor() const;
	LPCTSTR GetHotItemImage() const;
	DWORD GetDisabledItemTextColor() const;
	DWORD GetDisabledItemBkColor() const;
	LPCTSTR GetDisabledItemImage() const;
	DWORD GetItemLineColor() const;

    void SetMultiExpanding(bool bMultiExpandable); 
    int GetExpandedItem() const;
    //bool ExpandItem(int iIndex, bool bExpand = true);

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    IListCallbackUI* GetListCallback() const;
    void SetListCallback(IListCallbackUI* pCallback);

	virtual IListHeaderCallbackUI *GetListHeaderCallback() const; 
	virtual void SetListHeaderCallback( IListHeaderCallbackUI *pCallback ); 

	BOOLEAN ColumnWidthChanged(); 

protected:
    bool m_bScrollSelect;
    int m_iCurSel;
    int m_iExpandedItem;
	BOOLEAN m_bColumnWidthChanged; 
    IListCallbackUI* m_pCallback;
	IListHeaderCallbackUI *m_pListHeaderCallback; 
    //CListBodyUI* m_pList;
    CListHeaderUI* m_pHeader;
    TListInfoUI m_ListInfo;
};

class UILIB_API CListUI : public CListBaseUI, 
	public ISubItemManager
{
public:
	CListUI();
	~CListUI(); 

	LPCTSTR GetClass() const;
	ULONG GetClassId() const; 
    LPVOID GetInterface(LPCTSTR pstrName);

	int GetChildPadding() const;
	void SetChildPadding(int iPadding); 

	bool ExpandItem(int iIndex, bool bExpand = true); 
	void EnsureVisible(int iIndex); 

	bool SelectListItem(int iIndex);

	CListBodyUI* GetList() const; 

	
	void SetPos(RECT rc);
    void Scroll(int dx, int dy);

	//ISubItemManager
	void DoEvent(TEventUI& event); 

	bool AddItem(CControlUI* pControl); 
	//bool AddItemAt(CControlUI* pControl); 
	CControlUI* GetSubItemAt(int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID ) const;
	INT32 GetSubItemIndex(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID) const;
	bool SetSubItemIndex(CControlUI* pControl, int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID );
	int GetSubItemCount( ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID ) const;
	bool AddSubItem(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
	bool AddSubItemAt(CControlUI* pControl, int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
	bool RemoveSubItem(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
	bool RemoveSubItemAt(int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
	void RemoveSubAllItem( ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID); 

	//ui functions for list body
	SIZE GetScrollPos() const;
	SIZE GetScrollRange() const;
	void SetScrollPos(SIZE szPos);
	void LineUp();
	void LineDown();
	void PageUp();
	void PageDown();
	void HomeUp();
	void EndDown();
	void LineLeft();
	void LineRight();
	void PageLeft();
	void PageRight();
	void HomeLeft();
	void EndRight();
	void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
	virtual CScrollBarUI* GetVerticalScrollBar() const;
	virtual CScrollBarUI* GetHorizontalScrollBar() const;

protected:
	POINT pressed_pos; 
	SIZE pressed_scroll_pos; 
	BOOLEAN button_pressed; 
	HCURSOR previous_cursor; 

	CListBodyUI *m_pList; 
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderUI : public CHorizontalLayoutUI
{
public:
    CListHeaderUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    LPVOID GetInterface(LPCTSTR pstrName);
	void SetPos(RECT rc); 

    SIZE EstimateSize(SIZE szAvailable);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderItemUI : public CControlUI
{

public:
    CListHeaderItemUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    void SetEnabled(bool bEnable = true);

	bool IsDragable() const;
    void SetDragable(bool bDragable);
	DWORD GetSepWidth() const;
    void SetSepWidth(int iWidth);
	DWORD GetTextStyle() const;
    void SetTextStyle(UINT uStyle);
	DWORD GetTextColor() const;
    void SetTextColor(DWORD dwTextColor);
	void SetTextPadding(RECT rc);
	RECT GetTextPadding() const;
    void SetFont(int index);
    bool IsShowHtml();
    void SetShowHtml(bool bShowHtml = true);
    LPCTSTR GetNormalImage() const;
    void SetNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetHotImage() const;
    void SetHotImage(LPCTSTR pStrImage);
    LPCTSTR GetPushedImage() const;
    void SetPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetFocusedImage() const;
    void SetFocusedImage(LPCTSTR pStrImage);
    LPCTSTR GetSepImage() const;
    void SetSepImage(LPCTSTR pStrImage);

	IListUI* GetOwner(); 
	void SetOwner(CControlUI* pOwner); 

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    RECT GetThumbRect() const;

    void PaintText(HDC hDC);
    void PaintStatusImage(HDC hDC);

protected:
    POINT ptLastMouse;
    bool m_bDragable;
    UINT m_uButtonState;
    int m_iSepWidth;
    DWORD m_dwTextColor;
    int m_iFont;
    UINT m_uTextStyle;
    bool m_bShowHtml;
	RECT m_rcTextPadding;
    CStdString m_sNormalImage;
    CStdString m_sHotImage;
    CStdString m_sPushedImage;
    CStdString m_sFocusedImage;
    CStdString m_sSepImage;
    CStdString m_sSepImageModify;
	IListUI *m_pOwner; 
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListElementUI : public CControlUI, public IListItemUI
{
public:
    CListElementUI();

    LPCTSTR GetClass() const;
    ULONG GetClassId() const;
	UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetEnabled(bool bEnable = true);

    int GetIndex() const;
    void SetIndex(int iIndex);

    IListOwnerUI* GetOwner();
    void SetOwner(CControlUI* pOwner);
    void SetVisible(bool bVisible = true);

    bool IsSelected() const;
    bool Select(bool bSelect = true);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void DrawItemBk(HDC hDC, const RECT& rcItem);

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    IListOwnerUI* m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListLabelElementUI : public CListElementUI
{
public:
    CListLabelElementUI();

    LPCTSTR GetClass() const;
    ULONG GetClassId() const;
	LPVOID GetInterface(LPCTSTR pstrName);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void DrawItemText(HDC hDC, const RECT& rcItem);
};


/////////////////////////////////////////////////////////////////////////////////////
//

#define TEXT_ITEM_MAX_TEXT_LEN 256

class UILIB_API CListTextElementUI : public CListLabelElementUI
{
public:
    CListTextElementUI();
    ~CListTextElementUI();

    LPCTSTR GetClass() const;
	ULONG GetClassId() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    LPCTSTR GetText(int iIndex) const;
    void SetText(int iIndex, LPCTSTR pstrText);

    void SetOwner(CControlUI* pOwner);
    CStdString* GetLinkContent(int iIndex);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);

    void DrawItemText(HDC hDC, const RECT& rcItem);

protected:
    enum { MAX_LINK = 8 };
    int m_nLinks;
    RECT m_rcLinks[MAX_LINK];
    CStdString m_sLinks[MAX_LINK];
    int m_nHoverLink;
    IListUI* m_pOwner;
    CStdPtrArray m_aTexts;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListContainerElementUI : public CContainerUI, public IListItemUI
{
public:
    CListContainerElementUI();

	LPCTSTR GetClass() const;
	ULONG GetClassId() const;   
	UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    int GetIndex() const;
    void SetIndex(int iIndex);

    IListOwnerUI* GetOwner();
    void SetOwner(CControlUI* pOwner);
    void SetVisible(bool bVisible = true);
    void SetEnabled(bool bEnable = true);

    virtual bool IsSelected() const;
    bool Select(bool bSelect = true);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void DrawItemText(HDC hDC, const RECT& rcItem);    
    void DrawItemBk(HDC hDC, const RECT& rcItem);

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    IListOwnerUI* m_pOwner;
};

#define ADD_LIST_CONTAINER_ITEM 0x00000001

HRESULT add_list_item( CListUI* list, ULONG index/*, ULONG flags = 0*/ ); 

} // namespace DuiLib

#endif // __UILIST_H__
