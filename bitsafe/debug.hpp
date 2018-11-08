#ifndef _DEBUG_HPP
#define _DEBUG_HPP

#include <stdio.h>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN( __FUNCTION__ )

#ifdef UNICODE
#define __TFILE__	__WFILE__
#define __TFUNCTION__ __WFUNCTION__
#else
#define __TFILE__	__FILE__
#define __TFUNCTION__ __FUNCTION__
#endif

#define DBG_THREADSAFE

#define DBG_VERBOSE 1
#define DBG_TERSE   2
#define DBG_WARNING 3
#define DBG_ERROR   4

extern int		g_iDebugLevel;
extern bool		g_bSaveLogFile;
extern TCHAR	g_bLogSavePath[MAX_PATH];

#define __in 
BOOL DbgPrint(__in LPCTSTR lpszFormatString,...);

#define DBGMSG(level, prefix, msg) { \
	INT dbgLevel = level; \
	if (g_iDebugLevel <= (dbgLevel)) { \
	DbgPrint(_T("%s\t\tFILE:%s\t\tLINE:%d\t\t"), prefix, __TFILE__, __LINE__);\
	DbgPrint(msg); \
	} \
}

#define DBGPRINT(level, msg) { \
	INT dbgLevel = level; \
	if (g_iDebugLevel <= (dbgLevel)) { \
	DbgPrint(_T("FILE:%s\t\tLINE:%d\t\t"), __TFILE__, __LINE__);\
	DbgPrint(msg); \
	} \
}


#if _MSC_VER > 1310

#ifdef _DEBUG
#define VERBOSE(...) DBGPRINT(DBG_VERBOSE, ##__VA_ARGS__)
#define TERSE(...) DBGPRINT(DBG_TERSE, ##__VA_ARGS__)

#else // !DBG

#define VERBOSE(...)				__noop
#define TERSE(...)					__noop
#endif
#define WARNING(...) DBGMSG(DBG_WARNING,_T("WRN"), ##__VA_ARGS__)
#define ERR(...) DBGMSG(DBG_ERROR,_T("ERR"), ##__VA_ARGS__)

#else
#define VERBOSE(_x_) DBGPRINT _x_
#define TERSE(_x_) DBGPRINT _x_
#endif //_MSC_VER > 1310

#endif // _DEBUG_HPP