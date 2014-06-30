#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
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

	int id = 0;
	int semid = 0;
	
	unsigned int memLen = 0;
	unsigned int space = 0,valueOffset = 0;
	unsigned int maxPrime = getMaxPrime(size);//the max prime between 0 and size
	space = (maxPrime * 2);//
	valueOffset = SIZE_OF_ST_HASH_SHARE_MEM_HEAD + space*SIZE_OF_ST_MEM_INDEX;
	memLen += (valueOffset + space * MAX_LEN_OF_ELEMENT);
	printf("the len wanna get:%d\n",memLen);
	
	if ((semid = semget(IPC_PRIVATE,1,IPC_CREAT | 0600)) < 0) {
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
	int2chars(totalLen,(unsigned char*)shm_addr);
	int2chars(baseLen,(unsigned char*)shm_addr+4);
		
	memset(shm_addr+8,0,4);//totalused
	memset(shm_addr+12,0,4);//baseused
	int2chars(valueOffset,(unsigned char*)shm_addr+16);//value offset
	
	int2chars(memLen,(unsigned char*)shm_addr+20);//memLen
	
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
		
		chars2int((unsigned char*)shmaddr,&totalLen);
		head->totalLen = totalLen;
		chars2int((unsigned char*)(shmaddr+4),&baseLen);
		head->baseLen = baseLen;
		chars2int((unsigned char*)(shmaddr+8),&totalUsed);
		head->totalUsed = totalUsed;
		chars2int((unsigned char*)(shmaddr+12),&baseUsed);
		head->baseUsed = baseUsed;
		chars2int((unsigned char*)(shmaddr+16),&valueOffset);
		head->valueOffset = valueOffset;
		printf("the base len is %d\n",baseLen);
		char2int((unsigned char*)(shmaddr+20),&memLen);
		head->memLen = memLen;
	}
	return 0;
	
}

static void *getValueArea(STHashShareMemHead *head,void *shmaddr,
	unsigned short keyLen, unsigned short valueLen,
	unsigned int *index,unsigned int lastIndex) {
	if (*index >= head->totalLen) {
		return NULL;
	}
	{
		void *indexOffset = shmaddr + SIZE_OF_ST_HASH_SHARE_MEM_HEAD + *index * SIZE_OF_ST_MEM_INDEX;
		unsigned int dataOffset = 0;
		void *valueArea = NULL;
		unsigned char status = (unsigned char)(*indexOffset);
		
		
		switch (status) {
			case STATUS_DEL: {
				unsigned short existValueLen = 0;
				unsigned short existKeyLen = 0;
				chars2int(indexOffset+1,&dataOffset);
				valueArea = shmaddr + dataOffset;
				chars2short((unsigned char*)valueArea+4,&existKeyLen);///
				chars2short((unsigned char*)valueArea+6,&existValueLen);///
				if ((existKeyLen + existValueLen) <= (keyLen + valueLen)) {//find a index

					*((unsigned char*)indexOffset) = STATUS_INUSED;
					//int2chars(valueArea+4,keyLen);

					int2chars(head->totalUsed+1, (unsigned char*)shmaddr+8);
					
					if (lastIndex > head->baseLen) {//lastIndex is in zipper area
						
					} else {//lastIndex is in base area
						int2chars(head->baseUsed+12,(unsigned char*)shmaddr+1);						
					}
					return valueArea;
					//break;
				} else {// to test the next index in the zipper area
					unsigned int existIndex = 0;
					chars2int(((unsigned char*)indexOffset+5), &existIndex);
					if (existIndex > 0) {
						lastIndex = *index;
						*index = existIndex;
						
						return getValueArea(head,shmaddr,keyLen,valueLen,index,lastIndex);
					} else {
						lastIndex = *index;
						*index = head->baseLen + (head->totalUsed-head->baseUsed);
						return getValueArea(head,shmaddr,keyLen,valueLen,index,lastIndex);
					}
					
				}
			}
			break;
			case STATUS_INUSED: {// to test the next index in the zipper area
				unsigned int existIndex = 0;
				chars2int(((unsigned char*)indexOffset+5), &existIndex);
				if (existIndex > 0) {
					lastIndex = *index;
					*index = existIndex;
					
					return getValueArea(head,shmaddr,keyLen,valueLen,index,lastIndex);
				} else {
					lastIndex = *index;
					*index = head->baseLen + (head->totalUsed-head->baseUsed);
					return getValueArea(head,shmaddr,keyLen,valueLen,index,lastIndex);
				}
			}
			break;
			case STATUS_UNSED: {
				
				unsigned int newValueOffset = head->valueOffset + keyLen + valueLen;//the value offset will be used next 
				//head begin
				int2chars(head->totalUsed+1,(unsigned char*)shmaddr+8);//increase totalUsed
				
				int2chars(head->baseUsed+1,(unsigned char*)shmaddr+12);//increase baseUsed
									
				int2Chars(newValueOffset,(unsigned char*)shmaddr+16);//set value offset which will be used next
				
				//head finish
				//index area begin
				*((unsigned char*)indexOffset) = STATUS_INUSED;
				dataOffset = head->valueOffset;//
				int2chars(dataOffset,(unsigned char*)indexOffset+1);//Ð´ÈëÖµÆ«ÒÆÁ¿
				memset(indexOffset+5,0,4);//nextIndex = 0
				//index area finish
				return 	shmaddr+newValueOffset;
			}
			break;
			default:
			printf("invalid status:%d\n",*((unsigned char*)indexOffset));
			return NULL;
			break;
		}
	}
	
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
		int rv = 0;
		
		memset(&time, 0, sizeof(time));
		memset(&head,0,sizeof(STHashShareMemHead));
		
		sb.sem_num = 0;
		sb.sem_op = -1;
		sb.sem_flg = SEM_UNDO;
		time.tv_sec = 0;
		time.tv_nsec = MAX_WAIT_WHEN_GET_LOCAK;
		
		if(semtimedop(handle->semid,&sb,1,(const struct timespec *)&time)  == -1) {
			perror("semtimedop error:");
			rv = ERROR_GET_LOCK;
			goto end;
		}
		mm_getInfo(handle,&head);
		if (head.valueOffset + BASE_SIZE_OF_ST_MEM_DATA + keyLen + valueLen >= head.memLen) {
			rv = ERROR_NOT_ENOUGH;//not enough share memory
			goto end;
		}
		if (head.totalUsed < head.totalLen) {//can create an index 
			unsigned int index = getHashNum(key,strlen(key),head.baseLen);//get hash index number by the key string
			void *shmaddr = (void *)handle->shmaddr;
			//the MemIndex's offset in share memory
			void *valueArea = getValueArea(&head,shmaddr,keyLen,valueLen,&index,0);
			if (valueArea == NULL) {
				rv = ERROR_GET_INDEX;
				goto end;
			} else {
				int2chars(index,(unsigned char*)valueArea);
				valueArea += 4;
				memcpy(valueArea,key,keyLen);
				valueArea += keyLen;
				memcpy(valueArea,value,valueLen);
			}
			
			
		} else {
			rv = ERROR_FULL_USED;
			goto end;
		}
end:
		if (rv != ERROR_GET_LOCK) {
			sb.sem_num = 0;
			sb.sem_op = 1;
			sb.sem_flg = 0;
			if (semop(semid,&sb,1) == -1) {
				perror("sem release error:");
			}
		}
		return rv;
		
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