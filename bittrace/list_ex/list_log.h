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

#ifndef __UI_LIST_LOG_H__
#define __UI_LIST_LOG_H__

#include <string>

#define WM_EVENT_LOG_RECEIVE ( WM_USER + 2001 ) 
#define WM_EVENT_LOG_RECEIVE_DONE ( WM_USER + 2002 ) 

typedef enum _EVENT_BROWSER_MODE
{
	BROWSER_SIMPLE_MODE, 
	BROWSER_VERBOSE_MODE, 
} EVENT_BROWSER_MODE, *PEVENT_BROWSER_MODE; 

extern EVENT_BROWSER_MODE browser_mode; 

#ifdef _UI_DEBUG
#ifdef EVENT_INFO_LAYOUT_CONTAINER_NAME 
#undef EVENT_INFO_LAYOUT_CONTAINER_NAME 
#endif //EVENT_INFO_LAYOUT_CONTAINER_NAME 
#define EVENT_INFO_LAYOUT_CONTAINER_NAME _T("list_node")
#else
#define EVENT_INFO_LAYOUT_CONTAINER_NAME _T("main_layout")
#endif //_UI_DEBUG

#ifdef _UI_DEBUG
#define LIST_ITEM_NORMAL_HEIGHT 50
#else
#define LIST_ITEM_NORMAL_HEIGHT 16
#endif //_UI_DEBUG

#define LIST_ITEM_SELECTED_HEIGHT 16

#define CACHE_ITEM_PAGE_COUNT 20

#define DEFAULT_ICON_COLUMN_WIDTH 16
#define DEFAULT_ACTION_NAME_COLUMN_WIDTH 110
#define ACTION_ID_COLUMN_WIDTH 50
#define DEFAULT_TIME_COLUMN_WIDTH 132
#define PROCESS_ID_COLUMN_WIDTH 42
#define THREAD_ID_COLUMN_WIDTH 42
#define EXE_FILE_PATH_COLUMN_WIDTH 350
#define OBJECT_COLUME_WIDTH 490
#define DESC_COLUMN_WIDTH 600
#define DATA_COLUMN_WIDTH 200
#define CALL_STACK_COLUMN_WIDTH 230
#define RESULT_COLUMN_WIDTH 120

#define TEST_LIST_ITEM_COUNT 200240

#define LIST_ROW_WIDTH ( ULONG )( DEFAULT_ICON_COLUMN_WIDTH + DEFAULT_ICON_COLUMN_WIDTH + DEFAULT_ACTION_NAME_COLUMN_WIDTH + ACTION_ID_COLUMN_WIDTH + DEFAULT_TIME_COLUMN_WIDTH + PROCESS_ID_COLUMN_WIDTH + THREAD_ID_COLUMN_WIDTH + EXE_FILE_PATH_COLUMN_WIDTH + OBJECT_COLUME_WIDTH + DESC_COLUMN_WIDTH + DATA_COLUMN_WIDTH + CALL_STACK_COLUMN_WIDTH + RESULT_COLUMN_WIDTH )

extern ULONG log_list_def_column_width[]; 

namespace DuiLib
{
	//// tString is a TCHAR std::string
	//typedef std::basic_string<TCHAR> tString;

//struct FriendListItemInfo
//{
//	bool folder;
//	bool empty;
//	tString id;
//	tString logo;
//	tString nick_name;
//	tString description;
//};

	//typedef struct ACTION_INFO_CONTEXT 
	//{
	//	LPCSTR sym_path; 
	//} ACTION_INFO_CONTEXT, *PACTION_INFO_CONTEXT; 

LRESULT WINAPI close_info_dlgs(); 

class CListLogUI : public CListExUI, 
	//public IDialogBuilderCallback, 
	public IListHeaderCallbackUI
{
public:
	enum {SCROLL_TIMERID = 10};

	CListLogUI();

	~CListLogUI();

	LRESULT _Init(); 
	LRESULT _SetLogCount( ULONG filtered_count, ULONG all_count, ULONG notify_count, ULONG time ); 
	LRESULT SetLogCount( ULONG filtered_count, ULONG all_count, ULONG notify_count, ULONG time ); 
	LRESULT RefreshEventListUI(); 

	LRESULT set_auto_scroll( BOOLEAN auto_scroll ); 
	LRESULT clear_events(); 
	LRESULT hilight_by_key_word(); 
	LRESULT search_by_key_word( LPCWSTR key_words, BOOLEAN up_direction ); 
	LRESULT FilterEvents(); 

	CStdString get_cur_sel_obj_path()
	{
		CStdString obj_path; 
		CControlUI *ListItem; 
		CControlUI *SubItem; 

		do
		{

			ListItem = GetCurSelItem(); 

			if( ListItem == NULL )
			{
				break; 
			}

			SubItem = m_pManager->FindSubControlByName( ListItem, L"event_obj" ); 
			if( SubItem == NULL )
			{
				break; 
			}

			obj_path = SubItem->GetText(); 

		}while( FALSE ); 
	
		return obj_path; 
	}

	bool GetScrollSelect(); 
	void SetScrollSelect(bool bScrollSelect); 
	CControlUI *GetCurSelItem(); 
	LRESULT get_select_event_info(); 

	//bool AddItem(CControlUI* pControl);

	//bool AddItemAt(CControlUI* pControl, int iIndex);

	//bool RemoveItem(CControlUI* pControl);

	//bool RemoveItemAt(int iIndex);

	//void RemoveAllItem(); 

	//INT32 GetItemIndex(CControlUI* pControl) const;
	//
	//bool SetItemIndex(CControlUI* pControl, int iIndex);
	//
	//INT32 GetItemCount() const; 

	LPCTSTR GetClass(); 

	CListBodyUI* GetList() const; 

	void DoEvent(TEventUI& event);

	//void SetPos(RECT rc); 
	//LRESULT AddNode( sys_action_info_notify *action, ACTION_INFO_CONTEXT *InfoConext ); 
	//void RemoveNode(ListNode* node);

	//void SetChildVisible(ListNode* node, bool visible);

	//bool CanExpand(ListNode* node) const;

	bool SelectListItem(int iIndex); 
	VOID ResetItemSelectUI( PVOID item ); 
	LRESULT FindItemSelected( PVOID *ItemSelected ); 

	void DoPaint(HDC hDC, const RECT& rcPaint);

	//virtual CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager )
	//{
	//	if (_tcsicmp( pstrClass, _T( "ListSubElementEx" ) ) == 0 )
	//	{
	//		return new CListSubElementExUI(); 
	//	}
	//	else if (_tcsicmp( pstrClass, _T( "ListContainerElementEx" ) ) == 0 )
	//	{
	//		return new CListContainerElementExUI(); 
	//	}

	//	return NULL; 
	//}

	LRESULT OnMouseMove( TEventUI& event ); 
	LRESULT OnMouseHover( TEventUI& event ); 

	virtual LRESULT NotifyHeaderDragged(CControlUI* pList ); 
	LRESULT TestPrepare(); 

private:
	/******************************************
	the root to manage the all other node, to 
	hide it, or show it.
	
	there still have other method to do this:
	use one container to contain other node, 
	and set the property visible of the container.

	but use the root node can manage other state.
	******************************************/
    CDialogBuilder m_dlgBuilder; 

public:
	//CListHeaderItemUI* list_header_items[ 10 ]; 
	INT32 m_CurSelCacheIndex; 
};

} // DuiLib

#endif // __UI_LIST_LOG_H__