#include "syscall.h"
#include "string.h"
#include "parser.h"
#include "allocator.h"


int main(int argc, char **argv, char **env)
{   
    if (argc < 2)
    {
        write(1, "Usage: ./fop FILEPATH\n", 22);
        return 0;
    }

    int memory_buffer_size = 4096;
    void *memory_buffer = mmap2(0, memory_buffer_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    allocator a;
    initialize_memory_arena(&a, memory_buffer, memory_buffer_size);

    int fd = open(argv[1], 0, O_RDONLY);
    if (fd < 0)
    {
        write(1, "Error < 0\n", 10);
    }

    char buffer[4096] = {0};
    int  buffer_size = read(fd, buffer, ARRAY_COUNT(buffer) - 1);
    write(1, buffer, buffer_size);
    write(1, "EOF\n", 4);
    close(fd);

    lexer l =
    {
        .buffer = buffer,
        .buffer_size = buffer_size,
        .line = 1,
        .column = 1,
        .next_token_valid = false,
    };

    token t = lexer_get_token(&l);
    while ((t.type != TOKEN_EOF) && (t.type != TOKEN_INVALID))
    {
        char const *s = token_type_to_cstring(t.type);
        int sz = cstring_size_no0(s);

        write(1, s, sz);
        write(1, " ", 1);
        write(1, t.span, t.span_size);
        write(1, "\n", 1);

        lexer_eat_token(&l);
        t = lexer_get_token(&l);
    }

    munmap(memory_buffer, memory_buffer_size);

    return 0;
}

