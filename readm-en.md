# shmdb 
A memory database based on  shared memory.

## 1. can do 
shmdb shared memory multi-process aimed at resolving the problem. Under linux or windows, operating system provides a shared memory feature, you can apply for a memory area by calling the system function, shmdb is the use of this feature of the operating system, the framework of a key-value type of database. When used to set up a pair of key-value through the function `shmdb_put`, also to get a pair of key-value through the function `shmdb_get`. 

## 2. can not do 
shmdb does not provide network access functions can only be embedded into applications to run. It is not suitable for network databases. 

## 3.API 

### 3.1 shmdb_initParent parent process initializes 

	int shmdb_initParent (STHashShareHandle * handle, unsigned int size) 

 **Parameters**  

- [In | out] `STHashShareHandle * handle`	`shmid`,which is the   member variable of `handle` will ben initialize in this function  
- `unsigned int size` specified base area 
- [In] `STShmdbOption *option` the option of log,can be NULL.


 **The return value** 

- int result of the operation 

### 3.2 shmdb_initChild child process initialization 

	int shmdb_initChild (STHashShareHandle * handle) 
 **Parameters**  
   
- [In] `STHashShareHandle * handle` internal memory function to mount its own memory area based to the shared memory via the `handle`'s `shmid`    
 
 **The return value** 

- `int` result of the operation 

### 3.3 shmdb_put write value 

	int shmdb_put (STHashShareHandle * handle, const char * key, unsigned short keyLen, 
	const char * value, unsigned short valueLen) 

 **Parameters**   


- [In] `STHashShareHandle * handle` internal function to read the value of handle's shmid to manipulate shared memory     
- [In] `const char * key`   
- `unsigned short keyLen`  
- [In] `const char * value`  
- `unsigned short valueLen`  


 **The return value**  

- `int` result of the operation 

### 3.4 shmdb_get get the value 

	shmdb_get (STHashShareHandle * handle, const char * key, unsigned short keyLen, 
	char ** value, unsigned short * valueLen) 

 **Parameters**   

- [In] `STHashShareHandle * handle` internal function to read the value of handle's shmid to manipulate shared memory   
- [In] `const char * key`   
- `unsigned short keyLen`   
- [Out] `char ** value` internal function allocate the memory space of `value`, manually call the `free` function after this call is completed. If `valueLen` value is NULL, the function does not allocate the space of  `value`.   
- `unsigned short * valueLen `

 **The return value**  

- `int` result of the operation 

### 3.5 shmdb_delete delete values 

	int shmdb_delete (STHashShareHandle * handle, const char * key, unsigned short keyLen, 
	char ** value, unsigned short * valueLen) 

 **Parameters**  

- [In] `STHashShareHandle * handle` internal function to read the value of handle's shmid to manipulate shared memory   
- [In] `const char * key`   
- `unsigned short keyLen`   
- [Out] `char ** value` internal function allocate the memory space of `value`, manually call the `free` function after this call is completed. If `valueLen` value is NULL, the function does not allocate the space of  `value`.   
- `unsigned short * valueLen` 

 **The return value**  

- `int` result of the operation 
### 3.6 shmdb_destroy destruction 
> When the program exits normally, you should call this function manually to remove the shared memory from the operating system. 

	int shmdb_destroy (STHashShareHandle * handle) 

 **Parameters**  

- [In] `STHashShareHandle * handle` internal function to read the value of handle's shmid to manipulate shared memory 


 **The return value** 

- `int` result of the operation

## Contributors
[yunnysunny](https://github.com/yunnysunny) (maintainer)

**License:** [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)