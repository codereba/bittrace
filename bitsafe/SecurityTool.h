
#if !defined(AFX_SECURITYTOOL_H__3293B54B_D7E3_4DDF_AEF2_08461B3AA3F5__INCLUDED_)
#define AFX_SECURITYTOOL_H__3293B54B_D7E3_4DDF_AEF2_08461B3AA3F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//author: yy2better
//email: yy2better@126.com
class CSecurityTool  
{
public:
	CSecurityTool();
	virtual ~CSecurityTool();

public:
	//�õ���ǰ�����û�
	static BOOL GetCurrProcessUser(CString& strName);

	//��ȡXP��½�û�
	static BOOL GetLogUserXP(CString& strName);
	//��ȡwin2000��½�û�
	static BOOL GetLogUser2K(CString& strName);

private:
	static BOOL GetProcessUser(DWORD dwProcessID, TCHAR *szUserName, DWORD nNameLen);	
};

#endif // !defined(AFX_SECURITYTOOL_H__3293B54B_D7E3_4DDF_AEF2_08461B3AA3F5__INCLUDED_)
