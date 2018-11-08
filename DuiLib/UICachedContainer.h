#ifndef __CACHED_CONTAINER_H__
#define __CACHED_CONTAINER_H__

//typedef struct _cache_item
//{
//	CRITICAL_SECTION *lock; 
//	CControlUI *item; 
//} cache_item, *pcache_item; 

LONG GetRingItemIndex( LONG index, LONG item_count ); 

namespace DuiLib
{
	class IListContainerCallback
	{
	public:
		LRESULT LoadSubItemsContent( ULONG ItemBeginIndex, ULONG ItemCount ); 
	};

	//INLINE LONG GetRingItemIndex( LONG index, LONG item_count )
	//{
	//	LONG item_index; 
	//	
	//	item_index = ( index % item_count ); 
	//	if( item_index < 0 )
	//	{
	//		item_index = item_count + item_index; 
	//	}

	//	return item_index; 
	//}

	class CDialogBuilder; 

	/**********************************************************
	notice:
	1.status of the item set have 2 type:
		1.temporeryly, these status can just changing in the 
		display sub set.
		2.permanently, these status( like some ui status ) must
		setting to the backup store.
		(for some functions, need to add some field in database, 
		to store removing status etc.)
	**********************************************************/
	class CCachedItemManager 
	{
	public: 
#define INVALID_ITEM_COUNT ( ULONG )( -1 )
#define INVALID_ITEM_INDEX ( ULONG )( -1 )

		CCachedItemManager(); 
		~CCachedItemManager(); 

		LONG GetItemRingIndexOffset( LONG RingIndex )
		{
			LONG ItemOffset; 
			ASSERT( ItemRingBegin >= 0 ); 

			if( RingIndex < ItemRingBegin )
			{
				ItemOffset = ( ItemRingBegin - RingIndex ) + ( ItemRealCount - ItemRingBegin ); 
			}
			else
			{
				ItemOffset = RingIndex - ItemRingBegin; 
			}

			return ItemOffset; 
		}

		LONG GetBeginDisplayItemRealIndex()
		{
			LONG ItemRealIndex; 

			ItemRealIndex = GetItemRingIndexOffset( ItemDisplayBegin ); 
			ItemRealIndex += ItemCacheBeginIndex; 

			return ItemRealIndex; 
		}

		LRESULT GetItemDisplaySize( SIZE *Size )
		{
			LRESULT ret = ERROR_SUCCESS; 
			
			do 
			{
				if( Size == NULL )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				Size->cx = ItemDisplayWidth; 
				Size->cy = ItemDisplayHeight; 

			}while( FALSE );
		
			return ret; 
		}

		LRESULT SetItemDisplaySize( LONG Width, LONG Height )
		{
			LRESULT ret = ERROR_SUCCESS; 

			do 
			{
				ItemDisplayWidth = Width; 
				ItemDisplayHeight = Height; 

			}while( FALSE );

			return ret; 
		}
			
		//ULONG GetDisplayItemEndIndex()
		//{
		//	return ItemDisplayBegin + ItemDisplayCount; 
		//}

		virtual ULONG GetRealItemsCount(); 
		virtual ULONG GetItemShowCount(); 
		ULONG GetDisplayItemCount(); 

		LRESULT WINAPI InitItemCache()
		{
			DBG_BP(); 
			return ERROR_NOT_SUPPORTED; 
		}

		virtual LRESULT GetDisplayItemAt( ULONG Index, CControlUI **UIItem ); 

		virtual LRESULT FillSubItemsContent( LONG ItemBeginIndex, LONG ItemCount, LONG *ItemLoaded )
		{
			DBG_BP(); 
			return ERROR_NOT_SUPPORTED; 
		}

		virtual LRESULT GetRealContentSize( SIZE *Size ); 

		//virtual LRESULT GetDisplayItemSize( SIZE *Size ); 

		virtual ULONG GetRingIndexFromDisplayIndex( ULONG Index ); 

		virtual LRESULT SetDisplayPos( CControlUI *pControl, RECT *rcDisplay ); 

		virtual LRESULT OnScrollPosChanged( ULONG PosX, ULONG PosY, ULONG Height ); 
		//virtual LRESULT OnScrollPosChanged( LONG PosX, LONG PosY, LONG Height ); 
		//functions for cache
		//virtual LRESULT WINAPI InitItemCache(); 

		virtual LRESULT UninitItemCache(); 

