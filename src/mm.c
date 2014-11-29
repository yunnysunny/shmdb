#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include <sys/types.h>

#include <errno.h>

#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>
#if __IS_WIN__
#include <windows.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <execinfo.h>
#include <unistd.h>
#endif
#include "errcode.h"
#include "mm.h"
#include "prime.h"
#include "hash.h"
#include "transform.h"
#include "log.h"


typedef struct ValueAreaData {
	unsigned int index;
	unsigned short valueLen;
	void *pValue;
}STValueAreaData;


#define LEN_GLOBAL_ERROR_MSG 256
#define OPERATION_GET			1
#define OPERATION_DELETE		2

//static char globalErrorMsg[LEN_GLOBAL_ERROR_MSG] = {0};
static STHashShareHandle *handleBackup = NULL;
static int isParent = 1;

#if __IS_WIN__
typedef void (*SignalHandlerPointer)(int);

static void print_reason(int sig)
{
	SIM_TRACE("get a single [%d]\n",sig);
	switch(sig) {
		case SIGSEGV:
		case SIGFPE:
		case SIGILL:
		case SIGABRT: {
			SIM_ERROR("crash occurred.");
			shmdb_dump(handleBackup,"crash.dump");
			if (isParent == 1) {
				shmdb_destroy(handleBackup);
			}
			
			exit(-1);
		}
		break;
		case SIGINT: {

			SIM_INFO("normal exit\n");
			if (isParent == 1) {
				shmdb_destroy(handleBackup);
			}

			exit(0);
		}
		break;
		default:
		break;
	}
	
}
#else
static void print_reason(int sig, siginfo_t * info, void *secret)
{
	SIM_TRACE("get a single [%d]\n",sig);
	switch(sig) {
		case SIGSEGV:
		case SIGFPE:
		case SIGILL:
		case SIGBUS:
		case SIGABRT:
		case SIGSYS: {
		void *array[10];
		size_t size;
#ifdef PRINT_DEBUG
		char **strings;
		size_t i;
		size = backtrace(array, 10);
		strings = backtrace_symbols(array, size);
		SIM_ERROR("Obtained %zd stack frames.\n", size);
		for (i = 0; i < size; i++)
		SIM_ERROR("%s\n", strings[i]);

		free(strings);
#else
		int fd = open("err.log", O_WRONLY);
		size = backtrace(array, 10);
		backtrace_symbols_fd(array, size, fd);
		close(fd);
#endif
		shmdb_dump(handleBackup,"crash.dump");

		if (isParent == 1) {
			shmdb_destroy(handleBackup);
		}

		exit(-1);
		}
		break;
		case SIGINT:
		case SIGKILL: {

			SIM_INFO("normal exit\n");
			if (isParent == 1) {
				shmdb_destroy(handleBackup);
			}
			exit(0);
		}
		break;
		case SIGUSR1: {
			shmdb_dump(handleBackup,"normal.dump");
		}
		break;
		default:
		break;
	}
	
}
#endif

static void addEvent() {
#if __IS_WIN__
	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGSEGV,print_reason);
	previousHandler = signal(SIGFPE,print_reason);
	previousHandler = signal(SIGILL,print_reason);

	previousHandler = signal(SIGABRT,print_reason);
		
		
	previousHandler = signal(SIGINT,print_reason);
		
	previousHandler = signal(SIGTERM,print_reason);
#else
	struct sigaction myAction;
	myAction.sa_sigaction = print_reason;
	sigemptyset(&myAction.sa_mask);
	myAction.sa_flags = SA_RESTART | SA_SIGINFO;
	sigaction(SIGSEGV, &myAction, NULL);
	sigaction(SIGFPE, &myAction, NULL);
	sigaction(SIGILL, &myAction, NULL);
	sigaction(SIGBUS, &myAction, NULL);
	sigaction(SIGABRT, &myAction, NULL);
	sigaction(SIGSYS, &myAction, NULL);
		
	sigaction(SIGINT, &myAction, NULL);
	sigaction(SIGKILL, &myAction, NULL);
	sigaction(SIGTERM, &myAction, NULL);
	sigaction(SIGHUP, &myAction, NULL);
	sigaction(SIGSTOP, &myAction, NULL);
		
	sigaction(SIGUSR1, &myAction, NULL);
