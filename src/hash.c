#include "prime.h"
//#include <stdio.h>

unsigned int getHashNum(const char *str,unsigned int len,unsigned int maxPrime)
{
	register unsigned int sum = 0; 
	register unsigned int h = 0;
	register unsigned char *p = (unsigned char *)str;
	register unsigned char *s = (unsigned char *)str;
	//printf("str:%s,len:%d,h:%d,maxPrime:%d\n",p,len,h,maxPrime);

	while(p - s < len) 
	{
		register unsigned short a =0;
		//printf("value:%d[%c][%d],offset:%d\n",*p,*p,p,p-s);
		a= (unsigned short)*(p++) * (p-s);
		//printf("a:%d\n",a);
		sum += sum ^ a; 
		h += a;
	}
	//printf("sum:%d\n",sum);
	return ((sum << 16) | h) % maxPrime;
}
