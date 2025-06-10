#include "ir0_stream.h"


void ir0_push_0(ir0_stream *stream, int32 tag)
{
    ir0_instruction instruction = { .tag = tag };
    stream->instructions[stream->count++] = instruction;
}

// void ir0_push_1r(ir0_stream *stream);
// void ir0_push_1u(ir0_stream *stream);
// void ir0_push_1l(ir0_stream *stream);
// void ir0_push_2ru(ir0_stream *stream);
// void ir0_push_2rr(ir0_stream *stream);
// void ir0_push_3rrr(ir0_stream *stream);
// void ir0_push_3rru(ir0_stream *stream);

int32 ir0_push_label_at(ir0_stream *stream, string_view label, int32 at)
{
    int32 result = -1;
    if (stream->label_count < stream->label_capacity)
    {
        // copy string
        for (uint32 i = 0; i < label.size; i++)
        {
            stream->label_buffer[stream->label_buffer_size + i] = label.data[i];
        }
        stream->labels[stream->label_count] = (string_view)
        {
            .data = stream->label_buffer + stream->label_buffer_size,
            .size = label.size,
        };
        stream->label_at[stream->label_count] = at;
        result = stream->label_count;
        stream->label_buffer_size += label.size;
        stream->label_count += 1;
    }
    printf("Label buffer\n->|%.*s|<-\n", (int) stream->label_buffer_size, stream->label_buffer);
    return result;
}

int32 ir0_push_label(ir0_stream *stream, string_view label)
{
    return ir0_push_label_at(stream, label, -1);
}

int32 ir0_find_label(ir0_stream *stream, string_view label)
{
    for (uint32 label_index = 0; label_index < stream->label_count; label_index++)
        if (string_view_equal(stream->labels[label_index], label))
            return label_index;
    return -1;
}
