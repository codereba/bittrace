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

#ifndef __PROCESS_TIMES_H__
#define __PROCESS_TIMES_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlstr.h>
class ProcessTimes  
{
    public:

	    ProcessTimes();
	    virtual ~ProcessTimes();

        const CString& GetProcessStartTime() const { return m_csProcessStartTime; }
        CString& GetProcessStartTime() { return m_csProcessStartTime; }

        const CString& GetProcessExitTime() const { return m_csProcessExitTime; }
        CString& GetProcessExitTime() { return m_csProcessExitTime; }

        const CString& GetProcessKernelTime() const { return m_csProcessKernelTime; }
        CString& GetProcessKernelTime() { return m_csProcessKernelTime; }

        const CString& GetProcessUserTime() const { return m_csProcessUserTime; }
        CString& GetProcessUserTime() { return m_csProcessUserTime; }

        LRESULT GetProcessTimes( HANDLE hProcess );

    private:
        CString m_csProcessStartTime;
        CString m_csProcessExitTime;
        CString m_csProcessKernelTime;
        CString m_csProcessUserTime;
};

#endif // __PROCESS_TIMES_H__
