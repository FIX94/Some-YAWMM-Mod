#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>

#include "fat.h"
#include "sys.h"
#include "wpad.h"

extern void __exception_closeall();

//struct __argv arguments;
//char* m_argv[MAX_ARGV];
//u8* m_Buffer = NULL;

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
	
	FILE* f = fopen(path, "rb");

	if (f == NULL)
		return false;

	fseek(f, 0, SEEK_END);
	u32 size = ftell(f);
	rewind(f);

	if (size > 0x1000000)
	{
		fclose(f);
		return false;
	}
		
	u32 ret = fread(appBuffer, 1, size, f);
	DCFlushRange(appBuffer, (size + 31) & (~31));

	fclose(f);

	return (ret == size);
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

	u32 dumdum = 0;
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable(dumdum);
	__exception_closeall();
	Jump(entry);
	_CPU_ISR_Restore(dumdum);

	Sys_LoadMenu();
}