#include "syscall.h"
#include "string.h"
#include "parser.h"
#include "allocator.h"
#include "static_checks.h"


char const *spaces = "                                           ";

void print_ast(ast_node *node, int depth)
{
    switch (node->type)
    {
        case AST_NODE_BINARY_OPERATOR:
        {
            ast_node_binary_operator *binary_op = &node->binary_operator;
            if (binary_op->lhs) print_ast(binary_op->lhs, depth);
            if (binary_op->rhs) print_ast(binary_op->rhs, depth);
            write(1, binary_op->op.span, binary_op->op.span_size); 
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
            b32 is_constant = (node->type == AST_NODE_CONSTANT_DECLARATION);
            ast_node_variable_declaration *var = &node->variable_declaration;

            write(1, spaces, depth * 2);
            write(1, "VAR{", 4);
            write(1, var->var_name.span, var->var_name.span_size);
            write(1, ":", 1);
            if (var->var_type.type != TOKEN_INVALID)
            {
                write(1, var->var_type.span, var->var_type.span_size);
            }
            if (var->init)
            {
                write(1, is_constant ? ":" : "=", 1);
                print_ast(var->init, depth);
            }
            write(1, "}", 1);
        }
        break;

        case AST_NODE_BLOCK:
        {
            write(1, spaces, depth * 2);
            write(1, "{\n", 3);
            if (node->block.statement_list)
            {
                print_ast(node->block.statement_list, depth + 1);
            }
            write(1, spaces, depth * 2);
            write(1, "}\n", 2);
        }
        break;

        case AST_NODE_FUNCTION_DEFINITION:
        {
            write(1, "()", 2);
            if (node->function_definition.block)
            {
                write(1, "\n", 1);
                print_ast(node->function_definition.block, depth + 1);
            }
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        {
            write(1, node->function_call.name.span, node->function_call.name.span_size);
            write(1, "()", 2);
        }
        break;

        case AST_NODE_STATEMENT_LIST:
        {
            print_ast(node->statement_list.node, depth);
            write(1, ";\n", 2);
            if (node->statement_list.next)
            {
                print_ast(node->statement_list.next, depth);
            }
        }
        break;

        case AST_NODE_GLOBAL_DECLARATION_LIST:
        {
            print_ast(node->global_list.node, depth);
            write(1, "\n\n", 2);
            if (node->global_list.next)
            {
                print_ast(node->global_list.next, depth);
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
        write(1, "Usage: ./pinapl FILEPATH\n", 25);
        return 1;
    }

    usize memory_buffer_size = MEGABYTES(5);
    void *memory_buffer = mmap2(0, memory_buffer_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    void *memory_for_ast = memory_buffer;
    usize memory_for_ast_size = MEGABYTES(1);

    void *memory_for_scopes = memory_for_ast + memory_for_ast_size;
    usize memory_for_scopes_size = MEGABYTES(1);

    void *memory_for_err = memory_for_scopes + memory_for_scopes_size;
    usize memory_for_err_size = KILOBYTES(5);

    struct allocator ast_allocator;
    initialize_memory_arena(&ast_allocator, memory_for_ast, memory_for_ast_size);

    struct allocator scope_allocator;
    initialize_memory_arena(&scope_allocator, memory_for_scopes, memory_for_scopes_size);

    struct allocator err_allocator;
    initialize_memory_arena(&err_allocator, memory_for_err, memory_for_err_size);

    char const *filename = argv[1];

    int fd = open(filename, 0, O_RDONLY);
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

    struct pinapl_parser parser = pinapl_init_parser(&ast_allocator, &err_allocator, filename, buffer, buffer_size);
    
    // ast_node *expression = pinapl_parse_variable_declaration(&a, &l);
    // ast_node *expression = pinapl_parse_function_definition(&a, &l);
    // ast_node *expression = pinapl_parse_statement(&a, &l);
    // ast_node *expression = pinapl_parse_statement_list(&a, &l);
    ast_node *expression = pinapl_parse_global_declaration_list(&parser);

    token t = pinapl_get_token(&parser);
    if (expression && t.type == TOKEN_EOF)
    {
        write(1, "Language recognized!\n", 21);
        print_ast(expression, 0);
    
        pinapl_scope global_scope = {0};
        b32 good = pinapl_check_scopes(&scope_allocator, expression, &global_scope);
        if (good)
        {
            write(1, "Check is good\n", 14);
        }
        else
        {
            write(1, "Check is bad\n", 13);
        }
    }
    else
    {
        write(1, "Language is not recognized!\n", 28);
        struct string err = pinapl_parser_get_error_string(&parser);
        write(1, err.data, err.size);
    }

    return 0;
}

