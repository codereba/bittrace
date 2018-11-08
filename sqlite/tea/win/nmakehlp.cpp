/* ----------------------------------------------------------------------------
 * nmakehlp.c --
 *
 *	This is used to fix limitations within nmake and the environment.
 *
 * Copyright (c) 2002 by David Gravereaux.
 * Copyright (c) 2003 by Patrick Thoyts
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: nmakehlp.c,v 1.2 2003/10/01 14:56:00 patthoyts Exp $
 * ----------------------------------------------------------------------------
 */
#include <windows.h>
#include <stdio.h>
#include "sqlite3.h"

#include <vector>
#include <conio.h>

using namespace std; 

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "kernel32.lib")

/* protos */
int CheckForCompilerFeature (const char *option);
int CheckForLinkerFeature (const char *option);
int IsIn (const char *string, const char *substring);
DWORD WINAPI ReadFromPipe (LPVOID args);
int GetVersionFromHeader(const char *tclh, const char *tkh);

/* globals */
typedef struct {
    HANDLE pipe;
    char buffer[1000];
} pipeinfo;

pipeinfo Out = {INVALID_HANDLE_VALUE, '\0'};
pipeinfo Err = {INVALID_HANDLE_VALUE, '\0'};


#pragma comment( lib, "sqlite.lib" )

typedef int (*sqlite3_callback)(void*,int,char**, char**);

//sqlite3的回调函数
//sqlite 每查到一条记录,就调用一次这个回调
int LoadMyInfo( void * para, int n_column, char ** column_value, char ** column_name )
{
    //para是你在 sqlite3_exec 里传入的 void * 参数

    //通过para参数,你可以传入一些特殊的指针(比如类指针、结构指针),然后在这里面强制转换成对应的类型(这里面是void*类型,必须强制转换成你的类型才可用)。然后操作这些数据

    //n_column是这一条记录有多少个字段 (即这条记录有多少列)

    // char ** column_value 是个关键值,查出来的数据都保存在这里,它实际上是个1维数组(不要以为是2维数组),每一个元素都是一个 char * 值,是一个字段内容(用字符串来表示,以\0结尾)

    //char ** column_name 跟 column_value是对应的,表示这个字段的字段名称

    //这里,我不使用 para 参数。忽略它的存在.

    int i;

    printf( "记录包含 %d 个字段\n", n_column );

    for( i = 0 ; i < n_column; i ++ )
    {
        printf( "字段名:%s ?> 字段值:%s\n", column_name[i], column_value[i] );
    }

    printf( "------------------\n" );

    return 0;
}


int test_sqlite()
{
    sqlite3 * db;
    int result;
    char * errmsg = NULL;

    result = sqlite3_open( "c:\\Dcg_database.db", &db );
    if( result != SQLITE_OK )
    {
        //数据库打开失败
        return -1;
    }

    //数据库操作代码
    //创建一个测试表,表名叫 MyTable_1,有2个字段： ID 和 name。其中ID是一个自动增加的类型,以后insert时可以不去指定这个字段,它会自己从0开始增加
    result = sqlite3_exec( db, "create table MyTable_1( ID integer primary key autoincrement, name nvarchar(32) )", NULL, NULL, &errmsg );
    if(result != SQLITE_OK )
    {
        printf( "创建表失败,错误码:%d,错误原因:%s\n", result, errmsg );
    }
    //插入一些记录
    result = sqlite3_exec( db, "insert into MyTable_1( name ) values ( '走路' )", 0, 0, &errmsg );
    if(result != SQLITE_OK )
    {
        printf( "插入记录失败,错误码:%d,错误原因:%s\n", result, errmsg );
    }

    //开始查询数据库

    result = sqlite3_exec( db, "select * from MyTable_1", LoadMyInfo, NULL, &errmsg );

    //关闭数据库

    sqlite3_close( db );

    return 0;
}

