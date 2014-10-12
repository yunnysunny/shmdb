#include "transform.h"
#include <stdio.h>

int int2chars(unsigned int num,unsigned char* chars) {
	if (chars != NULL)
	{
		chars[0] = 	num>>24;
		chars[1] = (num<<8)>>24;
		chars[2] = (num<<16)>>24;
		chars[3] = (num<<24)>>24;
	}	
	return 0;
}

int chars2int(unsigned char* chars,unsigned int *num) {

	unsigned int a = chars[0];
	unsigned int b = chars[1];
	unsigned int c = chars[2];
	unsigned int d = chars[3];

	*num = (a<<24) + (b<<16) + (c<<8) + d;

	return 0;
}

int short2chars(unsigned  short num,unsigned char *chars) {
	if (chars != NULL) {
		chars[0] = num >> 8;
		chars[1] = (num << 8) >> 8; 
	}
	return 0;
}

int chars2short(unsigned char *chars,unsigned  short *num) {
	unsigned short a = chars[0];
	unsigned short b = chars[1];
	*num = (a<<8) + b;
	return 0;
}
