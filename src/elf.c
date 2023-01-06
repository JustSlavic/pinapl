#include <elf.h>
#include <stdio.h>


void elf_load(void *memory, usize size)
{
    struct elf_header *header = (struct elf_header *) memory;
    
    if ((header->e_ident[ELF_IDENT_MAGIC0] == 0x7f) &&
        (header->e_ident[ELF_IDENT_MAGIC1] == 'E') &&
        (header->e_ident[ELF_IDENT_MAGIC2] == 'L') &&
        (header->e_ident[ELF_IDENT_MAGIC3] == 'F'))
    {
        printf("ELF header:\n");
        printf("e_ident:\n");
        printf("  ELF_MAGIC_0 %s\n", header->e_ident[ELF_IDENT_MAGIC0] == 0x7f ? "OK" : "FAILURE");
        printf("  ELF_MAGIC_1 %s\n", header->e_ident[ELF_IDENT_MAGIC1] == 'E' ? "OK" : "FAILURE");
        printf("  ELF_MAGIC_2 %s\n", header->e_ident[ELF_IDENT_MAGIC2] == 'L' ? "OK" : "FAILURE");
        printf("  ELF_MAGIC_3 %s\n", header->e_ident[ELF_IDENT_MAGIC3] == 'F' ? "OK" : "FAILURE");

        uint32 cls = header->e_ident[ELF_IDENT_CLASS];
        printf("  ELF_CLASS: %s\n", cls == 1 ? "32-bit object" : cls == 2 ? "64-bit object" : "Invalid object class");

        printf("  ELF_DATA: %s\n", header->e_ident[ELF_IDENT_DATA] == ELF_DATA_2LSB ? "Little-endian" : "Big-endian");
        printf("  ELF_VERSION: %d\n", header->e_ident[ELF_IDENT_VERSION]);

        switch (header->e_type)
        {
            case ET_REL:  printf("e_type: Relocatable file\n"); break;
            case ET_EXEC: printf("e_type: Executable\n"); break;
            case ET_DYN:  printf("e_type: Shared object\n"); break;
            case ET_CORE: printf("e_type: coredump\n"); break;
            case ET_NONE:
            default:
                printf("e_type: Unknown elf type file\n");
        };

        switch (header->e_machine)
        {
            case EM_M32:   printf("e_machine: M32\n"); break;
            case EM_SPARC: printf("e_machine: SPARC\n"); break;
            case EM_386:   printf("e_machine: 386\n"); break;
            case EM_68K:   printf("e_machine: 68K\n"); break;
            case EM_88K:   printf("e_machine: 88K\n"); break;
            case EM_860:   printf("e_machine: 860\n"); break;
            case EM_MIPS:  printf("e_machine: MIPS\n"); break;

            case EM_NONE:
            default:
                printf("e_machine: %d\n", header->e_machine);
        }
        
        printf("e_version: %d%s\n", header->e_version, header->e_version == EV_CURRENT ? " (EV_CURRENT)" : "");
        printf("e_entry: 0x%x\n", header->e_entry);
        printf("e_phoff: %d bytes\n", header->e_phoff);
        printf("e_shoff: %d bytes\n", header->e_shoff);
        printf("e_flags: 0x%x\n", header->e_flags);
        printf("e_ehsize: %d bytes\n", header->e_ehsize);
        printf("e_phentsize: %d bytes\n", header->e_phentsize);
        printf("e_phnum: %d\n", header->e_phnum);
        printf("e_shentsize: %d bytes\n", header->e_shentsize);
        printf("e_shnum: %d\n", header->e_shnum);
        printf("e_shstrndx: %d\n", header->e_shstrndx);
        printf("\n");

        struct elf_program_header *ph_array = (struct elf_program_header *) ((uint8 *) memory + header->e_phoff);
        for (int program_header_index = 0; program_header_index < header->e_phnum; program_header_index++)
        {
            struct elf_program_header ph = ph_array[program_header_index];

            printf("Program Header %d:\n", program_header_index);
            
            switch (ph.p_type)
            {
                case ELF_PT_NULL: printf("  ELF_PT_NULL - The array element is unused.\n"); break;
                case ELF_PT_LOAD: printf("  ELF_PT_LOAD - The array element specifies a loadable segment, described by p_filesz and p_memsz.\n"); break;
                case ELF_PT_DYNAMIC: printf("  ELF_PT_DYNAMIC - The array element specifies dynamic linking information.\n"); break;
                case ELF_PT_INTERP: printf("  ELF_PT_INTERP - The array element specifies the location and size of a null-terminated path name to invoke as an interpreter.\n"); break;
                case ELF_PT_NOTE: printf("  ELF_PT_NOTE - The array element specifies the location and size of auxilary information.\n"); break;
                case ELF_PT_SHLIB: printf("  ELF_PT_SHLIB - Reserved.\n"); break;
                case ELF_PT_PHDR: printf("  ELF_PT_PHDR - The array element specifies the location and size of the program header table itself.\n");
                
                default:
                    printf("  0x%x - Processor-specific semantics.\n", ph.p_type);
            }

            printf("  p_offset = %d\n", ph.p_offset);
            printf("  p_vaddr = 0x%x\n", ph.p_vaddr);
            printf("  p_paddr = 0x%x\n", ph.p_paddr);
            printf("  p_filesz = %d\n", ph.p_filesz);
            printf("  p_memsz = %d\n", ph.p_memsz);
            printf("  p_flags = 0x%x\n", ph.p_flags);
            printf("  p_align = %d\n", ph.p_align);
            printf("\n");
        }
    }
}

