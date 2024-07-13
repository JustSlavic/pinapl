#include <base.h>

#include <stdio.h>
#include <stdlib.h>

#include "lexer.hpp"
#include "parser.hpp"

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
"((), (bool, int), int, int)"
;

// (int, bool) // functions and everywhere else
// (x : int, y : bool) // only in functions

int main()
{
    printf("Source:\n%s\n", source_code);

    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));

    pinapl::parser parser = {};

    parser.parse_type(&lex);

    printf("\nAST:\n");
    parser.debug_print_ast();
    printf("\n");

    // printf("Token stream:\n");
    // while (true)
    // {
    //     auto t = lex.eat_token();
    //     printf("%s\n", pinapl::token_kind_to_cstr(t.kind));
    //     if (t.kind == pinapl::TOKEN_EOF) break;
    // }

    return 0;
}

#include "lexer.cpp"
#include "parser.cpp"
#include <lexer.cpp>

#include <util.cpp>
