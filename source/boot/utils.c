#include "utils.h"

void sync_before_read(void *ptr, u32 len)
{
	u32 a, b;

	a = (u32)ptr & ~0x1f;
	b = ((u32)ptr + len + 0x1f) & ~0x1f;

	for ( ; a < b; a += 32)
		asm("dcbi 0,%0" : : "b"(a) : "memory");

	asm("sync ; isync");
}

void sync_after_write(const void *ptr, u32 len)
{
	u32 a, b;

	a = (u32)ptr & ~0x1f;
	b = ((u32)ptr + len + 0x1f) & ~0x1f;

	for ( ; a < b; a += 32)
		asm("dcbf 0,%0" : : "b"(a));

	asm("sync ; isync");
}

void _memcpy(void *ptr, const void *src, u32 size)
{
	char *ptr2 = ptr;
	u32 bsize = size;
	const char* src2 = src;
	while(size--) *ptr2++ = *src2++;
	
	sync_after_write(ptr, bsize);
}

void _memset32(u32 *address, u32 data, u32 length) 
{
	while(length--)
		*address++ = data;
}

int _memcmp(const void *s1, const void *s2, size_t n)
{
	unsigned char *us1 = (unsigned char *) s1;
	unsigned char *us2 = (unsigned char *) s2;
	while(n-- != 0) 
	{
		if (*us1 != *us2)
			return (*us1 < *us2) ? -1 : +1;
		us1++;
		us2++;
	}
	return 0;
}

size_t strnlen(const char *s, size_t count)
{
	const char *sc;

	for(sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

inline void write32(u32 dest, u32 value)
{
	*(u32*)dest = value;
	sync_after_write((void*)dest, 0x20);
}

inline u32 read32(u32 src)
{
	return *(u32*)src;
}