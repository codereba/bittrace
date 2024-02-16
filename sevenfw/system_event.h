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

#ifndef __SYSTEM_EVENT_H__
#define __SYSTEM_EVENT_H__

LRESULT read_system_event(); 

BOOL add_event_source( LPCWSTR csName, DWORD dwCategoryCount); 
BOOL write_event_log(LPCWSTR szEventMsg, WORD wEventType, LPCWSTR szSourceName); 

#endif //__SYSTEM_EVENT_H__