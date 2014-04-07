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
	len += (SIZE_OF_ST_HASH_SHARE_MEM + space*SIZE_OF_ST_MEM_INDEX + space * MAX_LEN_OF_ELEMENT);
	return len;
}

int mm_initParent(STHashShareHandle *handle,unsigned int size) {
	int rv;
	unsigned int memLen = getRealMapLen(size);
	unsigned int totalLen = size * 2;
	unsigned int baseLen = size;
	void *shm_addr;
	char totalLenBytes[4] = {0};
	char baseLenBytes[4] = {0};
	int id = 0;

	printf("the len wanna get:%d\n",memLen);
	
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
	printf("to set header:\n");
	int2chars(totalLen,totalLenBytes);
	int2chars(baseLen,baseLenBytes);
	memcpy(shm_addr,totalLenBytes,4);
	memcpy(shm_addr+4,baseLenBytes,4);
	memset(shm_addr+8,0,4);
	memset(shm_addr+12,0,4);
	
	printf("set header finish:\n");
	handle->shmid = id;
	handle->shmaddr = (long)shm_addr;
	printf("the shmid is %d\n",id);
	rv = 0;
	return rv;
}

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

int mm_getInfo(STHashShareHandle *handle) {
	if (handle == NULL) {
		printf("the handle is null\n");
		return ERROR_SHM_NOT_INIT;
	} else {
		//int shmid = handle->shmid;
		void *shmaddr = (void *)handle->shmaddr;
		int baseLen = 0;
		
		chars2int((const char*)(shmaddr+4),&baseLen);
		printf("the base len is %d\n",baseLen);
		
	}
	return 0;
	
}

int mm_put(STHashShareHandle *handle,const char*key,const char *value) {
	
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
			system("ipcs -m");
			rvc = mm_initChild(&childHandle);
			if (rvc == 0) {
				printf("init child success\n");
				mm_getInfo(&childHandle);
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