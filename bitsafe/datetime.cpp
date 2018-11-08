#include "StdAfx.h"
#include "bitsafe_common.h"
#include "datetime.h"

int API_StringToTime(const string &strDateStr,time_t &timeData)
{
    char *str_begin = (char*) strDateStr.c_str();
    char *time_delim = strstr(str_begin,"-");
    if(time_delim == NULL)
    {
        return -1;
    }
    int year = atoi(str_begin);
    int iMonth = atoi(time_delim + 1);

    time_delim = strstr(time_delim + 1,"-");
    if(time_delim == NULL)
    {
        return -1;
    }

    int iDay = atoi(time_delim + 1);

    struct tm sourcedate;
    memset((void*)&sourcedate, 0, sizeof(sourcedate));
    sourcedate.tm_mday = iDay;
    sourcedate.tm_mon = iMonth - 1;
    sourcedate.tm_year = year - 1900;

    timeData = mktime(&sourcedate);  

    return 0;
}

/*
    time_t to string
*/
int API_TimeToString(string &strDateStr,const time_t &timeData)
{
    char chTmp[15];
    memset(chTmp,0, sizeof(chTmp));

    struct tm *p;
    p = localtime(&timeData);

    p->tm_year = p->tm_year + 1900;

    p->tm_mon = p->tm_mon + 1;


    _snprintf(chTmp,sizeof(chTmp),"%04d-%02d-%02d",
        p->tm_year, p->tm_mon, p->tm_mday);

    strDateStr = chTmp;
    return 0;
}

/*
   string to time_t   
   time format: 2009-3-24 0:00:08 or 2009-3-24
*/

INT32 str2time( LPCTSTR time_str,time_t *time_out )
{
	INT32 ret = 0; 
	struct tm time_tm;
    LPCTSTR str_begin;
    LPCTSTR time_delim; 

	ASSERT( time_out != NULL ); 

	memset( ( PVOID )&time_tm,0, sizeof( time_tm ) ); 

	str_begin = time_str; 
	time_delim = _tcschr( str_begin, _T( '-' ) ); 
    if( time_delim == NULL )
    {
        __Trace( _T( "time [%s] format error \n" ), time_str ); 
        ret = -1; 
		goto _return; 
    }

    time_tm.tm_year = _tcstol( str_begin, NULL, 10 );
    time_tm.tm_mon = _tcstol( time_delim + 1, NULL, 10 );
    time_delim = _tcschr( time_delim + 1, _T( '-' ) );
    if( time_delim == NULL )
    {
        __Trace( _T( "time [%s] format error \n" ), time_str ); 
		ret = -1; 
		goto _return; 
    }
    
	time_tm.tm_mday = _tcstol( time_delim + 1, NULL, 10 );

    time_delim = _tcschr( time_delim + 1, _T( ' ' ) ); 

    if( time_delim != NULL )
    {
        time_tm.tm_hour = _tcstol( time_delim + 1, NULL, 10 ); 
		time_delim = _tcschr( time_delim + 1, _T( ':' ) ); 
        
		if( time_delim != NULL )
        {
            time_tm.tm_min = _tcstol( time_delim + 1, NULL, 10 );
            time_delim = _tcschr( time_delim + 1, _T( ':' ) );
            if( time_delim != NULL )
            {
                time_tm.tm_sec = _tcstol( time_delim + 1, NULL, 10 ); 
            }
        }
    }

	time_tm.tm_mon -= 1; 
	time_tm.tm_year -= 1900; 

	*time_out = mktime( &time_tm ); 

_return:
    return ret;
}
/*
   time_t to string 时间格式 2009-3-24 0:00:08
   */
int API_TimeToStringEX(string &strDateStr,const time_t &timeData)
{
    char chTmp[100];
    memset(chTmp,0, sizeof(chTmp));
    struct tm *p;
    p = localtime(&timeData);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

    _snprintf(chTmp,sizeof(chTmp),"%04d-%02d-%02d %02d:%02d:%02d",
            p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
    strDateStr = chTmp;
    return 0;
}

