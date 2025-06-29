#include "ir0_to_bytecode.h"


usize ir0_to_bytecode(byte *memory, usize memory_size, ir0_stream *stream)
{
    usize running_index = 0;
    for (usize i = 0; i < stream->count; i++)
    {
        uint32 bytecode = 0;

        ir0_instruction instruction = stream->instructions[i];
        if (instruction.tag == Ir0_Mov)
        {
            if (instruction.args[0].tag == Ir0_ArgumentRegister)
            {
                uint32 v = instruction.args[0].u32;
                bytecode = bytecode | (v << BYTECODE_REG1_SHIFT);
            }
            else
            {
                printf("BytecodeGen Error: The first argument of 'mov' instruction could not be anything except register!\n");
                return 0;
            }

            if (instruction.args[1].tag == Ir0_ArgumentRegister)
            {
                uint32 v = instruction.args[1].u32;
                bytecode = bytecode | (Bytecode_Mov_RR << BYTECODE_OPCODE_SHIFT);
                bytecode = bytecode | (v << BYTECODE_REG2_SHIFT);
            }
            else if (instruction.args[1].tag == Ir0_ArgumentImmediate)
            {
                uint32 v = instruction.args[1].u32;
                bytecode = bytecode | (Bytecode_Mov_RI << BYTECODE_OPCODE_SHIFT);
                bytecode = bytecode | (v << BYTECODE_IMM_SHIFT);
            }
            else
            {
                printf("BytecodeGen Error: The second argument of 'mov' instruction could be only register or immediate!\n");
                return 0;
            }
        }
        if (instruction.tag == Ir0_Ldr)
        {
            if (instruction.args[0].tag == Ir0_ArgumentRegister)
            {
                uint32 v = instruction.args[0].u32;
                bytecode = bytecode | (v << BYTECODE_REG1_SHIFT);
            }
            else
            {
                printf("BytecodeGen Error: The first argument of 'ldr' instruction could not be anything except register!\n");
                return 0;
            }

            if (instruction.args[1].tag == Ir0_ArgumentAddress)
            {
                int32 c = instruction.args[1].addr.c;
                int32 r1 = instruction.args[1].addr.r1;
                int32 r2 = instruction.args[1].addr.r2;
                int32 a = instruction.args[1].addr.a;

                uint32 address_calculation_type = BytecodeAddress_Invalid;
                if (r1 == -1 && r2 == -1 && a != 0)
                {
                    address_calculation_type = BytecodeAddress_A;
                }
                else if (c != 0 && r1 != -1)
                {
                    if (r2 == -1 && a == 0)
                        address_calculation_type = BytecodeAddress_CR;
                    if (r2 == -1 && a != 0)
                        address_calculation_type = BytecodeAddress_CR_A;
                    if (r2 != -1 && a == 0)
                        address_calculation_type = BytecodeAddress_CR_R;
                    if (r2 != -1 && a != 0)
                        address_calculation_type = BytecodeAddress_CR_R_A;
                }
                bytecode = bytecode | ((address_calculation_type << BYTECODE_ADDRESS_SHIFT) & BYTECODE_ADDRESS_MASK);
                bytecode = bytecode | (Bytecode_Ldr_RA << BYTECODE_OPCODE_SHIFT);
                bytecode = bytecode | ((r1 << BYTECODE_REG2_SHIFT) & BYTECODE_REG2_MASK);
                bytecode = bytecode | ((r2 << BYTECODE_REG3_SHIFT) & BYTECODE_REG3_MASK);
                bytecode = bytecode | ((c  << BYTECODE_C_SHIFT) & BYTECODE_C_MASK);
                bytecode = bytecode | ((a  << BYTECODE_A_SHIFT) & BYTECODE_A_MASK);
            }
            else
            {
                printf("BytecodeGen Error: The second argument of 'ldr' instruction could not be anything except address calculation\n");
            }
        }


        if (running_index + 4 < memory_size)
        {
            memcpy(memory + running_index, &bytecode, sizeof(bytecode_t));
        }
        else
        {
            printf("BytecodeGen Error: Not enough space to store bytecode\n");
        }
        running_index += sizeof(bytecode_t);
    }

    return running_index;
}