int	test_sqlite2()
{
	sqlite3	* db;
	int	result;
	char * errmsg =	NULL;

	char **dbResult; //是 char ** 类型,两个*号

	int	nRow, nColumn;

	int	i ,	j;

	int	index;

	result = sqlite3_open( "c:\\Dcg_database.db",	&db	);
	if(	result != SQLITE_OK	)
	{
		//数据库打开失败
		return -1;
	}

	//数据库操作代码

	//假设前面已经创建了 MyTable_1 表

	//开始查询,传入的 dbResult	已经是 char	**,这里又加了一个 & 取地址符,传递进去的就成了	char ***

	result = sqlite3_get_table(	db,	"select * from	MyTable_1", &dbResult,	&nRow, &nColumn, &errmsg );

	if(	SQLITE_OK == result	)
	{
		//查询成功
		index =	nColumn; //前面说过	dbResult 前面第一行数据是字段名称,从 nColumn 索引开始才是真正的数据
		printf(	"查到%d条记录\n",	nRow );

		for( i = 0;	i <	nRow ; i++ )
		{
			printf(	"第 %d	条记录\n",	i+1	);
			for( j = 0 ; j < nColumn; j++ )
			{
				printf(	"字段名:%s	?> 字段值:%s\n", dbResult[j], dbResult[index] );
				++index; //	dbResult 的字段值是连续的,从第0索引到第 nColumn - 1索引都是字段名称,从第 nColumn 索引开始,后面都是字段值,它把一个二维的表(传统的行列表示法)用一个扁平的形式来表示
			}
			printf(	"-------\n" );

		}
	}

	//到这里,不论数据库查询是否成功,都释放 char**	查询结果,使用 sqlite 提供的功能来释放
	sqlite3_free_table(	dbResult );
	//关闭数据库
	sqlite3_close( db );

	return 0;
}

/* exitcodes: 0 == no, 1 == yes, 2 == error */
int
main (int argc, char *argv[])
{
    char msg[300];
    DWORD dwWritten;
    int chars;

	ULONGLONG test = 34019827348374927; 
	CHAR test_str[ 1024 ]; 
	INT32 ret = 0; 
	WCHAR test_str2[ 128 ]; 
	ULONG test_val = 0; 

	vector< ULONG > test_vector; 
	sqlite3 *db; 

	for( INT32 i = 0; i < 100; i ++ )
	{
		test_vector.push_back( i ); 
	}

	for( INT32 i = 0; i < 100; i ++ )
	{
		test_val = test_vector[ i ]; 
		printf( "test value is %u \n", test_val ); 

	}
	
	_snwprintf( test_str2, 128,  L"%s", L"" ); 

	sprintf( test_str, "%I64u", test ); 

	test_sqlite2(); 
	test_sqlite(); 

	ret = sqlite3_open( "C:\\test.db", &db ); 
	if( ret != SQLITE_OK )
	{
		goto _return; 
	}

	sqlite3_close( db ); 

    if (argc > 1 && *argv[1] == '-') {
	switch (*(argv[1]+1)) {
	case 'c':
	    if (argc != 3) {
		chars = wsprintf(msg, "usage: %s -c <compiler option>\n"
			"Tests for whether cl.exe supports an option\n"
			"exitcodes: 0 == no, 1 == yes, 2 == error\n", argv[0]);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, chars, &dwWritten, NULL);
		return 2;
	    }
	    return CheckForCompilerFeature(argv[2]);
	case 'l':
	    if (argc != 3) {
		chars = wsprintf(msg, "usage: %s -l <linker option>\n"
			"Tests for whether link.exe supports an option\n"
			"exitcodes: 0 == no, 1 == yes, 2 == error\n", argv[0]);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, chars, &dwWritten, NULL);
		return 2;
	    }
	    return CheckForLinkerFeature(argv[2]);
	case 'f':
	    if (argc == 2) {
		chars = wsprintf(msg, "usage: %s -f <string> <substring>\n"
		    "Find a substring within another\n"
		    "exitcodes: 0 == no, 1 == yes, 2 == error\n", argv[0]);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, chars, &dwWritten, NULL);
		return 2;
	    } else if (argc == 3) {
		/* if the string is blank, there is no match */
		return 0;
	    } else {
		return IsIn(argv[2], argv[3]);
	    }
	case 'v':
	    if (argc != 4) {
		chars = wsprintf(msg, "usage: %s -v <tcl.h> <tk.h>\n"
		    "Search for versions from the tcl and tk headers.",
		    argv[0]);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, chars, &dwWritten, NULL);
		return 0;
	    }
	    return GetVersionFromHeader(argv[2], argv[3]);
	}
    }
    chars = wsprintf(msg, "usage: %s -c|-l|-f ...\n"
	    "This is a little helper app to equalize shell differences between WinNT and\n"
	    "Win9x and get nmake.exe to accomplish its job.\n",
	    argv[0]);
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, chars, &dwWritten, NULL);
    
	getch(); 
