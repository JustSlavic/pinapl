#ifndef PINAPL__PARSER_H
#define PINAPL__PARSER_H

#include <base.h>
#include "lexer.h"


enum ast_node_kind
{
    AST__INVALID = 0,

    // Statements
    AST__STATEMENT,
    AST__DECLARATION,
    AST__BLOCK,
    // AST__FUNCTION_DEFINITION,
    // AST__RETURN_STATEMENT,

    // Expressions
    AST__BINARY_OPERATOR,
    AST__LITERAL_INT,
    AST__VARIABLE,
    AST__FUNCTION_CALL,
    AST__TUPLE,
    AST__TYPE,
    AST__TYPE_TUPLE,
};

struct ast_node;

struct ast_type_tuple
{
    struct ast_node *type;
    struct ast_node *next;
};

struct ast_tuple
{
    struct ast_node *value;
    struct ast_node *next;
};

struct ast_type
{
    struct string_view name;
};

struct ast_binary_operator
{
    char operator;
    struct ast_node *lhs;
    struct ast_node *rhs;
};

struct ast_literal_int
{
    int64 value;
};

struct ast_variable
{
    struct string_view name;
};

struct ast_function_call
{
    struct string_view name;
    struct ast_node *arg1;
};

struct ast_declaration
{
    bool32 is_constant;
    struct string_view name;
    struct ast_node *type;
    struct ast_node *init;
};

struct ast_statement
{
    struct ast_node *stmt;
    struct ast_node *next;
};

struct ast_block
{
    struct ast_node *statements;
};

struct ast_node
{
    enum ast_node_kind kind;
    union
    {
        struct ast_binary_operator binary_operator;
        struct ast_literal_int     literal_int;
        struct ast_variable        variable;
        struct ast_function_call   function_call;
        struct ast_declaration     declaration;
        struct ast_statement       statement;
        struct ast_block           block;
        struct ast_type            type;
        struct ast_type_tuple      type_tuple;
        struct ast_tuple           tuple;
    };
};


struct parser
{
    struct token *token_stream;
    usize token_count;

    usize cursor;

    struct ast_node *ast;
    usize ast_node_count;
    usize ast_buffer_size;
};


struct ast_node *parse_statements              (struct parser *p);

bool32 debug_print_ast(struct ast_node *ast, int depth);



#endif // PINAPL__PARSER_H
