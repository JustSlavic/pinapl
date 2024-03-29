#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "backend_c.h"


/*
    todos:

    * scopes
     \__type inference
        \__ type checking
    * translation to C
    * error reporting
    * tightening up the parser
      (correct error handling, all cases handling, and not leaking)
*/


char const source_code[] =
"f :: (x : int, y : int, z : int) -> (result : int, error : bool) {\n"
"    return (1, true);\n"
"}\n"
"g :: () {\n"
"    (result, error) := f(x, x + 3, 700);\n"
"}\n"
;


int main()
{
    usize token_stream_size = 1024;
    usize ast_buffer_size = 1024;

    struct lexer lexer = {
        .buffer = (char const *) source_code,
        .buffer_size = sizeof(source_code),

        .cursor = 0,

        .line = 1,
        .column = 0,

        .token_stream = malloc(sizeof(struct token) * token_stream_size),
        .token_count = 0,
        .token_stream_size = token_stream_size,
    };

    make_token_stream(&lexer);
    debug_print_token_stream(&lexer);

    printf("=============\n");
    printf("%s\n", source_code);
    printf("=============\n");

    struct parser parser = {
        .token_stream = lexer.token_stream,
        .token_count = lexer.token_count,

        .cursor = 0,

        .ast = malloc(sizeof(struct ast_node) * ast_buffer_size),
        .ast_node_count = 0,
        .ast_buffer_size = ast_buffer_size,

        .types.count = 1, // types.entries[0] == TYPE__VOID
    };

    struct ast_node *stmts = parse_statements(&parser);
    debug_print_ast(stmts, 0);

    printf("=============\n");

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
#include "backend_c.c"
#include <string_builder.c>
