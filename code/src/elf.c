#include <elf.h>
#include <stdio.h>


char const *spaces = "                                                                                                                                                           ";

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
            printf("  p_flags = %d: ", ph.p_flags);
            if (ph.p_flags == 0) printf("0;\n");
            b32 printed = false;
            if (ph.p_flags & ELF_PF_X)
            {
                printf("PH_X");
                printed = true;
            }
            if (ph.p_flags & ELF_PF_W)
            {
                if (printed) printf(" | ");
                printf("PH_W");
                printed = true;
            }
            if (ph.p_flags & ELF_PF_R)
            {
                if (printed) printf(" | ");
                printf("PH_R");
            }
            printf("\n");
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

            b32 is_text_section = false;
            if (section_name[0] == '.' && section_name[1] == 't' && section_name[2] == 'e' && section_name[3] == 'x' && section_name[4] == 't') is_text_section = true;

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
                printf("  sh_link = %d - The section header index of the associated symbol table.\n", sh.sh_link);
                printf("  sh_info = %d - The header index of the section to which the relocation applies.\n", sh.sh_info);
            }
            else if ((sh.sh_type == ELF_SHT_SYMTAB) || (sh.sh_type == ELF_SHT_DYNSYM))
            {
                printf("  sh_link = %d - The section header index of the associated string table.\n", sh.sh_link);
                printf("  sh_info = %d - One greater than the symbol table index of the last local sumbol (binding STB_LOCAL).\n", sh.sh_info);
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
            printf("%s\n", is_text_section ? ".text" : "");
#if 1
            uint8 *bytes = (uint8 *) memory + sh.sh_offset;

            if (is_text_section)
            {
                if (sh.sh_size % 4 == 0) printf("Ok!\n"); else printf("Size of section is not divisible by 4! (for 32 bit instructions)\n");
                int offset = 0;
                printf("cond \n");
                while (offset < sh.sh_size)
                {
                    uint32 word = *(uint32 *) (bytes + offset);
                    uint32 mask = 0x80000000;
                    while (mask)
                    {
                        if (mask == 0x08000000) printf(" ");
                        printf("%d", (word & mask) > 0);
                        mask >>= 1;
                    }

                    if ((word & 0x0ffffff0) == 0x012fff10)
                    {
                        uint32 rn = (word & 0x0000000f);
                        printf("  BX r%d", rn);
                    }
                    else if (((word & 0x0f000000) == 0x0a000000) || ((word & 0x0f000000) == 0x0b000000))
                    {
                        b32 with_link = (word & 0x01000000) > 0;
                        int32 offset_base = (word & 0x007fffff);
                        int32 offset_sign = (word & 0x00800000) << 8;
                        int32 offset = (offset_sign >> 8) | offset_base;
                        if (with_link) printf("  BL "); else printf("  B ");
                        printf(" #%d", offset);
                    }
                    else if ((word & 0x0c000000) == 0x04000000)
                    {
                        b32 immediate_offset  = (word & 0x02000000) > 0;
                        b32 pre_post_indexing = (word & 0x01000000) > 0;
                        b32 up_down_bit       = (word & 0x00800000) > 0;
                        b32 byte_word_bit     = (word & 0x00400000) > 0;
                        b32 write_back_bit    = (word & 0x00200000) > 0;
                        b32 load_store_bit    = (word & 0x00100000) > 0;
                        uint32 rn     = (word & 0x000f0000) >> 16;
                        uint32 rd     = (word & 0x0000f000) >> 12;
                        uint32 offset = (word & 0x00000fff);

                        if (load_store_bit == 0) printf("  STR "); else printf("  LDR ");
                        printf("r%d at [r%d + #%d]; ", rd, rn, offset);
                        if (immediate_offset == 0) printf("offset is an immediate value; ");
                        if (up_down_bit == 0) printf("sub offset ");
                            else              printf("add offset ");
                        if (pre_post_indexing == 0) printf("after; ");
                            else                    printf("before; ");

                        if (byte_word_bit == 0) printf("transfer word; ");
                            else                printf("transfer byte; ");
                        if (write_back_bit == 0) printf("no write back; ");
                            else                 printf("write address into base; ");

                    }
                    else if ((word & 0x0f000000) == 0x0f000000)
                    {
                        printf("  SWI ");
                    }
                    else if ((word & 0x0fc000f0) == 0x00000090)
                    {
                        // Multiply and Multiply-Accumulate Instructions
                        bool32 accumulate = (word & 0x00200000) > 0;
                        bool32 set_cond = (word & 0x00100000) > 0;
                        uint32 rd = (word & 0x000f0000) >> 16;
                        uint32 rn = (word & 0x0000f000) >> 12;
                        uint32 rs = (word & 0x00000f00) >> 8;
                        uint32 rm = (word & 0x0000000f);

                        if (accumulate)
                            printf("  MLA%s r%d := r%d * r%d + r%d; ", set_cond ? "S" : "", rd, rm, rs, rn);
                        else
                            printf("  MUL%s r%d := r%d * r%d; ", set_cond ? "S" : "", rd, rm, rs);
                    }
                    else if ((word & 0x0c000000) == 0)
                    {
                        // Data Processing Instruction
                        bool32 is_immediate = (word & 0x02000000) > 0;
                        uint32 opcode = (word & 0x01e00000) >> 21;
                        uint32 set_cond = (word & 0x00100000) > 0;
                        uint32 rn = (word & 0x000f0000) >> 16;
                        uint32 rd = (word & 0x0000f000) >> 12;
                        // uint32 op2 = (word & 0x00000fff);

                        switch (opcode)
                        {
                            case 0x0: printf("  AND%s ", set_cond ? "S" : ""); break;
                            case 0x1: printf("  EOR "); break;
                            case 0x2: printf("  SUB%s r%d := r%d - ", set_cond ? "S" : "", rd, rn); break;
                            case 0x3: printf("  RSB "); break;
                            case 0x4: printf("  ADD%s r%d := r%d + ", set_cond ? "S" : "", rd, rn); break;
                            case 0x5: printf("  ADC "); break;
                            case 0x6: printf("  SBC "); break;
                            case 0x7: printf("  RSC "); break;
                            case 0x8: printf("  TST "); break;
                            case 0x9: printf("  TEQ "); break;
                            case 0xa: printf("  CMP "); break;
                            case 0xb: printf("  CMN "); break;
                            case 0xc: printf("  ORR "); break;
                            case 0xd: printf("  MOV%s r%d := ", set_cond ? "S" : "", rd); break;
                            case 0xe: printf("  BIC "); break;
                            case 0xf: printf("  MVN "); break;
                        }

                        if (is_immediate)
                        {
                            uint32 rot = (word & 0x00000f00) >> 8;
                            uint32 imm = (word & 0x000000ff);
                            if (rot > 0)
                            {
                                printf("(#%d >> %d)", imm, rot);
                            }
                            else
                            {
                                printf("#%d", imm);
                            }
                        }
                        else
                        {
                            uint32 shift = (word & 0x00000ff0) >> 4;
                            uint32 rm = (word & 0x0000000f);
                            if (shift > 0)
                            {
                                printf("(r%d >> %d)", rm, shift);
                            }
                            else
                            {
                                printf("r%d", rm);
                            }
                        }
                    }

                    printf("\n");

                    offset += sizeof(uint32);
                }
            }
            else
            {
               int offset = 0;
               int table_width = 8;
               while (offset < sh.sh_size)
                {
                    uint8 *row_start = bytes + offset;
                    int width = sh.sh_size - offset;
                    if (width > table_width) { width = table_width; }

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
            }
            printf("\n");
#endif
        }
    }
}