_return:
	return ret;
}

int
CheckForCompilerFeature (const char *option)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    DWORD threadID;
    char msg[300];
    BOOL ok;
    HANDLE hProcess, h, pipeThreads[2];
    char cmdline[100];

    hProcess = GetCurrentProcess();

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags   = STARTF_USESTDHANDLES;
    si.hStdInput = INVALID_HANDLE_VALUE;

    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    /* create a non-inheritible pipe. */
    CreatePipe(&Out.pipe, &h, &sa, 0);

    /* dupe the write side, make it inheritible, and close the original. */
    DuplicateHandle(hProcess, h, hProcess, &si.hStdOutput, 
	    0, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    /* Same as above, but for the error side. */
    CreatePipe(&Err.pipe, &h, &sa, 0);
    DuplicateHandle(hProcess, h, hProcess, &si.hStdError, 
	    0, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    /* base command line */
    strcpy(cmdline, "cl.exe -nologo -c -TC -Fdtemp ");
    /* append our option for testing */
    strcat(cmdline, option);
    /* filename to compile, which exists, but is nothing and empty. */
    strcat(cmdline, " nul");

    ok = CreateProcess(
	    NULL,	    /* Module name. */
	    cmdline,	    /* Command line. */
	    NULL,	    /* Process handle not inheritable. */
	    NULL,	    /* Thread handle not inheritable. */
	    TRUE,	    /* yes, inherit handles. */
	    DETACHED_PROCESS, /* No console for you. */
	    NULL,	    /* Use parent's environment block. */
	    NULL,	    /* Use parent's starting directory. */
	    &si,	    /* Pointer to STARTUPINFO structure. */
	    &pi);	    /* Pointer to PROCESS_INFORMATION structure. */

    if (!ok) {
	DWORD err = GetLastError();
	int chars = wsprintf(msg, "Tried to launch: \"%s\", but got error [%u]: ", cmdline, err);

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK, 0L, err, 0, &msg[chars],
		(300-chars), 0);
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, strlen(msg), &err, NULL);
	return 2;
    }

    /* close our references to the write handles that have now been inherited. */
    CloseHandle(si.hStdOutput);
    CloseHandle(si.hStdError);

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);

    /* start the pipe reader threads. */
    pipeThreads[0] = CreateThread(NULL, 0, ReadFromPipe, &Out, 0, &threadID);
    pipeThreads[1] = CreateThread(NULL, 0, ReadFromPipe, &Err, 0, &threadID);

    /* block waiting for the process to end. */
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

    /* clean up temporary files before returning */
    DeleteFile("temp.idb");
    DeleteFile("temp.pdb");

    /* wait for our pipe to get done reading, should it be a little slow. */
    WaitForMultipleObjects(2, pipeThreads, TRUE, 500);
    CloseHandle(pipeThreads[0]);
    CloseHandle(pipeThreads[1]);

    /* look for the commandline warning code in both streams. */
    return !(strstr(Out.buffer, "D4002") != NULL || strstr(Err.buffer, "D4002") != NULL);
}

