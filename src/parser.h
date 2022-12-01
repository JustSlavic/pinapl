#ifndef LEXER_H
#define LEXER_H

#include <base.h>
#include <allocator.h>


typedef b32 predicate(char);


typedef enum
{
    TOKEN_INVALID = 0,

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_EQUALS = '=',
    TOKEN_COLON = ':',
    TOKEN_SEMICOLON = ';',
    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_ASTERICS = '*',
    TOKEN_SLASH = '/',

    TOKEN_IDENTIFIER = 256,
    TOKEN_LITERAL_INT = 257,

    TOKEN_DOUBLE_COLON = 270,
    TOKEN_ARROW_RIGHT = 271,
    
    TOKEN_KW_RETURN = 300,
    
    TOKEN_EOF = 500,
} token_type;


char *token_type_to_cstring(token_type);


typedef struct
{
    token_type type;

    int line;
    int column;

    int integer_value;

    char *span;
    int span_size;
} token;


typedef struct
{
    char *buffer;
    usize buffer_size;

    usize index;

    usize line;
    usize column;

    token next_token;
    b32 next_token_valid;
} lexer;


b32 is_ascii_space(char c);
b32 is_ascii_whitespace(char c);
b32 is_ascii_alpha(char c);
b32 is_ascii_digit(char c);
b32 is_valid_identifier_head(char c);
b32 is_valid_identifier_body(char c);

char lexer_get_char(lexer *);
char lexer_eat_char(lexer *);
void consume_while(lexer *, predicate *);
void consume_until(lexer *, predicate *);

token lexer_get_token(lexer *);
token lexer_eat_token(lexer *);


typedef enum ast_node_type
{
    AST_NODE_INVALID = 0,
    AST_NODE_PROCEDURE,

    AST_NODE_STATEMENT,

    AST_NODE_VARIABLE,
    AST_NODE_LITERAL_INT,

    // Expressions
    AST_NODE_BINARY_OPERATOR,
} ast_node_type;


typedef struct ast_node
{
    ast_node_type type;
    token t;

    union
    {
        struct  // function
        {
            struct ast_node *statements;
        };
        struct  // statement
        {
            struct ast_node *expression;
            struct ast_node *next_statement;
        };
        struct  // binary operation
        {
            struct ast_node *lhs;
            struct ast_node *rhs;
        };
        struct  // variable
        {
            char *span;
            usize span_size;
        };
        struct  // integer literal
        {
            int integer_value;
        };
    };
} ast_node;


ast_node *parser_parse_expression(allocator *a, lexer *l, int precedence);


#endif // LEXER_H
