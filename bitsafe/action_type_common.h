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

#ifndef __ACTION_TYPE_COMMON_H__
#define __ACTION_TYPE_COMMON_H__ 

#define _MAX_CLASS_NAME_LEN 64 
#define _MAX_DESC_LEN 256
#define NAME_PARAM_INDEX 1
#define _MAX_FILE_NAME_LEN 260
#define _MAX_COM_NAME_LEN 260 
#define _MAX_REG_VALUE_LEN	( 1024 )
#define _MAX_REG_PATH_LEN 1024 //2048
#define _MAX_URL_LEN 1024 
#define _MAX_TEXT_LEN _MAX_REG_PATH_LEN 
#define MAX_RULE_PARAM_NUM 5
#define MAX_DOMAIN_NAME_LEN 256
#define MAX_DATA_FLOW_COUNT 4096

#define STATUS_UNKNOWN 0xffffffff //( 0xE0FA0011 )

#endif //__ACTION_TYPE_COMMON_H__