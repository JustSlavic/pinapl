# Text to binary (ttb)

This tool allows you to write text, and convert it into a binary.

You write binary in the following form:

1. Two sequantial symbols from `[A-Fa-f0-9]`
2. Any number of other symbols between those pairs are ignored
3. Comments start with `#` and go until the end of the line

Example:

```
0123456789abcdef # This is the comment
dead beef ca fe be be
```

Then run this program, to create a binary file, exactly matching the text representation.

We will use this program to create the executable ELF file of the same program.

## Compilation

I provide source code of ttb in four forms: C program, x86_64 assembly, ELF 64-bit binary, and the text form of the binary file.

1. You could compile C program with the following command:

```bash
gcc -o ttbc ttb.c
```

2. x86_64 assembly in the GNU Assembler with Intel syntax. You could compile it with `as` with the following command:

```bash
as -msyntax=intel -mmnemonic=intel -mnaked-reg -o ttb.o ttb.S
ld -z noexecstack -o ttb ttb.o
```

3. If you are on the x86_64 Linux (`uname -si` prints `Linux x86_64`), you could try to run the ELF 64-bit binary in the `bin` directory.

If you do not trust this binary, you can compare binary with its text form `ttb.txt`, where I explain every single byte.

4. If you do not have C compiler, assebler, and binary doesn't run, unfortunately you'll have to investigate the reason yourself, write your own text-to-binary program, or even type binary by hand.

The file `ttb.txt` contains the most exhaustive explanation of every single byte I could make. You could learn what's going on in this file, because it's relatively small, so you could read it in reasonable time.

## Running the program

The ttb usage: `./ttb [input_path] [output_path]`

Unfortunately, in order to reduce the size of the instruction stream, to make the code of ttb manageable, it does not check the correctness of the input text.

If `output_path` does not exits, it will be created with the mode `0744 (rwxr--r--)`.

## Double check

After the compilation, you should be able to run ttb on the file `ttb.txt` itself, and produce the same program again.

```bash
./ttb ttb.txt ttb2
```

Now you can run ttb2 and produce ttb3:

```bash
./ttb2 ttb.txt ttb3
```

the `ttb2` and `ttb3` should be exactly the same files:

```bash
$ cmp ttb2 ttb3
$ echo $?
0
```

## Explanation of ELF64 file format, and x86_64 instruction encoding

I said, that you could read and understand `ttb.txt` file, but if you don't know the ELF file structure, or x86_64 encoding, you can consult the following information:

### ELF 64-bit

#### ELF64 header

| Name          | Size in bytes  | Meaning |
|:------------- |:-------------- |:------- |
| `e_ident`     | 16 | Identifies the file as ELF object file, and provide information about the data representation of the object file structures |
| `e_type`      | 2  | Identifies the object file type |
| `e_machine`   | 2  | Identifies the target architecture |
| `e_version`   | 4  | Identifies the version of the object file format, `EV_CURRENT` currently is the only possible value |
| `e_entry`     | 8  | The virtual address of the program entry point (zero if no entry point) |
| `e_phoff`     | 8  | Contains the file offset, in bytes, of the program header table |
| `e_shoff`     | 8  | Contains the file offset, in bytes, of the section header table |
| `e_flags`     | 4  | Processor-specific flags |
| `e_ehsize`    | 2  | Contains the size, in bytes, of the ELF header |
| `e_phentsize` | 2  | Contains the size, in bytes, of a program header table entry (they all the same size) |
| `e_phnum`     | 2  | The number of entries in the program header table |
| `e_shentsize` | 2  | Contains the size, in bytes, of a section header table entry (they all the same size) |
| `e_shnum`     | 2  | The number of entries in the section header table |
| `e_shstrndx`  | 2  | Contains the index of the section containing the section name string table, if there's no section name string table, it's zero |

```c
typedef struct
{
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_header;
```

##### ELF Identification

