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

#include "stdafx.h"
#include "process_module.h"

Module::Module() : m_hModuleHandle( 0 )
{
    Clear();
}


Module::~Module()
{
    Clear();
}

void Module::Clear()
{
    m_hModuleHandle && CloseHandle( m_hModuleHandle );
    m_hModuleHandle = 0;

    ::ZeroMemory( &m_stModuleEntry32, sizeof( m_stModuleEntry32 ));
    m_stModuleEntry32.dwSize = sizeof( m_stModuleEntry32 );
}