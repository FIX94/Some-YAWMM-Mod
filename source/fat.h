#ifndef _FAT_H_
#define _FAT_H_

/* libfat header */
#include <fat.h>
#include <sys/dir.h>

/* SD headers */
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>


/* 'FAT Device' structure */
typedef struct {
	/* Device mount point */
	char *mount;

	/* Device name */
	char *name;

	/* Device interface */
	const DISC_INTERFACE *interface;
} fatDevice;

/* 'FAT File' structure */
typedef struct {
	/* Filename */
	char filename[128];
	/* 1 = Batch Install, 2 = Batch Uninstall - Leathl */
	int install;
	
	int installstate;

	/* Filestat */
	bool isdir;
	size_t fsize;
	int iswad;
	int isdol;
	int iself;
} fatFile;


/* Prototypes */
s32   Fat_Mount(fatDevice *);
void  Fat_Unmount(fatDevice *);
char *Fat_ToFilename(const char *);

#endif
 
