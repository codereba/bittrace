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

#ifndef __R3_RW_LOCK_H__
#define __R3_RW_LOCK_H__

class CReadWriteLock
{
public:
	CReadWriteLock(void);
	virtual ~CReadWriteLock(void);

	void LockReader();
	void UnlockReader();

	void LockWriter();
	void UnlockWriter();

	BOOLEAN TryLockWriter()
	{
		return FALSE; 
	}

private:
	CReadWriteLock(const CReadWriteLock &cReadWriteLock);
	const CReadWriteLock& operator=(const CReadWriteLock &cReadWriteLock);

	void IncrementReaderCount();
	void DecrementReaderCount();

	HANDLE m_hWriterEvent;
	HANDLE m_hNoReadersEvent;
	int m_nReaderCount;

	CRITICAL_SECTION m_csLockWriter;
	CRITICAL_SECTION m_csReaderCount;
};

#endif //__R3_RW_LOCK_H__