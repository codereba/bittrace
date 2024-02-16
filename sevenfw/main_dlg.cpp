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

#define X_STL_NET 1 

#include "resource.h"

#include "common_func.h"
#include "local_strings.h"
#include "ui_ctrl.h"
#include "ui_ctrl_mgr.h"
#include "main_dlg.h"
#include "USkin.h"

#pragma comment( lib, "USkin.lib")

HMODULE rich_edit_mod = NULL; 
//#define FILE_DEVICE_UNKNOWN             0x00000022
//#define IOCTL_UNKNOWN_BASE              FILE_DEVICE_UNKNOWN
//#define IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME			CTL_CODE(IOCTL_UNKNOWN_BASE, 0x080A, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
INT32 common_init()
{
	INT32 ret; 
	
	//ULONG test; 
	//test = IOCTL_SEVEN_FW_SET_HTTP_FILTER_NAME; 

	//printf( "file name %s \n", __FILE__ ); 

	
	rich_edit_mod  = LoadLibraryA( /*"RICHED20.DLL"*/"msftedit.dll" );
	if( NULL == rich_edit_mod  )
	{
		//RichEditMod = LoadLibraryA("RICHED32.DLL");
		//if( NULL == RichEditMod )
		//{
		return -1;
		//}
	}

	ret = adjust_proc_token();
	if( ret < 0 )
	{
		return -1;
	}

	USkinInit(NULL,NULL,_T("sfw.dat" ) );

	return 0; 
}

INT32 common_uninit()
{
	ASSERT( NULL != rich_edit_mod  ); 

	FreeLibrary( rich_edit_mod  ); 

	USkinExit(); 

	return 0; 
}
// -------------------------------------------------------------------------

#define INST_MUTEX_NAME "Local\\SingleInst"
HANDLE hInstMutex = NULL; 

//class B{
//
//public:
//
//	int m_iNum;
//
//	virtual void foo();
//
//};
//
//class D:public B{
//
//public:
//
//	char *m_szName[100];
//
//};
//
//
//
//void func(B *pb){
//
//	D *pd1 = static_cast<D *>(pb);
//
//	D *pd2 = dynamic_cast<D *>(pb);
//
//}
//
////�� ����Ĵ�����У����pbָ��һ��D���͵Ķ���pd1��pd2��һ���ģ����Ҷ�������ָ��ִ��D���͵��κβ������ǰ�ȫ�ģ����ǣ����pbָ�����һ�� B���͵Ķ�����ôpd1����һ��ָ��ö����ָ�룬��������D���͵Ĳ������ǲ���ȫ�ģ������m_szName������pd2����һ����ָ�롣����Ҫע �⣺BҪ���麯�����������������static_cast��û��������ơ�������������ʱ���ͼ����Ҫ����ʱ������Ϣ���������Ϣ�洢������麯���� �������麯�����ĸ����ϸ�ɼ�<Inside c++ object model>���У�ֻ�ж������麯����������麯������û�ж����麯��������û���麯�����ġ�
//
//class A{
//
//public:
//
//	int m_iNum;
//
//	virtual void f(){}
//
//};
//
//
//
//class E:public A{
//
//};
//
//
//
//class F:public A{
//
//};
//
//
//
//void foo(){
//
//	E *pb = new E;
//
//	pb->m_iNum = 100;
//
//	F *pd1 = static_cast<F *>(pb); //copile error
//
//	F *pd2 = dynamic_cast<F *>(pb); //pd2 is NULL
//
//	delete pb;
//
//}

#include <strsafe.h>
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	INT32 ret; 

	InitCommonControls(); 
	common_init(); 

	hInstMutex = CreateMutex( NULL, FALSE, INST_MUTEX_NAME ); 
	if( hInstMutex == NULL || ( hInstMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS ) )
	{
		MessageBox( NULL, "7�����ǽֻ������һ��ʵ������\n", NULL, MB_OK ); 
		ExitProcess( 0 ); 
		return TRUE; 
	}

	ret = init_all_ui_ctrls(); 

	if( ret < 0 )
	{
		return ret; 
	}

	main_dlg dlg;
	dlg.DoModal();

	release_ui_ctrls(); 
	common_uninit(); 

	return 0;
}
