/* 
 * File:   hash.h
 * Author: sunny
 *
 * Created on 2014年3月30日, 上午10:16
 */

#ifndef HASH_H
#define	HASH_H

#ifdef	__cplusplus
extern "C" {
#endif

unsigned int getHashNum(const char *str,unsigned int len,unsigned int maxPrime);


#ifdef	__cplusplus
}
#endif

#endif	/* HASH_H */

