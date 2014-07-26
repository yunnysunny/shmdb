#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mm.h"

int main()
{
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
				const char *key = "key";
				unsigned short keyLen = (unsigned short)(strlen(key));
				const char *value = "value";
				unsigned short valueLen = (unsigned short)(strlen(value));
				printf("init child success\n");
				mm_getInfo(&childHandle,NULL);
                rvc = mm_put(&childHandle,key,keyLen,value,valueLen);
				
				printf("the result of mm_put:%x\n",rvc);
				if (rvc == 0) {
					char *getValue = NULL;
					unsigned short getValueLen = 0;
					mm_dump(&childHandle,"/tmp/dump.data");
					rvc = mm_get(&childHandle,key,keyLen,&getValue,&getValueLen);
					printf("the result of mm_get:%x\n",rvc);
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
							
							rvc = mm_put(&childHandle,key,keyLen,svalue,(unsigned short)strlen(svalue));
							if (rvc > 0) {
								printf("mm_put second error:%x\n",rvc);
								return rvc;
							}
							printf("mm_put second success\n");
							mm_dump(&childHandle,"/tmp/dump2.data");
							rvc = mm_get(&childHandle,key,keyLen,&getValue,&getValueLen);
							if (rvc > 0) {
								printf("mm_get second error:%x\n",rvc);
								return rvc;
							}
							printf("mm_get second success\n");
							if (getValue != NULL) {
								printf("the second value is :%s\n",getValue);
							}
						}
						
						
					}
				}
			}
		} else {
			int status;
			wait(&status);
		}
	}
	return 0;
}