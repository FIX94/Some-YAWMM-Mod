#include <gccore.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "loader.h"
#include "elf_abi.h"

bool ExecIsElf(const u8* buffer)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)buffer;

    if (!IS_ELF(*ehdr))
        return false;

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
        return false;

    if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB)
        return false;

    if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
        return false;

    if (ehdr->e_type != ET_EXEC)
        return false;

    if (ehdr->e_machine != EM_PPC)
        return false;

    return true;
}

bool LoadElf(entrypoint* entry, const u8* buffer)
{
    int i;

    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)buffer;

    if (ehdr->e_phoff == 0 || ehdr->e_phnum == 0)
        return false;

    if (ehdr->e_phentsize != (sizeof(Elf32_Phdr)))
        return false;

    Elf32_Phdr* phdrs = (Elf32_Phdr*)(buffer + ehdr->e_phoff);

    for (i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_type == PT_LOAD)
        {
            phdrs[i].p_paddr &= 0x3FFFFFFF;
            phdrs[i].p_paddr |= 0x80000000;

            if (phdrs[i].p_filesz > phdrs[i].p_memsz)
                return false;

            if (phdrs[i].p_filesz)
            {
                memmove((void*)phdrs[i].p_paddr, (void*)(buffer + phdrs[i].p_offset), phdrs[i].p_filesz);
                DCFlushRange((void*)phdrs[i].p_paddr, phdrs[i].p_memsz);

                if (phdrs[i].p_flags & PF_X)
                    ICInvalidateRange((void*)phdrs[i].p_paddr, phdrs[i].p_memsz);
            }
        }
    }

    *entry = (entrypoint)((ehdr->e_entry & 0x3FFFFFFF) | 0x80000000);
    return true;
}

bool LoadDol(entrypoint* entry, const u8* buffer)
{
	u32 i;
	dolhdr* dol = (dolhdr*)buffer;

	for (i = 0; i < 7; i++)
	{
		if (dol->sizeText[i] == 0 || dol->addressText[i] < 0x100)
			continue;

        //printf(" Move text section %u @ 0x%08x -> 0x%08x (0x%0X bytes)\n", i, (u32)(buffer + dol->offsetText[i]), dol->addressText[i], dol->sizeText[i]);
        memmove((void*)dol->addressText[i], buffer + dol->offsetText[i], dol->sizeText[i]);
        DCFlushRange((void*)dol->addressText[i], dol->sizeText[i]);
        ICInvalidateRange((void*)dol->addressText[i], dol->sizeText[i]);

	}

	for (i = 0; i < 11; i++)
	{
		if (dol->sizeData[i] == 0)
			continue;

        //printf(" Move data section %u @ 0x%08x -> 0x%08x (0x%0X bytes)\n", i, (u32)(buffer + dol->offsetData[i]), dol->addressData[i], dol->sizeData[i]);
        memmove((void*)dol->addressData[i], buffer + dol->offsetData[i], dol->sizeData[i]);
        DCFlushRange((void*)dol->addressData[i], dol->sizeData[i]);
	}

	*entry = (entrypoint)dol->entrypoint;

	return true;
}

