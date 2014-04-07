#ifndef _ERRCODE_H
#define _ERRCODE_H

#ifdef __cplusplus 
extern "C" { 
#endif

#define ERROR_SUCCESS							0
#define	ERROR_SHM_BASE							0x10000000
#define ERROR_TOO_BIG_SIZE						(ERROR_SHM_BASE + 1)
#define ERROR_GET_SHM_FAILED					(ERROR_SHM_BASE + 2)
#define ERROR_SHM_NOT_INIT						(ERROR_SHM_BASE + 3)

#ifdef __cplusplus 
} 
#endif 

#endif