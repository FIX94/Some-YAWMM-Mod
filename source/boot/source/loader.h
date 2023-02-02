#ifndef _LOADER_H_
#define _LOADER_H_

#include <gctypes.h>

typedef void (*entrypoint) (void);

typedef struct _dolhdr
{
	u32 offsetText[7];
	u32 offsetData[11];
	u32 addressText[7];
	u32 addressData[11];
	u32 sizeText[7];
	u32 sizeData[11];
	u32 addressBSS;
	u32 sizeBSS;
	u32 entrypoint;
} dolhdr;

bool ExecIsElf(const u8* buffer);
bool LoadElf(entrypoint* entry, const u8* buffer);
bool LoadDol(entrypoint* entry, const u8* buffer);


#endif