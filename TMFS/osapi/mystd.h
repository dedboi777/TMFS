#pragma once
#include <stdio.h>
#include "osapi.h"

#define kprintf(f, args ...)printf(f "\n", args)

#define cmp(dst,src,n)          memorycmp(dst,src,n,false)
#define copy(dst,src,n)         memorycopy(dst,src,n,false)
#define stringcopy(dst,src,n)   memorycopy(dst,src,n,true)
#define stringcmp(dst, src, n)  memorycmp(dst,src,n,true)
#define findcharl(str, char)         findchar(str,char,true)
#define findcharr(str, char)         findchar(str,char,false)


//we start from the left of the byte and shift to the left
//  by "position" to get the bit and we do that by masking 
// the byte with 1 followed by "position" zeros
#define getbit_(b,p)      ((b &(1 << (p))) >> (p)) //b: byte, p: position
#define unsetbit_(b,p)     (b & ~(1 << (p)))
#define setbit_(b,p)     (b | (1 << (p))) 

internal void zero(int8*, int16);
internal void memorycopy(int8*, int8*, int16, bool);
internal bool memorycmp(int8*, int8*, int16, bool);
internal int16 stringlen(int8*);

internal bool getbit(int8*, int16);
internal void setbit(int8*,int16,bool);

internal void tolowercase(int8*);
internal int8 low(int8);

internal int8 *findchar(int8*, int8, bool);