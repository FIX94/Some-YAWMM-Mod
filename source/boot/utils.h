#ifndef __UTILS_H__
#define __UTILS_H__

typedef unsigned char u8;
typedef unsigned char uint8_t;
typedef unsigned short u16;
typedef unsigned short uint16_t;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef int bool;
typedef unsigned int sec_t;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;
typedef volatile signed long long vs64;

typedef s32 size_t;
typedef u32 u_int32_t;

#define NULL 		((void*)0)
#define true 		1
#define false		0
#define IsWiiU		((*(u32*)0xcd8005A0 >> 16 ) == 0xCAFE)

void sync_before_read(void* ptr, u32 len);
void sync_after_write(const void* ptr, u32 len);
void _memcpy(void *ptr, const void *src, u32 size);
void _memset32(unsigned int *addr, unsigned int data, unsigned int count);
int _memcmp(const void *s1, const void *s2, size_t n);
size_t strnlen(const char *s, size_t count);
void write32(u32 value, u32 dest);
u32 read32(u32 src);

#endif