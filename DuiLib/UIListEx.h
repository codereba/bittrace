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
 *
 *  2007.6.10	Ji Jie Shi added this file based on the duilib. 
 */

#ifndef __LIST_EX_H__
#define __LIST_EX_H__

namespace DuiLib
{
	double CalculateDelay( double state ); 

	class UILIB_API CListExUI : public CListBaseUI, 
		public ISubItemManager 
	{
	public:
		CListExUI(); 

		LPCTSTR GetClass() const;
		ULONG GetClassId() const; 
		LPVOID GetInterface(LPCTSTR pstrName);

		UINT GetControlFlags() const; 

		int GetChildPadding() const;
		void SetChildPadding(int iPadding); 

		bool ExpandItem(int iIndex, bool bExpand = true); 
		void EnsureVisible(int iIndex); 

		INT32 GetCurCacheSel() const; 
		VOID SetCurCacheSel( INT32 iIndex ); 
		bool SelectListItem(int iIndex);

		CListBodyExUI* GetListEx() const; 

		TListInfoUI* GetListInfo();
		int GetCurSel() const;
		//bool SelectListItem(int iIndex);
		BOOLEAN ColumnWidthChanged(); 
		LRESULT ChangeSelectListItem( LONG Index ); 


		void SetPos(RECT rc);
		void Scroll(int dx, int dy);

		void DoEvent(TEventUI& event); 

		CControlUI* GetSubItemAt(int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID ) const;
		INT32 GetSubItemIndex(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID) const;
		bool SetSubItemIndex(CControlUI* pControl, int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID );
		int GetSubItemCount( ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID ) const;
		bool AddSubItem(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
		bool AddSubItemAt(CControlUI* pControl, int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
		bool RemoveSubItem(CControlUI* pControl, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
		bool RemoveSubItemAt(int iIndex, ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID);
		void RemoveSubAllItem( ULONG ClassId = UI_LIST_ELEMENT_CLASS_ID); 

		bool AddItem(CControlUI* pControl); 
		bool AddItemAt(CControlUI* pControl, int iIndex); 
		
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

		CControlUI* GetItemAt(int iIndex) const; 
		INT32 GetItemCount() const; 

		virtual LRESULT NotifyHeaderDragged(CControlUI* pList )
		{
			UNREFERENCED_PARAMETER( pList ); 
			return ERROR_NOT_SUPPORTED; 
		}

	protected:

		//IListContainerCallback *m_pContentCallback; 
		POINT pressed_pos; 
		SIZE pressed_scroll_pos; 
		BOOLEAN button_pressed; 
		HCURSOR previous_cursor; 

		CListBodyExUI* m_pList;
	};
} //namespace DuiLib

#endif //__LIST_EX_H__