| Name            | Value | Meaning |
|:--------------- |:----- |:------- |
| `EI_MAG0`       | 0     | `e_ident[EI_MAG0] == 0x7f` |
| `EI_MAG1`       | 1     | `e_ident[EI_MAG1] == 0x45 /* 'E' */` |
| `EI_MAG2`       | 2     | `e_ident[EI_MAG2] == 0x4c /* 'L' */` |
| `EI_MAG3`       | 3     | `e_ident[EI_MAG3] == 0x46 /* 'F' */` |
| `EI_CLASS`      | 4     | Identifies the file class, value `#define ELFCLASS32 1` means 32-bit object, `#define ELFCLASS64 2` means 64-bit object. |
| `EI_DATA`       | 5     | Specifies the data encoding of the object file data, `ELFDATA2LSB` or `ELFDATA2MSB`. |
| `EI_VERSION`    | 6     | The version of the object file format, `EV_CURRENT` currently is the only possible value. |
| `EI_OSABI`      | 7     | Identifies the OS and ABI for which the object is prepared. |
| `EI_ABIVERSION` | 8     | Identifies the version of the ABI for which the object is prepared. |
| `EI_PAD`        | 9     ||
| `EI_NIDENT`     | 16    ||

```c
enum
{
    EI_MAG0 = 0,
    EI_MAG1 = 1,
    EI_MAG2 = 2,
    EI_MAG3 = 3,
    EI_CLASS = 4,
    EI_DATA = 5,
    EI_VERSION = 6,
    EI_OSABI = 7,
    EI_ABIVERSION = 8,
    EI_PAD = 9,
    EI_NIDENT = 16,
};

enum
{
    ELFCLASS32 = 1, /* 32-bit object */
    ELFCLASS64 = 2, /* 64-bit object */
};

enum
{
    ELFDATA2LSB = 1, /* Little-endian */
    ELFDATA2MSB = 2, /* Big-endian */
};

enum
{
    EV_CURRENT = 1, /* Current ELF Format version */
};

enum
{
    ELFOSABI_SYSV = 0, /* System V ABI */
    ELFOSABI_STANDALONE = 255, /* Standalone (embedded) application */
};
```

##### Object file types

| Name        | Value  | Meaning |
|:----------- |:------ |:------- |
| `ET_NONE`   | 0      | No file type |
| `ET_REL`    | 1      | Relocatable object file |
| `ET_EXEC`   | 2      | Executable file |
| `ET_DYN`    | 3      | Shared object file |
| `ET_CORE`   | 4      | Core file |
| `ET_LOOS`   | 0xfe00 | Environment-specific |
| `ET_HIOS`   | 0xfeff ||
| `ET_LOPROC` | 0xff00 | Processor-specific |
| `ET_HIPROC` | 0xffff ||

```c
enum
{
    ET_NONE = 0, /* No file type */
    ET_REL = 1, /* Relocatable object file */
    ET_EXEC = 2, /* Executable file */
    ET_DYN = 3, /* Shared object file */
    ET_CORE = 4, /* Core file */
    ET_LOOS = 0xfe00,
    ET_HIOS = 0xfeff,
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff,
};
```

#### Program header

| Name       | Size in bytes  | Meaning |
|:-----------|:-------------- |:------- |
| `p_type`   | 4 | Identifies the type of segment. |
| `p_flags`  | 4 | Contains the segment attributes. |
| `p_offset` | 8 | Contains the offset, in bytes, of the segment from the beginning of the file. |
| `p_vaddr`  | 8 | Contains the virtual address of the segment in memory. |
| `p_paddr`  | 8 | Reserved for systems with physical addressing. |
| `p_filesz` | 8 | Contains the size, in bytes, of the segment in the file. |
| `p_memsz`  | 8 | Contains the size, in bytes, of the segment in memory. |
| `p_align`  | 8 | Specifies the alignment of the segment. Must be a power of two. The values of p_offset and p_vaddr must be congruent modulo the alignment. |

```c
typedef struct
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} elf64_program_header;
```

##### Segment types `p_type`

