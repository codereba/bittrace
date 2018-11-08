/*
 *
 * Copyright 2010 JiJie Shi
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

//global string define
#define BITSAFE_EMAIL __t( "codereba@hotmail.com" )
#define BITSAFE_WEBSITE __t( "www.codereba.com" )

#endif //__STRING_COMMON_H__