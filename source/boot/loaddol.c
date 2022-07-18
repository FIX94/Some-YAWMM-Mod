
#include "loaddol.h"

static void memcopy(void* address, void* buffer, u32 size)
{
	_memcpy(address, buffer, size);
	sync_after_write(address, (size + 31) & (~31));
}

u32 LoadDol(void* buffer)
{
	u32 i;
	struct dolhdr* dol = (struct dolhdr*)buffer;

	for (i = 0; i < 7; i++)
	{
		if (dol->sizeText[i] == 0)
			continue;

		memcopy((void*)dol->addressText[i], buffer + dol->offsetText[i], dol->sizeText[i]);
	}

	for (i = 0; i < 11; i++)
	{
		if (dol->sizeData[i] == 0)
			continue;

		memcopy((void*)dol->addressData[i], buffer + dol->offsetData[i], dol->sizeData[i]);
	}

	return dol->entrypoint;
}