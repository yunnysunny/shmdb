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

#define LOG_NONE      0  //不记录日志
#define LOG_FAULT      1  //致命错误，会导致返回错误或引起程序异常
#define LOG_ERROR     2  //一般错误，内部错误不向上层返回错误，建议解决
#define LOG_WARN   3  //警告信息，不是期望的结果，不会引起错误，但要用户引起重视
#define LOG_INFO      4  //重要变量，有助于解决问题
#define LOG_DEBUG     5  //调试信息，可打印二进制数据
#define LOG_TRACE     6  //跟踪执行，用于跟踪逻辑判断

#define DEFAULT_LOG_MODULE "logger"

extern unsigned int log_level;     //Global

#define LOG(lvl, rv, msg) \
        do { \
        if ((lvl) <= log_level) {\
        	LogMessage(DEFAULT_LOG_MODULE, lvl, __FILE__, __LINE__, rv, msg);} \
        } while (0)

void LogMessage(char* sModule, int nLogLevel, char *sFile,int nLine,unsigned int unErrCode, char *sMessage);

int errorReturn(int errorCode,char *tag,char *msg);

#define LOG_WITH_TAG(lvl, rv, tag,msg) LogMessage(lvl, tag, __FILE__, __LINE__, rv, msg)

#define SIM_TRACE_TAG(tag,msg)	LOG_WITH_TAG(LOG_TRACE,0,tag,msg)

#define SIM_TRACE(msg)	LOG_WITH_TAG(LOG_TRACE,0,DEF_LOG_MODULE,msg)

#define SIM_ERROR_TAG(rv,tag,msg)	LOG_WITH_TAG(LOG_ERROR,rv,tag,msg)

#define SIM_ERROR(rv,msg)	LOG_WITH_TAG(LOG_ERROR,rv,DEF_LOG_MODULE,msg)

#define SIM_WARN_TAG(tag,rv,msg)	LOG_WITH_TAG(LOG_WARN,rv,tag,msg)

#define SIM_WARN(rv,msg)	LOG_WITH_TAG(LOG_WARNING,rv,DEF_LOG_MODULE,msg)

#define SIM_INFO_TAG(tag,rv,msg)	LOG_WITH_TAG(LOG_INFO,rv,tag,msg)

#define SIM_INFO(rv,msg)	LOG_WITH_TAG(LOG_INFO,rv,DEF_LOG_MODULE,msg)

#define ERROR_RETURN(rv,msg) errorReturn(rv,DEF_LOG_MODULE,msg)
#define ERROR_RETURN_TAG(rv,tag,msg) errorReturn(rv,tag,msg)

#ifdef __cplusplus 
} 
#endif 

#endif /*#ifndef LOG_H_*/
