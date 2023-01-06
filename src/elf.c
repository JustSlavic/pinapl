#include <elf.h>


struct elf_header elf_load(void *memory, usize size)
{
    struct elf_header header = *(struct elf_header *) memory;
    
    if ((header.e_ident[ELF_IDENT_MAGIC0] == 0x7f) &&
        (header.e_ident[ELF_IDENT_MAGIC1] == 'E') &&
        (header.e_ident[ELF_IDENT_MAGIC2] == 'L') &&
        (header.e_ident[ELF_IDENT_MAGIC3] == 'F'))
    {
        enum elf_class cls = header.e_ident[ELF_IDENT_CLASS];
        enum elf_data_encoding encoding = header.e_ident[ELF_IDENT_DATA];
        uint32 version = header.e_ident[ELF_IDENT_VERSION];
    }

    return header;
}

