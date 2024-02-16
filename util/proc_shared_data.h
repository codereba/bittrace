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
