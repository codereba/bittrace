#ifndef __DATETIME_H__
#define __DATETIME_H__

int API_StringToTime(const string &strDateStr,time_t &timeData); 

/*
    time_t to string
*/
int API_TimeToString(string &strDateStr,const time_t &timeData); 

INT32 str2time( LPCTSTR time_str,time_t *time_out ); 

int API_TimeToStringEX(string &strDateStr,const time_t &timeData); 

#endif //__DATETIME_H__