#endif
}

/**
 * the format of one share memory hashmap:
 * HashShareMemHead|MemIndex|MemData
 * the format of the HashShareMemHead:
 * totalLenBytes(4)|baseLenBytes(4)|totalUsedBytes(4)|baseUsedBytes(4)|valueOffsetBytes(4)|memLenBytes(4)
 * memLenBytes contains the number total bytes the share memory,
 * valueOffsetBytes contains the number ,indicateing the offset of the begin offset of MemData,
 * totalLenBytes is always the number of double of parameter `size`,
 * baseLenBytes is always the number of parameter `size`,
 * totalUsedBytes is the number of how many indexs have been used,
 * baseUsedBytes is the number of indexs used in base area.
 * 
 * initialize the share memory and semaphore in parent process.
 * @param [in] STHashShareHandle *handle
 * @param [in] unsigned int size the base size,which is the value of baseLenBytes
 * @param [in] STShmdbOption *option
 *
 * @return int the result
 */
int shmdb_initParent(STHashShareHandle *handle,unsigned int size,STShmdbOption *option)
{
	int rv;	
	
	unsigned int totalLen = size * 2;
	unsigned int baseLen = size;
	LPVOID shm_addr;

	HANDLE id = 0;
	HANDLE semid = 0;
	
	unsigned int memLen = 0;
	unsigned int space = 0,valueOffset = 0;
	unsigned int maxPrime = getMaxPrime(size);//the max prime between 0 and size
#if __IS_WIN__	
	SECURITY_ATTRIBUTES sa;
#endif	

	if (option != NULL) {
		setLogLevel(option->logLevel);
	}	


	space = (maxPrime * 2);//total index count
	totalLen = space;
	baseLen = maxPrime;
	valueOffset = SIZE_OF_ST_HASH_SHARE_MEM_HEAD + space*SIZE_OF_ST_MEM_INDEX;
	memLen += (valueOffset + space * MAX_LEN_OF_ELEMENT);
	SIM_TRACE("the len wanna get:%d\n",memLen);

#if __IS_WIN__		
	sa.nLength=sizeof(sa);
	sa.lpSecurityDescriptor=NULL;
	sa.bInheritHandle=TRUE;
	if ((semid = CreateMutex(&sa,FALSE,NULL)) == 0) {
		perror("CreateMutex error:");
		rv = GetLastError();
		return rv;
	}
	SIM_TRACE("get semid:%d\n",semid);

	
	if ((id=CreateFileMapping(INVALID_HANDLE_VALUE,&sa,PAGE_READWRITE,0,memLen,NULL))==0) {
		perror("CreateFileMapping error");
		rv = GetLastError();
		handle = NULL;
		return rv;
	}
	
	if ((shm_addr= MapViewOfFile(id,FILE_MAP_ALL_ACCESS, 0, 0/*memory start address*/, 0))==0) {
		perror("MapViewOfFile error");
		rv = GetLastError();
		handle = NULL;
		return rv;
	}
#else
	if ((semid = semget(IPC_PRIVATE,1,IPC_CREAT | 0600)) < 0) {
		perror("semget error:");
		rv = errno;
		return rv;
	} else {                
		semctl(semid, 0, SETVAL, 1); //初始化信号量为1
	}
	SIM_TRACE("create semid:%d\n",semid);
	
	if ((id=shmget(IPC_PRIVATE,memLen,0600))<0) {
		perror("shmget error");
		rv = errno;
		handle = NULL;
		return rv;
	}
	
	if ((shm_addr=shmat(id,0,0))==(void*)-1) {
		perror("shmat error");
		rv = errno;
		handle = NULL;
		return rv;
	}
#endif
	
	SIM_TRACE("to set header,totalLen:%d:\n",totalLen);
	int2chars(totalLen,(unsigned char*)shm_addr);
	int2chars(baseLen,(unsigned char*)shm_addr+4);
		
	memset((unsigned char*)shm_addr+8,0,4);//totalused
	memset((unsigned char*)shm_addr+12,0,4);//baseused
	int2chars(valueOffset,(unsigned char*)shm_addr+16);//value offset
	
	int2chars(memLen,(unsigned char*)shm_addr+20);//memLen
	memset((char*)shm_addr+24,0,memLen-24);//clear the index area and the value area
	
	SIM_TRACE("set header finish:\n");
	handle->shmid = id;
	handle->semid = semid;
	handle->shmaddr = shm_addr;
	SIM_TRACE("the shmid is %d\n",id);
	rv = 0;
	handleBackup = handle;

	addEvent();
	return rv;
}
/**
 * initialize the share memory and semaphore in child process.
 * repeat the calling of shmat to let child process attach to the share memory.
 *
 * @param [in] STHashShareHandle *handle
 *
 * @return int the result
 */
