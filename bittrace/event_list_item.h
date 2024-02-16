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
 */

#ifndef __EVENT_LIST_ITEM_H__
#define __EVENT_LIST_ITEM_H__

typedef struct _event_list_sub_items
{
	CControlUI *proc_icon; 
	CControlUI *event_icon; 
	CControlUI *event_name; 
	CControlUI *time; 
	CControlUI *action_id; 
	CControlUI *proc_id; 
	CControlUI *thread_id; 
	CControlUI *exe_file; 
	CControlUI *event_desc; 
	CControlUI *event_obj; 
	CControlUI *result; 
	CControlUI *data; 
	CControlUI *call_stack; 
} event_list_sub_items, *pevent_list_sub_items; 

class CEventListItemUI : public CListContainerElementExUI
{
public:
	CEventListItemUI() 
		: filtered_index( -1 ), 
		  real_index( -1 ) 
	{
		items.action_id = NULL; 
		items.call_stack = NULL; 
		items.data = NULL; 
		items.event_desc = NULL; 
		items.event_icon = NULL; 
		items.event_name = NULL; 
		items.event_obj = NULL; 
		items.exe_file = NULL; 
		items.proc_icon = NULL; 
		items.proc_id = NULL; 
		items.result = NULL; 
		items.thread_id = NULL; 
		items.time = NULL; 
	}

	~CEventListItemUI() 
	{

	}

	LPCTSTR GetClass() const
	{
		return UI_EVENT_LIST_ELEMENT_CLASS_NAME; 
	}

	//ULONG GetClassId() const
	//{
	//	return UI_EVENT_LIST_ELEMENT_CLASS_ID;
	//}	

	LONG get_filtered_index()
	{
		return filtered_index; 
	}

	LONG get_real_index()
	{
		return real_index; 
	}

	VOID set_filtered_index( LONG index )
	{
		filtered_index = index; 
	}

	VOID set_real_index( LONG index )
	{
		real_index = index; 
	}

	bool IsSelected() const; 

	bool Select(bool bSelect); 

	LRESULT init_sub_item_holder(); 

public: 
	event_list_sub_items items; 

protected: 
public: 
	LONG real_index; 
	LONG filtered_index; 
};

#endif //__EVENT_LIST_ITEM_H__