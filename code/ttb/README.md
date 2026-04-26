# Text to binary (ttb)

This tool allows you to write text, and convert it into a binary.

You write binary in the following form:

1. Two sequantial symbols from `[A-Fa-f0-9]`
2. Any number of other symbols between those pairs are ignored
3. Comments start with `#` and go until the end of the line

Example:

```
dead beef  # This is the comment
ca fe be be
```

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

## Encoding

I said, that you could read and understand `ttb.txt` file, but if you don't know the ELF file structure, and x86_64 encoding, you can consult the following information:

### ELF 64-bit



### Encoding

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

Full documentation for x86_64 encoding you can find on the Intel site: [Intel Architecture Manual volume 2a](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf), [Intel Architecture Manual volume 2b](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf), [Intel Architecture Manual volume 2c](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2c-manual.pdf)

- `/r` indicates that `ModRM.reg` field is used as a register
- `/digit` a digit between 0 and 7 indicates that `ModRM.reg` field used as an opcode extension

| Instruction     | Opcode            | Example              | Description  |
| --------------- |:-------------     |:-------------------- |:------------ |
| MOV r/m64,imm32 | REX.W + c7 /0     | `mov rax, 0`         |              |
| MOV r/m64,r64   | REX.W + 89        | `mov rax, rbx`       ||
| MOV r64,r/m64   | REX.W + 8b        | `mov rdi, [rsp]`     ||
| MOV r/m8,r8     | REX.W + 88        |  ||
| LEA r64,m       | REX.W + 8d        | `lea rsi, [rsp + 8]` ||
| CALL            | e8                | `call main` ||
| SYSCALL         | 0f 05             |||
| RET             | c3                |||
| AND             |                   |||
| OR r/m64,r64    | REX.W + 09 /r     |||
| XOR r/m64,r64   | REX.W + 31        |||
| SHR             |                   |||
| SHL r/m64,imm8  | REX.W + c1 /4 _ib_  |||
| CMP             | REX.W + 83 /7 _ib_  | `cmp rdi, 3` ||
| CMP r/m64,r64   | REX.W + 39 /r     |||
| JL rel32        | 0f 8c _cd_          | `jl L_return` ||
| JE/JZ           | 0f 84             |||
| JGE | 7d _cb_ | `jge L_break` |  |
|  |  |  |  |
