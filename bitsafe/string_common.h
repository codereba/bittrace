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
 
 #ifndef __STRING_COMMON_H__
#define __STRING_COMMON_H__

#ifdef _UNICODE
typedef WCHAR char_t, *pchar_t; 
#define __t( txt ) L##txt
#else
typedef char char_t, *pchar_t; 
#define __t( txt ) txt
#endif

typedef const char_t cchar_t, *pcchar_t; 

#define BITSAFE_EMAIL __t( "codereba@hotmail.com" )
#define BITSAFE_WEBSITE __t( "www.codereba.com" )

#endif //__STRING_COMMON_H__