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
 #include "common_func.h"
#include "register.h"

#if 0
LRESULT program_func_ctrl()
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

LRESULT register_user()
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

LRESULT check_user_license()
{
	LRESULT ret = ERROR_SUCCESS; 

	return ret; 
}

	HKEY			hKey;
	DWORD			dwSize;
	TCHAR			szTemp[40];
	CString			sClass,sFlash;
	
	//初始化szTemp
	int i = 0;
	for(szTemp[i] = '0';i <= 40;i++);
	if(RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\AESS"), &hKey) != ERROR_SUCCESS )
	{
		AfxMessageBox("打开AESS注册表出错");
		return "FAIL";
	}
	else
	{
		dwSize = sizeof(szTemp);
		if(RegQueryValueEx(hKey, _T("ClassID"), NULL, NULL, (LPBYTE)szTemp, &dwSize)!=ERROR_SUCCESS)
		{
			AfxMessageBox("打开AESS注册表出错");
			return "FAIL";
		}
		else
			sClass = szTemp;
		if(RegQueryValueEx(hKey, _T("FlashTime"), NULL, NULL, (LPBYTE)szTemp, &dwSize)!=ERROR_SUCCESS)
		{
			sFlash = "";
		}
		else
			sFlash = szTemp;
	}
		
	//结束注册表的设置
	RegCloseKey(hKey);
	if(Flag ==1)
		return sClass;
	else
		return sFlash;

		UpdateData();
	if(m_csFlash == "" || m_csTimeOut == "" || m_csStop == "")
	{
		AfxMessageBox("时限设置不允许为空");
		return;
	}

	HKEY				hKey;
	DWORD				dwSize;

	if(RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\AESS"), &hKey) != ERROR_SUCCESS )
	{
		AfxMessageBox("设置出错");
		return;
	}
	else
	{
		dwSize = m_csFlash.GetLength();
		RegSetValueEx(hKey,"FlashTime",NULL,REG_SZ ,(LPBYTE)(LPCSTR)m_csFlash,dwSize);
	}

	IEconomyDataPtr			pApp(__uuidof(EconomyData));
	_RecordsetPtr			pRs(__uuidof(Recordset));

	long					nStatus;
	CString					sClass;

	sClass = theApp.GetClassID(1);
	try
	{
		nStatus = pApp->SetTimeOut((_bstr_t)sClass,atoi(m_csTimeOut),atoi(m_csStop));
	}
	catch(_com_error &e)
	{
		AfxMessageBox(e.Description());
		return;
	}
	if(nStatus == 0)
		MessageBox("设置成功!","恭喜您");
	else if(nStatus == -1)
		AfxMessageBox("数据库操作失败!");
	else if(nStatus == -2)
		AfxMessageBox("数据库连接失败!");
	

		CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	IEconomyDataPtr			pApp(__uuidof(EconomyData));
	_RecordsetPtr			pRs(__uuidof(Recordset));
	IDispatch				*pDisp;

	long					nStatus;
	CString					sClass,sDelay;

	sDelay = theApp.GetClassID(0);
	sClass = theApp.GetClassID(1);
	try
	{
		nStatus = pApp->QueryTimeOut((_bstr_t)sClass,&pDisp);

		if(nStatus == 0)
			pDisp->QueryInterface(IID__Recordset,(void**)&pRs);
	
	}
	catch(_com_error &e)
	{
		AfxMessageBox(e.Description());
		return FALSE;
	}
	
	char   cTime[10];
	if(nStatus == 0)
	{
		if(pRs->BOF == VARIANT_TRUE && pRs->EndOfFile == VARIANT_TRUE)
			;
		else
		{
			pRs->MoveFirst();
			int nTime = pRs->Fields->GetItem("delayTime")->Value.intVal;
			_itoa(nTime,cTime,10);
			m_csTimeOut = cTime;
			nTime = pRs->Fields->GetItem("overTime")->Value.intVal;
			_itoa(nTime,cTime,10);
			m_csStop = cTime;
		}
	}
	else if(nStatus == -1)
	{
		AfxMessageBox("数据库操作失败，请重新启动程序!");
		return FALSE;
	}

	HKEY					hKey;
	DWORD					dwSize;
	TCHAR					szTemp[50];
	CString					csTime;

	if(RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\AESS"), &hKey) == ERROR_SUCCESS )
	{
		dwSize = sizeof(szTemp);
		if(RegQueryValueEx(hKey, _T("FlashTime"), NULL, NULL, (LPBYTE)szTemp, &dwSize) == ERROR_SUCCESS )
			csTime = szTemp;
		else
			csTime = "20";
	}
	m_csFlash = csTime;
	
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTime_limitDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}



	#include <stdio.h> 
#include <conio.h> 
#include <string.h> 
void main() 
{ 
  unsigned long DBaseIndex, DFeInfo,  DCPUBaseInfo,isSupport; 

  unsigned long m_eax,m_ebx,m_ecx,m_edx; 
  char cCom[13]; 
  char cProStr[49]; 
  char OEMString[13]; 
   
  unsigned int j,flag=0; 
  unsigned short id[6]; 
  _asm {   
    mov eax,0   
    cpuid   
    mov DWORD PTR OEMString,ebx   
    mov DWORD PTR OEMString+4,edx   
  mov DWORD PTR OEMString+8,ecx   
    mov BYTE PTR OEMString+12,0   
    }   

if(strcmp(OEMString,"AuthenticAMD")==0) 
flag=0; 
if(strcmp(OEMString,"CyrixInstead")==0) 
flag=1; 

  if(strcmp(OEMString,"GenuineIntel")==0) 
flag=2; 
   


if(!flag){ 
  _asm{ 
    xor eax, eax 
    cpuid 
    mov DBaseIndex      ,eax 
    mov dword ptr cCom    ,ebx 
     
    mov dword ptr cCom+4  ,edx 
    mov dword ptr cCom+8  ,ecx 
    } 
} 
     

else { 

_asm{ 
        xor eax, eax 
    cpuid 
    mov DBaseIndex      ,eax 
    mov dword ptr cCom    ,ebx 
     mov dword ptr cCom+4  ,ecx //AMD CPU要把ecx改为edx 
    mov dword ptr cCom+8  ,edx //AMD CPU要把edx改为ecx 
} 
    } 








    _asm{ 
    mov eax, 1 
    cpuid 
    mov DCPUBaseInfo, eax 
    mov DWORD PTR id+8,eax 
    mov m_ebx,ebx 
    mov BYTE PTR OEMString+12,0 
    mov  isSupport,edx 
} 

memcpy(&id[0], &m_ebx, 4); 

DFeInfo=m_ebx; 


     
_asm{ 
    mov eax, 3 
    cpuid 
    mov  m_ecx,ecx   
    mov m_edx,edx 
} 


isSupport=(isSupport>>23)&0x1; 

if(isSupport==0){ 
printf("\nInter MMX Not isSupport."); 
getch(); 
return; 
} 
else 
printf("\nInter MMX Support\n\n"); 



memcpy(&id[4], &m_ecx, 4); 
memcpy(&id[2], &m_edx, 4); 

    _asm{ 
    mov eax, 0x80000002 
    cpuid 
    mov dword ptr cProStr    , eax 
    mov dword ptr cProStr + 4  , ebx 
    mov dword ptr cProStr + 8  , ecx 
    mov dword ptr cProStr + 12  ,edx 

    mov eax, 0x80000003 
    cpuid 
    mov dword ptr cProStr + 16  , eax 
    mov dword ptr cProStr + 20  , ebx 
    mov dword ptr cProStr + 24  , ecx 
    mov dword ptr cProStr + 28  , edx 

    mov eax, 0x80000004 
    cpuid 
    mov dword ptr cProStr + 32  , eax 
    mov dword ptr cProStr + 36  , ebx 
    mov dword ptr cProStr + 40  , ecx 
    mov dword ptr cProStr + 44  , edx 
  } 


  cCom[12] = '\0'; //加一个结尾符 
  printf( "CPU厂商:  %s\n", cCom ); 

  printf( "CPU字串:  %s\n", cProStr ); 

  printf( "CPU基本参数: Family:%X  Model:%X  Stepping ID:%X\n", (DCPUBaseInfo & 0x0F00) >> 8, 
      (DCPUBaseInfo & 0xF0) >> 4, DCPUBaseInfo & 0xF ); 

  printf ( "处理器APIC物理标号:%ld\n",DFeInfo ); 

  printf ( "处理器ID号:%u%u%u%u%u%u\n",id[0],id[1],id[2],id[3],id[4],id[5]); 

  printf( "\nok" ); 
  getch(); 
} 

{
	
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	unsigned char buffer[255]={0};
	unsigned long length;
	unsigned long type;
	HKEY hKey;

	RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\dm",&hKey);
	RegQueryValueEx(hKey,"Num",NULL,&type,buffer,&length);
	RegCloseKey(hKey);

	int Count;
	Count = atoi((const char*)buffer);
	if(Count == 0)
	{
		RegCreateKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\dm",&hKey);
		RegSetValueEx(hKey,"Num",0,REG_SZ,(const unsigned char *)"5",strlen("5"));
		RegCloseKey(hKey);
		MessageBox(NULL,"最后用5次!","Hello",MB_OK);			
	}
	else if(Count == 1)
	{
		MessageBox(NULL,"请注册!","Hello",MB_OK);
		return true;
	}
	else
	{
		char buf[255]={0};
		Count -= 1;
		itoa(Count,buf,10);
		CString str;
		str.Format ("最后用%d次!",Count);
		MessageBox(NULL,str,"Hello",MB_OK);
		RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\dm",&hKey);
		RegSetValueEx(hKey,"Num",0,REG_SZ,(const unsigned char*)buf,strlen(buf));
		RegCloseKey(hKey);
	}	


	CMyDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
}

{
	#include "stdafx.h"
#include "硬盘分区卷标号与注册v2读文件.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;


#include <afx.h>
#include <iostream> 

using namespace std;

void main()
{
    DWORD dwIDESerial;
	
CString str,str1, str2,  str4;

//获取
GetVolumeInformation("C:\\",NULL,NULL,&dwIDESerial,NULL,NULL,NULL,NULL);
str.Format( "%X%X",HIWORD(dwIDESerial),LOWORD(dwIDESerial));
str1.Format( "%X",HIWORD(dwIDESerial));
str2.Format( "%X",LOWORD(dwIDESerial));

/*
int number=6338;
str4.Format(_T("%X"),number);
cout<<"str4:"<<str4<<endl;
cout<<str<<endl;
cout<<str1<<endl;
cout<<str2<<endl;
cout<<str3<<endl;
cout<<HIWORD(dwIDESerial)<<endl;
cout<<LOWORD(dwIDESerial)<<endl;
*/
//--
	char data[128]; //
FILE *fp; 
if((fp=fopen("sn.ini","r"))==NULL) 
{ printf("Open File Error!!\n"); 
system("pause");
    exit(1); 
} 
while(!feof(fp)) 
{ fgets(data,128,fp);/////////////////


//：CString str3=("18C267A5");
CString str3=data;




//
if (str==str3)
{cout<<"注册成功"<<endl;} else {cout<<"注册失败 "<<endl;} 

cout<<str<<endl;
cout<<str1<<endl;

}}

}

#endif //0