| Name         | Value      | Meaning |
|:------------ |:---------- |:------- |
| `PT_NULL`    | 0          | Unused |
| `PT_LOAD`    | 1          | Loadable segment |
| `PT_DYNAMIC` | 2          | Dynamic linking tables |
| `PT_INTERP`  | 3          | Program interpreter path name |
| `PT_NOTE`    | 4          | Note section |
| `PT_SHLIB`   | 5          | Reserved |
| `PT_PHDR`    | 6          | Program header table |
| `PT_LOOS`    | 0x60000000 | Environment-specific |
| `PT_HIOS`    | 0x6fffffff | |
| `PT_LOPROC`  | 0x70000000 | Processor-specific |
| `PT_HIPROC`  | 0x7fffffff | |

```c
enum
{
    PT_NULL = 0,
    PT_LOAD = 1,
    PT_DYNAMI = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_SHLIB = 5,
    PT_PHDR = 6,
    PT_LOOS = 0x60000000,
    PT_HIOS = 0x6fffffff,
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7fffffff,
};
```

##### Segment attributes `p_flags`

| Name          | Value      | Meaning |
|:------------- |:---------- |:------- |
| `PF_X`        | 0x1        | Execute permission |
| `PF_W`        | 0x2        | Write permission |
| `PF_R`        | 0x4        | Read permission |
| `PF_MASKOS`   | 0x00ff0000 | Bits reserved for enviromnent-specific flags |
| `PF_MASKPROC` | 0xff000000 | Bits reserved for processor-specific flags |

```c
enum
{
    PF_X = 0,
    PF_W = 1,
    PF_R = 2,
    PF_MASKOS = 3,
    PF_MASKPROC = 4,
};
```

#### Section header

| Name           | Size in bytes  | Meaning |
|:-------------- |:-------------- |:------- |
| `sh_name`      | 4 | Offset, in bytes, to the section name, relative to the start of the section name string table. |
| `sh_type`      | 4 | Identifies the section type (see table below). |
| `sh_flags`     | 8 | Identifies the attributes of the section (see table below). |
| `sh_addr`      | 8 | Contains the virtual address of the beginning of the section in memory. If the section is not allocated to the memory image of the program, this field should be zero |
| `sh_offset`    | 8 | Contains the offset in bytes of the beginning of the section in file. |
| `sh_size`      | 8 | Contains the size, in bytes, of the section (except for SHT_NOBITS). |
| `sh_link`      | 4 | Contains the section index of an associated section. |
| `sh_info`      | 4 | Contains extra information. |
| `sh_addralign` | 8 | Required alignment of the section. This field MUST be a power of two. |
| `sh_entsize`   | 8 | The size, in bytes, of each entry for sections that contain fixed-size entries, otherwise zero. |

```c
typedef struct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} elf64_section_header;
```

##### Section header types `sh_type`

| Name           | Value | Meaning |
|:-------------- |:----- |:------- |
| `SHT_NULL`     | 0     | Marks an unused/reserved section header |
| `SHT_PROGBITS` | 1     | Information defined by the program |
| `SHT_SYMTAB`   | 2     | A linker symbol table |
| `SHT_STRTAB`   | 3     | A string table |
| `SHT_RELA`     | 4     | "Rela" type relocation entries |
| `SHT_HASH`     | 5     | A symbol hash table |
| `SHT_DYNAMIC`  | 6     | Dynamic linking tables |
| `SHT_NOTE`     | 7     | Note information |
| `SHT_NOBITS`   | 8     | Uninitialized space; does not occupy any space in the file |
| `SHT_REL`      | 9     | "Rel" type relocation entries |
| `SHT_SHLIB`    | 10    | Reserved |
| `SHT_DYNSYM`   | 11    | A dynamic loader symbol table |
| `SHT_LOOS`     | 0x60000000 | Environment-specific |
| `SHT_HIOS`     | 0x6fffffff ||
| `SHT_LOPROC`   | 0x70000000 | Processor-specific |
| `SHT_HIPROC`   | 0x7fffffff ||

