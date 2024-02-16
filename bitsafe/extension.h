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
 
#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#define EXTENSION_FOLDER L"extensions"

LRESULT load_extensions(); 

LRESULT list_extensions(); 

LRESULT remove_extensions(); 

LRESULT run_extesions(); 

#endif //__EXTENSION_H__