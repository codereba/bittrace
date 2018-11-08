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

#ifndef __TOOL_FRAME_H__
#define __TOOL_FRAME_H__


class tool_frame : public CHorizontalLayoutUI
{
public:
	tool_frame ( CPaintManagerUI* pManager)
	{
		CDialogBuilder builder;
		control_builder cb; 

		do 
		{	
			CContainerUI* tool_tab_ui = static_cast<CContainerUI*>(builder.Create(_T("tool_tab.xml"), NULL, &cb, pManager ));

			ASSERT( pManager != NULL ); 

			if( tool_tab_ui != NULL ) 
			{
				this->AddItem(tool_tab_ui);
			}
			else 
			{
				this->RemoveAllItem();
				break; 
			}

			this->SetName( _T( "tool_tab" ) ); 

		}while( FALSE ); 
	}
}; 

#endif //__TOOL_FRAME_H__