#ifndef SRC_ELF_H
#define SRC_ELF_H

#include <base.h>


#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

enum elf_ident_index
{
    ELF_IDENT_MAGIC0  = 0,
    ELF_IDENT_MAGIC1  = 1,
    ELF_IDENT_MAGIC2  = 2,
    ELF_IDENT_MAGIC3  = 3,
    ELF_IDENT_CLASS   = 4,
    ELF_IDENT_DATA    = 5,
    ELF_IDENT_VERSION = 6,
    ELF_IDENT_OSABI   = 7,
    ELF_IDENT_ABIVERSION = 8,
    ELF_IDENT_PAD     = 9,
    ELF_IDENT_SIZE    = 16,
};

enum elf_class
{
    ELF_CLASS_NONE = 0,
    ELF_CLASS_32   = 1, // 32-bit objects
    ELF_CLASS_64   = 2, // 64-bit objects
};

enum elf_data_encoding
{
    ELF_DATA_NONE = 0,
    ELF_DATA_2LSB = 1, // Little-endian encoding
    ELF_DATA_2MSB = 2, // Big-endian encoding
};

enum elf_os_abi
{
    ELF_OS_ABI_NONE    = 0,  // Invalid ELF data format
    ELF_OS_ABI_SYSV    = 0,  // Unix System V
    ELF_OS_ABI_HPUX    = 1,  // HP-UX
    ELF_OS_ABI_NETBSD  = 2,  // NetBSD
    ELF_OS_ABI_LINUX   = 3,  // Linux with GNU extensions
    ELF_OS_ABI_SOLARIS = 6,  // Solaris
    ELF_OS_ABI_AIX     = 7,  // AIX
    ELF_OS_ABI_IRIX    = 8,  // SGI Irix
    ELF_OS_ABI_FREEBSD = 9,  // FreeBSD
    ELF_OS_ABI_TRU64   = 10, // Compaq TRU64 UNIX
    ELF_OS_ABI_MODESTO = 11, // Novel Modesto
    ELF_OS_ABI_OPENBSD = 12, // OpenBSD
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
    EM_NONE         = 0, // No machine
    EM_M32          = 1, // AT&T WE 32100
    EM_SPARC        = 2, // SPARC
    EM_386          = 3, // Intel 80386
    EM_68K          = 4, // Motorola 68000
    EM_88K          = 5, // Motorola 88000

    EM_860          = 7, // Intel 80860
    EM_MIPS         = 8, // MIPS RS3000 (big-endian only)
    EM_S370         = 9,
    EM_MIPS_RS3_LE  = 10,

    EM_PARISC       = 15, // HP/PA

    EM_VPP500       = 17,
    EM_SPARC32PLUS  = 18, // SPARC with enhanced instruction set
    EM_960          = 19, 
    EM_PPC          = 20, // PowerPC
    EM_PPC64        = 21, // PowerPC 64-bit
    EM_S390         = 22, // IBM S/390

