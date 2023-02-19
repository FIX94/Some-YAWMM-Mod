#include <stdio.h>
#include <malloc.h>

#include "fileops.h"


bool FSOPFileExists(const char* file)
{
	FILE* f;
	f = fopen(file, "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool FSOPFolderExists(const char* path)
{
	DIR* dir;
	dir = opendir(path);
	if (dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}

size_t FSOPGetFileSizeBytes(const char* path)
{
	FILE* f;
	size_t size = 0;

	f = fopen(path, "rb");
	if (!f) 
		return 0;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fclose(f);

	return size;
}

void FSOPDeleteFile(const char* file)
{
	if (!FSOPFileExists(file))
		return;

	remove(file);
}

void FSOPMakeFolder(const char* path)
{
	if (FSOPFolderExists(path))
		return;

	mkdir(path, S_IREAD | S_IWRITE);
}

s32 FSOPReadOpenFile(FILE* fp, void* buffer, u32 offset, u32 length)
{
	fseek(fp, offset, SEEK_SET);
	return fread(buffer, length, 1, fp);
}

s32 FSOPReadOpenFileA(FILE* fp, void** buffer, u32 offset, u32 length)
{
	*buffer = memalign(32, length);
	if (!*buffer)
		return -1;

	s32 ret = FSOPReadOpenFile(fp, *buffer, offset, length);
	if (ret < 0) 
		free(*buffer);

	return ret;
}
