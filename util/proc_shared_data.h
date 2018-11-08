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
 
 #pragma once

class CProcessSharedData
{
public:
	CProcessSharedData(void);
	virtual ~CProcessSharedData(void);

	BOOL	Create(LPCTSTR lpName, DWORD dwSize);
	BOOL	Open(LPCTSTR lpName, DWORD dwSize);
	void	Close();

	
	PVOID	Lock(DWORD dwTimeout) const {
		DWORD dwWait = WaitForSingleObject(m_hMutex, dwTimeout);
		return (dwWait == WAIT_OBJECT_0) ? m_lpFileData : NULL;
	}

	void	Unlock() const {
		ReleaseMutex(m_hMutex);
	}
	
	HANDLE	GetLockHandle()  const {
		return m_hMutex;
	}

	PVOID	GetData() const {
		return m_lpFileData;
	}

	DWORD	GetSize() const {
		return m_dwFileSize;
	}

private:
	HANDLE	m_hFileMap;
	LPVOID	m_lpFileData;
	DWORD	m_dwFileSize;
	HANDLE	m_hMutex;
};
