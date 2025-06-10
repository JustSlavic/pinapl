#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "ir0_stream.h"
#include "ir0_parser.h"
#include "ir0_interpreter.h"


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: ir0 FILENAME\n");
        return 1;
    }

    char *filename = argv[1];
    printf("filename = \"%s\"\n", filename);

    int32 file_content_capacity = 1024;
    char *file_content = malloc(file_content_capacity);
    int32 file_size = 0;

    int fd = open(filename, O_RDONLY, 0);
    if (fd >= 0)
    {
        int read_bytes = read(fd, file_content, file_content_capacity);
        if (read_bytes > 0) file_size = read_bytes;
        close(fd);

        printf("file_size = %d\n", file_size);

        ir0_stream stream = ir0_parse_text(file_content, file_size);
        (void) stream;

        ir0_interpreter interpreter =
        {
            .stream = &stream,
            .memory = malloc(1 << 10),
            .memory_size = 1 << 10,
        };
        ir0_interpreter_run(&interpreter);
        ir0_interpreter_print_state(&interpreter);
    }
    else
    {
        printf("Could not open the file \"%s\"\n", filename);
        return 1;
    }

    return 0;
}


#include "ascii.c"
#include "string_view.c"
#include "lexer.c"
#include "ir0_lexer.c"
#include "ir0_parser.c"
#include "ir0_stream.c"
#include "ir0_interpreter.c"
