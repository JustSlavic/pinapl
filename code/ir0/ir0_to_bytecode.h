#ifndef IR0_TO_BYTECODE_H
#define IR0_TO_BYTECODE_H

#include "../common.h"
#include "ir0_stream.h"


usize ir0_to_bytecode(byte *memory, usize memory_size, ir0_stream *stream);


#endif // IR0_TO_BYTECODE_H
