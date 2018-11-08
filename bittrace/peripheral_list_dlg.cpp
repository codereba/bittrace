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
#include <SetupAPI.h>
#include "peripheral_list.h"
#include "peripheral_list_dlg.h"
#include "action_display.h"
#include "action_ui_support.h"
#include "ui_ctrl.h"

PERIPHERAL_INFORMATION *peripheral_devices = NULL; 
ULONG peripheral_count = 0; 

LRESULT WINAPI init_peripheral_devices_info()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		peripheral_devices = ( PERIPHERAL_INFORMATION* )malloc( sizeof( PERIPHERAL_INFORMATION ) * MAX_PERIPHERAL_COUNT ); 

		if( NULL == peripheral_devices )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_peripheral_devices_info()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( NULL == peripheral_devices )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		free( peripheral_devices ); 
		peripheral_devices = NULL; 

	}while( FALSE );

	return ret; 
}

static CDialogBuilder dlg_builder; 


class peripheral_info_list_item_builder: public IDialogBuilderCallback
{
public:
	peripheral_info_list_item_builder( )
	{ 
	}

public:
	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager ) 
	{
		if( _tcscmp(pstrClass, _T("PeriperaInfolListElement")) == 0 ) 
		{
			return new CListContainerElementExUI(); 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		return NULL;
	}
};


LRESULT periperal_list_dlg::add_peripheral_information( PERIPHERAL_INFORMATION *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	CListContainerElementExUI *item; 
	peripheral_info_list_item_builder list_item_builder; 

	do 
	{
		if( !dlg_builder.GetMarkup()->IsValid() ) {

#define PERIPHERAL_LIST_ITEM_XML_FILE _T( "peripheral_list_item.xml" ) 

			item = static_cast<CListContainerElementExUI*>(dlg_builder.Create(PERIPHERAL_LIST_ITEM_XML_FILE, (UINT)0, &list_item_builder, &m_pm));
		}
		else 
		{
			item = static_cast<CListContainerElementExUI*>(dlg_builder.Create( &list_item_builder, &m_pm ) ); 
		}

		if( NULL != m_pOwner )
		{
			item->SetOwner( m_pOwner ); 
		}

		item->SetManager( &m_pm, peripheral_list, false ); 

		item->SetVisible( true ); 

		item->SetFixedHeight( 16 ); 

		{
			CControlUI *text_name; 
			text_name = m_pm.FindSubControlByName( item, L"name" ); 
		
			text_name->SetText( info->name ); 
		}

		if( true == peripheral_list->AddItem( item ) )
		{
			item = NULL; 
		}

	}while( FALSE ); 

	if( NULL != item ) 
	{
		delete item; 
	}

	return ret; 
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

LRESULT WINAPI test_enum_pnp()
{
	LRESULT ret = ERROR_SUCCESS; 
	PERIPHERAL_INFORMATION infos[ 60 ]; 
	ULONG info_count; 

	ret = enumerate_all_peripherals(infos, 60, &info_count ); 

	return ret; 
}
LRESULT WINAPI set_filtering_peripheral_devices()
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG i; 

	do 
	{
		//ret = reset_filter_peripheral( NULL ); 

		//for( i = 0; i < peripheral_count; i ++ )
		{
			//ret = set_filter_peripheral( NULL ); 
		}

		for( i = 0; i < peripheral_count; i ++ )
		{
			if( FALSE == peripheral_devices[ i ].filtering )
			{
				//ret = reset_filter_peripheral( peripheral_devices[ i ].symbolic_path ); 
			}
		}
	} while ( FALSE ); 

	return ret; 
}