		virtual LRESULT __fastcall AllocOneItemCache( CDialogBuilder *dlgBuilder, CControlUI** item_out )
		{
			DBG_BP(); 
			return ERROR_NOT_SUPPORTED; 
		}

		//virtual LRESULT LoadItemCache( ULONG item_begin ); 

		virtual LRESULT OnItemAdded( ULONG item_count ); 

		virtual LRESULT RemoveAllCachedItems(); 

		virtual LRESULT RemoveDisplayItem( INT32 ItemIndex ); 
	
		virtual IListContainerCallback *GetListContainerCallback() const; 

		virtual void SetListContainerCallback( IListContainerCallback *pCallback ); 

		virtual ULONG GetMaxCachedItemCount()
		{
			return MaxItemCacheCount; 
		}

		virtual LRESULT SetItemRealCount( ULONG ItemCount )
		{
			ULONG OldCount; 

			OldCount = ItemRealCount; 
			ItemRealCount = ItemCount; 
//#ifdef _DEBUG
//			if( ItemRealCount != all_filtered_actions.size() )
//			{
//				__asm int 3; 
//			}
//#endif //_DEBUG

			
			return ItemRealCount; 
		}

	public:
		//ULONG ItemCount; 

		//items for control display cache.

		//CPaintManagerUI* m_pManager; 
		//CRITICAL_SECTION cache_lock; 
		//cache_item **ControlUICache; 
		CControlUI **CachedControlUI; 
		//LONG ItemFilteredBeginIndex; 
		LONG ItemRealCount; 

		//the index which is filtered. 
		LONG ItemCacheBeginIndex; 
		LONG ItemCacheCount; 
		LONG ItemRingBegin; 
		LONG MaxItemCacheCount; 
		LONG ItemDisplayBegin; 
		//ULONG ItemDisplayOffset; 
		LONG ItemDisplayCount; 
		//SIZE ItemDisplaySize; 
		LONG ItemDisplayWidth; 
		LONG ItemDisplayHeight; 

		IListContainerCallback *m_pContainerCallback; 
	};

	class UILIB_API CCachedContainerUI : public CBaseContainerUI 
	{
	public:
		CCachedContainerUI();
		virtual ~CCachedContainerUI();

	public:
		void DoEvent(TEventUI& event);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		void SetMouseEnabled(bool bEnable = true);

		//virtual RECT GetInset() const;
		//virtual void SetInset(RECT rcInset); // 设置内边距，相当于设置客户区
		//virtual int GetChildPadding() const;
		//virtual void SetChildPadding(int iPadding);

		//virtual bool IsMouseChildEnabled() const;
		//virtual void SetMouseChildEnabled(bool bEnable = true);

		virtual int FindSelectable(int iIndex, bool bForward = true) const;

		void SetPos(RECT rc);
		void SetPos2(RECT rc);

		void DoPaint(HDC hDC, const RECT& rcPaint);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
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
		virtual void LineLeft();
		virtual void LineRight();
		virtual void PageLeft();
		virtual void PageRight();
		virtual void HomeLeft();
		virtual void EndRight();
		virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
		virtual CScrollBarUI* GetVerticalScrollBar() const;
		virtual CScrollBarUI* GetHorizontalScrollBar() const;

		//VOID SetRealItemsCount( ULONG Count ); 

		//ULONG GetRealItemsCount(); 

		//ULONG GetCachedItemIndex( ULONG Index ); 

		//LRESULT FillSubItemsContent( ULONG RealIndex, ULONG Count ); 

		//LRESULT GetRealContentSize( SIZE *Size ); 

		//LRESULT LoadActionEventFromFile( ULONG BeginIndex, ULONG EndIndex ); 

		//LRESULT OnScrollPosChanged( LONG PosX, LONG PosY ); 

		//ULONG GetDisplayCount(); 

		//LRESULT GetDisplayItem( ULONG Index, CControlUI **UIItem ); 

		//LRESULT GetDisplayItemSize( SIZE *Size ); 

		virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);

	protected:
		virtual void SetFloatPos(int iIndex);

	protected:
		RECT m_rcInset;
		int m_iChildPadding;
		bool m_bMouseChildEnabled;
		bool m_bScrollProcess; // 防止SetPos循环调用

		CScrollBarUI* m_pVerticalScrollBar;
		CScrollBarUI* m_pHorizontalScrollBar;

		//CCachedItemManager *CacheManager; 
	};
} //namespace DuiLib

#endif //__CACHED_CONTAINER_H__