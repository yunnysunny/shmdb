/**
Open Source Initiative OSI - The MIT License (MIT):Licensing

The MIT License (MIT)
Copyright (c) <2012> <yunnysunny>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

@author yunnysunny<yunnysunny@gmail.com>

*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <stdlib.h> 
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pthread.h>
#endif

#include "log.h"
#define TIME_MAX 32

unsigned int log_level     = LOG_ERROR;//Global

void LogMessage(char* sModule, int nLogLevel, char *sFile,int nLine,unsigned int unErrCode, char *sMessage)
{
	struct tm newTimeST;
#if defined(WIN32) || defined(WIN64)
	DWORD nThreadID;	
#else
	unsigned int nThreadID;
#endif
	
	struct tm *newtime = &newTimeST;
	time_t aclock;
	char stTmp[TIME_MAX];
	
	/*Get current time*/
  	time( &aclock );                 
	
	
	/*Get current threadid*/
#if defined(WIN32) || defined(WIN64)
	localtime_s(newtime, &aclock ); 
	nThreadID = GetCurrentProcessId();
	nThreadID = (nThreadID << 16) + GetCurrentThreadId();
#else
	newtime = localtime( &aclock ); 
	nThreadID = getpid();
	nThreadID = (nThreadID << 16) + pthread_self();
#endif

	switch(nLogLevel)
	{
	case LOG_FAULT:			
			printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Fault>[0x%08x]%s(%s:%d)",
				newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
				newtime->tm_min,newtime->tm_sec,sModule,nThreadID,unErrCode, sMessage, sFile,nLine);
		break;
	case LOG_ERROR:
			printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Error>[0x%08x]%s(%s:%d)",
				newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
				newtime->tm_min,newtime->tm_sec,sModule,nThreadID,unErrCode, sMessage, sFile,nLine);
		break;
	case LOG_WARN:
		printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Warning>%s<%d>(%s:%d)",
			newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
			newtime->tm_min,newtime->tm_sec,sModule,nThreadID, sMessage, unErrCode, sFile,nLine);
		break;
	case LOG_INFO:
		printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Info>%s(%d)(%s:%d)",
			newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
			newtime->tm_min,newtime->tm_sec,sModule,nThreadID, sMessage,  unErrCode, sFile,nLine);
		break;
	case LOG_DEBUG:
		printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Info>%s(%d)(%s:%d)",
			newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
			newtime->tm_min,newtime->tm_sec,sModule,nThreadID, sMessage,  unErrCode, sFile,nLine);
		break;
	case LOG_TRACE:
		printf("\n<%4d-%02d-%02d %02d:%02d:%02d><%s><%ud><Trace>%s(%s:%d)",
			newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
			newtime->tm_min,newtime->tm_sec,sModule,nThreadID, sMessage, sFile,nLine);
		break;
	default:
		break;
	} 
}

int errorReturn(int errorCode,char *tag,char *msg)
{
	SIM_ERROR_TAG(errorCode,tag,msg);
	return errorCode;
}