int shmdb_initChild(STHashShareHandle *handle)
{
	if (handle == NULL || handle->shmid == 0) {
		SIM_TRACE("the handle is null\n");
		return ERROR_SHM_NOT_INIT;
	} else {
		HANDLE shmid = handle->shmid;
		LPVOID shm_addr;
		int rv;
#if __IS_WIN__
		if ((shm_addr=MapViewOfFile(shmid,FILE_MAP_ALL_ACCESS, 0, 0/*memory start address*/, 0))==0) {
			rv = GetLastError();
			SIM_TRACE("MapViewOfFile error:%d\n",rv);
			return rv;
		}
#else
		if ((shm_addr=shmat(shmid,0,0))==(void*)-1) {
			perror("shmat error");
			rv = GetLastError();
			return rv;
		}
#endif
			
			
		handle->shmaddr = shm_addr;

		handleBackup = handle;

		isParent = 0;

		addEvent();
	}
	return 0;
}
/**
 * get the hashtable's head
 */
int shmdb_getInfo(STHashShareHandle *handle, STHashShareMemHead *head)
{
	if (handle == NULL || handle->shmid == 0) {
		SIM_TRACE("the handle is null\n");
		return ERROR_SHM_NOT_INIT;
	} else if (head != NULL){
		//int shmid = handle->shmid;
		LPVOID shmaddr = (LPVOID)handle->shmaddr;
		unsigned int baseLen = 0;
		unsigned int totalLen = 0;
		unsigned int totalUsed = 0;
		unsigned int baseUsed = 0;
		unsigned int valueOffset = 0;
		unsigned int memLen = 0;
		
		SIM_TRACE("shmaddr:%d\n",handle->shmaddr);
		
		chars2int((unsigned char*)shmaddr,&totalLen);
		head->totalLen = totalLen;
		chars2int((unsigned char*)(shmaddr)+4,&baseLen);
		head->baseLen = baseLen;
		chars2int((unsigned char*)(shmaddr)+8,&totalUsed);
		head->totalUsed = totalUsed;
		chars2int((unsigned char*)(shmaddr)+12,&baseUsed);
		head->baseUsed = baseUsed;
		chars2int((unsigned char*)(shmaddr)+16,&valueOffset);
		head->valueOffset = valueOffset;
		SIM_TRACE("the base len is %d\n",baseLen);
		chars2int((unsigned char*)(shmaddr)+20,&memLen);
		head->memLen = memLen;
	}
	return 0;
	
}
/**
* give the offset of value area when put a new data in mm.
*
* @param [in] STHashShareMemHead *head
* @param [in] void *shmaddr
* @param [in] char *key
* @param unsigned short keyLen
* @param unsigned short valueLen
* @param [in|out]unsigned int *index
* @param unsigned int lastIndex
* 
* @return void * the address of value area
*/
static void *getValueArea(STHashShareMemHead *head,LPVOID shmaddr,
	const char *key,unsigned short keyLen, unsigned short valueLen,
	unsigned int *index,unsigned int lastIndex)
{
	if (*index >= head->totalLen) {
		return NULL;
	}
	{
		void *indexOffset = (unsigned char*)shmaddr + SIZE_OF_ST_HASH_SHARE_MEM_HEAD + *index * SIZE_OF_ST_MEM_INDEX;
		unsigned int dataOffset = 0;
		void *valueArea = NULL;
		unsigned char status = *((unsigned char*)indexOffset);
		
		
		switch (status) {
			case STATUS_DEL: {
				unsigned short existValueLen = 0;
				unsigned short existKeyLen = 0;
				chars2int((unsigned char*)indexOffset+1,&dataOffset);
				valueArea = (unsigned char*)shmaddr + dataOffset;
				chars2short((unsigned char*)valueArea+4,&existKeyLen);///
				chars2short((unsigned char*)valueArea+6+existKeyLen,&existValueLen);///
				if ((existKeyLen + existValueLen) <= (keyLen + valueLen)) {//find a index

					*((unsigned char*)indexOffset) = STATUS_INUSED;
					//int2chars(valueArea+4,keyLen);
					
					int2chars(head->totalUsed+1, (unsigned char*)shmaddr+8);
					
					if (*index > head->baseLen) {//the current index is in zipper area
						
					} else {//this current index is in base area
						int2chars(head->baseUsed+12,(unsigned char*)shmaddr+1);						
					}
                    memset(valueArea,0,existKeyLen + existValueLen);
					return valueArea;
					//break;
				} else {
					

					unsigned int existIndex = 0;// to test the next index in the zipper area
					chars2int(((unsigned char*)indexOffset+5), &existIndex);
					if (existIndex > 0) {
						lastIndex = *index;
						*index = existIndex;
						
						return getValueArea(head,shmaddr,key,keyLen,valueLen,index,lastIndex);
					} else {
						lastIndex = *index;
						*index = head->baseLen + (head->totalUsed-head->baseUsed);
						int2chars(*index,(unsigned char*)indexOffset+5);
						return getValueArea(head,shmaddr,key,keyLen,valueLen,index,lastIndex);
					}
				
					
					
				}
			}
			break;
			case STATUS_INUSED: {
				unsigned short existValueLen = 0;
				unsigned short existKeyLen = 0;
				chars2int((unsigned char*)indexOffset+1,&dataOffset);
				valueArea = (unsigned char*)shmaddr + dataOffset;
				chars2short((unsigned char*)valueArea+4,&existKeyLen);///
				chars2short((unsigned char*)valueArea+6+existKeyLen,&existValueLen);///
				if (existKeyLen == keyLen
					&& strncmp((char *)valueArea+6,key,existKeyLen) == 0) {//
					if (existValueLen >= valueLen) {
						return valueArea;
					} else {
						*((unsigned char*)indexOffset) = STATUS_DEL;//
					}
				}
				{// to test the next index in the zipper area
					unsigned int existIndex = 0;
				
					// to test the next index in the zipper area
					chars2int(((unsigned char*)indexOffset+5), &existIndex);
					if (existIndex > 0) {
						lastIndex = *index;
						*index = existIndex;
						
						return getValueArea(head,shmaddr,key,keyLen,valueLen,index,lastIndex);
					} else {
						lastIndex = *index;
						*index = head->baseLen + (head->totalUsed-head->baseUsed);
						int2chars(*index,(unsigned char*)indexOffset+5);
						return getValueArea(head,shmaddr,key,keyLen,valueLen,index,lastIndex);
					}
				}
				
			}
			break;
			case STATUS_UNSED: {
				
				unsigned int newValueOffset = head->valueOffset + 4 + 2 + keyLen + 2 + valueLen;//the value offset will be used next 
				//head begin
				int2chars(head->totalUsed+1,(unsigned char*)shmaddr+8);//increase totalUsed
				
				int2chars(head->baseUsed+1,(unsigned char*)shmaddr+12);//increase baseUsed
									
				int2chars(newValueOffset,(unsigned char*)shmaddr+16);//set value offset which will be used next
				
				//head finish
				//index area begin
				*((unsigned char*)indexOffset) = STATUS_INUSED;
				dataOffset = head->valueOffset;//
				int2chars(dataOffset,(unsigned char*)indexOffset+1);//写入值偏移量
				memset((unsigned char*)indexOffset+5,0,4);//nextIndex = 0
				//index area finish
				return 	(unsigned char*)shmaddr+dataOffset;
			}
			break;
			default:
			SIM_TRACE("invalid status:%d\n",*((unsigned char*)indexOffset));
			return NULL;
			break;
		}
	}
	
}

