#include "ir0_stream.h"


void ir0_push_0(ir0_stream *stream, int32 tag)
{
    ir0_instruction instruction = { .tag = tag };
    ir0->stream[ir0->count++] = instruction;
}

void ir0_push_1r(ir0_stream *stream);
void ir0_push_1u(ir0_stream *stream);
void ir0_push_1l(ir0_stream *stream);
void ir0_push_2ru(ir0_stream *stream);
void ir0_push_2rr(ir0_stream *stream);
void ir0_push_3rrr(ir0_stream *stream);
void ir0_push_3rru(ir0_stream *stream);
