# Text to binary (ttb)

This tool allows you to write text, and convert it into a binary.

You write binary in the following form:

1. Two sequantial symbols from `[A-Fa-f0-9]`
2. Any number of other symbols between those pairs
3. Comments start with `#` and go until the end of the line

Example:

```
dead beef  # This is the comment
cafe bebe
```

## Compilation

I provide source code of ttb in four forms: C program, x86_64 assembly, ELF 64-bit binary,

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

I said, that you could read and understand `ttb.txt` file, but before that you should learn about ELF file structure, and x86_64 encoding.

### ELF 64-bit

### Encoding

#### REX prefix

#### ModRM

#### Registers

| X.Reg | Register |
| ----  |:-------- |
| 0.000 | al       |
| 0.001 | cl       |
| 0.010 | dl       |
| 0.011 | bl       |
| 0.100 | sp       |
| 0.101 | bp       |
| 0.110 | si       |
| 0.111 | di       |
| 1.000 | r8       |
| 1.001 | r9       |
| 1.010 | r10      |
| 1.011 | r11      |
| 1.100 | r12      |
| 1.101 | r13      |
| 1.110 | r14      |
| 1.111 | r15      |

#### Instructions

- `/r` indicates that `reg` field is used as a register
- `/digit` a digit between 0 and 7 indicates that `reg` field used as an opcode extension

| Instruction     | Opcode        |
| --------------- |:------------- |
| MOV r/m64,imm32 | REX.W + c7 /0 |
| MOV r/m64,r64   | REX.W 89      |
| CALL            |               |
| RET             | c3            |