static int getLock(HANDLE semid,void *sbSt) {
	int rv = 0;		
	
#if __IS_WIN__
	if (WaitForSingleObject(semid,MAX_WAIT_WHEN_GET_LOCAK) != WAIT_OBJECT_0) {
		perror("WaitForSingleObject error:");
		rv = ERROR_GET_LOCK;			
	}
#else
	struct sembuf *sb = (struct sembuf *)sbSt;
	struct timespec time;
	memset(&time, 0, sizeof(time));

	sb->sem_num = 0;
	sb->sem_op = -1;
	sb->sem_flg = SEM_UNDO;
	time.tv_sec = 0;
	time.tv_nsec = MAX_WAIT_WHEN_GET_LOCAK;

	if (semop(semid,sb,1) == -1) {
		perror("semtimedop error:");
		rv = ERROR_GET_LOCK;		
	}
#endif
	SIM_TRACE("semid : %d\n",semid);
	return rv;
}

static void releaseLock(HANDLE semid,void *sbSt) {
#if __IS_WIN__
	ReleaseMutex(semid);
#else
	struct sembuf *sb = (struct sembuf *)sbSt;
	sb->sem_num = 0;
	sb->sem_op = 1;
	sb->sem_flg = 0;
	if (semop(semid,sb,1) == -1) {
		perror("sem release error:");
	}
#endif
}
/**
 * put the value into hashtable.
 * 
 * @param [in] STHashShareHandle *handle
 * @param [in] const char *key
 * @param unsigned short keyLen
 * @param [in] const char*value
 * @param unsigned short valueLen
 * 
 * @return int the result
 */
