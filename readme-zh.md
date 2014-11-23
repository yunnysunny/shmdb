#shmdb 
基于共享内存的内存数据库

## 1.能做什么
shmdb旨在解决多进程的内存共享问题。在linux或者windows下，操作系统提供共享内存的功能，可以通过调用系统函数申请一段内存区域，shmdb就是利用了操作系统的这个特性，构架一个key-value类型的数据库。使用时，通过函数`shmdb_put`来设置一对key-value,同样通过函数`shmdb_get`来获取一对key-value。

## 2.不能做什么
shmdb没有提供网络访问功能，仅仅只能嵌入到应用程序中来运行。所以不适合做网络数据库使用。

## 3.API

### 3.1 shmdb_initParent 初始化父进程

	int shmdb_initParent(STHashShareHandle *handle,unsigned int size)

**参数**

- [in|out] STHashShareHandle *handle 函数内部会给handle的shmid成员变量赋值
- unsigned int size 指定base区域的长度  
- [in] STShmdbOption *option 日志选项，可以为NULL


**返回值**	

- int 操作结果

### 3.2 shmdb_initChild 初始化子进程

	int shmdb_initChild(STHashShareHandle *handle)	
**参数** 
   
- [in] STHashShareHandle *handle 函数内部会根据handle的shmid值来将共享内存挂载到自己的内存区域  
 
**返回值**	

- int 操作结果

### 3.3 shmdb_put 写入值

	int shmdb_put(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	const char *value,unsigned short valueLen)

**参数** 
   
- [in] STHashShareHandle *handle 函数内部读取handle内shmid值来操作共享内存  
- [in] const char *key   
- unsigned short keyLen   
- [in] const char *value   
- unsigned short valueLen   

**返回值**	

- int 操作结果

### 3.4 shmdb_get 获取值

	shmdb_get(STHashShareHandle *handle,const char*key,unsigned short keyLen,
	char **value,unsigned short *valueLen)

**参数** 
   
- [in] STHashShareHandle *handle 函数内部读取handle内shmid值来操作共享内存  
- [in] const char *key   
- unsigned short keyLen   
- [out]  char **value 函数在内部申请`value`的内存空间，调用完成后要手动调用free释放。如果`valueLen`的值为NULL，则函数内部不会对`value`申请内存。                          
- unsigned short *valueLen 

**返回值**	

- int 操作结果

### 3.5 shmdb_delete 删除值

	int shmdb_delete(STHashShareHandle *handle,const char *key,unsigned short keyLen,
	char **value,unsigned short *valueLen) 

**参数** 
   
- [in] STHashShareHandle *handle 函数内部读取handle内shmid值来操作共享内存  
- [in] const char *key   
- unsigned short keyLen   
- [out]  char **value 函数在内部申请得到的内容的内存空间，调用完成后要手动调用free释放。如果`valueLen`的值为NULL，则函数内部不会对`value`申请内存。                          
- unsigned short *valueLen 

**返回值**	

- int 操作结果

### 3.6 shmdb_destroy 销毁
> 在程序正常退出时，应该手动调用该函数将共享内存从操作系统中移除。

	int shmdb_destroy(STHashShareHandle *handle)

**参数** 
   
- [in] STHashShareHandle *handle 函数内部读取handle内shmid值来操作共享内存  


**返回值**	

- int 操作结果

## 贡献者
[yunnysunny](https://github.com/yunnysunny) (maintainer)

**License:** [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
	

