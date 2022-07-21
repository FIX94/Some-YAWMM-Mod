
#include "loaddol.h"
#include "loadelf.h"
#include "utils.h"

typedef void (*entrypoint)();

void _main(void)
{
	void* buffer = (void*)0x92000000;
	entrypoint entry;

	u32 argumentsSize = *(vu32*)0x91000000;
	
	if (ExecIsElf(buffer))
		entry = (entrypoint)LoadElf(buffer);
	else
		entry = (entrypoint)LoadDol(buffer);

	if (!entry)
		return;

	if (argumentsSize > 0)
	{
		u32* ptr = (u32*)entry;

		if (ptr[1] == 0x5F617267)
		{
			struct Arguments* argv = (struct Arguments*)&ptr[2];

			argv->magic = 0x5F617267;
			argv->cmdLine = (char*)0x91000020;
			argv->length = argumentsSize;

			sync_after_write(&ptr[2], 4);
		}
	}

	entry();
}