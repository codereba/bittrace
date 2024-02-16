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
 
#include "_stdafx.h"
#include "text_msg_box.h"

LRESULT dbg_show_text_msg( HWND parent, LPCTSTR msg, LPCTSTR title, ULONG mode )
{
	dlg_ret_state ret_state; 

	return show_text_msg( parent, msg, &ret_state, title, mode ); 
}