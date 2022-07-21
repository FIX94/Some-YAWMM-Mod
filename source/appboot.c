#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>

#include "fat.h"
#include "sys.h"
#include "appmetadata.h"

extern void __exception_closeall();

struct __argv arguments;
char* m_argv[256];

u8* metaBuffer = NULL;
u32 metaSize = 0;

u8* appBuffer = NULL;
u32 appSize = 0;

typedef void (*entrypoint)();
u32 appEntry = 0;

#include "appboot_bin.h"

static void Jump(entrypoint EntryPoint)
{
	appEntry = (u32)EntryPoint;

	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();
	__lwp_thread_closeall();
	asm volatile (
		"lis %r3, appEntry@h\n"
		"ori %r3, %r3, appEntry@l\n"
		"lwz %r3, 0(%r3)\n"
		"mtlr %r3\n"
		"blr\n"
		);
	IRQ_Restore(level);
}

bool LoadApp(const char* path)
{
	appBuffer = (u8*)0x92000000;
	
	char currentPath[256];
	snprintf(currentPath, sizeof(currentPath), "%s/boot.dol", path);
	
	FILE* f = fopen(currentPath, "rb");

	if (f == NULL)
	{
		snprintf(currentPath, sizeof(currentPath), "%s/boot.elf", path);
		f = fopen(currentPath, "rb");

		if (f == NULL)
			return false;
	}

	fseek(f, 0, SEEK_END);
	appSize = ftell(f);
	rewind(f);

	if (appSize > 0x1000000)
	{
		fclose(f);
		return false;
	}
		
	u32 ret = fread(appBuffer, 1, appSize, f);
	DCFlushRange(appBuffer, (appSize + 31) & (~31));

	fclose(f);

	snprintf(currentPath, sizeof(currentPath), "%s/meta.xml", path);
	u16 argumentsSize = 0;
	char* Arguments = LoadArguments(currentPath, &argumentsSize);
	
	if (Arguments)
	{
		*(vu32*)0x91000000 = argumentsSize;
		memcpy((void*)0x91000020, Arguments, argumentsSize);
		DCFlushRange((u8*)0x91000020, argumentsSize);
		free(Arguments);
	}

	return (ret == appSize);
}

u8* GetApp(u32* size)
{
	*size = appSize;
	return appBuffer;
}

void LaunchApp(void)
{
	entrypoint entry;

	memcpy((u8*)0x93000000, appboot_bin, appboot_bin_size);
	DCFlushRange((u8*)0x93000000, appboot_bin_size);
	entry = (entrypoint)0x93000000;

	Jump(entry);
	Sys_LoadMenu();
}