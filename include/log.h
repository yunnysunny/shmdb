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
#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus 
extern "C" { 
#endif

#define LEVEL_NONE      0  //
#define LEVEL_FAULT      1  //
#define LEVEL_ERROR     2  //
#define LEVEL_WARN   3  //
#define LEVEL_INFO      4  //
#define LEVEL_DEBUG     5  //
#define LEVEL_TRACE     6  //

#define DEFAULT_LOG_MODULE "logger"

void setLogLevel(int nLogLevel);

void setLogFile(const char *fileName);

void printLog(char* sModule, int nLogLevel, char *sFile,int nLine,char *fmt, ...);

int errorReturn(int errorCode,char *tag,char *msg);

#define LOG(lvl, rv, msg) printLog(DEFAULT_LOG_MODULE, lvl, __FILE__, __LINE__,"[%08x][%s]", rv, msg);

#define LOG_WITH_TAG(lvl,  tag, rv, msg) printLog( tag,lvl, __FILE__, __LINE__, "[%08x][%s]", rv,msg)

#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#ifndef _MSC_VER  
#define _MSC_VER 1600 
#endif 
#define SIM_TRACE(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_TRACE, __FILE__, __LINE__,format,##__VA_ARGS__)
#define SIM_DEBUG(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_DEBUG, __FILE__, __LINE__,format,##__VA_ARGS__)
#define SIM_INFO(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_INFO, __FILE__, __LINE__,format,##__VA_ARGS__)
#define SIM_WARN(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_WARN, __FILE__, __LINE__,format,##__VA_ARGS__)
#define SIM_ERROR(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_ERROR, __FILE__, __LINE__,format,##__VA_ARGS__)
#define SIM_FAULT(format,...) printLog(DEFAULT_LOG_MODULE, LEVEL_FAULT, __FILE__, __LINE__,format,##__VA_ARGS__)
#else
#define SIM_TRACE(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_TRACE, __FILE__, __LINE__,format,##args)
#define SIM_DEBUG(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_DEBUG, __FILE__, __LINE__,format,##args)
#define SIM_INFO(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_INFO, __FILE__, __LINE__,format,##args)
#define SIM_WARN(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_WARN, __FILE__, __LINE__,format,##args)
#define SIM_ERROR(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_ERROR, __FILE__, __LINE__,format,##args)
#define SIM_FAULT(format,args...) printLog(DEFAULT_LOG_MODULE, LEVEL_FAULT, __FILE__, __LINE__,format,##args)
#endif

#ifdef __cplusplus 
} 
#endif 

#endif /*#ifndef LOG_H_*/