```c
enum
{
    SHT_NULL = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB = 2,
    SHT_STRTAB = 3,
    SHT_RELA = 4,
    SHT_HASH = 5,
    SHT_DYNAMIC = 6,
    SHT_NOTE = 7,
    SHT_NOBITS = 8,
    SHT_REL = 9,
    SHT_SHLIB = 10,
    SHT_DYNSYM = 11,
    SHT_LOOS = 0x60000000,
    SHT_HIOS = 0x6fffffff,
    SHT_LOPROC = 0x70000000,
    SHT_HIPROC = 0x7fffffff,
};
```

##### Section header attributes `sh_flags`

| Name            | Value | Meaning |
|:--------------- |:----- |:------- |
| `SHF_WRITE`     | 0x1   | Section contains writable data |
| `SHF_ALLOC`     | 0x2   | Section is allocated in memory image of program |
| `SHF_EXECINSTR` | 0x4   | Section contains executable instructions |
| `SHF_MASKOS`    | 0x0f000000 | Environment-specific |
| `SHF_MASKPROC`  | 0xf0000000 | Processor-specific |

```c
enum
{
    SHF_WRITE = 0x1,
    SHF_ALLOC = 0x2,
    SHF_EXECINSTR = 0x4,
    SHF_MASKOS = 0x0f000000,
    SHF_MASKPROC = 0xf0000000,
};
```

### x86_64 Encoding

#### ModRM

The ModRM byte is the part of x86 encoding, which can be traced back to the 8086 processor.

This is one byte that consists of three fields:

  - mod = 2-bit field determines the mode of operation:
    - `00` = memory (no displacement)
    - `01` = memory + disp8
    - `10` = memory + disp32
    - `11` = register
  - reg can be either a register, or an extension of instruction opcode
  - r/m can be either a register, or can be equal to `100` that means SIB byte is present

#### SIB byte

The SIB byte is used to encode the address computation (for example `[rbp - 8]`).

The SIB byte consists of three fields:

  - scale = `2^scale` multiplier
  - index = register that will be added to base (100 = no index)
  - base = register from which address is computed

#### REX prefix

This prefix always goes first, before the opcode, and determines the instruction/register extensions.

The REX prefix looks like that: `0100WRXB`.

It always starts with 4h (0100b), with four bits following:

  - W = sets operand size to 64-bit values
  - R = extends `ModRM.reg`
  - X = extends `SIB.index`
  - B = extends `ModRM.r/m` or `SIB.base`, if SIB byte present

#### Registers

| X.Reg | Reg .W=0 | Reg .W=1  |
| ----- |:-------- |:--------- |
| 0.000 | al       | rax       |
| 0.001 | cl       | rcx       |
| 0.010 | dl       | rdx       |
| 0.011 | bl       | rbx       |
| 0.100 | sp / SIB | rsp / SIB |
| 0.101 | bp / rip | rbp / rip |
| 0.110 | si       | rsi       |
| 0.111 | di       | rdi       |
| 1.000 | r8b      | r8        |
| 1.001 | r9b      | r9        |
| 1.010 | r10b     | r10       |
| 1.011 | r11b     | r11       |
| 1.100 | r12b     | r12       |
| 1.101 | r13b     | r13       |
| 1.110 | r14b     | r14       |
| 1.111 | r15b     | r15       |

#### Instructions

In order to make learning an instruction encoding manageable, I will use a very limited number of instructions.

Full documentation for x86_64 encoding you can find on the Intel site:
1. [Intel Architecture Manual volume 2a](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf)
2. [Intel Architecture Manual volume 2b](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf)
3. [Intel Architecture Manual volume 2c](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2c-manual.pdf)

- /r indicates that `ModRM.reg` field is used as a register
- /digit a digit between 0 and 7 indicates that `ModRM.reg` field used as an opcode extension
- _cb_ 1-byte value following the opcode
- _cd_ 4-byte value following the opcode
- _ib_ 1-byte immediate operand to the instruction
- _id_ 4-byte immediate operand to the instruction

