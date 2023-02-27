#include <stdio.h>
#include <stdarg.h>
#include <ogcsys.h>
#include <malloc.h>
#include <string.h>

#include "nand.h"
#include "fileops.h"

#define BLOCK 2048

/* Buffer */
static u32 inbuf[8] ATTRIBUTE_ALIGN(32);
static bool gNandInitialized = false;

#if 0
static void NANDFATify(char* ptr, const char* str)
{
	char ctr;
	while ((ctr = *(str++)) != '\0')
	{
		const char* esc;
		switch (ctr)
		{
			case '"':
				esc = "&qt;";
				break;
			case '*':
				esc = "&st;";
				break;
			case ':':
				esc = "&cl;";
				break;
			case '<':
				esc = "&lt;";
				break;
			case '>':
				esc = "&gt;";
				break;
			case '?':
				esc = "&qm;";
				break;
			case '|':
				esc = "&vb;";
				break;
			default:
				*(ptr++) = ctr;
				continue;
		}
		strcpy(ptr, esc);
		ptr += 4;
	}
	*ptr = '\0';
}
#endif


s32 Nand_Mount(nandDevice *dev)
{
	s32 fd, ret;

	/* Open FAT module */
	fd = IOS_Open("fat", 0);
	if (fd < 0)
		return fd;

	/* Mount device */
	ret = IOS_Ioctlv(fd, dev->mountCmd, 0, 0, NULL);

	/* Close FAT module */
	IOS_Close(fd);

	return ret;
}

s32 Nand_Unmount(nandDevice *dev)
{
	s32 fd, ret;

	/* Open FAT module */
	fd = IOS_Open("fat", 0);
	if (fd < 0)
		return fd;

	/* Unmount device */
	ret = IOS_Ioctlv(fd, dev->umountCmd, 0, 0, NULL);

	/* Close FAT module */
	IOS_Close(fd);

	return ret;
}

s32 Nand_Enable(nandDevice *dev)
{
	s32 fd, ret;

	/* Open /dev/fs */
	fd = IOS_Open("/dev/fs", 0);
	if (fd < 0)
		return fd;

	/* Set input buffer */
	inbuf[0] = dev->mode;

	/* Enable NAND emulator */
	ret = IOS_Ioctl(fd, 100, inbuf, sizeof(inbuf), NULL, 0);

	/* Close /dev/fs */
	IOS_Close(fd);

	return ret;
} 

s32 Nand_Disable(void)
{
	s32 fd, ret;

	/* Open /dev/fs */
	fd = IOS_Open("/dev/fs", 0);
	if (fd < 0)
		return fd;

	/* Set input buffer */
	inbuf[0] = 0;

	/* Disable NAND emulator */
	ret = IOS_Ioctl(fd, 100, inbuf, sizeof(inbuf), NULL, 0);

	/* Close /dev/fs */
	IOS_Close(fd);

	return ret;
}

bool NANDInitialize()
{
	if(!gNandInitialized)
	{
		if (ISFS_Initialize() == ISFS_OK)
			gNandInitialized = true;
	}

	return gNandInitialized;
}

u8* NANDReadFromFile(const char* path, u32 offset, u32 length, u32* size)
{
	*size = ISFS_EINVAL;

	if (NANDInitialize())
	{
		s32 fd = IOS_Open(path, 1);

		if (fd < 0)
		{
			*size = fd;
			return NULL;
		}

		if (!length)
			length = IOS_Seek(fd, 0, SEEK_END);

		u8* data = (u8*)memalign(0x40, length);
		if (!data)
		{
			*size = 0;
			IOS_Close(fd);
			return NULL;
		}

		*size = IOS_Seek(fd, offset, SEEK_SET);
		if (*size < 0)
		{
			IOS_Close(fd);
			free(data);
			return NULL;
		}

		*size = IOS_Read(fd, data, length);
		IOS_Close(fd);
		if (*size != length)
		{
			free(data);
			return NULL;
		}

		return data;
	}
	
	return NULL;
}

u8* NANDLoadFile(const char* path, u32* size)
{
	return NANDReadFromFile(path, 0, 0, size);
}

s32 NANDWriteFileSafe(const char* path, u8* data, u32 size)
{
	NANDInitialize();
	
	char* tmpPath = (char*)memalign(0x40, ISFS_MAXPATH);
	u32 i;

	for (i = strlen(path); i > 0; --i)
	{
		if (path[i] == '/')
			break;
	}

	sprintf(tmpPath, "/tmp%s", path + i);

	s32 ret = ISFS_CreateFile(tmpPath, 0, 3, 3, 3);
	if (ret == -105)
	{
		ISFS_Delete(tmpPath);
		ret = ISFS_CreateFile(tmpPath, 0, 3, 3, 3);

		if (ret < 0)
		{
			free(tmpPath);
			return ret;
		}
	}
	else
	{
		if (ret < 0)
		{
			free(tmpPath);
			return ret;
		}
	}

	s32 fd = IOS_Open(tmpPath, 2);
	if (fd < 0)
	{
		free(tmpPath);
		return fd;
	}

	ret = IOS_Write(fd, data, size);


	IOS_Close(fd);
	if (ret != size)
	{
		free(tmpPath);
		return ret - 3;
	}

	if (strcmp(tmpPath, path))
		ret = ISFS_Rename(tmpPath, path);
	else
		ret = 0;
	
	free(tmpPath);
	return ret;
}