int shmdb_put(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	const char *value,unsigned short valueLen)
{
	if (handle == NULL || handle->shmid == 0) {		
		return ERROR_SHM_NOT_INIT;
	}
	if (keyLen > MAX_LEN_OF_KEY) {
		return ERROR_TOO_LONG_KEY;
	}
	if (valueLen > MAX_LEN_OF_ELEMENT) {
		return ERROR_TOO_LONG_VALUE;
	}
	if (handle == NULL) {
		return ERROR_SHM_NOT_INIT;
	}
	if (key == NULL) {
		return ERROR_NULL_KEY;
	}
	if (value == NULL) {
		return ERROR_NULL_VALUE;
	} else {

		STHashShareMemHead head;
		int rv = 0;
#if __IS_WIN__
		rv = getLock(handle->semid,NULL);
#else
		struct sembuf sb;
		rv = getLock(handle->semid,&sb);
#endif
		if (rv != 0) {				
			goto end;
		}

		memset(&head,0,sizeof(STHashShareMemHead));
		shmdb_getInfo(handle,&head);
		if (head.valueOffset + BASE_SIZE_OF_ST_MEM_DATA + keyLen + valueLen >= head.memLen) {
			rv = ERROR_NOT_ENOUGH;//not enough share memory
			goto end;
		}
		if (head.totalUsed < head.totalLen) {//can create an index 
			unsigned int index = getHashNum(key,keyLen,head.baseLen);//get hash index number by the key string
			LPVOID shmaddr = (LPVOID)handle->shmaddr;
			//the MemIndex's offset in share memory
			unsigned char*valueArea = (unsigned char*)getValueArea(&head,shmaddr,key,keyLen,valueLen,&index,0);
			
			SIM_TRACE("the index wanna put:%d,dataoffset:0x%x\n",index,(valueArea-(unsigned char*)shmaddr));//
			if (valueArea == NULL) {
				rv = ERROR_GET_INDEX;
				goto end;
			} else {
				int2chars(index,(unsigned char*)valueArea);//save the index in value area
				valueArea += 4;
				short2chars(keyLen,(unsigned char*)valueArea);//save the key length in value area

				valueArea += 2;
				memcpy(valueArea,key,keyLen);//key bytes data
				valueArea += keyLen;
				short2chars(valueLen,(unsigned char*)valueArea);//value length
				valueArea += 2;
				memcpy(valueArea,value,valueLen);//value bytes data
			}
			
			
		} else {
			rv = ERROR_FULL_USED;
			goto end;
		}
end:
		if (rv != ERROR_GET_LOCK) {
#if __IS_WIN__
			releaseLock(handle->semid,NULL);
#else
			releaseLock(handle->semid,&sb);
#endif
		}
		return rv;
		
	}	
	
}
/**
* search the position of the index when get data
*/
static int getIndex(LPVOID shmaddr,
	const char *key,unsigned short keyLen,unsigned int index,STValueAreaData *stValueAreaData) {
	//
	void *indexOffset = (unsigned char*)shmaddr + SIZE_OF_ST_HASH_SHARE_MEM_HEAD + index * SIZE_OF_ST_MEM_INDEX;
	unsigned int dataOffset = 0;
	void *valueArea = NULL;
	unsigned char status = *((unsigned char*)indexOffset);
	int rv = ERROR_NOT_FOUND_INDEX;
	if (stValueAreaData == NULL) {
		return ERROR_NULL_MEMDATA;
	}
	SIM_TRACE("index:%d,status:%d\n",index,status);
	switch(status) {
		case STATUS_UNSED:
		break;
		case STATUS_INUSED:
		{
			
			unsigned short existKeyLen = 0;
			chars2int((unsigned char*)indexOffset+1,&dataOffset);//get the data area address
			SIM_TRACE("dataOffset:%d\n",dataOffset);
			valueArea = (unsigned char*)shmaddr + dataOffset;
			chars2short((unsigned char*)valueArea+4,&existKeyLen);///get key length
			SIM_TRACE("existKeyLen:%d\n",existKeyLen);
			if (keyLen == existKeyLen && strncmp((char *)valueArea+6,key,keyLen) == 0) {
				unsigned short existValueLen = 0;
				stValueAreaData->index = index;				
				chars2short((unsigned char*)valueArea+6+keyLen,&existValueLen);///
				stValueAreaData->valueLen = existValueLen;
				stValueAreaData->pValue = (unsigned char*)valueArea+6+keyLen+2;
				return 0;
			} else {
				chars2int((unsigned char*)valueArea+5,&index);
				if (index == 0) {
					return ERROR_NOT_FOUND_INDEX;
				} else {
					return getIndex(shmaddr,key,keyLen,index,stValueAreaData);
				}
			}			
		}		
		break;
		case STATUS_DEL: {
			chars2int((unsigned char*)indexOffset+5,&index);
			if (index == 0) {
				return ERROR_NOT_FOUND_INDEX;
			} else {
				return getIndex(shmaddr,key,keyLen,index,stValueAreaData);
			}
		}
		
		break;
		default:
		break;
	}
	return rv;
}

