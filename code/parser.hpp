#ifndef PINAPL__PARSER_H
#define PINAPL__PARSER_H

#include <base.h>
#include <console.hpp>
#include <array.hpp>
#include "lexer.hpp"


/*
    Parser todo:

    - Expressions:
        1        (literal ints)
        true     (literal bools)
        ex + ex  (binary operators)
        (ex, ex) (tuples)
        ()       (void analog)

    - Types
        void
        int
        bool
        (int, bool)

    - Declarations:
        x : int = 0;
        x : int : 0;
        x := 0;
        x :: 0;

    - Tuples:
        (x, y) :=
        f (x, 1)
        f :: (x : int, y : int)



    - Tuples in declarations:
        (x, y) := ;

    - Type tuples:
        (x, y) : (int, int) = (1, 2);

    - Tuples of types allow declarations of names for members inside:
        tpl : (x : int, y : int) = (1, 2);
        tpl.x;
        tpl.y;

    - Functions:
        f :: int -> int {}

    - Functions allow names to be assigned to in and out parameters
        f :: (x : int) -> (result : int) {}
                          it just declares variables,
                          you still need to return result

    - Functions allow arguments be default value
        f :: (x : int = 0) {}
        g :: (x := 0) {}

    - Calling functions can be usual way:
        f(1);
    - Or with assigning variables in it's arguments:
        g(x = 1);
*/


namespace pinapl
{


struct ast_node;


enum ast_node_kind
{
    AST_NODE__NONE,

    AST_NODE__VARIABLE,
    AST_NODE__INT_LIT,
    AST_NODE__EXPR_TUPLE,
};

struct ast_node
{
    ast_node_kind kind;
    union
    {
        string_view     m_variable;
        int64           m_int_literal;
        static_array<ast_node *, 4> m_expr_tuple;
    };

    static ast_node make_variable(string_view name);
    static ast_node make_int_literal(int64 n);
    static ast_node make_expr_tuple();
};

struct parser
{
    static_array<ast_node, 32> ast;

    ast_node *parse_variable(lexer *lex);
    ast_node *parse_int_literal(lexer *lex);
    ast_node *parse_expr_tuple(lexer *lex);
    ast_node *parse_expression_operand(lexer *lex);
    ast_node *parse_expression(lexer *lex, int precedence);
};

void debug_print_ast(ast_node *w, int depth = 0);


} // namespace pinapl


#endif // PINAPL__PARSER_H
