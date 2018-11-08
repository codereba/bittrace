#ifndef __ITEM_MANAGER_H__
#define __ITEM_MANAGER_H__

namespace DuiLib {

#define ITEM_MANAGER_INTERFACE_NAME _T("IItemManager")

typedef struct UILIB_API _IItemManager
{
public:
	virtual CControlUI* GetItemAt(int iIndex) const = 0;
	virtual INT32 GetItemIndex(CControlUI* pControl) const = 0;
	virtual bool SetItemIndex(CControlUI* pControl, int iIndex) = 0;
	virtual INT32 GetItemCount() const = 0; 
	virtual bool AddItem(CControlUI* pControl ) = 0;
	virtual bool AddItemAt(CControlUI* pControl, int iIndex ) = 0;
	virtual bool RemoveItem(CControlUI* pControl) = 0;
	virtual bool RemoveItemAt(int iIndex) = 0;
	virtual void RemoveAllItem() = 0; 
} IItemManager, *PIItemManager; 

typedef struct UILIB_API _ISubItemManager
{
public:
	virtual CControlUI* GetSubItemAt(int iIndex, ULONG ClassId = UI_CONTROL_CLASS_ID ) const = 0;
	virtual INT32 GetSubItemIndex(CControlUI* pControl, ULONG ClassId = UI_CONTROL_CLASS_ID ) const = 0;
	virtual bool SetSubItemIndex(CControlUI* pControl, int iIndex, ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0;
	virtual int GetSubItemCount( ULONG ClassId = UI_CONTROL_CLASS_ID ) const = 0;
	virtual bool AddSubItem(CControlUI* pControl, ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0;
	virtual bool AddSubItemAt(CControlUI* pControl, int iIndex, ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0;
	virtual bool RemoveSubItem(CControlUI* pControl, ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0;
	virtual bool RemoveSubItemAt(int iIndex, ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0;
	virtual void RemoveSubAllItem( ULONG ClassId = UI_CONTROL_CLASS_ID ) = 0; 

} ISubItemManager, *PISubItemManager; 

class UILIB_API CCommonItemManager : public IItemManager
{

public:
	CCommonItemManager(); 
	~CCommonItemManager(); 

	CControlUI* GetItemAt(int iIndex) const; 
	INT32 GetItemIndex(CControlUI* pControl) const;
	bool SetItemIndex(CControlUI* pControl, int iIndex);
	INT32 GetItemCount() const; 

	bool AddItem(CControlUI* pControl);
	bool AddItemAt(CControlUI* pControl, int iIndex);
	bool RemoveItem(CControlUI* pControl);
	bool RemoveItemAt(int iIndex);
	void RemoveAllItem(); 

	bool RemoveNoUpdate(CControlUI* pControl); 
	bool AddNoUpdate(CControlUI* pControl); 
	bool AddAtNoUpdate(CControlUI* pControl, int iIndex); 
	bool RemoveAtNoUpdate(int iIndex); 

	bool IsAutoDestroy() const; 
	void SetAutoDestroy(bool bAuto); 
	bool IsDelayedDestroy() const; 
	void SetDelayedDestroy(bool bDelayed); 

	void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

protected:
	CStdPtrArray m_items;
	bool m_bAutoDestroy;
	bool m_bDelayedDestroy;
}; 

class UILIB_API CCModalControl : public CControlUI
{
public:
	CCModalControl()
	{

	}

	~CCModalControl()
	{

	}

protected:
	void SetPos( RECT rc ); 
};

//class UILIB_API CCachedItemManager : public IItemManager
//{
//public:
//	CCachedItemManager(); 
//	~CCachedItemManager(); 
//
//	CControlUI* GetItemAt(int iIndex) const; 
//	INT32 GetItemIndex(CControlUI* pControl) const;
//	bool SetItemIndex(CControlUI* pControl, int iIndex);
//	INT32 GetItemCount() const; 
//
//	bool Add(CControlUI* pControl);
//	bool AddAt(CControlUI* pControl, int iIndex);
//	bool Remove(CControlUI* pControl);
//	bool RemoveAt(int iIndex);
//	void RemoveAll();
//
//protected:
//	CStdPtrArray m_items;
//	
//	CCModalControl m_ModalCtrl; 
//
//	//bool m_bAutoDestroy;
//	//bool m_bDelayedDestroy;
//}; 

} //namespace DuiLib 
#endif //__ITEM_MANAGER_H__