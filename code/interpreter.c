#include "interpreter.h"


void interpret_instruction_ret(interpreter *interp, uint32 instruction)
{
    (void) interp;
    (void) instruction;
    printf("Interpreter Error: 'ret' instruction is not implemented;\n");
}

void interpret_instruction_mov_rr(interpreter *interp, uint32 instruction)
{
    uint32 reg1 = (instruction & BYTECODE_REG1_MASK) >> BYTECODE_REG1_SHIFT;
    uint32 reg2 = (instruction & BYTECODE_REG2_MASK) >> BYTECODE_REG2_SHIFT;
    printf("Interpreter: mov r%d, r%d\n", reg1, reg2);
    interp->r[reg1] = interp->r[reg2];
}

void interpret_instruction_mov_ri(interpreter *interp, uint32 instruction)
{
    uint32 reg = (instruction & BYTECODE_REG1_MASK) >> BYTECODE_REG1_SHIFT;
    uint32 imm = (instruction & BYTECODE_IMM_MASK) >> BYTECODE_IMM_SHIFT;
    printf("Interpreter: mov r%d, %d\n", reg, imm);
    interp->r[reg] = imm;
}

void interpret_instruction_ldr_ra(interpreter *interp, uint32 instruction)
{
    uint32 type = (instruction & BYTECODE_ADDRESS_MASK) >> BYTECODE_ADDRESS_SHIFT;
    uint32 r1 = (instruction & BYTECODE_REG1_MASK) >> BYTECODE_REG1_SHIFT;
    uint32 r2 = (instruction & BYTECODE_REG2_MASK) >> BYTECODE_REG2_SHIFT;
    uint32 r3 = (instruction & BYTECODE_REG3_MASK) >> BYTECODE_REG3_SHIFT;
    int32 c = (instruction & BYTECODE_C_MASK) >> BYTECODE_C_SHIFT;
    if (c & 0x8)
        c = c | 0xfffffff0;
    int32 a = (instruction & BYTECODE_A_MASK) >> BYTECODE_A_SHIFT;
    if (a & 0x80)
        a = a | 0xffffff00;

    uint32 addr = 0;
    switch (type)
    {
        case BytecodeAddress_A: addr = a; break;
        case BytecodeAddress_CR: addr = c * interp->r[r2]; break;
        case BytecodeAddress_CR_A: addr = c * interp->r[r2] + a; break;
        case BytecodeAddress_CR_R: addr = c * interp->r[r2] + interp->r[r3]; break;
        case BytecodeAddress_CR_R_A: addr = c * interp->r[r2] + interp->r[r3] + a; break;

        case BytecodeAddress_Invalid:
        default:
            printf("BytecodeInterpreterError: Address calculation type is invalid (0x%u)\n", type);
    }
    uint32 value;
    memcpy(&value, interp->memory + addr, sizeof(uint32));
    interp->r[r1] = value;
}

typedef void interp_function_t(interpreter *, uint32);
static interp_function_t *interp_functions[] =
{
    NULL, //  0 -
    NULL, //  1 Ret
    interpret_instruction_mov_rr, //  2 Mov_RR
    interpret_instruction_mov_ri, //  3 Mov_RI
    interpret_instruction_ldr_ra, //  4 Ldr_RA
    NULL, //  5 Str_RA
    NULL, //  6 -
    NULL, //  7 -
    NULL, //  8 Add_RR
    NULL, //  9 Add_RI
    NULL, // 10 Sub_RR
    NULL, // 11 Sub_RI
    NULL, // 12 Mul_RR
    NULL, // 13 Mul_RI
    NULL, // 14 Div_RR
    NULL, // 15 Div_RI
    NULL, // 16 Cmp_RR
    NULL, // 17 Cmp_RI
    NULL, // 18 -
    NULL, // 19 Jmp
    NULL, // 20 Jeq
    NULL, // 21 Jne
    NULL, // 22 Jlt
    NULL, // 23 Jle
    NULL, // 24 Jgt
    NULL, // 25 Jge
};



int32 interpreter_step(interpreter *interp)
{
    int32 ec = 0;

    uint32 instruction;
    memcpy(&instruction, interp->memory + interp->ip, sizeof(bytecode_t));

    interp->ip += sizeof(bytecode_t);
    uint32 opcode = (instruction & BYTECODE_OPCODE_MASK) >> BYTECODE_OPCODE_SHIFT;
    interp_function_t *f = interp_functions[opcode];

    if (f)
    {
        f(interp, instruction);
    }
    else
    {
        printf("BytecodeInterpreterError: bytecode 0x%08x at memory location 0x%08x\n", *(uint32 *) (interp->memory + interp->ip), interp->ip);
        ec = 1;
    }

    return ec;
}

void interpreter_print_state(interpreter *interp)
{
    printf("Registers:            Address      | Memory                  | Ascii\n");
    for (int i = 0; i < 16; i++)
    {
        if (i < 10) printf(" ");
        printf("r%d = %10u      ", i, interp->r[i]);
        printf("0x%010x |", i * 8);
        for (int j = 0; j < 8; j++)
        {
            printf(" %02x", interp->memory[i*8 + j] & 0xff);
        }
        printf(" | ");
        for (int j = 0; j < 8; j++)
        {
            char c = interp->memory[i*8 + j];
            printf("%c", c < 'A' ? '.' : c);
        }
        printf("\n");
    }
}


