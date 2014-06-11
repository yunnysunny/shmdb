#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

int int2chars(int num,unsigned char* chars) ;

int chars2int(unsigned char* chars,unsigned int *num);

#ifdef __cplusplus
}
#endif

#endif