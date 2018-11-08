#include "StdAfx.h"

namespace DuiLib {

CCommonItemManager::CCommonItemManager() : 
m_bAutoDestroy(true),
m_bDelayedDestroy(true)
{
	
}

CCommonItemManager::~CCommonItemManager()
{
	m_bDelayedDestroy = false;
	RemoveAllItem();
}

CControlUI* CCommonItemManager::GetItemAt(int iIndex) const
{
	if( iIndex < 0 || iIndex >= m_items.GetSize() ) return NULL;
	return static_cast<CControlUI*>(m_items[iIndex]);
}

int CCommonItemManager::GetItemIndex(CControlUI* pControl) const
{
	for( int it = 0; it < m_items.GetSize(); it++ ) {
		if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
			return it;
		}
	}

	return -1;
}

bool CCommonItemManager::SetItemIndex(CControlUI* pControl, int iIndex)
{
	for( int it = 0; it < m_items.GetSize(); it++ ) {
		if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
			//NeedUpdate();            
			m_items.Remove(it);
			return m_items.InsertAt(iIndex, pControl);
		}
	}

	return false;
}

INT32 CCommonItemManager::GetItemCount() const
{
	return m_items.GetSize();
}


bool CCommonItemManager::AddItem(CControlUI* pControl)
{
	if( pControl == NULL) return false;

	//if( m_pManager != NULL ) m_pManager->InitControls(pControl, this);
	//if( IsVisible() ) NeedUpdate();
	else pControl->SetInternVisible(false);
	return m_items.Add(pControl);   
}

bool CCommonItemManager::AddItemAt(CControlUI* pControl, int iIndex)
{
	if( pControl == NULL) return false;

	//if( m_pManager != NULL ) m_pManager->InitControls(pControl, this);
	//if( IsVisible() ) NeedUpdate();
	else pControl->SetInternVisible(false);
	return m_items.InsertAt(iIndex, pControl);
}

bool CCommonItemManager::RemoveItem(CControlUI* pControl)
{
	if( pControl == NULL) return false;

	for( int it = 0; it < m_items.GetSize(); it++ ) {
		if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
			//NeedUpdate();
			if( m_bAutoDestroy ) {
				//if( m_bDelayedDestroy && m_pManager ) m_pManager->AddDelayedCleanup(pControl);             
				//else delete pControl;
			}
			return m_items.Remove(it);
		}
	}
	return false;
}

bool CCommonItemManager::RemoveItemAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (pControl != NULL) {
		return CCommonItemManager::RemoveItem(pControl);
	}

	return false;
}

void CCommonItemManager::RemoveAllItem()
{
	for( int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++ ) {
		//if( m_bDelayedDestroy && m_pManager ) m_pManager->AddDelayedCleanup(static_cast<CControlUI*>(m_items[it]));             
		//else delete static_cast<CControlUI*>(m_items[it]);
	}
	m_items.Empty();
	//NeedUpdate();
}

bool CCommonItemManager::IsAutoDestroy() const
{
	return m_bAutoDestroy;
}

void CCommonItemManager::SetAutoDestroy(bool bAuto)
{
	m_bAutoDestroy = bAuto;
}

bool CCommonItemManager::IsDelayedDestroy() const
{
	return m_bDelayedDestroy;
}

void CCommonItemManager::SetDelayedDestroy(bool bDelayed)
{
	m_bDelayedDestroy = bDelayed;
}

void CCommonItemManager::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
	//CControlUI::SetManager(pManager, pParent, bInit);
}

//CCachedItemManager::CCachedItemManager()
//{
//}
//
//CCachedItemManager::~CCachedItemManager()
//{
//}
//
//CControlUI* CCachedItemManager::GetItemAt(int iIndex) const
//{
//	return NULL; 
//}
//
//void CCModalControl::SetPos( RECT rc )
//{
//
//}
//
//INT32 CCachedItemManager::GetItemIndex(CControlUI* pControl) const
//{
//	return m_items.GetSize(); 
//}
//
//bool CCachedItemManager::SetItemIndex(CControlUI* pControl, int iIndex)
//{
//	return false; 
//}
//
//INT32 CCachedItemManager::GetItemCount() const
//{
//	return 0; 
//}
//
//bool CCachedItemManager::Add(CControlUI* pControl)
//{
//	return false; 
//}
//
//bool CCachedItemManager::AddAt(CControlUI* pControl, int iIndex)
//{
//	return false; 
//}
//
//bool CCachedItemManager::Remove(CControlUI* pControl)
//{
//	return false; 
//}
//
//bool CCachedItemManager::RemoveAt(int iIndex)
//{
//	return false; 
//}
//
//void CCachedItemManager::RemoveAll()
//{
//
//}

//bool CCachedItemManager::AddItemCache(CControlUI* pControl)
//{
//	return false; 
//}
//
//bool CCachedItemManager::AddItemCacheAt(CControlUI* pControl, int iIndex)
//{
//	return false; 
//}
//
//bool CCachedItemManager::Remove(CControlUI* pControl)
//{
//	return false; 
//}
//
//bool CCachedItemManager::RemoveAt(int iIndex)
//{
//	return false; 
//}
//
//void CCachedItemManager::RemoveAll()
//{
//
//}


} //namespace DuiLib 