    EM_V800         = 36,
    EM_FR20         = 37,
    EM_RH32         = 38,
    EM_RCE          = 39,
    EM_ARM          = 40, // Advanced RISC Machine
    EM_FAKE_ALPHA   = 41,
    EM_SH           = 42, // Renesas SuperH
    EM_SPARCV9      = 43, // SPARC v9 64-bit
    EM_TRICORE      = 44,
    EM_ARC          = 45,
    EM_H8_300       = 46,
    EM_H8_300H      = 47,
    EM_H8S          = 48,
    EM_H8_500       = 49,
    EM_IA_64        = 50, // Intel Itanium
    EM_MIPS_X       = 51,
    EM_COLDFIRE     = 52,
    EM_68HC12       = 53,
    EM_MMA          = 54,
    EM_PCP          = 55,
    EM_NCPU         = 56,
    EM_NDR1         = 57,
    EM_STARCORE     = 58,
    EM_ME16         = 59,
    EM_ST100        = 60,
    EM_TINYJ        = 61,
    EM_X86_64       = 62, // AMD x86-64
    EM_PDSP         = 63,
    EM_FX66         = 66,
    EM_ST9PLUS      = 67,
    EM_ST7          = 68,
    EM_68HC16       = 69,
    EM_68HC11       = 70,
    EM_68HC08       = 71,
    EM_68HC05       = 72,
    EM_SVX          = 73,
    EM_ST19         = 74,
    EM_VAX          = 75, // DEC Vax
    EM_CRIS         = 76,
    EM_JAVELIN      = 77,
    EM_FREEPATH     = 78,
    EM_ZSP          = 79,
    EM_MMIX         = 80,
    EM_HUANY        = 81,
    EM_PRISM        = 82,
    EM_AVR          = 83,
    EM_FR30         = 84,
    EM_D10V         = 85,
    EM_D30V         = 86,
    EM_V850         = 87,
    EM_M32R         = 88,
    EM_MN10300      = 89,
    EM_MN10200      = 90,
    EM_PJ           = 91,
    EM_OPENRISC     = 92,
    EM_ARC_A5       = 93,
    EM_XTENSA       = 94,

    EM_AARCH64      = 183,

    EM_TILEPRO      = 188,
    EM_MICROBLAZE   = 189,

    EM_TILEGX = 191,
};

enum elf_version
{
    EV_NONE    = 0, // Invalid version
    EV_CURRENT = 1, // Current version (will change to reflect the current version number?)
};

enum elf_arm_flags
{
    EF_ARM_ABI_VERSION_MASK = 0xff000000,
    EF_ARM_ABI_CURRENT_VERSION = 0x05000000,
    EF_ARM_BE8 = 0x00800000, // The ELF file contains BE-8 code, suitable for execution on an Arm Architecture v6 processor. This flag must only be set on an executable file.
    EF_ARM_GCCMASK = 0x00400FFF, // Legacy code (ABI version 4 and earlier) generated by gcc-arm-xxx might use these bits.
    EF_ARM_ABI_FLOAT_HARD = 0x00000400, // Set in executable file headers (e_type = ET_EXEC or ET_DYN) to note that the executable file was built to conform to the hardware floating-point procedure-call standard.
    EF_ARM_ABI_FLOAT_SOFT = 0x00000200, // Set in the file headers (e_type = ET_EXEC or ET_DYN) to note explicitly that the executable file was built to conform to the software floating-point porcedure-call standard (the base standard). If both EF_ARM_ABI_FLOAT_XXXX bits are clear, conformance to the base procedure-call standard is implied.
};

struct elf_section_header
{
    uint32 sh_name;      // Name of the section (index into the section header string table section)
    uint32 sh_type;      // Section type
    uint32 sh_flags;     // Flags
    uint32 sh_addr;      // Address at which the section's first byte should appear in the memory image of the process
    uint32 sh_offset;    // The offset from the beginning of the file to the first byte in the section
    uint32 sh_size;      // Section's size in bytes
    uint32 sh_link;      // Section header table index link
    uint32 sh_info;      // Extra information
    uint32 sh_addralign; // Section alignment constrains (only positive integral powers of two are allowed), 0 and 1 mean no constrain
    uint32 sh_entsize;   // Some sections hold a table of fixed-size elements, this member holds the size in bytes of such table
};

enum elf_section_header_name
{
    ELF_SHN_UNDEF     = 0,      // This value marks an undefined, missing, irrelevant, or otherwise meaningless section reference.
    ELF_SHN_LORESERVE = 0xff00, // This value specifies the lower bound of the range of reserved indexes

    // Reserved for processor-specific semantics
    ELF_SHN_LOPROC    = 0xff00,
    ELF_SHN_HIPROC    = 0xff1f,

    ELF_SHN_ABS       = 0xfff1, // This value specifies absolute values for the corresponding reference
    ELF_SHN_COMMON    = 0xfff2, // Symbols defined relative to this section are common symbols, such as FORTRAN COMMON or unallocated C external variables
    ELF_SHN_HIRESERVE = 0xffff, // This value specifies the upper bound of the range or reserved indexes
};

