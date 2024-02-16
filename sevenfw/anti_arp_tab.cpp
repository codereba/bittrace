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

 
#include "common_func.h"
#include "ui_ctrl.h"
#include "anti_arp_tab.h"

INT32 anti_arp_ui_output( ULONG ui_action_id, 
						 INT32 ret_val, 
								PBYTE data, 
								ULONG length, 
								PVOID param )
{
	INT32 ret; 
	anti_arp_tab *output_wnd; 

	ret = 0; 
	switch( ui_action_id )
	{
	case ANTI_ARP_INIT:
		break; 
	case ANTI_ARP_GET_ARP_INFO:

		output_wnd = ( anti_arp_tab* )param; 

		ret = output_wnd->output_arp_info( ( PARP_HOST_MAC_LISTS ) data ); 

		break; 
	}

	return ret; 
}