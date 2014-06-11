#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "errcode.h"
#include "mm.h"
#include "prime.h"
#include "hash.h"
#include "transform.h"

#define BUFFER_SIZE 2048

static unsigned int getRealMapLen(unsigned int size) {
	unsigned int len = 0,space = 0;
	unsigned int maxPrime = getMaxPrime(size);
	space = (maxPrime * 2);
	len += (SIZE_OF_ST_HASH_SHARE_MEM_HEAD + space*SIZE_OF_ST_MEM_INDEX + space * MAX_LEN_OF_ELEMENT);
	return len;
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
 */
int mm_initParent(STHashShareHandle *handle,unsigned int size) {
	int rv;	
	
	unsigned int totalLen = size * 2;
	unsigned int baseLen = size;
	void *shm_addr;
	char totalLenBytes[4] = {0};
	char baseLenBytes[4] = {0};
	char valueOffsetBytes[4] = {0};
	char memLenBytes[4] = {0};
	int id = 0;
	int semid = 0;
	
	unsigned int memLen = 0;
	unsigned int space = 0,valueOffset = 0;
	unsigned int maxPrime = getMaxPrime(size);//the max prime between 0 and size
	space = (maxPrime * 2);//
	valueOffset = SIZE_OF_ST_HASH_SHARE_MEM_HEAD + space*SIZE_OF_ST_MEM_INDEX;
	memLen += (valueOffset + space * MAX_LEN_OF_ELEMENT);
	printf("the len wanna get:%d\n",memLen);
	
	if ((semid = semget(IPC_PRIVATE,1,IPC_CREATE | 0600)) < 0) {
		perror("semget error:");
		rv = errno;
		return rv;
	}
	
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
	//memset(shm_addr+16,0,memLen-16);
	
	printf("to set header:\n");
	int2chars(totalLen,totalLenBytes);
	int2chars(baseLen,baseLenBytes);
	memcpy(shm_addr,totalLenBytes,4);
	memcpy(shm_addr+4,baseLenBytes,4);
	memset(shm_addr+8,0,4);//totalused
	memset(shm_addr+12,0,4);//baseused
	int2chars(valueOffset,valueOffsetBytes);
	memset(shm_addr+16,valueOffsetBytes,4);//value offset
	int2chars(memLen,memLenBytes);
	memset(shm_addr+20,memLenBytes);
	
	printf("set header finish:\n");
	handle->shmid = id;
	handle->semid = semid;
	handle->shmaddr = (long)shm_addr;
	printf("the shmid is %d\n",id);
	rv = 0;
	return rv;
}
/**
 * initialize the share memory and semaphore in child process.
 * repeat the calling of shmat to let child process attach to the share memory.
 *
 */
int mm_initChild(STHashShareHandle *handle) {
	if (handle == NULL) {
		printf("the handle is null\n");
		return ERROR_SHM_NOT_INIT;
	} else {
		int shmid = handle->shmid;
		void *shm_addr;
		if ((shm_addr=shmat(shmid,0,0))==(void*)-1) {
			perror("shmat error");
			
			return errno;
		}
		handle->shmaddr = (long)shm_addr;
	}
	return 0;
}
/**
 * get the hashtable's head
 */
int mm_getInfo(STHashShareHandle *handle, STHashShareMemHead *head) {
	if (handle == NULL) {
		printf("the handle is null\n");
		return ERROR_SHM_NOT_INIT;
	} else if (head != NULL){
		//int shmid = handle->shmid;
		void *shmaddr = (void *)handle->shmaddr;
		unsigned int baseLen = 0;
		unsigned int totalLen = 0;
		unsigned int totalUsed = 0;
		unsigned int baseUsed = 0;
		unsigned int valueOffset = 0;
		unsigned int memLen = 0;
		
		chars2int((const char*)shmadd,&totalLen);
		head->totalLen = totalLen;
		chars2int((const char*)(shmaddr+4),&baseLen);
		head->baseLen = baseLen;
		chars2int((const char*)(shmadd+8),&totalUsed);
		head->totalUsed = totalUsed;
		chars2int((const char*)(shmadd+12),&baseUsed);
		head->baseUsed = baseUsed;
		chars2int((const char*)(shmaddr+16),&valueOffset);
		head->valueOffset = valueOffset;
		printf("the base len is %d\n",baseLen);
		char2int((const char*)(shmaddr+20),&memLen);
		head->memLen = memLen;
	}
	return 0;
	
}
/**
 * put the value into hashtable.
 * 
 */
int mm_put(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	const char *value,unsigned short valueLen) {
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
		struct sembuf sb;
		struct timespec time;
		STHashShareMemHead head;
		
		memset(&time, 0, sizeof(time));
		memset(&head,0,sizeof(STHashShareMemHead));
		
		sb.sem_num = 0;
		sb.sem_op = -1;
		sb.sem_flg = SEM_UNDO;
		time.tv_sec = 0;
		time.tv_nsec = MAX_WAIT_WHEN_GET_LOCAK;
		
		if(semtimedop(handle->semid,&sb,1,&time)  == -1) {
			perror("semtimedop error:");
			return ERROR_GET_LOCK;
		}
		mm_getInfo(handle,&head);
		if (head.valueOffset + BASE_SIZE_OF_ST_MEM_DATA + keyLen + valueLen >= head.memLen) {
			return ERROR_NOT_ENOUGH;//not enough share memory
		}
		if (head.totalUsed < head.totalLen) {//can create an index 
			unsigned int index = getHashNum(key,strlen(key),head.baseLen);//get hash index number by the key string
			void *shmaddr = (void *)handle->shmaddr;
			//the MemIndex's offset in share memory
			void *indexOffset = shmaddr + SIZE_OF_ST_HASH_SHARE_MEM_HEAD + index * SIZE_OF_ST_MEM_INDEX;
			char status = *indexOffset;//the first byte of MemIndex indicate the stauts of it
			unsigned int dataOffset = 0;
			//char bytes[4] = {0};
			void *valueArea = shmaddr + head.valueOffset;//get the value offset
			
			
			switch(status) {
				case STATUS_UNSED: {//the MemIndex is not used
					char bytes[4] = {0};
					//head begin
					int2chars(head.totalUsed+1,bytes);
					memcpy(shmadd+8,);
					//head finish
					//index area begin
					*indexOffset = STATUS_INUSED;
					dataOffset = head.valueOffset;//
					int2chars(dataOffset,(unsigned char*)indexOffset+1);//Ð´ÈëÖµÆ«ÒÆÁ¿
					memset(indexOffset+5,0,4);//nextIndex = 0
					//index area finish
					
					//data area begin
					int2chars(index,(unsigned char*)valueArea);
					valueArea += 4;
				}	
				break;
				
			}
		} else {
			return ERROR_FULL_USED;
		}
		
	}
	
	
	
}


int main() {
	STHashShareHandle handle;
	int rv = mm_initParent(&handle,60);
	printf("Created shared memory status:\n");
	
    system("ipcs -m");
	if (rv == 0) {
		int pid = 0;
		printf("return shmid:%d\n",handle.shmid);
		if ((pid = fork()) < 0) {
			perror("fork error");
			
		} else if (pid == 0) {
			STHashShareHandle childHandle;
			int rvc;
			childHandle.shmid = handle.shmid;
			childHandle.semid = handle.semid;
			system("ipcs -m");
			rvc = mm_initChild(&childHandle);
			if (rvc == 0) {
				printf("init child success\n");
				mm_getInfo(&childHandle,NULL);
			}
		}
	}
	/*  if (rv == 0) {
		char buffer[128] = {0};
		printf("delete the get shm\n");
		sprintf(buffer,"ipcrm -m %d",shmid);
		printf("%s\n",buffer);
		system(buffer);
		system("ipcs -m");
	}  */
}