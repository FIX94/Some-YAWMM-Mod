
#include "loadelf.h"

bool ExecIsElf(void* address)
{
    struct Elf32_Ehdr* ehdr = (struct Elf32_Ehdr*)address;

    if (*(u8*)address + 0 != 0x7F || *(u8*)address + 1 != 'E' || *(u8*)address + 2 != 'L' || *(u8*)address + 3 != 'F')
        return false;

    if (ehdr->e_type != 2) // Executable
        return false;

    if (ehdr->e_machine != 20) // PowerPC
        return false;

    return true;
}

u32 LoadElf(void* address)
{
    int i;

    struct Elf32_Ehdr* ehdr = (struct Elf32_Ehdr*)address;
    struct Elf32_Shdr* shdr = (struct Elf32_Shdr*)(address + ehdr->e_shoff + (ehdr->e_shstrndx * sizeof(struct Elf32_Shdr)));

    for (i = 0; i < ehdr->e_shnum; i++) 
    {
        shdr = (struct Elf32_Shdr*)(address + ehdr->e_shoff + (i * sizeof(struct Elf32_Shdr)));

        if (!(shdr->sh_flags & 0x02) || shdr->sh_addr == 0 || shdr->sh_size == 0)
            continue;

        shdr->sh_addr &= 0x3FFFFFFF;
        shdr->sh_addr |= 0x80000000;

        if (shdr->sh_type == 8)
            _memset32((void*)shdr->sh_addr, 0, shdr->sh_size);
        else
            _memcpy((void*)shdr->sh_addr, (void*)(address + shdr->sh_offset), shdr->sh_size);
        
        sync_after_write((void*)shdr->sh_addr, (shdr->sh_size + 31) & (~31));

    }

    return (ehdr->e_entry & 0x3FFFFFFF) | 0x80000000;
}