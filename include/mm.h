#ifndef _shmdb_H
#define _shmdb_H

#ifdef __cplusplus 
extern "C" { 
#endif

#include "platform.h"

#if __IS_WIN__
#include <windows.h>
#endif


/**
* the index area of the hashmap.it contains the value  cacluated from 
* the result of hash arithmetic,whose input is the sum of the key string.
* |status|dataOffset|nextIndex|
*/
typedef struct MemIndex {
	//unsigned int keyMapIndex;
	/** the offset of the value in the memery */
	unsigned int dataOffset;
	/** 
	* when the key cacluated with same index,it will be save in zipper area.
	* And the nextIndex is the index of the zipper area.
	*/
	unsigned int nextIndex;
	/**
	* the status of the current memery index.
	*/
	unsigned char status;//0:normal,1:deleted
}STMemIndex;
/**
* the value area of the hashmap.
* |indexValue|keyLen|key|valueLen|value|
*/
typedef struct MemData {
	/** which indicate the index of STMemIndex */
	unsigned int indexValue;
	unsigned short valueLen;
	unsigned short keyLen;
	char *key;	
	char *value;
}STMemData;
/**
* the head of the hashmap.
* the hashmap's index area is divided into two part :
* the base area and the zipper area.
* the value of base area is smaller than baseLen,while 
* the value of zipper area is bigger than baseLen.
*/
typedef struct HashShareMemHead {
	unsigned int totalLen;
	unsigned int baseLen;
	/***/
	unsigned int totalUsed;
	unsigned int baseUsed;
	/** the current offset of MemData which is unused.
	the value of it is cacluated form the beginning of the share memory.  */
	unsigned int valueOffset;
	unsigned int memLen;
}STHashShareMemHead;

#if __IS_WIN__
//#define fopen	fopen_s
#else
typedef int HANDLE;
#define GetLastError() (errno)
#endif

typedef struct HashShareHandle {
	HANDLE shmid;/*share memory handel*/
	HANDLE semid;/*semaphore handel*/
	long shmaddr;/*share memory attach to current process*/	
}STHashShareHandle;


#define INT_LENGTH 				(sizeof(int))
#define CHAR_LENGTH				(sizeof(char))
#define	SHORT_LENGTH			(sizeof(short))
#define STATUS_UNSED			0
#define STATUS_DEL				1
#define STATUS_INUSED			0xff


#define SIZE_OF_ST_MEM_INDEX	(INT_LENGTH * 2 + CHAR_LENGTH)
#define BASE_SIZE_OF_ST_MEM_DATA		(INT_LENGTH + SHORT_LENGTH * 2)
#define MAX_LEN_OF_KEY			0xff
#define MAX_LEN_OF_ELEMENT		(0x400 * 4)//4KB
#define MAX_LEN_OF_VALUE		(MAX_LEN_OF_ELEMENT - MAX_LEN_OF_KEY - BASE_SIZE_OF_ST_MEM_DATA)
#define SIZE_OF_ST_HASH_SHARE_MEM_HEAD		(INT_LENGTH * 6)

#if __IS_WIN__
#define MAX_WAIT_WHEN_GET_LOCAK			100
#else
#define MAX_WAIT_WHEN_GET_LOCAK			100*1000*1000
#endif


int shmdb_initParent(STHashShareHandle *handle,unsigned int size);

int shmdb_initChild(STHashShareHandle *handle);

int shmdb_getInfo(STHashShareHandle *handle, STHashShareMemHead *head);

int shmdb_put(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	const char *value,unsigned short valueLen);

int shmdb_get(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	char **value,unsigned short *valueLen);

int shmdb_delete(STHashShareHandle *handle,const char *key,unsigned short keyLen,
	char **value,unsigned short *valueLen);

int shmdb_dump(STHashShareHandle *handle,char *path);

int shmdb_destroy(STHashShareHandle *handle);

#ifdef __cplusplus 
} 
#endif 

#endif