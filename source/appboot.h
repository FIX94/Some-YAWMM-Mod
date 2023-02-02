#ifndef __APPBOOT_H__
#define __APPBOOT_H__

typedef void (*entrypoint)();

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

bool LoadApp(const char* path);
u8* GetApp(u32* size);
void LaunchApp(void);
void SetIos(int ios);
bool LoadBooter(entrypoint* entry);

#endif