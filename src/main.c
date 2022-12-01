#include "syscall.h"
#include "string.h"
#include "parser.h"
#include "allocator.h"


void print_ast(ast_node *node)
{
    switch (node->type)
    {
        case AST_NODE_BINARY_OPERATOR:
        {
            if (node->lhs) print_ast(node->lhs);
            if (node->rhs) print_ast(node->rhs);
            write(1, node->t.span, node->t.span_size); 
        }
        break;

        case AST_NODE_VARIABLE:
        {
            write(1, node->t.span, node->t.span_size);
        }
        break;

        case AST_NODE_LITERAL_INT:
        {
            write(1, node->t.span, node->t.span_size);
        }
        break;

        default:
            write(1, "<!!!>", 5);
    }
}


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

    ast_node *expression = parser_parse_expression(&a, &l, 0);
    token t = lexer_get_token(&l);
    if (expression && t.type == TOKEN_EOF)
    {
        write(1, "Language recognized!\n", 21);
        print_ast(expression);
    }
    else
    {
        write(1, "Language is not recognized!\n", 28);
    }

    munmap(memory_buffer, memory_buffer_size);

    return 0;
}

