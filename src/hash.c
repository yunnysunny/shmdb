#include "prime.h"

unsigned int getHashNum(char *str,unsigned int len,unsigned int maxPrime)
{
     register unsigned int sum = 0; 
     register unsigned int h = 0;
     register unsigned short *p = (unsigned short *)str;
     register unsigned short *s = (unsigned short *)str;


     while(p - s < len) 
     {
          register unsigned short a = *(p++) * (p-s);
          sum += sum ^ a; 
          h += a;
     }
     
     return ((sum << 16) | h) % maxPrime;
}
