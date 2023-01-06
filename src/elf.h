#ifndef SRC_ELF_H
#define SRC_ELF_H

#include <base.h>


enum elf_ident_index
{
    ELF_IDENT_MAGIC0  = 0,
    ELF_IDENT_MAGIC1  = 1,
    ELF_IDENT_MAGIC2  = 2,
    ELF_IDENT_MAGIC3  = 3,
    ELF_IDENT_CLASS   = 4,
    ELF_IDENT_DATA    = 5,
    ELF_IDENT_VERSION = 6,
    ELF_IDENT_PAD     = 7,
    ELF_IDENT_SIZE    = 16,
};

enum elf_class
{
    ELF_CLASS_NONE = 0, // Invalid class
    ELF_CLASS_32   = 1, // 32-bit objects
    ELF_CLASS_64   = 2, // 64-bit objects
};

enum elf_data_encoding
{
    ELF_DATA_NONE = 0,
    ELF_DATA_2LSB = 1,
    ELF_DATA_2MSB = 2,
};

enum elf_type
{
    ET_NONE = 0, // No file type
    ET_REL  = 1, // Relocatable file
    ET_EXEC = 2, // Executable file
    ET_DYN  = 3, // Shared object file
    ET_CORE = 4, // Core dump

    // Processor-specific:
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff,
};


enum elf_machine
{
    EM_NONE  = 0, // No machine
    EM_M32   = 1, // AT&T WE 32100
    EM_SPARC = 2, // SPARC
    EM_386   = 3, // Intel 80386
    EM_68K   = 4, // Motorola 68000
    EM_88K   = 5, // Motorola 88000
    EM_860   = 7, // Intel 80860
    EM_MIPS  = 8, // MIPS RS3000
};

enum elf_version
{
    EV_NONE    = 0, // Invalid version
    EV_CURRENT = 1, // Current version (will change to reflect the current version number?)
};

struct elf_header
{
    uint8  e_ident[ELF_IDENT_SIZE]; // Identification
    uint16 e_type;       // Object file type
    uint16 e_machine;    // Required architecture
    uint32 e_version;    // Object file version
    uint32 e_entry;      // Virtual address of the entry point
    uint32 e_phoff;      // Program header table's file offset in bytes
    uint32 e_shoff;      // Section header table's file offset in bytes
    uint32 e_flags;      // Processor-specific flags
    uint16 e_ehsize;     // ELF header's size in bytes
    uint16 e_phentsize;  // Program header table entry size in bytes (all entries are the same size)
    uint16 e_phnum;      // Program header table number of entries
    uint16 e_shentsize;  // Section header table entry size in bytes (all entries are the same size)
    uint16 e_shnum;      // Section header table number of entries
    uint16 e_shstrndx;   // Section name string table entry index in the section header table
};


struct elf_header elf_load(void *memory, usize size);


#endif // SRC_ELF_H

