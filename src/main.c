#include <syscall.h>
#include <allocator.h>
#include <print.h>
#include <string.h>
#include <parser.h>
#include <string_id.h>


char const *spaces = "                                           ";

void print_ast(ast_node *node, int depth)
{
    switch (node->type)
    {
        case AST_NODE_EMPTY_LIST:
        break;

        case AST_NODE_BINARY_OPERATOR:
        {
            ast_node_binary_operator *binary_op = &node->binary_operator;
            if (binary_op->lhs) print_ast(binary_op->lhs, depth);
            if (binary_op->rhs) print_ast(binary_op->rhs, depth);
            print_n(binary_op->op.span, binary_op->op.span_size); 
        }
        break;

        case AST_NODE_VARIABLE:
        {
            print_n(node->variable.span, node->variable.span_size);
            print("(%d)", node->variable.symbol_id);
        }
        break;

        case AST_NODE_LITERAL_INT:
        {
            print_n(node->integer_literal.span, node->integer_literal.span_size);
        }
        break;
        
        case AST_NODE_VARIABLE_DECLARATION:
        case AST_NODE_CONSTANT_DECLARATION:
        {
            b32 is_constant = (node->type == AST_NODE_CONSTANT_DECLARATION);
            ast_node_variable_declaration *var = &node->variable_declaration;

            print("VAR(%d) {", var->symbol_id);
            print_string(get_string_by_id(var->name));
            print(":");
            if (var->var_type.type != TOKEN_INVALID)
            {
                print_n(var->var_type.span, var->var_type.span_size);
            }
            if (var->init)
            {
                print(is_constant ? ":" : "=");
                print_ast(var->init, depth);
            }
            print("}");
        }
        break;

        case AST_NODE_BLOCK:
        {
            print("{\n");
            if (node->block.statement_list)
            {
                print_ast(node->block.statement_list, depth + 1);
            }
            print_n(spaces, depth * 2);
            print("}\n");
        }
        break;

        case AST_NODE_FUNCTION_DEFINITION:
        {
            print("()");
            if (node->function_definition.block)
            {
                print("\n");
                print_n(spaces, (depth + 1) * 2);
                print_ast(node->function_definition.block, depth + 1);
            }
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        {
            print_n(node->function_call.name.span, node->function_call.name.span_size);
            print("()");
        }
        break;

        case AST_NODE_STATEMENT_LIST:
        {
            print_n(spaces, depth * 2);
            print_ast(node->list.node, depth);
            if (node->list.node->type != AST_NODE_BLOCK)
            {
                print(";");
            }
            print("\n");
            if (node->list.next)
            {
                print_ast(node->list.next, depth);
            }
        }
        break;

        case AST_NODE_GLOBAL_DECLARATION_LIST:
        {
            print_ast(node->list.node, depth);
            print("\n");
            if (node->list.next)
            {
                print_ast(node->list.next, depth);
            }
        }
        break;

        case AST_NODE_RETURN_STATEMENT:
        {
            print("return ");
            print_ast(node->return_statement.expression, depth);
        }
        break;

        default:
        {
            print("<!!!>");
        }
    }
}


void print_tacs(struct pinapl_tac *codes, usize code_count)
{
    usize code_index = 0;
    while (code_index < code_count)
    {
        struct pinapl_tac *code = codes + code_index++;
        
        if (code->type & TAC_NOP)
        {
            print("    nop");
        }
        else if (code->type & TAC_LABEL)
        {
            print_string(get_string_by_id(code->label));
            print(":\n");
        }
        else if (code->type & TAC_MOV)
        {
            print("    mov  r%d", code->dst);
        }
        else if (code->type & TAC_ADD)
        {
            print("    add  r%d", code->dst);
        }
        else if (code->type & TAC_SUB)
        {
            print("    sub  r%d", code->dst);
        }
        else if (code->type & TAC_MUL)
        {
            print("    mul  r%d", code->dst);
        }
        else if (code->type & TAC_DIV)
        {
            print("    div  r%d", code->dst);
        }
        else if (code->type & TAC_RET)
        {
            print("    ret  r%d", code->dst);
        }
        else
        {
            ASSERT_FAIL();
        }

        if (code->type & TAC_LHS_REG)
        {
            print(", r%d", code->lhs);
        }
        else if (code->type & TAC_LHS_INT)
        {
            print(", #%d", code->lhs);
        }
        
        if (code->type & TAC_RHS_REG)
        {
            print(", r%d", code->rhs);
        }
        else if (code->type & TAC_RHS_INT)
        {
            print(", #%d", code->rhs);
        }
        
        print("\n");
    }
}


int main(int argc, char **argv, char **env)
{
    struct memory_block memory_buffer = allocate_pages(MEGABYTES(5));

    struct allocator arenas;
    initialize_memory_arena(&arenas, memory_buffer.memory, memory_buffer.size);

    usize memory_for_print_buffer_size = KILOBYTES(5);
    void *memory_for_print_buffer = ALLOCATE_BUFFER_(&arenas, memory_for_print_buffer_size);
    initialize_print_buffer(memory_for_print_buffer, memory_for_print_buffer_size);

    usize memory_for_string_id_buffer_size = KILOBYTES(5);
    void *memory_for_string_id_buffer = ALLOCATE_BUFFER_(&arenas, memory_for_string_id_buffer_size);
    initialize_string_id_storage(memory_for_string_id_buffer, memory_for_string_id_buffer_size);

    if (argc < 2)
    {
        print("Usage: ./pinapl FILEPATH\n");
        return 1;
    }

    struct allocator ast_allocator;
    {
        usize memory_for_ast_size = MEGABYTES(1);
        void *memory_for_ast = ALLOCATE_BUFFER_(&arenas, memory_for_ast_size);
        initialize_memory_arena(&ast_allocator, memory_for_ast, memory_for_ast_size);
    }

    struct allocator scope_allocator;
    {
        usize memory_for_scopes_size = MEGABYTES(1);
        void *memory_for_scopes = ALLOCATE_BUFFER_(&arenas, memory_for_scopes_size); 
        initialize_memory_arena(&scope_allocator, memory_for_scopes, memory_for_scopes_size);
    }

    usize memory_for_three_address_codes_size = MEGABYTES(1);
    void *memory_for_three_address_codes = ALLOCATE_BUFFER_(&arenas, memory_for_three_address_codes_size);

    usize memory_for_labels_size = KILOBYTES(5);
    void *memory_for_labels = ALLOCATE_BUFFER_(&arenas, memory_for_labels_size);

    struct allocator err_allocator;
    {
        usize memory_for_err_size = KILOBYTES(5);
        void *memory_for_err = ALLOCATE_BUFFER_(&arenas, memory_for_err_size);
        initialize_memory_arena(&err_allocator, memory_for_err, memory_for_err_size);
    }

    struct allocator elf_allocator;
    {
        usize memory_for_elf_allocator_size = MEGABYTES(1);
        void *memory_for_elf_allocator = ALLOCATE_BUFFER_(&arenas, memory_for_elf_allocator_size);
        initialize_memory_arena(&elf_allocator, memory_for_elf_allocator, memory_for_elf_allocator_size);
    }

    char const *filename = argv[1];

    int fd = open(filename, O_RDONLY, 0);
    if (fd <= 0)
    {
        print("Error < 0\n");
        return 1;
    }

    char buffer[4096] = {0};
    int  buffer_size = read(fd, buffer, ARRAY_COUNT(buffer) - 1);
    close(fd);

    // print_n(buffer, buffer_size);
    // print("EOF\n\n");

    struct pinapl_parser parser = pinapl_init_parser(&ast_allocator, &err_allocator, filename, buffer, buffer_size);
    ast_node *ast = pinapl_parse_global_declaration_list(&parser);

    token t = pinapl_get_token(&parser);
    if (ast && t.type == TOKEN_EOF)
    {
        struct pinapl_scope global_scope = {0};

        struct pinapl_rename_stage rename_stage =
        {
            .global_variable_counter = 0,
            .scope_allocator = &scope_allocator,
            .err_allocator = &err_allocator,
        };

        b32 good = pinapl_check_and_rename_variables(&rename_stage, ast, &global_scope);
        if (good)
        {
            // print("Check is good\n");
            // print_ast(ast, 0);

            struct pinapl_flatten_stage flatten_stage =
            {
                .global_variable_counter = rename_stage.global_variable_counter,

                .codes = memory_for_three_address_codes,
                .codes_size = memory_for_three_address_codes_size / sizeof(struct pinapl_tac),
                .code_count = 0,

                .labels = memory_for_labels,
                .labels_size = memory_for_labels_size,
                .label_count = 0,
            };

            pinapl_flatten_ast(&flatten_stage, ast);

            // print_tacs(flatten_stage.codes, flatten_stage.code_count);

            struct pinapl_liveness_table liveness = pinapl_make_liveness_table(&ast_allocator, &flatten_stage);
            // pinapl_print_liveness_table(&liveness);

            struct pinapl_connectivity_graph graph = pinapl_make_connectivity_graph(&liveness);
            // pinapl_print_connectivity_graph(&graph);

            struct pinapl_instruction_stream stream = pinapl_make_instruction_stream(&ast_allocator);
 
            pinapl_arm_push_section(&stream, STRID(".text"));
            pinapl_arm_push_global(&stream, STRID("_start"));
           
            pinapl_arm_push_instructions_from_flatten_stage(&stream, &flatten_stage, &graph);

            pinapl_arm_push_label(&stream, STRID("_start"));
            pinapl_arm_push_rd(&stream, ARM_LDR, ARM_R0, ARM_SP);
            pinapl_arm_push_rri(&stream, ARM_ADD, ARM_R3, ARM_SP, 4);
            pinapl_arm_push_rrrr(&stream, ARM_MLA, ARM_R2, ARM_R0, ARM_R3, ARM_R1);
            pinapl_arm_push_ri(&stream, ARM_ADD, ARM_R2, 4);
            pinapl_arm_push_l(&stream, ARM_BL, STRID("main"));
            pinapl_arm_push_label(&stream, STRID("_exit"));
            // pinapl_arm_push_ri(&stream, ARM_MOVS, ARM_R0, 0);
            pinapl_arm_push_ri(&stream, ARM_MOVS, ARM_R7, 1);
            pinapl_arm_push_i(&stream, ARM_SVC, 0x0);

            pinapl_arm_print_instruction_stream(&stream);

            pinapl_arm_dump_elf("a.out", &stream, &elf_allocator);
        }
        else
        {
            print("Check is bad\n");
        }
    }
    else
    {
        print("Language is not recognized!\n");
        struct string err = pinapl_parser_get_error_string(&parser);
        print_n(err.data, err.size);
    }

    print_flush();
    return 0;
}