s32 NANDBackUpFile(const char* src, const char* dst, u32* size)
{
	NANDInitialize();
	u8* buffer = NANDLoadFile(src, size);
	if (!buffer)
		return *size;

	s32 ret = NANDWriteFileSafe(dst, buffer, *size);

	free(buffer);
	return ret;
}

s32 NANDGetFileSize(const char* path, u32* size)
{
	NANDInitialize();
	s32 fd = IOS_Open(path, 1);

	if (fd < 0)
	{
		*size = 0;
		return fd;
	}

	*size = IOS_Seek(fd, 0, SEEK_END);
	return IOS_Close(fd);
}

s32 NANDDeleteFile(const char* path)
{
	NANDInitialize();
	return ISFS_Delete(path);
}

#if 0
s32 NANDGetNameList(const char* src, NameList** entries, s32* count)
{
	*count = 0;
	u32 numEntries = 0;
	char currentEntry[ISFS_MAXPATH];
	char entryPath[ISFS_MAXPATH + 1];

	s32 ret = ISFS_ReadDir(src, NULL, &numEntries);

	if (ret < 0)
		return ret;

	char* names = (char*)memalign(0x40, ISFS_MAXPATH * numEntries);

	if (!names)
		return ISFS_ENOMEM;

	ret = ISFS_ReadDir(src, names, &numEntries);
	if (ret < 0)
	{
		free(names);
		return ret;
	}

	*count = numEntries;
	
	free(*entries);
	*entries = (NameList*)memalign(0x20, sizeof(NameList) * numEntries);
	if (!*entries)
	{
		free(names);
		return ISFS_ENOMEM;
	}
	
	s32 i, j, k;
	u32 dummy;
	for (i = 0, k = 0; i < numEntries; i++)
	{
		for (j = 0; names[k] != 0; j++, k++)
			currentEntry[j] = names[k];

		currentEntry[j] = 0;
		k++;

		strcpy((*entries)[i].name, currentEntry);
		
		if (src[strlen(src) - 1] == '/')
			snprintf(entryPath, sizeof(entryPath), "%s%s", src, currentEntry);
		else
			snprintf(entryPath, sizeof(entryPath), "%s/%s", src, currentEntry);

		ret = ISFS_ReadDir(entryPath, NULL, &dummy);
		(*entries)[i].type = ret < 0 ? 0 : 1;
	}

	free(names);
	return 0;
}

s32 NANDDumpFile(const char* src, const char* dst)
{
	printf("Dump file: %s\n", src);
	
	s32 fd = ISFS_Open(src, ISFS_OPEN_READ);
	if (fd < 0)
		return fd;
	
	fstats* status = (fstats*)memalign(32, sizeof(fstats));
	if (status == NULL)
		return ISFS_ENOMEM;

	s32 ret = ISFS_GetFileStats(fd, status);
	if (ret < 0)
	{
		ISFS_Close(fd);
		free(status);
		return ret;
	}
	
	FSOPDeleteFile(dst);
	FILE* file = fopen(dst, "wb");

	if (!file)
	{
		ISFS_Close(fd);
		free(status);
		return ISFS_EINVAL;
	}

	u8* buffer = (u8*)memalign(32, BLOCK);
	if (!buffer)
	{
		ISFS_Close(fd);
		free(status);
		return ISFS_ENOMEM;
	}

	u32 toRead = status->file_length;
	while (toRead > 0)
	{
		u32 size = toRead < BLOCK ? toRead : BLOCK;

		ret = ISFS_Read(fd, buffer, size);
		if (ret < 0)
		{
			ISFS_Close(fd);
			fclose(file);
			free(status);
			free(buffer);
			return ret;
		}

		ret = fwrite(buffer, 1, size, file);
		if (ret < 0)
		{
			ISFS_Close(fd);
			fclose(file);
			free(status);
			free(buffer);
			return ret;
		}
		
		toRead -= size;
	}

	fclose(file);
	ISFS_Close(fd);
	free(status);
	free(buffer);

	return ISFS_OK;
}

s32 NANDDumpFolder(const char* src, const char* dst)
{
	NameList* names = NULL;
	s32 count = 0;
	s32 i;

	char nSrc[ISFS_MAXPATH + 1];
	char nDst[1024];
	char tDst[1024];

	NANDGetNameList(src, &names, &count);
	FSOPMakeFolder(dst);
	
	for (i = 0; i < count; i++)
	{
		
		if (src[strlen(src) - 1] == '/')
			snprintf(nSrc, sizeof(nSrc), "%s%s", src, names[i].name);
		else
			snprintf(nSrc, sizeof(nSrc), "%s/%s", src, names[i].name);

		if (!names[i].type)
		{
			NANDFATify(tDst, nSrc);
			snprintf(nDst, sizeof(nDst), "%s%s", dst, tDst);
			NANDDumpFile(nSrc, nDst);
		}
		else
		{
			NANDFATify(tDst, nSrc);
			snprintf(nDst, sizeof(nDst), "%s%s", dst, tDst);
			FSOPMakeFolder(nDst);
			NANDDumpFolder(nSrc, dst);
		}
	}
	
	free(names);
	return 0;
}
#endif
