#ifndef _PROCESS_IO_COUNTERS_H_
#define _PROCESS_IO_COUNTERS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlstr.h>

class ProcessIOCounters  
{
    public:
	    ProcessIOCounters();
	    virtual ~ProcessIOCounters();

        const CString& GetReadOperationCount() const { return m_csReadOprCount; }
        CString& GetReadOperationCount() { return m_csReadOprCount; }

        const CString& GetWriteOperationCount() const { return m_csWriteOprCount; }
        CString& GetWriteOperationCount() { return m_csWriteOprCount; }

        const CString& GetOtherOperationCount() const { return m_csOtherOprCount; };
        CString& GetOtherOperationCount() { return m_csOtherOprCount; }

        const CString& GetReadTransferCount() const { return m_csReadTrnsCount; }
        CString& GetReadTransferCount() { return m_csReadTrnsCount; }

        const CString& GetWriteTransferCount() const { return m_csWriteTrnsCount; }
        CString& GetWriteTransferCount() { return m_csWriteTrnsCount; }

        const CString& GetOtherTransferCount() const { return m_csOtherTrnsCount; }
        CString& GetOtherTransferCount() { return m_csOtherTrnsCount; }

        LRESULT GetProcessIoCounters( HANDLE ProcessHandle );

    private:

        CString m_csReadOprCount;
        CString m_csWriteOprCount;
        CString m_csOtherOprCount;
        CString m_csReadTrnsCount;
        CString m_csWriteTrnsCount;
        CString m_csOtherTrnsCount;
};

#endif // _PROCESS_IO_COUNTERS_H_