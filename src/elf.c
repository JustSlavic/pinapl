#include <elf.h>
#include <stdio.h>


char const *spaces = "                                                ";

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

            printf("Program Header %d (%d bytes):\n", program_header_index, sizeof(ph));
            printf("  p_type = %d", ph.p_type);
            
            switch (ph.p_type)
            {
                case ELF_PT_NULL: printf(" ELF_PT_NULL - The array element is unused.\n"); break;
                case ELF_PT_LOAD: printf(" ELF_PT_LOAD - The array element specifies a loadable segment, described by p_filesz and p_memsz.\n"); break;
                case ELF_PT_DYNAMIC: printf(" ELF_PT_DYNAMIC - The array element specifies dynamic linking information.\n"); break;
                case ELF_PT_INTERP: printf(" ELF_PT_INTERP - The array element specifies the location and size of a null-terminated path name to invoke as an interpreter.\n"); break;
                case ELF_PT_NOTE: printf(" ELF_PT_NOTE - The array element specifies the location and size of auxilary information.\n"); break;
                case ELF_PT_SHLIB: printf(" ELF_PT_SHLIB - Reserved.\n"); break;
                case ELF_PT_PHDR: printf(" ELF_PT_PHDR - The array element specifies the location and size of the program header table itself.\n");
                
                default:
                    printf(" 0x%x - Processor-specific semantics.\n", ph.p_type);
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

        struct elf_section_header *section_header_table = (struct elf_section_header *) ((uint8 *) memory + header->e_shoff);
        for (int section_header_index = 0; section_header_index < header->e_shnum; section_header_index++)
        {
            struct elf_section_header sh = section_header_table[section_header_index];

            printf("Section Header %d (%d bytes):\n", section_header_index, sizeof(sh));

            struct elf_section_header *string_table_header = section_header_table + header->e_shstrndx;
            char const *section_name = (char const *) memory + string_table_header->sh_offset + sh.sh_name;

            printf("  sh_name = %d (%s)\n", sh.sh_name, section_name);
            printf("  sh_type = %d ", sh.sh_type);

            switch (sh.sh_type)
            {
                case ELF_SHT_NULL:     printf("(SHT_NULL)\n"); break;
                case ELF_SHT_PROGBITS: printf("(SHT_PROGBITS)\n"); break;
                case ELF_SHT_SYMTAB:   printf("(SHT_SYMTAB)\n"); break;
                case ELF_SHT_STRTAB:   printf("(SHT_STRTAB)\n"); break;
                case ELF_SHT_RELA:     printf("(SHT_RELA)\n"); break;
                case ELF_SHT_HASH:     printf("(SHT_HASH)\n"); break;
                case ELF_SHT_DYNAMIC:  printf("(SHT_DYNAMIC)\n"); break;
                case ELF_SHT_NOTE:     printf("(SHT_NOTE)\n"); break;
                case ELF_SHT_NOBITS:   printf("(SHT_NOBITS)\n"); break;
                case ELF_SHT_REL:      printf("(SHT_REL)\n"); break;
                case ELF_SHT_SHLIB:    printf("(SHT_SHLIB)\n"); break;
                case ELF_SHT_DYNSYM:   printf("(SHT_DYNSYM)\n"); break;
                case ELF_SHT_LOPROC:
                case ELF_SHT_HIPROC:
                    printf("\\in (LOPROC, HIPROC) Reserved for processor-specific semantic.\n");
                    break;
                case ELF_SHT_LOUSER:
                case ELF_SHT_HIUSER:
                    printf("\n");
                    break;
                default:
                    printf("???\n");
            }

            printf("  sh_flags = 0x%x\n", sh.sh_flags);
            printf("             SHF_WRITE: %d\n", sh.sh_flags & ELF_SHF_WRITE);
            printf("             SHF_ALLOC: %d\n", sh.sh_flags & ELF_SHF_ALLOC);
            printf("             SHF_EXECINSTR: %d\n", sh.sh_flags & ELF_SHF_EXECINSTR);
            printf("             SHF_MASKPROC: %d\n", sh.sh_flags & ELF_SHF_MASKPROC);
            printf("  sh_addr = 0x%x\n", sh.sh_addr);
            printf("  sh_offset = %d\n", sh.sh_offset);
            printf("  sh_size = %d\n", sh.sh_size);

            if (sh.sh_type == ELF_SHT_DYNAMIC)
            {
                printf("  sh_link = %d - The section header index of the string table used by entries in the section.\n", sh.sh_link);
                printf("  sh_info = %d\n", sh.sh_info);
                if (sh.sh_info != 0)
                {
                    printf("For section header index %d, of type %d (ELF_SHT_DYNAMIC), sh_info should be 0 but it's %d.\n"
                           "The ELF file is not conforming to the specification!\n", section_header_index, sh.sh_type, sh.sh_info);
                    return;
                }
            }
            else if (sh.sh_type == ELF_SHT_HASH)
            {
                printf("  sh_link = %d - The section header index of the symbol table to which the hash table applies.\n", sh.sh_link);
                printf("  sh_info = %d\n", sh.sh_info);
                if (sh.sh_info != 0)
                {

                    printf("For section header index %d, of type %d (ELF_SHT_HASH), sh_info should be 0 but it's %d.\n"
                           "The ELF file is not conforming to the specification!\n", section_header_index, sh.sh_type, sh.sh_info);
                    return;
                }
            }
            else if ((sh.sh_type == ELF_SHT_REL) || (sh.sh_type == ELF_SHT_RELA))
            {
                printf("  sh_link = %d - The section header index of the associated symbol table.\n");
                printf("  sh_info = %d - The header index of the section to which the relocation applies.\n");
            }
            else if ((sh.sh_type == ELF_SHT_SYMTAB) || (sh.sh_type == ELF_SHT_DYNSYM))
            {
                printf("  sh_link = %d - The section header index of the associated string table.\n");
                printf("  sh_info = %d - One greater than the symbol table index of the last local sumbol (binding STB_LOCAL).\n");
            }
            else
            {
                printf("  sh_link = %d\n", sh.sh_link);
                printf("  sh_info = %d\n", sh.sh_info);
                if (sh.sh_link != ELF_SHN_UNDEF)
                {
                    printf("For section header index %d, of type %d, sh_link should be 0 (ELF_SHN_UNDEF), but it's %d\n", section_header_index, sh.sh_type, sh.sh_link);
                    break;
                }
                if (sh.sh_info != 0)
                {
                    printf("For section header index %d, of type %d, sh_info should be 0, but it's %d\n", section_header_index, sh.sh_type, sh.sh_info);
                    break;

                }
            }

            printf("  sh_addralign = %d\n", sh.sh_addralign);
            printf("  sh_entsize = %d\n", sh.sh_entsize);
            printf("\n");

            if (section_header_index == 0)
            {
                b32 section_correct = (sh.sh_name == 0) && (sh.sh_type == 0)
                    && (sh.sh_flags == 0) && (sh.sh_addr == 0) && (sh.sh_offset == 0)
                    && (sh.sh_size == 0) && (sh.sh_link == 0) && (sh.sh_info == 0)
                    && (sh.sh_addralign == 0) && (sh.sh_entsize == 0);
                if (!section_correct)
                {
                    printf("Section header index 0 is not confirming to the specification!\n");
                    return;
                }
            }

            uint8 *bytes = (uint8 *) memory + sh.sh_offset;
            int offset = 0;
            int table_width = 8;

            while (offset < sh.sh_size)
            {
                uint8 *row_start = bytes + offset;
                int width = sh.sh_size - offset;
                if (width > 8) { width = 8; }

                int count = 0;
                for (int column = 0; column < width; column++)
                {
                    count += printf("0x%x ", row_start[column]);
                }
                printf("%.*s", 60-count, spaces);
                for (int column = 0; column < width; column++)
                {
                    if (row_start[column] == 0) 
                        printf(".");
                    else
                        printf("%c", row_start[column]);
                }
                printf("\n");
                offset += width;
            }
            printf("\n");
        }
    }
}

