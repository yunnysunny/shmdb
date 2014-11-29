#ifndef _ERRCODE_H
#define _ERRCODE_H

#ifdef __cplusplus 
extern "C" { 
#endif

//#define ERROR_SUCCESS							0
#define	ERROR_SHM_BASE							0x10000000
#define ERROR_TOO_BIG_SIZE						(ERROR_SHM_BASE + 1)
#define ERROR_GET_SHM_FAILED					(ERROR_SHM_BASE + 2)
#define ERROR_SHM_NOT_INIT						(ERROR_SHM_BASE + 3)
#define ERROR_NULL_KEY							(ERROR_SHM_BASE + 4)
#define ERROR_NULL_VALUE						(ERROR_SHM_BASE + 5)
#define ERROR_GET_LOCK							(ERROR_SHM_BASE + 6)
#define ERROR_FULL_USED							(ERROR_SHM_BASE + 7)
#define ERROR_TOO_LONG_KEY						(ERROR_SHM_BASE + 8)
#define ERROR_TOO_LONG_VALUE					(ERROR_SHM_BASE + 9)
#define ERROR_NOT_ENOUGH						(ERROR_SHM_BASE + 10)
#define ERROR_GET_INDEX							(ERROR_SHM_BASE + 11)

#define ERROR_MALLOC_MEMORY						(ERROR_SHM_BASE + 12)
#define ERROR_NULL_MEMDATA						(ERROR_SHM_BASE + 13)
#define ERROR_NOT_FOUND_INDEX					(ERROR_SHM_BASE + 14)

#define ERROR_PATH_NULL							(ERROR_SHM_BASE + 15)
#define ERROR_FOPEN_ERROR						(ERROR_SHM_BASE + 16)

#define ERROR_DESTORY_SHM						(ERROR_SHM_BASE + 17)
#ifdef __cplusplus 
} 
#endif 

#endif