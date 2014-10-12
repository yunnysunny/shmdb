#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mm.h"
#include "errcode.h"

int main()
{
	STHashShareHandle handle;
	int rv = shmdb_initParent(&handle,60);
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
			rvc = shmdb_initChild(&childHandle);
			if (rvc == 0) {
				const char *key = "key";
				unsigned short keyLen = (unsigned short)(strlen(key));
				const char *value = "value";
				unsigned short valueLen = (unsigned short)(strlen(value));
				printf("init child success\n");
				shmdb_getInfo(&childHandle,NULL);
                rvc = shmdb_put(&childHandle,key,keyLen,value,valueLen);
				
				printf("the result of shmdb_put:%x\n",rvc);
				if (rvc == 0) {
					char *getValue = NULL;
					unsigned short getValueLen = 0;
					shmdb_dump(&childHandle,"/tmp/dump.data");
					rvc = shmdb_get(&childHandle,key,keyLen,&getValue,&getValueLen);
					printf("the result of shmdb_get:%x\n",rvc);
					if (rvc == 0) {
						printf("point getValue:0x%x\n getValueLen:%d\n",getValue,getValueLen);
						if (getValue != NULL) {
							printf("the value is %s\n",getValue);
							free(getValue);
							getValue = NULL;
						} else {
							
						}
						{
							char *svalue = "this is new value";
							
							rvc = shmdb_put(&childHandle,key,keyLen,svalue,(unsigned short)strlen(svalue));
							if (rvc > 0) {
								printf("shmdb_put second error:%x\n",rvc);
								return rvc;
							}
							printf("shmdb_put second success\n");
							shmdb_dump(&childHandle,"/tmp/dump2.data");
							rvc = shmdb_get(&childHandle,key,keyLen,&getValue,&getValueLen);
							if (rvc > 0) {
								printf("shmdb_get second error:%x\n",rvc);
								return rvc;
							}
							printf("shmdb_get second success\n");
							if (getValue != NULL) {
								printf("the second value is :%s\n",getValue);
							} else {
								free(getValue);
								getValue = NULL;
							}
							rvc = shmdb_delete(&childHandle,key,keyLen,NULL,NULL);
							if (rvc > 0) {
								printf("shmdb_delete error:%x\n",rvc);
								return rvc;
							}
							rvc = shmdb_get(&childHandle,key,keyLen,&getValue,&getValueLen);
							if (rvc == ERROR_NOT_FOUND_INDEX) {
								printf("the key:%s has been deleted.\n",key);
							} else {
								printf("the result of shmdb_get:%x,something is error.\n",rvc);
							}
						}
						
						
					}
				}
			}
		} else {
			int status;
			wait(&status);
			/* while(1) {
				sleep(1);
			} */
			shmdb_destroy(&handle);
		}
	}
	return 0;
}