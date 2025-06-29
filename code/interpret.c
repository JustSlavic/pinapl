#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "string_view.h"
#include "ir0/ir0_stream.h"
#include "ir0/ir0_parser.h"
#include "ir0/ir0_to_bytecode.h"
#include "bytecode.h"
#include "interpreter.h"


typedef struct command_line_arguments
{
    string_view file;
} command_line_arguments;


command_line_arguments parse_command_line_arguments(int argc, char **argv)
{
    command_line_arguments result = {};

    string_view arg_file_1 = make_string_view_from_cstring("--file");
    string_view arg_file_2 = make_string_view_from_cstring("-f");

    for (int i = 0; i < argc; i++)
    {
        string_view arg = make_string_view_from_cstring(argv[i]);
        if (string_view_equal(arg, arg_file_1) || string_view_equal(arg, arg_file_2))
        {
            if ((i + 1) < argc)
            {
                result.file = make_string_view_from_cstring(argv[i + 1]);
            }
        }
    }

    return result;
}


int main(int argc, char **argv)
{
    command_line_arguments args = parse_command_line_arguments(argc, argv);
    printf("filename = " STRING_VIEW_FMT "\n", STRING_VIEW_PRINT(args.file));

    int32 file_content_capacity = 1024;
    char *file_content = malloc(file_content_capacity);
    int32 file_size = 0;

    int fd = open(args.file.data, O_RDONLY, 0);
    if (fd >= 0)
    {
        int read_bytes = read(fd, file_content, file_content_capacity);
        if (read_bytes > 0) file_size = read_bytes;
        close(fd);
    }
    else
    {
        printf("Could not open the file \"%s\"\n", args.file.data);
        return 1;
    }

    printf("file_size = %d\n", file_size);

    ir0_stream stream = ir0_parse_text(file_content, file_size);
    if (stream.count == 0) return 1;

    printf("stream_count = %d\n", stream.count);

    usize bytecode_count = 256;
    usize bytecode_size = bytecode_count * sizeof(bytecode_t);
    byte *bytecode = malloc(bytecode_size);
    usize bytes_written = ir0_to_bytecode(bytecode, bytecode_size, &stream);

    usize interpreter_memory_size = 1 << 15; // 32k
    interpreter interp =
    {
        .memory = malloc(interpreter_memory_size),
        .memory_size = interpreter_memory_size,
    };

    memcpy(interp.memory, bytecode, bytes_written);

    int32 ec = 0;
    while (ec == 0)
    {
        ec = interpreter_step(&interp);
    }
    interpreter_print_state(&interp);
    
    return 0;
}


#include "string_view.c"
#include "interpreter.c"
#include "ascii.c"
#include "lexer.c"
#include "ir0/ir0_lexer.c"
#include "ir0/ir0_parser.c"
#include "ir0/ir0_stream.c"
#include "ir0/ir0_to_bytecode.c"
