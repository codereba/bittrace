#ifndef __UICONTROL_H__
#define __UICONTROL_H__

#pragma once

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//

//typedef enum _IMAGE_DATA_TYPE
//{
//	IMAGE_DATA_CONFIG_STRING = 0, 
//	IMAGE_DATA_HICON, 
//	IMAGE_DATA_MAX, 
//} IMAGE_DATA_TYPE, *PIMAGE_DATA_TYPE; 

//typedef struct _IMAGE_INFO
//{
//	IMAGE_DATA_TYPE Type; 
//	PVOID Data; 
//} IMAGE_INFO, *PIMAGE_INFO; 

typedef CControlUI* (CALLBACK* FINDCONTROLPROC)(CControlUI*, LPVOID);

class UILIB_API CControlUI
{
public:
    CControlUI();
    virtual ~CControlUI();

public:
    virtual CStdString GetName() const;
    virtual void SetName(LPCTSTR pstrName);
    virtual LPCTSTR GetClass() const;
	virtual ULONG GetClassId() const;	
    virtual LPVOID GetInterface(LPCTSTR pstrName);
    virtual UINT GetControlFlags() const;

    virtual bool Activate();
    virtual CPaintManagerUI* GetManager() const;
    virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
    virtual CControlUI* GetParent() const;

    // 文本相关
    virtual CStdString GetText() const;
    virtual void SetText(LPCTSTR pstrText);

    // 图形相关
    DWORD GetBkColor() const;
    void SetBkColor(DWORD dwBackColor);
    DWORD GetBkColor2() const;
    void SetBkColor2(DWORD dwBackColor);
    DWORD GetBkColor3() const;
    void SetBkColor3(DWORD dwBackColor);
    LPCTSTR GetBkImage();
    void SetBkImage(LPCTSTR pStrImage);
    DWORD GetBorderColor() const;
    void SetBorderColor(DWORD dwBorderColor);
	DWORD GetFocusBorderColor() const;
	void SetFocusBorderColor(DWORD dwBorderColor);
    bool IsColorHSL() const;
    void SetColorHSL(bool bColorHSL);
    int GetBorderSize() const;
    void SetBorderSize(int nSize);
    SIZE GetBorderRound() const;
	void SetBorderRound(SIZE cxyRound);
	bool DrawImage(HDC hDC, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL ); 

	virtual void SetPosNoLayout(RECT rc)
	{
		m_rcItem = rc; 
	}

    // 位置相关
    virtual const RECT& GetPos();
    virtual void SetPos(RECT rc);
    virtual int GetWidth() const;
    virtual int GetHeight() const;
    virtual int GetX() const;
    virtual int GetY() const;
    virtual RECT GetPadding() const;
    virtual void SetPadding(RECT rcPadding); // 设置外边距，由上层窗口绘制
    virtual SIZE GetFixedXY() const;         // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    virtual void SetFixedXY(SIZE szXY);      // 仅float为true时有效
    virtual int GetFixedWidth() const;       // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    virtual void SetFixedWidth(int cx);      // 预设的参考值
    virtual int GetFixedHeight() const;      // 实际大小位置使用GetPos获取，这里得到的是预设的参考值
    virtual void SetFixedHeight(int cy);     // 预设的参考值
    virtual int GetMinWidth() const;
    virtual void SetMinWidth(int cx);
    virtual int GetMaxWidth() const;
    virtual void SetMaxWidth(int cx);
    virtual int GetMinHeight() const;
    virtual void SetMinHeight(int cy);
    virtual int GetMaxHeight() const;
    virtual void SetMaxHeight(int cy);
    virtual void SetRelativePos(SIZE szMove,SIZE szZoom);
    virtual void SetRelativeParentSize(SIZE sz);
    virtual TRelativePosUI GetRelativePos() const;
    virtual bool IsRelativePos() const;

	LRESULT MoveFixedPos( LONG x, LONG y )
	{
		m_cXY.cx += x; 
		m_cXY.cy += y; 
	}

    // 鼠标提示
    virtual CStdString GetToolTip() const;
    virtual void SetToolTip(LPCTSTR pstrText);

    // 快捷键
    virtual TCHAR GetShortcut() const;
    virtual void SetShortcut(TCHAR ch);

    // 菜单
    virtual bool IsContextMenuUsed() const;
    virtual void SetContextMenuUsed(bool bMenuUsed);

    // 用户属性
    virtual const CStdString& GetUserData(); // 辅助函数，供用户使用
    virtual void SetUserData(LPCTSTR pstrText); // 辅助函数，供用户使用
    virtual UINT_PTR GetTag() const; // 辅助函数，供用户使用
    virtual void SetTag(UINT_PTR pTag); // 辅助函数，供用户使用

    // 一些重要的属性
    virtual bool IsVisible() const;
    virtual void SetVisible(bool bVisible = true);
    virtual void SetInternVisible(bool bVisible = true); // 仅供内部调用，有些UI拥有窗口句柄，需要重写此函数
    virtual bool IsEnabled() const;
    virtual void SetEnabled(bool bEnable = true);
    virtual bool IsMouseEnabled() const;
    virtual void SetMouseEnabled(bool bEnable = true);
    virtual bool IsFocused() const;
    virtual void SetFocus();
    virtual bool IsFloat() const;
    virtual void SetFloat(bool bFloat = true);

    virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

    void Invalidate();
    virtual bool IsUpdateNeeded() const;
    void NeedUpdate();
	void NeedUpdateNoInvalidate(); 
    void NeedParentUpdate();
    DWORD GetAdjustColor(DWORD dwColor);

    virtual void Init();
    virtual void DoInit();

    virtual void Event(TEventUI& event);
    virtual void DoEvent(TEventUI& event);

    virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    CControlUI* ApplyAttributeList(LPCTSTR pstrList);

    virtual SIZE EstimateSize(SIZE szAvailable);

    virtual void DoPaint(HDC hDC, const RECT& rcPaint);
    virtual void PaintBkColor(HDC hDC);
    virtual void PaintBkImage(HDC hDC);
    virtual void PaintStatusImage(HDC hDC);
    virtual void PaintText(HDC hDC);
    virtual void PaintBorder(HDC hDC);

    virtual void DoPostPaint(HDC hDC, const RECT& rcPaint);
	
	void dump_contro_ui_info( CControlUI *ctrl )
	{
		TRACE( _T( "enter %s" ), __TFUNCTION__ ); 

		TRACE( _T( "name is %s" ), ctrl->m_sName.GetData() ); 
		TRACE( _T( "need update %d\n" ), ctrl->m_bUpdateNeeded ); 
		TRACE( _T( "use menu %d\n" ), ctrl->m_bMenuUsed ); 
		TRACE( _T( "item rect (%d,%d,%d,%d)\n" ), ctrl->m_rcItem ); 
		TRACE( _T( "padding rect (%d,%d,%d,%d)\n" ), ctrl->m_rcPadding ); 
		TRACE( _T( "x, y (%d,%d)\n" ), ctrl->m_cXY.cx, ctrl->m_cXY.cy ); 
		TRACE( _T( "x, y fixed (%d,%d)\n" ), ctrl->m_cxyFixed ); 
		TRACE( _T( "x, y min (%d,%d)\n" ), ctrl->m_cxyMin ); 
		TRACE( _T( "x, y max (%d,%d)\n" ), ctrl->m_cxyMax ); 
		TRACE( _T( "visible %d\n" ), ctrl->m_bVisible ); 
		TRACE( _T( "intern visible %d\n" ), ctrl->m_bInternVisible ); 
		TRACE( _T( "mouse enabled %d\n" ), ctrl->m_bMouseEnabled ); 
		TRACE( _T( "focused %d\n" ), ctrl->m_bFocused ); 
		TRACE( _T( "float %d\n" ), ctrl->m_bFloat ); 
		TRACE( _T( "is set pos %d\n" ), ctrl->m_bSetPos ); 
		//TRACE( _T( "" ), ctrl->m_tRelativePos ); 
		TRACE( _T( "text %s\n" ), ctrl->m_sText ); 
		TRACE( _T( "tool tip %s\n" ), ctrl->m_sToolTip ); 
		TRACE( _T( "shortcut %d" ), ctrl->m_chShortcut ); 
		TRACE( _T( "user data %s\n" ), ctrl->m_sUserData ); 
		TRACE( _T( "tag 0x%0.8x\n" ), ctrl->m_pTag ); 
		TRACE( _T( "bk color 1 0x%0.8x\n" ), ctrl->m_dwBackColor ); 
		TRACE( _T( "bk color 2 0x%0.8x\n" ), ctrl->m_dwBackColor2 ); 
		TRACE( _T( "bk color 3 0x%0.8x\n" ), ctrl->m_dwBackColor3 ); 
		TRACE( _T( "bk image %s\n" ), ctrl->m_sBkImage ); 
		TRACE( _T( "border color 0x%0.8x\n" ), ctrl->m_dwBorderColor ); 
		TRACE( _T( "focused border color 0x%0.8x\n" ), ctrl->m_dwFocusBorderColor ); 
		TRACE( _T( "use hsl color %d\n" ), ctrl->m_bColorHSL ); 
		TRACE( _T( "border size %d\n" ), ctrl->m_nBorderSize ); 
		TRACE( _T( "border round (%d,%d)\n" ), ctrl->m_cxyBorderRound ); 
		TRACE( _T( "paint rect (%d,%d,%d,%d)\n" ), ctrl->m_rcPaint ); 

		TRACE( _T( "leave %s" ), __TFUNCTION__ ); 
	}; 

public:
    CEventSource OnInit;
    CEventSource OnDestroy;
    CEventSource OnSize;
    CEventSource OnEvent;
    CEventSource OnNotify;

protected:
    CPaintManagerUI* m_pManager;
    CControlUI* m_pParent;
    CStdString m_sName;
    bool m_bUpdateNeeded;
    bool m_bMenuUsed;
    RECT m_rcItem;
    RECT m_rcPadding;
    SIZE m_cXY;
    SIZE m_cxyFixed;
    SIZE m_cxyMin;
    SIZE m_cxyMax;
    bool m_bVisible;
    bool m_bInternVisible;
    bool m_bEnabled;
    bool m_bMouseEnabled;
    bool m_bFocused;
    bool m_bFloat;
    bool m_bSetPos; // 防止SetPos循环调用
    TRelativePosUI m_tRelativePos;

    CStdString m_sText;
    CStdString m_sToolTip;
    TCHAR m_chShortcut;
    CStdString m_sUserData;
    UINT_PTR m_pTag;

    DWORD m_dwBackColor;
    DWORD m_dwBackColor2;
    DWORD m_dwBackColor3;
    CStdString m_sBkImage;
    DWORD m_dwBorderColor;
	DWORD m_dwFocusBorderColor;
    bool m_bColorHSL;
    int m_nBorderSize;
    SIZE m_cxyBorderRound;
    RECT m_rcPaint;
};

#ifndef ERROR_ERRORS_ENCOUNTERED
#define ERROR_ERRORS_ENCOUNTERED 0xC0000001 
#endif //ERROR_ERRORS_ENCOUNTERED
INT32 __fastcall DebugControlPaint( CControlUI *Control ); 
INT32 SetDebugNames( LPCWSTR ClassName, LPCWSTR ControlName ); 

} // namespace DuiLib

#endif // __UICONTROL_H__
