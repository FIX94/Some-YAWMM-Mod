#include <gccore.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ogc/machine/processor.h>

#include "loader.h"

extern void __exception_closeall();
static void* xfb = NULL;
static GXRModeObj* rmode = NULL;

struct Arguments
{
	int magic;
	char* cmdLine;
	int length;
};

void VideoInit(void)
{
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	printf("\x1b[2;0H");
}

bool IsDollZ(const u8* buffer)
{
	return (buffer[0x100] == 0x3C);
}

bool IsSpecialELF(const u8* buffer)
{
	return (read32((u32)buffer) == 0x7F454C46 && buffer[0x24] == 0);
}

int main(void) 
{
	VideoInit();
	
	u8* buffer = (u8*)0x92000000;
	entrypoint entry;
	bool execLoaded = false;

	if (ExecIsElf(buffer))
	{
		//printf(" Loading ELF @ address 0x%08X:\n\n", (u32)buffer);
		execLoaded = LoadElf(&entry, buffer);
	}	
	else
	{
		//printf(" Loading DOL @ address 0x%08X::\n\n", (u32)buffer);
		execLoaded = LoadDol(&entry, buffer);
	}

	if (!execLoaded)
		return -1;

	u8* execPtr = (u8*)entry;

	if (execPtr[0x20] == 0x41)
	{
		execPtr[0x21] = 0x40;
		DCFlushRange(&execPtr[0x20], 1);
	}

	if (!IsDollZ(buffer) && !IsSpecialELF(buffer))
	{
		u32 argumentsSize = *(vu32*)0x91000000;
		if (argumentsSize > 0)
		{
			u32* ptr = (u32*)entry;

			if (ptr[1] == 0x5F617267)
			{
				struct Arguments* argv = (struct Arguments*)&ptr[2];

				argv->magic = 0x5F617267;
				argv->cmdLine = (char*)0x91000020;
				argv->length = argumentsSize;

				DCFlushRange(&ptr[2], 4);
			}
		}
	}

	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	u32 level = IRQ_Disable();
	__exception_closeall();
	entry();
	IRQ_Restore(level);

	return 0;
}
