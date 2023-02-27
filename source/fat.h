#ifndef _FAT_H_
#define _FAT_H_


/* 'FAT File' structure */
typedef struct 
{
	/* Filename */
	char filename[128];
	/* 1 = Batch Install, 2 = Batch Uninstall - Leathl */
	int install;
	
	int installstate;

	/* Filestat */
	bool isdir;
	bool isdol;
	bool iself;
	bool iswad;
	size_t fsize;
} fatFile;

/* Prototypes */
void FatMount();
void FatUnmount();
char* FatGetDeviceName(u8 index);
char* FatGetDevicePrefix(u8 index);
s32 FatGetDeviceCount();

#endif
