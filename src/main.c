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
            write(1, node->op.span, node->op.span_size); 
        }
        break;

        case AST_NODE_VARIABLE:
        {
            write(1, node->var_span, node->var_span_size);
        }
        break;

        case AST_NODE_LITERAL_INT:
        {
            write(1, node->literal_span, node->literal_span_size);
        }
        break;
        
        case AST_NODE_VARIABLE_DECLARATION:
        case AST_NODE_CONSTANT_DECLARATION:
        {
            write(1, "VAR{", 4);
            write(1, node->var_name.span, node->var_name.span_size);
            write(1, ":", 1);
            if (node->var_type.type != TOKEN_INVALID)
            {
                write(1, node->var_type.span, node->var_type.span_size);
            }
            if (node->init)
            {
                write(1, node->is_constant ? ":" : "=", 1);
                print_ast(node->init);
            }
            write(1, "}", 1);
        }
        break;

        case AST_NODE_FUNCTION_DEFINITION:
        {
            write(1, "(){\n", 4);
            if (node->statement_list)
            {
                print_ast(node->statement_list);
            }
            write(1, "\n}\n", 3);
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        {
            write(1, node->function_name.span, node->function_name.span_size);
            write(1, "()", 2);
        }
        break;

        case AST_NODE_STATEMENT_LIST:
        {
            print_ast(node->statement);
            write(1, ";\n", 2);
            if (node->next_statement)
            {
                print_ast(node->next_statement);
            }
        }
        break;

        case AST_NODE_GLOBAL_DECLARATION_LIST:
        {
            print_ast(node->declaration);
            write(1, "\n\n", 2);
            if (node->next_declaration)
            {
                print_ast(node->next_declaration);
            }
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
        return 1;
    }

    int memory_buffer_size = 4096;
    void *memory_buffer = mmap2(0, memory_buffer_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    allocator a;
    initialize_memory_arena(&a, memory_buffer, memory_buffer_size);

    int fd = open(argv[1], 0, O_RDONLY);
    if (fd <= 0)
    {
        write(1, "Error < 0\n", 10);
        return 1;
    }

    char buffer[4096] = {0};
    int  buffer_size = read(fd, buffer, ARRAY_COUNT(buffer) - 1);
    close(fd);
    write(1, buffer, buffer_size);
    write(1, "EOF\n", 4);

    lexer l =
    {
        .buffer = buffer,
        .buffer_size = buffer_size,
        .line = 1,
        .column = 1,
        .next_token_valid = false,
    };

    // ast_node *expression = pinapl_parse_variable_declaration(&a, &l);
    // ast_node *expression = pinapl_parse_function_definition(&a, &l);
    // ast_node *expression = pinapl_parse_statement(&a, &l);
    // ast_node *expression = pinapl_parse_statement_list(&a, &l);
    ast_node *expression = pinapl_parse_global_declaration_list(&a, &l);

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

