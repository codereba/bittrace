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

#include "StdAfx.h"
#include "common_func.h"
#include "acl_define.h"
#include "event_store.h"
#include "process_property_dlg.h"
#include "process_info.h"

static CDialogBuilder dlg_builder; 

LRESULT process_properies_dlg::set_event( PVOID event )
{
	this->event = event; 

	return ERROR_SUCCESS; 
}

/**********************************************************************
<HorizontalLayout >
<List name="image_list" bkcolor="#FFFFFFFF" inset="0,0,0,0" itemshowhtml="true" vscrollbar="true" hscrollbar="true" headerbkimage="file='list_header_bg.png'" itemalign="left" itemtextpadding="5,0,0,0" itembkcolor="#FFE2DDDF" itemaltbk="true" menu="true">
<ListHeader height="24" menu="true">
<ListHeaderItem text="模块"  font="10" width="100" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="基地址" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="大小" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
<ListHeaderItem text="路径" font="10" width="240" hotimage="file='list_header_hot.png'" pushedimage="file='list_header_pushed.png'" sepimage="file='list_header_sep.png'" sepwidth="1"/>
</ListHeader>
</List>
</HorizontalLayout>
**********************************************************************/

#include "module_info.h"
LRESULT process_properies_dlg::load_process_information() 
{
	LRESULT ret = ERROR_SUCCESS; 
	boolean _ret; 
	Process *process_info; 
	CStdString text; 
	WCHAR _text[ MAX_PATH ]; 
	CControlUI *ctrl; 
	CListUI *list; 

	PROCESS_MODULE_INFO_TABLE *proc_info = NULL; 
	ULONG process_id;
	r3_action_notify_buf *action; 

	try
	{
	do 
	{
		if( event == NULL )
		{
			break; 
		}

		action = ( r3_action_notify_buf* )event; 

		process_id = action->action.action.ctx.proc_id;

		ret = _get_proc_module_info( process_id, 
			&action->action.action.ctx.time, 
			&proc_info ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ASSERT( proc_info != NULL ); 

		process_info = &proc_info->info; 

		if( *action->action.action.ctx.proc_name != L'\0' )
		{
			ctrl = m_pm.FindControl( L"process_icon" ); 
			ASSERT( ctrl != NULL ); 
			HRESULT hr; 
#define MAX_BK_FILE_STRING 512
			WCHAR szBuf[ MAX_BK_FILE_STRING ]; 
			//ULONG ccb_ret_len; 
			action->action.action.ctx.proc_name[ ARRAYSIZE( action->action.action.ctx.proc_name ) - 1 ] = L'\0'; 

			//_ret = convert_native_path_to_dos( action->action.action.ctx.proc_name, 
			//	action->action.action.ctx.proc_name_len, 
			//	file_dos_path, 
			//	ARRAYSIZE( file_dos_path ), 
			//	&ccb_ret_len ); 

			//*file_dos_path = L'\0'; 
			//hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"filepath=\'%s\' source=\'0,0,32,32\' mask=\'0xffff00ff\'", ctx->proc_name ); 

			//}
			//else
			{
				hr = StringCbPrintfW( szBuf, sizeof( szBuf ), L"filepath=\'%s\' source=\'0,0,32,32\' mask=\'0xffff00ff\'", action->action.action.ctx.proc_name ); 
			}

			if( SUCCEEDED( hr ) )
			{
				ctrl->SetBkImage( szBuf ); 
			}
		}

		{
			CString file_name; 
			
			file_name = wcsrchr( process_info->GetFullPath(), L'\\' ); 

			ctrl = m_pm.FindControl( L"product_name" ); 
			ASSERT( ctrl != NULL ); 

			ctrl->SetText( file_name.GetBuffer() ); 
			ctrl->SetToolTip( file_name.GetBuffer() ); 
		}

		ctrl = m_pm.FindControl( L"image_file_path" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( process_info->GetFullPath() ); 
		ctrl->SetToolTip( process_info->GetFullPath() ); 

		ctrl = m_pm.FindControl( L"process_id" ); 
		ASSERT( ctrl != NULL ); 

		text.SmallFormat( L"%u", action->action.action.ctx.proc_id ); 
		ctrl->SetText( text ); 
		ctrl->SetToolTip( text ); 


		ctrl = m_pm.FindControl( L"user_name" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( process_info->GetUserName() ); 
		ctrl->SetToolTip( process_info->GetUserName() ); 

		//ctrl = m_pm.FindControl( L"process_parent_id" ); 
		//ASSERT( ctrl != NULL ); 

		//text.SmallFormat( L"%u", 0 ); 
		//ctrl->SetText( text ); 

		ctrl = m_pm.FindControl( L"process_command" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( process_info->GetProcessCommandLine() ); 
		ctrl->SetToolTip( process_info->GetProcessCommandLine() ); 

		ctrl = m_pm.FindControl( L"start_time" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( process_info->m_ProcTimes.GetProcessStartTime() ); 
		ctrl->SetToolTip( process_info->m_ProcTimes.GetProcessStartTime() ); 

		//if( proc_info->duration.start_time.QuadPart != 0 )
		//{
		//	ret = sprint_local_time( _text, ARRAYSIZE( _text ), proc_info->duration.start_time ); 
		//	
		//	if( ret == ERROR_SUCCESS )
		//	{
		//		ctrl->SetText( _text ); 
		//		ctrl->SetToolTip( _text ); 
		//	}
		//}
		//else
		//{
		//	ctrl->SetText( L"" ); 
		//	ctrl->SetToolTip( L"" ); 
		//}

		ctrl = m_pm.FindControl( L"end_time" ); 
		ASSERT( ctrl != NULL ); 

		ctrl->SetText( process_info->m_ProcTimes.GetProcessExitTime() ); 
		ctrl->SetToolTip( process_info->m_ProcTimes.GetProcessExitTime() ); 

		//if( proc_info->duration.end_time.QuadPart != 0 )
		//{
		//	ret = sprint_local_time( _text, ARRAYSIZE( _text ), proc_info->duration.end_time ); 

		//	if( ret == ERROR_SUCCESS )
		//	{
		//		ctrl->SetText( _text ); 
		//		ctrl->SetToolTip( _text ); 
		//	}
		//}
		//else
		//{
		//	ctrl->SetText( L"" ); 
		//	ctrl->SetToolTip( L"" ); 
		//}

		ctrl = m_pm.FindControl( L"session_id" ); 
		ASSERT( ctrl != NULL ); 

		ret = StringCchPrintfW( _text, ARRAYSIZE( _text ), L"%u", process_info->GetSessionId() ); 

		ctrl->SetText( _text ); 
		ctrl->SetToolTip( _text ); 

		if( 0 != process_info->GetParentProcessId() )
		{
			ctrl = m_pm.FindControl( L"process_parent_id" ); 
			ASSERT( ctrl != NULL ); 

			ret = StringCchPrintfW( _text, ARRAYSIZE( _text ), L"%u", process_info->GetParentProcessId() ); 

			ctrl->SetText( _text ); 
			ctrl->SetToolTip( _text ); 
		}

		list = static_cast< CListUI* >( m_pm.FindControl( L"image_list" ) ); 
		ASSERT( list ); 

		//process_info->EnumModules(); 

		{
			//Module *module_info; 
			//std::vector<Module*>::iterator it;  
			MODULE_INFO_ITERATOR it; 
			MODULE_INFO *module_info; 
			LPCWSTR _file_name; 

			for( it = proc_info->modules.modules.begin(); 
				it != proc_info->modules.modules.end(); 
				it ++ )
			{

				CListContainerElementExUI *pListElement; 

				do 
				{
					module_info = *it; 

					if( module_info->duration.end_time.QuadPart != 0 )
					{
						log_trace( ( MSG_IMPORTANT, "this module %ws is already unloaded\n", module_info->file_name ) );  
						break; 
					}

					if( !dlg_builder.GetMarkup()->IsValid() ) 
					{
#define ACTION_FILTER_LIST_ITEM_XML_FILE _T( "image_info_item.xml" ) 
						pListElement = static_cast<CListContainerElementExUI*>( dlg_builder.Create(ACTION_FILTER_LIST_ITEM_XML_FILE, (UINT)0, NULL, &m_pm ));
					}
					else 
					{
						pListElement = static_cast<CListContainerElementExUI*>( dlg_builder.Create( NULL, &m_pm ));
					}

					if( pListElement == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					pListElement->SetOwner( list ); 

					pListElement->SetManager( &m_pm, list, false ); 

					pListElement->SetVisible( true ); 

#define MODULE_INFO_LIST_ITEM_WIDTH 300
#define MODULE_INFO_LIST_ITEM_HEIGTH 20

#define MODULE_INFO_LAYOUT_CONTAINER_NAME L"image_info_layout"

					CContainerUI* main_layout = static_cast< CContainerUI* >( m_pm.FindSubControlByName( pListElement, MODULE_INFO_LAYOUT_CONTAINER_NAME ) ); 

					if( main_layout != NULL )
					{
						main_layout->SetFixedWidth( MODULE_INFO_LIST_ITEM_WIDTH ); 
						main_layout->SetVisible( true );
					}

					pListElement->SetFixedHeight( MODULE_INFO_LIST_ITEM_HEIGTH ); 

					ctrl = static_cast< CControlUI* >( m_pm.FindSubControlByName( pListElement, L"image" ) ); 
					ASSERT( ctrl != NULL ); 

					_file_name = wcsrchr( module_info->file_name, L'\\');  
					ctrl->SetText( _file_name == NULL ? L"" : _file_name ); 
					ctrl->SetToolTip( _file_name == NULL ? L"" : _file_name ); 

					ctrl = static_cast< CControlUI* >( m_pm.FindSubControlByName( pListElement, L"base_addr" ) ); 
					ASSERT( ctrl != NULL ); 
					text.SmallFormat( L"%p", module_info->base_addr ); 
					ctrl->SetText( text ); 
					ctrl->SetToolTip( text ); 

					ctrl = static_cast< CControlUI* >( m_pm.FindSubControlByName( pListElement, L"size" ) ); 
					ASSERT( ctrl != NULL ); 
					text.SmallFormat( L"%u", module_info->size ); 
					ctrl->SetText( text ); 	
					ctrl->SetToolTip( text ); 

					ctrl = static_cast< CControlUI* >( m_pm.FindSubControlByName( pListElement, L"path" ) ); 
					ASSERT( ctrl != NULL ); 
					ctrl->SetText( module_info->file_name ); 
					ctrl->SetToolTip( module_info->file_name ); 

					_ret = list->AddSubItem( pListElement, UI_LIST_CONTAINER_ELEMENT_CLASS_ID ); 
					if( _ret == false )
					{
						ASSERT( FALSE && "add module information to list error" ); 
					}

				}while( FALSE ); 
			}
		}

	}while( FALSE ); 
	}
	catch( ... )
	{
		ULONG test = 0; 
	}

	if( NULL != proc_info )
	{
		_release_proc_module_info( proc_info ); 
	}

	return ret; 
}