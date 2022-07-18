
#include "loaddol.h"
#include "loadelf.h"

typedef void (*entrypoint)();

void _main(void)
{
	void* buffer = (void*)0x92000000;
	entrypoint entry;
	
	if (ExecIsElf(buffer))
		entry = (entrypoint)LoadElf(buffer);
	else
		entry = (entrypoint)LoadDol(buffer);

	if (!entry)
		return;

	entry();
}