| Instruction     | Opcode              | Example              | Description  |
| --------------- |:------------------- |:-------------------- |:------------ |
| MOV r/m64,imm32 | REX.W + c7 /0 _id_  | `mov rax, 0`         | Move imm32 sign extended to 64-bits to r/m64. |
| MOV r/m64,r64   | REX.W + 89 /r       | `mov rax, rbx`       | Move r64 to r/m64. |
| MOV r64,r/m64   | REX.W + 8b /r       | `mov rdi, [rsp]`     | Move r/m64 to r64. |
| MOV r8,r/m8     | REX + 8a /r         | `mov al, [hex_table + rax]` | Move r/m8 to r8. |
| MOV r/m8,r8     | REX + 88 /r         | `mov [r15], al`      | Move r8 to r/m8. |
| MOV r/m8,r8     | 88 /r               | `mov bl, cl`         | Mov r8 to r/m8. |
| MOV r8,r/m8     | 8a /r               | `mov al, [rax + 0x400300]` | Move r/m8 to r8. |
| LEA r64,m       | REX.W + 8d /r       | `lea rsi, [rsp + 8]` | Store effective address for m in register r64. |
| CMP r/m64,imm8  | REX.W + 83 /7 _ib_  | `cmp rdi, 3`         | Compare imm8 with r/m64. |
| CMP r/m64,r64   | REX.W + 39 /r       | `cmp r11, r13`       | Compare r64 with r/m64. |
| CMP r/m8,imm8   | 80 /7 _ib_          | `cmp al, 0xff`       | Compare imm8 with r/m8. |
| CMP AL,imm8     | 3c _ib_             | `cmp al, 0xff`       | Compare imm8 with AL.|
| TEST r/m8,r8    | 84 /r               | `test bl, bl`        | AND r8 with r/m8; set SF,ZF,PF according to result. |
| INC r/m64       | REX.W + ff /0       | `inc rax` | Increment r/m quadword by 1. |
| AND r/m8,r8     | 20 /r               | `and dl, cl`         | r/m8 AND r8. |
| OR r/m8,r8      | 08 /r               | `or cl, dl`          | r/m8 OR r8. |
| OR r/m64,r64    | REX.W + 09 /r       | `or rax, rbx`        | r/m64 OR r64. |
| XOR r/m64,r64   | REX.W + 31 /r       | `xor rax, rax`       | r/m64 XOR r64 |
| SHR r/m64,imm8  | REX.W + c1 /5 _ib_  | `shr rax, 4` | Unsigned divide r/m64 by 2, imm8 times. |
| SHL r/m64,imm8  | REX.W + c1 /4 _ib_  | `shl rax, 4` | Multiply r/m64 by 2, imm8 times. |
| JE rel8         | 74 _cb_             | `je L_return`        | Jump short if equal (ZF=1) |
| JGE rel8 / JNL rel8 | 7d _cb_         | `jge L_break`        | Jump short if greater or equal / not less (SF=OF). |
| JNE rel8 / JNZ rel8 | 75 _cb_         | `jne L_break`        | Jump short if not equal / not zero (ZF=0). |
| JMP rel8        | eb _cb_             | `jmp L_return`       | Jump short RIP = RIP + 8-bit displacement sign extended to 64-bits. |
| JL rel32        | 0f 8c _cd_          | `jl L_return`        | Jump near if less (SF≠OF). Not supported in 64-bit mode. |
| JE/JZ rel32     | 0f 84 _cd_          | `je L_break`         | Jump near if equal (ZF=1). |
| CALL            | e8 _cd_             | `call main`          | Call near, relative, displacement relative to next instruction. 32-bit displacement sign extended to 64-bit in 64-bit mode. |
| SYSCALL         | 0f 05               | `syscall`            | Fast call to privilege level 0 system procedures. |
| RET             | c3                  | `ret` | Near return to calling procedure. |
| SETE r/m8       | 0F 94               | `sete cl`            | Set byte if equal (ZF=1). |
| SETNE r/m8      | 0F 95               | `setne dl`           | Set byte if not equal (ZF=0). |