int
CheckForLinkerFeature (const char *option)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    DWORD threadID;
    char msg[300];
    BOOL ok;
    HANDLE hProcess, h, pipeThreads[2];
    char cmdline[100];

    hProcess = GetCurrentProcess();

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags   = STARTF_USESTDHANDLES;
    si.hStdInput = INVALID_HANDLE_VALUE;

    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    /* create a non-inheritible pipe. */
    CreatePipe(&Out.pipe, &h, &sa, 0);

    /* dupe the write side, make it inheritible, and close the original. */
    DuplicateHandle(hProcess, h, hProcess, &si.hStdOutput, 
	    0, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    /* Same as above, but for the error side. */
    CreatePipe(&Err.pipe, &h, &sa, 0);
    DuplicateHandle(hProcess, h, hProcess, &si.hStdError, 
	    0, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    /* base command line */
    strcpy(cmdline, "link.exe -nologo ");
    /* append our option for testing */
    strcat(cmdline, option);
    /* filename to compile, which exists, but is nothing and empty. */
//    strcat(cmdline, " nul");

    ok = CreateProcess(
	    NULL,	    /* Module name. */
	    cmdline,	    /* Command line. */
	    NULL,	    /* Process handle not inheritable. */
	    NULL,	    /* Thread handle not inheritable. */
	    TRUE,	    /* yes, inherit handles. */
	    DETACHED_PROCESS, /* No console for you. */
	    NULL,	    /* Use parent's environment block. */
	    NULL,	    /* Use parent's starting directory. */
	    &si,	    /* Pointer to STARTUPINFO structure. */
	    &pi);	    /* Pointer to PROCESS_INFORMATION structure. */

    if (!ok) {
	DWORD err = GetLastError();
	int chars = wsprintf(msg, "Tried to launch: \"%s\", but got error [%u]: ", cmdline, err);

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK, 0L, err, 0, &msg[chars],
		(300-chars), 0);
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, strlen(msg), &err, NULL);
	return 2;
    }

    /* close our references to the write handles that have now been inherited. */
    CloseHandle(si.hStdOutput);
    CloseHandle(si.hStdError);

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);

    /* start the pipe reader threads. */
    pipeThreads[0] = CreateThread(NULL, 0, ReadFromPipe, &Out, 0, &threadID);
    pipeThreads[1] = CreateThread(NULL, 0, ReadFromPipe, &Err, 0, &threadID);

    /* block waiting for the process to end. */
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);

    /* wait for our pipe to get done reading, should it be a little slow. */
    WaitForMultipleObjects(2, pipeThreads, TRUE, 500);
    CloseHandle(pipeThreads[0]);
    CloseHandle(pipeThreads[1]);

    /* look for the commandline warning code in the stderr stream. */
    return !(strstr(Out.buffer, "LNK1117") != NULL || strstr(Err.buffer, "LNK1117") != NULL);
}

DWORD WINAPI
ReadFromPipe (LPVOID args)
{
    pipeinfo *pi = (pipeinfo *) args;
    char *lastBuf = pi->buffer;
    DWORD dwRead;
    BOOL ok;

again:
    ok = ReadFile(pi->pipe, lastBuf, 25, &dwRead, 0L);
    if (!ok || dwRead == 0) {
	CloseHandle(pi->pipe);
	return 0;
    }
    lastBuf += dwRead;
    goto again;

    return 0;  /* makes the compiler happy */
}

int
IsIn (const char *string, const char *substring)
{
    return (strstr(string, substring) != NULL);
}

	
static double
ReadVersionFromHeader(const char *file, const char *macro)
{
    double d = 0.0;
    CHAR szBuffer[100];
    LPSTR p;
    DWORD cbBuffer = 100;
    FILE *fp = fopen(file, "r");
    if (fp != NULL) {
	while (fgets(szBuffer, cbBuffer, fp) != NULL) {
	    if ((p = strstr(szBuffer, macro)) != NULL) {
		while (*p && !isdigit(*p)) ++p;
		d = strtod(p, NULL);
		break;
	    }
	}
	fclose(fp);
    }
    return d;
}

int
GetVersionFromHeader(const char *tclh, const char *tkh)
{
    double dTcl = 0.0, dTk = 0.0;
    
    if (tclh != NULL)
	dTcl = ReadVersionFromHeader(tclh, "TCL_VERSION");
    if (tkh != NULL)
	dTk = ReadVersionFromHeader(tkh, "TK_VERSION");

    if (dTcl > 0 || dTk > 0) {
	FILE *ofp = fopen("version.vc", "w");
	if (dTcl > 0)
	    fprintf(ofp, "TCL_DOTVERSION\t= %0.1f\nTCL_VERSION\t= %u\n",
		    dTcl, (int)(dTcl * 10.0));
	if (dTk > 0)
	    fprintf(ofp, "TK_DOTVERSION\t= %0.1f\nTK_VERSION\t= %u\n",
		    dTk, (int)(dTk * 10.0));
	fclose(ofp);
	return 0;
    }
    return 1;
}
