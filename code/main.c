#include <stdio.h>
#include <stdlib.h>

#include <base.h>
#include <memory_allocator.h>
#include <logger.h>

#include "lexer.h"
#include "parser.h"


/*
    todos:

    type_tuple {
        type_entry *type_names[16];
    };

    ast_tuple {
        string_view names[16];
        type_entry *type;
    }

    * three types of tuples in the language
     \__ type tuples                         f :: () -> (int, int) {};
        \__ variable to declare tuples       (x, y) := f();
           \__ typed variables to declare    f :: (x : int, y : int) {}
              \__
                 * scopes
                  \__ type inference
                     \__ type checking
    * translation to C
    * error reporting
    * tightening up the parser
      (correct error handling, all cases handling, and not leaking)
    * placeholder _, and all the places where it's allowed
      - return values from function: (_, x) := foo(); discards first value in tuple
*/


char const source_code[] =
"(bool, int)"
;


int main()
{
    usize memory_size_for_logger = MEGABYTES(1);
    usize token_stream_capacity = 1024;
    usize ast_buffer_size = 1024;

    memory_block memory_for_logger = {
        .memory = malloc(memory_size_for_logger),
        .size = memory_size_for_logger,
    };

    struct logger logger = {
        .sb = {
            .memory = memory_for_logger,
            .used = 0,
        }
    };

    struct lexer lexer = {
        .buffer = (char const *) source_code,
        .buffer_size = sizeof(source_code),

        .cursor = 0,

        .line = 1,
        .column = 0,

        .stream = {
            .tokens = malloc(sizeof(struct token) * token_stream_capacity),
            .count = 0,
            .capacity = token_stream_capacity,
        },

        .logger = &logger,
    };

    lexer__tokenize(&lexer);
    debug_print_token_stream(&lexer);

    printf("=============\n");

    logger__flush_file(&logger, 1);

    struct parser parser = {
        .token_stream = lexer.stream,
        .token_cursor = 0,

        .ast = malloc(sizeof(ast_node) * ast_buffer_size),
        .ast_count = 0,
        .ast_capacity = ast_buffer_size,

        .rollback_buffer_count = 0,

        .type_registry = make_type_registry(32),

        
        .logger = &logger,
    };

    // ast_node *ast = parse_expression(&parser, 0);
    // debug__print_ast(ast, 0);

    struct type *t = parse_type(&parser);
    debug__print_type(t);
    printf("=============\n");
    debug__print_type_registry(parser.type_registry);

#if 0

    printf("=============\n");
    printf("%s\n", source_code);
    printf("=============\n");


        .types.entry_count = 1, // types.entries[0] == TYPE__VOID

        .scopes.count = 1,

        .global_scope = parser.scopes.scopes,

    struct ast_node *stmts = parse_statements(&parser, parser.global_scope);
    debug_print_ast(stmts, 0);

    // type_entry *tuple_of_types = parse_tuple_of_types(&parser);
    // debug_print_type(tuple_of_types);

    // ast_node *tpl = parse_tuple_of_variables(&parser);
    // ast_node *tpl = parse_tuple_of_expressions(&parser);
    // ast_node *tpl = parse_tuple_of_declarations(&parser);

    // debug_print_ast(tpl, 0);

    printf("=============\n");
#endif
#if 0
    usize sb_size = MEGABYTES(1);
    struct string_builder sb = {
        .memory.memory = malloc(sb_size),
        .memory.size = sb_size,
        .used = 0,
    };

    struct translator_to_c translator = {
        .output = &sb,
    };

    translator_c__predeclare_types(&translator, &parser.types);
    translate_to_c(&translator, stmts, 0);

    struct memory_block str = string_builder__get_string(&sb);
    printf("%.*s\n", (int) str.size, str.memory);
#endif

    return 0;
}


#include "lexer.c"
#include "parser.c"
#include <string_builder.c>
#include <logger.c>
