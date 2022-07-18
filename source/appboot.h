#ifndef __APPBOOT_H__
#define __APPBOOT_H__

bool LoadApp(const char* path);
u8* GetApp(u32* size);
void LaunchApp(void);

#endif