enum elf_section_header_type
{
    ELF_SHT_NULL      = 0,  // The section header is inactive
    ELF_SHT_PROGBITS  = 1,  // The section holds information defined by the program, whose format and meaning are determined solely by the program
    ELF_SHT_SYMTAB    = 2,  // The section holds a symbol table; It provides symbols for link editing, though it may also be used for dynamic linking
    ELF_SHT_STRTAB    = 3,  // The section holds a string table
    ELF_SHT_RELA      = 4,  // The section holds a relocation entries with explicit addends
    ELF_SHT_HASH      = 5,  // The section holds a symbol hash table
    ELF_SHT_DYNAMIC   = 6,  // The section holds information for dynamic linking
    ELF_SHT_NOTE      = 7,  // The section holds information that marks the file in some way
    ELF_SHT_NOBITS    = 8,  // A section of this type occupies no space in the file but otherwise resembles ELF_SHT_PROGBITS
    ELF_SHT_REL       = 9,  // The section holds relocation entries without explicit addends
    ELF_SHT_SHLIB     = 10, // The section type is reserved but has unspecified semantics; Programs that contain this sections do not conform to the ABI
    ELF_SHT_DYNSYM    = 11, // Section holds a symbol table; Minimal set of dynamic linking symbols, to save space

    // Reserved for processor-specific semantics
    ELF_SHT_LOPROC    = 0x70000000,
    ELF_SHT_HIPROC    = 0x7fffffff,
    
    // Reserved for application programs
    ELF_SHT_LOUSER    = 0x80000000,
    ELF_SHT_HIUSER    = 0xffffffff,
};

//
// The section header of index 0 is reserved and should hold the following:
//
// sh_name = 0;
// sh_type = ELF_SHT_NULL;
// sh_flags = 0;
// sh_addr = 0;
// sh_offset = 0;
// sh_size = 0;
// sh_link = ELF_SHN_UNDEF;
// sh_info = 0;
// sh_addralign = 0;
// sh_entsize = 0;
//

enum elf_section_header_flags
{
    ELF_SHF_WRITE     = 0x1, // The section contains the data that should be writable during process execution
    ELF_SHF_ALLOC     = 0x2, // The section occupies memory during process execution
    ELF_SHF_EXECINSTR = 0x4, // The section contains executable machine instructions
    ELF_SHF_MASKPROC  = 0xf0000000, // All bits included in this mask are reserved for processor-specific semantics
};

//
// The two members of the section header, 'sh_link' and 'sh_info' hold special information
// depending on section type:
//
// if sh_type == ELF_SHT_DYNAMIC
//    sh_link = The section header index of the string table used by entries in the section.
//    sh_info = 0;
//
// if sh_type == ELF_SHT_HASH
//    sh_link = The section header index of the symbol table to which the hash table applies.
//    sh_info = 0;
//
// if sh_type == ELF_SHT_REL || sh_type == ELF_SHT_RELA
//    sh_link = The section header index of the associated symbol table.
//    sh_info = The section header index of the section to which the relocation applies.
// 
// if sh_type == ELF_SHT_SYMTAB || sh_type == ELF_SHT_DYNSYM
//    sh_link = The section header index of the associated string table.
//    sh_info = One greater than the symbol table index of the last local symbol (binding STB_LOCAL)
//
// else
//    sh_link = ELF_SHN_UNDEF
//    sh_info = 0
//

struct elf_symbol_table
{
    uint32 st_name;  // An index into the object file's symbol string table
    uint32 st_value; // The value of the associated symbol (may be an absolute value, an address, etc.)
    uint32 st_size;  // Many symbols have associated sizes. For example, a data object's size is the number of bytes contained in the object.
    uint8  st_info;  // This member specifies the symbol's type and binding attributes
    uint8  st_other; // Should be 0.
    uint16 st_shndx; // Relevant section header table index.
};

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

