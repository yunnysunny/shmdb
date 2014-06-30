#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

int int2chars(unsigned int num,unsigned char* chars) ;

int chars2int(unsigned char* chars,unsigned int *num);

int short2chars(unsigned  short num,unsigned char *chars);

int chars2short(unsigned char *chars,unsigned  short *num);

#ifdef __cplusplus
}
#endif

#endif