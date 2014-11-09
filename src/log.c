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
#define __IS_WIN__ 1
#else
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pthread.h>
#define __IS_WIN__ 0
#endif

#include "log.h"
#define TIME_MAX										32
#define COUNT_ERROR_LEVEL								6
#define MAX_LOG_FILE_NAME_LEN							256
char *errorLevel[COUNT_ERROR_LEVEL]						= {"FAULT","ERROR","WARN","INFO","DEBUG","TRACE"};

static int logLevel							= LEVEL_TRACE;//
static char logFileName[MAX_LOG_FILE_NAME_LEN]          = {0};

void setLogLevel(int nLogLevel) {
	if (nLogLevel < 1 || nLogLevel > COUNT_ERROR_LEVEL) {
		printf("error log level:%d\n",nLogLevel);
		return;
	}
	logLevel = nLogLevel;
}

void setLogFile(const char *fileName) {
	if (fileName == NULL) {
		return;
	}
	if (strlen(fileName) == 0) {
		return;
	}
	if (strlen(fileName) > MAX_LOG_FILE_NAME_LEN - 1) {
		printf("too long file name:%d\n",strlen(fileName));
		return;
	}
	{
		FILE *f = NULL;
#if __IS_WIN__
		if ((fopen_s(&f,fileName,"rw")) != 0) {
#else
		if ((f = fopen(fileName,"rw")) == NULL) {
#endif
			printf("open log file [%s] error\n",fileName);
			return;
		}
		fclose(f);
#if __IS_WIN__
		strncpy_s(logFileName,MAX_LOG_FILE_NAME_LEN,fileName,strlen(fileName));
#else
		strncpy(logFileName,fileName,MAX_LOG_FILE_NAME_LEN);
#endif
	}
}


void LogMessage(char* sModule, int nLogLevel, char *sFile,int nLine,char *fmt, ...)
{
	struct tm newTimeST;
#if __IS_WIN__
	DWORD nThreadID;	
#else
	unsigned int nThreadID;
#endif
	
	struct tm *newtime = &newTimeST;
	time_t aclock;
	
	FILE *logFile = NULL;
	va_list ap1, ap2;;

	if (nLogLevel < 1 || nLogLevel > COUNT_ERROR_LEVEL) {
		printf("error log level:%d\n",nLogLevel);
		return;
	}

	if (nLogLevel > logLevel) {
		return;
	}

  	time( &aclock );                 

#if __IS_WIN__
	localtime_s(newtime, &aclock ); 
	nThreadID = GetCurrentProcessId();
	nThreadID = (nThreadID << 16) + GetCurrentThreadId();
#else
	newtime = localtime( &aclock ); 
	nThreadID = getpid();
	nThreadID = (nThreadID << 16) + pthread_self();
#endif
	if (strlen(logFileName) > 0) {
#if __IS_WIN__
		if ((fopen_s(&logFile,logFileName,"rb+")) != 0) {
#else
		if ((logFile = fopen(logFileName,"rb+")) == NULL) {
#endif
			printf("open log file [%s] error\n",logFileName);			
		}		
	}
	printf("\n[%4d-%02d-%02d %02d:%02d:%02d][%s][%ud][%s:%d][%s] ",
		newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
		newtime->tm_min,newtime->tm_sec,sModule,nThreadID,sFile,nLine,errorLevel[nLogLevel-1]);
	va_start(ap1, fmt);
	if (logFile != NULL) {
#if __IS_WIN__
		ap2 = ap1;
#else
		va_copy(ap2, ap1);
#endif
		vfprintf(logFile,fmt,ap2);
	}
	vprintf(fmt,ap1);
	if (logFile != NULL) {
		va_end(ap2);
	}
    va_end(ap1);
}

int errorReturn(int errorCode,char *tag,char *msg)
{
	LOG_WITH_TAG(LEVEL_ERROR,tag,errorCode,msg);
	return errorCode;
}

