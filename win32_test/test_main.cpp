#include <stdio.h>
#include <windows.h>
#include "../include/mm.h"

int main() {
	int ret = 0;

	STARTUPINFO si={sizeof(si)};
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);

	STHashShareHandle handle;
	int rv = shmdb_initParent(&handle,60);
	if (rv != 0) {
		return rv;
	}
	//printf("Created shared memory status:\n");
	const char *key = "key";
	unsigned short keyLen = (unsigned short)(strlen(key));
	const char *value = "value";
	unsigned short valueLen = (unsigned short)(strlen(value));
	shmdb_getInfo(&handle,NULL);
	rv = shmdb_put(&handle,key,keyLen,value,valueLen);
	if (rv != 0) {
		printf("shmdb_put error:0x%x\n",rv);
		return rv;
	}
	char *getValue = NULL;
	unsigned short getValueLen = 0;
	shmdb_dump(&handle,"d:\\temp\\dump1.dp");
	rv = shmdb_get(&handle,key,keyLen,&getValue,&getValueLen);
	if (rv != 0) {
		printf("shmdb_get error:0x%x\n",rv);
		return rv;
	}
	if (getValue != NULL) {
		printf("the value is %s\n",getValue);
		free(getValue);
		getValue = NULL;
		shmdb_dump(&handle,"d:\\temp\\dump2.dp");
	} else {
							
	}

	char sConLin[256] = {0};
	sprintf_s(sConLin,"win32_test_sub.exe %d %d",(int)(handle.semid),(int)(handle.shmid));

	ret = CreateProcess(  
        NULL,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
        sConLin, // 命令行字符串  
        NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
        NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
        TRUE,//    指示新进程是否从调用进程处继承了句柄。   
        0,  //  指定附加的、用来控制优先类和进程的创建的标  
            //  CREATE_NEW_CONSOLE  新控制台打开子进程  
            //  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
        NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
        NULL, //    指定子进程的工作路径  
        &si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
        &pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
        );
	const char *key2 = "xx";
	unsigned short key2Len = (unsigned short)(strlen(key2));
	for(int i=0;i<5;i++) {
		rv = shmdb_get(&handle,key2,key2Len,&getValue,&getValueLen);
		if (rv == 0) {
			printf("%dth get xx,value is %s\n",i,getValue);
			free(getValue);
		} else {
			printf("%dth get failed:0x%x\n",rv);
		}
		Sleep(1000);
	}
	WaitForSingleObject( pi.hProcess, INFINITE );  /*Wait until child process exit*/
	if (ret != 0) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);                          /*Close handle*/
	}
	getchar();
	return 0;
}