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

#ifndef __PROCESS_MODULE_H__
#define __PROCESS_MODULE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "file_version_info.h"
#include <TLHELP32.H>

class Module
{
    public:

        Module();
        virtual ~Module();

        void SetModuleEntry32( const MODULEENTRY32& stModuleEntry32_i )
        {
            m_stModuleEntry32 = stModuleEntry32_i;
        }

        const MODULEENTRY32& GetGetModuleEntry32() const
        {
            return m_stModuleEntry32;
        }

        const HMODULE& GetModuleHandle() const { return m_hModuleHandle; }

        void Clear();

    private:
        HMODULE m_hModuleHandle;

        MODULEENTRY32 m_stModuleEntry32;
        FileVersionInfo m_fviVersion;

};

#endif //__PROCESS_MODULE_H__
