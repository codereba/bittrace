/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __UI_LIST_BODY_EX_H__
#define __UI_LIST_BODY_EX_H__

#define	DEFAULT_CACHE_ITEM_PAGE_COUNT 20

namespace DuiLib
{
	class UILIB_API CListBodyUI : public CVerticalLayoutUI
	{
	public:
		CListBodyUI(CListUI* pOwner);

		void SetScrollPos(SIZE szPos);
		void SetPos(RECT rc);
		void DoEvent(TEventUI& event);

	protected:
		CListUI* m_pOwner;
	};

	typedef CRITICAL_SECTION ITEM_CACHE_LOCK; 

	class UILIB_API CListBodyExUI : public CCachedContainerUI, public CCachedItemManager 
	{
	public:

		CListBodyExUI(CListExUI* pOwner); //:m_bMouseChildEnabled( true ) 
		~CListBodyExUI(); 

		void SetScrollPos(SIZE szPos);
		void SetPos(RECT rc);
		LPCTSTR GetClass() const
		{
			return UI_LIST_BODY_EX_CLASS_NAME;
		}

		void DoEvent(TEventUI& event);

		void DoPaint(HDC hDC, const RECT& rcPaint); 

		LRESULT __fastcall AllocOneItemCache( CDialogBuilder *dlgBuilder, CControlUI** item_out ); 

		LRESULT InitItemCacheWorker(); 
		LRESULT UninitItemCacheWorker(); 

		LRESULT TestLogLoading( LONG Indexes[], ULONG Counts[], ULONG InputsCount ); 

		LRESULT FilteredIndex2RealIndex( LONG FilteredIndex, LONG *RealIndex ); 

		virtual LRESULT FillSubItemsContent( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded ); 
		LRESULT fill_cached_item_aio( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded ); 
		LRESULT on_cache_item_loaded( PVOID item, PVOID context ); 
		LRESULT on_cache_item_loaded_done( PVOID context ); 

#define CACHE_WORK_LOADED 0x00000001
		LRESULT check_and_update_ui( LONG ItemIndex, ULONG loaded_count, LONG ItemCount, ULONG flags ); 

#define LOAD_ITEM_DECREMENTAL 0x00000001
		LRESULT AdjustItemCacheIndexes( LONG item_count, ULONG flags ); 

		CControlUI* GetItemAt(int iIndex) const; 
		int GetItemIndex(CControlUI* pControl) const; 
		bool SetItemIndex(CControlUI* pControl, int iIndex); 
		int GetSubItemCount( ULONG ClassId ) const; 
		int GetItemCount() const; 
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags); 

#define DEFAULT_CACHE_LOCK_COUNT ( ULONG )8
		/*virtual */LRESULT InitCacheLocks(); 
		/*virtual */LRESULT UninitCacheLocks(); 

		virtual LRESULT InitItemCache(); 

		virtual LRESULT UninitItemCache(); 

		LRESULT ResetItemCache(); 
		//virtual LRESULT __fastcall AllocOneItemCache( CControlUI** item_out ); 

		virtual LRESULT GetDisplayItemAt( LONG Index, CControlUI **UIItem ); 
		//virtual LRESULT LoadItemCache( ULONG item_begin ); 
			
		ULONG GetRingIndexFromDisplayIndex( ULONG ItemIndex ); 

//#define _UI_DEBUG
#ifdef _UI_DEBUG
		LRESULT SetTempControlContent( LONG ItemRealIndex, LONG ItemSlotIndex ); 
#endif //_UI_DEBUG	

		virtual LRESULT OnItemAdded( LONG item_count ); 
		//LRESULT OnScrollPosChanged( LONG PosX, LONG PosY, LONG Height ); 
		virtual LRESULT OnScrollPosChanged( LONG PosX, LONG PosY, LONG Height ); 
		LRESULT DisplayRegionFilled( LONG DisplayItem ); 
		LRESULT GetItemDisplaySize( SIZE *Size ); 
		LRESULT SetItemDisplaySize( LONG Width, LONG Height ); 

		bool IsUpdateNeeded() const; 
		LRESULT SetItemRealCount( LONG ItemCount ); 
	
		LRESULT SetDisplayIndexFromRealIndex( LONG RealIndex ); 
		LRESULT InvalidateListItem( LONG BeginIndex, LONG EndIndex ); 

		void SetAutoScroll( BOOLEAN AutoScroll )
		{
			m_bAutoScroll = AutoScroll; 
		}

	public:
		CListExUI* m_pOwner; 

		//LONG SelectedIndexBegin; 
		//LONG SelectedIndexEnd; 
#define ALL_DISPLAY_ITEM_PAINTING 0x00000001
#define ALL_DISPLAY_ITEM_PAINTED 0x00000002
		BYTE ItemDisplayOnReset; 

	protected:
		//CControlUI *ModalCtrl; 
		//BOOLEAN CacheLockInited; 
		BOOLEAN m_bAutoScroll; 
		//ITEM_CACHE_LOCK *CacheLocks; 
		//ULONG ListItemUpdateCount; 

		//CStdPtrArray NeedUpdateCtrls; 
	};
} // namespace DuiLib
#endif //__UI_LIST_BODY_EX_H__
