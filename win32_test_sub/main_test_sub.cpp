#include <stdio.h>
#include <windows.h>
#include<iostream>  

#include "../include/mm.h"
int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("参数个数必须为3个\n");
		return -1;
	}
	HANDLE shmid     = NULL;  
	HANDLE semid	= NULL;
	//LPVOID shmaddr    = NULL; 
	int rv;
	sscanf_s(argv[1],"%d",&semid);
	sscanf_s(argv[2],"%d",&shmid);
	printf("%d %d\n",shmid,semid);

	STHashShareHandle stHashShareHandle;
	memset(&stHashShareHandle,0,sizeof(STHashShareHandle));
	stHashShareHandle.semid = semid;
	stHashShareHandle.shmid = shmid;
	rv = shmdb_initChild(&stHashShareHandle);
	if (rv != 0) {
		printf("shmdb_initChild error:0x:%x\n",rv);
		return rv;
	} else {
		printf("init child success %d\n",rv);
	}

	char *getValue = NULL;
	unsigned short getValueLen = 0;
	const char *key = "key";
	unsigned short keyLen = (unsigned short)(strlen(key));
	shmdb_dump(&stHashShareHandle,"d:\\temp\\dump3.dp");
	rv = shmdb_get(&stHashShareHandle,key,keyLen,&getValue,&getValueLen);
	shmdb_dump(&stHashShareHandle,"d:\\temp\\dump4.dp");
	if (rv != 0) {
		printf("shmdb_get error:0x%x\n",rv);
		return rv;
	}
	if (getValue != NULL) {
		printf("the value is %s\n",getValue);
		free(getValue);
		getValue = NULL;
	} else {
							
	}
	getchar();
	return 0;
}