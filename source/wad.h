#ifndef _WAD_H_
#define _WAD_H_

/* Prototypes */
s32 Wad_Install(FILE* fp);
s32 Wad_Uninstall(FILE* fp);
s32 GetSysMenuRegion(u16* version, char* region);
const char* GetSysMenuRegionString(const char* region);

#endif
