#ifndef __APPMETADATA_H__
#define __APPMETADATA_H__

#include <gctypes.h>

struct MetaData
{
	char* name;
	char* coder;
	char* version;
	char* shortDescription;
	char* longDescription;
	char* releaseDate;
};

struct MetaData* LoadMetaData(const char* path);
void FreeMetaData(struct MetaData* metaData);
char* LoadArguments(const char* path, u16* length);

#endif