enum elf_symbol_binding
{
    ELF_STB_LOCAL  = 0,  // Local symbols are not visible outside the object file
    ELF_STB_GLOBAL = 1,  // Global symbols are visible to all object files being combined
    ELF_STB_WEAK   = 2,  // Weak symbols resemble global symbols, but their definitions have lower precedence

    // Reserved for processor-specific semantics
    ELF_STB_LOPROC = 13, 
    ELF_STB_HIPROC = 15, 
};

enum elf_symbol_type
{
    ELF_STT_NOTYPE  = 0,  // The symbol's type is not specified
    ELF_STT_OBJECT  = 1,  // The symbol is associated with a data object, such as a variable, an array, etc.
    ELF_STT_FUNC    = 2,  // The symbol is associated with a function or other executable code
    ELF_STT_SECTION = 3,  // The symbol is associated with a section
    ELF_STT_FILE    = 4,  // Conventionally the symbol's name gives the name of the source file associated with the object file
    
    // Reserved for processor-specific semantics
    ELF_STT_LOPROC  = 13,
    ELF_STT_HIPROC  = 15,
};

//
// The symbol table entry for index 0 (ELF_STN_UNDEF) is reserved; it holds the following values:
//
// st_name = 0;
// st_value = 0;
// st_size = 0;
// st_info = 0;
// st_other = 0;
// st_shndx = ELF_SHN_UNDEF; 
//

struct elf_relocation
{
    uint32 r_offset; // The location at which to apply the relocation action.
    uint32 r_info;   // This member gives both the symbol table index with respect to which the relocation must be made, and the type of relocation to apply.
};

struct elf_relocation_with_addend
{
    uint32 r_offset;
    uint32 r_info;
     int32 r_addend; // This member specifies a constant addend used to compute the value to be stored into the relocatable field.
};

#define ELF32_R_SYM(i)    ((i)>>8)
#define ELF32_R_TYPE(i)   ((uint8) (i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(uint8)(t))

struct elf_program_header
{
    uint32 p_type;   // Kind of segment this array element describes or how to interpret the array element's information.
    uint32 p_offset; // Offset from the beginning of the file at which the first byte of the segment resides.
    uint32 p_vaddr;  // Virtual address at which the first byte of the segment resides in memory.
    uint32 p_paddr;  // Segment's physical address for systems where it is relevant.
    uint32 p_filesz; // Number of bytes in the file image of the segment (may be 0).
    uint32 p_memsz;  // Number of bytes in the memory image of the segment (may be 0).
    uint32 p_flags;  // Flags relevant to the segment.
    uint32 p_align;  // Value to which the segments are aligned in memory and in the file. Values 0 and 1 mean no alignment is required, otherwise p_align must be a positive, integral power of 2, and p_vaddr should equal p_offset, modulo p_align.
};

enum elf_program_type
{
    ELF_PT_NULL    = 0, // The array element is unused.
    ELF_PT_LOAD    = 1, // The array element specifies a loadable segment, described by p_filesz and p_memsz.
    ELF_PT_DYNAMIC = 2, // The array element specifies dynamic linking information.
    ELF_PT_INTERP  = 3, // The array element specifies the location and size of a null-terminated path name to invoke as an interpreter.
    ELF_PT_NOTE    = 4, // The array element specifies the location and size of auxilary information.
    ELF_PT_SHLIB   = 5, // Reserved.
    ELF_PT_PHDR    = 6, // The array element specifies the location and size of the program header table itself.

    // Reserved for processor-specific semantics.
    ELF_PT_LOPROC = 0x70000000,
    ELF_PT_HIPROC = 0x7fffffff,
};

enum elf_program_header_flags
{
    ELF_PF_X = 0x1,
    ELF_PF_W = 0x2,
    ELF_PF_R = 0x4,
    
    // Reserved for operating system-specific semantics.
    ELF_PF_MASKOS = 0x0ff00000,
    // Reserved for processor-specific semantics.
    ELF_PF_MASKPROC = 0xf0000000,
};

void elf_load(void *memory, usize size);

 
#endif // SRC_ELF_H
 
