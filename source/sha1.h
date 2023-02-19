#ifndef _SHA1_H_
#define _SHA1_H_

void SHA1(unsigned char *, unsigned int, unsigned char *);
int CompareHash(unsigned char* first, unsigned int firstSize, unsigned char* second, unsigned int secondSize);

#endif
