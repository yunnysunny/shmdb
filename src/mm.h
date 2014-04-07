#ifndef _MM_H
#define _MM_H

#ifdef __cplusplus 
extern "C" { 
#endif
/**
* the index area of the hashmap.it contains the value  cacluated from 
* the result of hash arithmetic,whose input is the sum of the key string.
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
	unsigned char status;//
}STMemIndex;
/**
* the value area of the hashmap.
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
*/
typedef struct HashShareMem {
	unsigned int totalLen;
	unsigned int baseLen;
	unsigned int totalUsed;
	unsigned int baseUsed;
}STHashShareMem;

typedef struct HashShareHandle {
	int shmid;
	long shmaddr;
}STHashShareHandle;

#define INT_LENGTH 				(sizeof(int))
#define CHAR_LENGTH				(sizeof(char))
#define	SHORT_LENGTH			(sizeof(short))
#define STATUS_OK				0
#define STATUS_DEL				1

#define SIZE_OF_ST_MEM_INDEX	(INT_LENGTH * 2 + CHAR_LENGTH)
#define BASE_SIZE_OF_ST_MEM_DATA		(INT_LENGTH + SHORT_LENGTH * 2)
#define MAX_LEN_OF_KEY			0xff
#define MAX_LEN_OF_ELEMENT		(0x400 * 4)//4KB
#define MAX_LEN_OF_VALUE		(MAX_LEN_OF_ELEMENT - MAX_LEN_OF_KEY - BASE_SIZE_OF_ST_MEM_DATA)
#define SIZE_OF_ST_HASH_SHARE_MEM		(INT_LENGTH * 4)

int mm_initParent(STHashShareHandle *handle,unsigned int size);

int mm_initChild(STHashShareHandle *handle);

int mm_getInfo(STHashShareHandle *handle);

int mm_put(STHashShareHandle *handle,const char*key,const char *value);

int mm_get(STHashShareHandle *handle,const char*key, char *value);

int mm_delete(STHashShareHandle *handle,const char *key);



#ifdef __cplusplus 
} 
#endif 

#endif