static int shmdb_getOrDelete(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	char **value,unsigned short *valueLen,int operation) {
	STHashShareMemHead head;

	unsigned int index = 0;
	int rv = 0;
#if __IS_WIN__	
#else
	struct sembuf sb;
#endif
	if (handle == NULL || handle->shmid == 0) {		
		return ERROR_SHM_NOT_INIT;
	}

#if __IS_WIN__
	rv = getLock(handle->semid,NULL);
#else	
	rv = getLock(handle->semid,&sb);	
#endif
	
	if(rv != 0) {		
		goto end;
	} else {
		LPVOID shmaddr = (LPVOID)handle->shmaddr;
		STValueAreaData valueAreaData;
		
		memset(&valueAreaData,0,sizeof(STValueAreaData));
		memset(&head,0,sizeof(STHashShareMemHead));
		shmdb_getInfo(handle,&head);
		index = getHashNum(key,keyLen,head.baseLen);
		SIM_TRACE("get the index:%d\n",index);
		
		rv = getIndex(shmaddr,key,keyLen,index,&valueAreaData);
		if (rv == 0) {
			SIM_TRACE("get value Length:%d\n",valueAreaData.valueLen);

			if (valueLen != NULL) {
				*valueLen = valueAreaData.valueLen;
				*value = (char *)malloc(sizeof(char)*(valueAreaData.valueLen+1));
				if (*value == NULL) {
					rv = ERROR_MALLOC_MEMORY;
					goto end;
				}
				memset(*value,0,valueAreaData.valueLen+1);
				memcpy(*value,valueAreaData.pValue,valueAreaData.valueLen);
			}
			
			if (operation == OPERATION_DELETE) {
				
				void *indexOffset = (unsigned char*)shmaddr + SIZE_OF_ST_HASH_SHARE_MEM_HEAD + valueAreaData.index * SIZE_OF_ST_MEM_INDEX;
				*((unsigned char*)indexOffset) = STATUS_DEL;
			}
		}
	}

	
	
end:
	if (rv != ERROR_GET_LOCK) {
#if __IS_WIN__
			releaseLock(handle->semid,NULL);
#else
			releaseLock(handle->semid,&sb);
#endif
	}
	return rv;
}
/**
* get value of hashtable via key
*
* @param [in] STHashShareHandle *handle
* @param [in] const char *key
* @param unsigned short keyLen
* @param [out]  char**value 
*               if the pointer of valueLen is NULL,it would not allocate memory for `value`
* @param [out] unsigned short *valueLen
* 
* @return int the result
*/
int shmdb_get(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	char **value,unsigned short *valueLen) {
	return shmdb_getOrDelete(handle,key,keyLen,value,valueLen,OPERATION_GET);
}
/**
* delete value of hashtable via key,you will get the deleted value if you pass the pointer
* of valueLen not NULL.
*
* @param [in] STHashShareHandle *handle
* @param [in] const char *key
* @param unsigned short keyLen
* @param [out]  char**value the value of 
*               if the pointer of valueLen is NULL,it would not allocate memory for `value`
* @param [out] unsigned short *valueLen
*
* @return int the result
*/
int shmdb_delete(STHashShareHandle *handle,const char *key,unsigned short keyLen,
	char **value,unsigned short *valueLen) {
	return shmdb_getOrDelete(handle,key,keyLen,value,valueLen,OPERATION_DELETE);
}
/**
* dump current hashmap to a local file.
* @notProcessSafe
* 
* @param [in] STHashShareHandle *handle
* @param [in]char *path the path you wanna save the dump file
*/
int shmdb_dump(STHashShareHandle *handle,char *path) {
	int rv = 0;
	if (handle == NULL || handle->shmid == 0) {		
		return ERROR_SHM_NOT_INIT;
	}
	if (path == NULL) {
		return ERROR_PATH_NULL;
	} else {
		FILE *fp = NULL;
		STHashShareMemHead head;
#if __IS_WIN__
		if ((fopen_s(&fp,path,"wb")) != 0) {
			return ERROR_FOPEN_ERROR;
#else
		if ((fp = fopen(path,"wb")) == NULL) {
			return ERROR_FOPEN_ERROR;
#endif
		
		} else {
			void *shmaddr = (void *)handle->shmaddr;
			shmdb_getInfo(handle,&head);
			fwrite(shmaddr,sizeof(char),head.memLen,fp);
		}
		
	}
	return rv;
}
/**
* destroy the shmdb,it will remove the shared memory form the system.
* you should call this function when you exist you application normal.
* @param STHashShareHandle *handle
* 
*/
int shmdb_destroy(STHashShareHandle *handle) {
	
	if (handle != NULL) {
#if __IS_WIN__
		if (handle->semid > 0) {
			CloseHandle(handle->semid);
			handle->semid = NULL;
		}		
		
		if (handle->shmid > 0) {			
			UnmapViewOfFile((LPVOID)handle->shmaddr);
			handle->shmaddr = 0;
			CloseHandle(handle->shmid);
			handle->semid = NULL;
		}
#else 
		HANDLE shmid = handle->shmid;
		
		if (shmid > 0) {
			int ret;
			ret = shmctl(shmid, IPC_RMID, 0);
			if (ret) {
				perror("delete shm:");
				return ERROR_DESTORY_SHM;
			} else {
				handle->shmid = 0;
			}
		}
#endif
	}

	return 0;
}

