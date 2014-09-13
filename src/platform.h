#ifndef _PLATFORM_H
#define _PLATFORM_H

#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#define __IS_WIN__  1
#else
#define __IS_WIN__	0
#endif

#endif