#ifndef __DOL_H__
#define __DOL_H__

#include "utils.h"

struct dolhdr
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
};


u32 LoadDol(void* buffer);

#endif