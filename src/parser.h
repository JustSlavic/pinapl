#ifndef LEXER_H
#define LEXER_H

#include <base.h>
#include <allocator.h>


typedef b32 predicate(char);


struct pinapl_lexer;
struct pinapl_parser;
struct ast_node;


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


struct pinapl_line_info
{
    usize start_index_in_buffer;
    usize length;
};


struct pinapl_lexer
{
    char *buffer;
    usize buffer_size;

    usize index;

    usize line;
    usize column;

    token next_token;
    b32 next_token_valid;

    // @todo: figure out better solution
    struct pinapl_line_info lines[4096]; // Did you ever see a programm more than 4096 lines? Me neither. 
    usize lines_count;
};


b32 is_ascii_space(char c);
b32 is_ascii_whitespace(char c);
b32 is_ascii_alpha(char c);
b32 is_ascii_digit(char c);
b32 is_valid_identifier_head(char c);
b32 is_valid_identifier_body(char c);

struct pinapl_parser pinapl_init_parser(struct allocator *ast_allocator, struct allocator *err_allocator, char const *filename, char *buffer, usize size);
char pinapl_get_char_at(struct pinapl_lexer *, usize);
char pinapl_get_char(struct pinapl_lexer *);
char pinapl_eat_char(struct pinapl_lexer *);
void pinapl_consume_while(struct pinapl_lexer *, predicate *);
void pinapl_consume_until(struct pinapl_lexer *, predicate *);

token pinapl_get_token(struct pinapl_parser *);
token pinapl_eat_token(struct pinapl_parser *);


typedef enum ast_node_type
{
    AST_NODE_INVALID = 0,

    AST_NODE_EMPTY_LIST,
    AST_NODE_GLOBAL_DECLARATION_LIST,
    AST_NODE_BLOCK,
    AST_NODE_STATEMENT_LIST,

    // Statements
    AST_NODE_VARIABLE_DECLARATION,
    AST_NODE_CONSTANT_DECLARATION,
    AST_NODE_FUNCTION_DEFINITION,

    // Expressions
    AST_NODE_BINARY_OPERATOR,
    AST_NODE_LITERAL_INT,
    AST_NODE_VARIABLE,
    AST_NODE_FUNCTION_CALL,

} ast_node_type;


typedef struct ast_node_list
{
    struct ast_node *node;
    struct ast_node *next;
} ast_node_list;


typedef struct ast_node_block
{
    struct ast_node *statement_list;
} ast_node_block;


typedef struct ast_node_function_definition
{
    struct ast_node *parameter_list;
    struct ast_node *return_type;
    struct ast_node *block;
} ast_node_function_definition;


typedef struct ast_node_function_call
{
    token name;
    struct ast_node *argument_list;
} ast_node_function_call;


typedef struct ast_node_variable_declaration
{
    token var_name;
    token var_type;
    // for compound types:
    // struct ast_node *var_type;
    struct ast_node *init;
} ast_node_variable_declaration;


typedef struct ast_node_binary_operator
{
    token op;
    struct ast_node *lhs;
    struct ast_node *rhs;
} ast_node_binary_operator;

struct ast_node_variable
{
    char *span;
    usize span_size;
};

struct ast_node_integer_literal
{
    char *span;
    usize span_size;
    int integer_value;
};


typedef struct ast_node
{
    ast_node_type type;

    union
    {
        struct ast_node_list global_list;
        struct ast_node_block block;
        struct ast_node_list statement_list;
        struct ast_node_function_definition function_definition;
        struct ast_node_function_call function_call;
        struct ast_node_variable_declaration variable_declaration;
        struct ast_node_binary_operator binary_operator;
        struct ast_node_variable variable;
        struct ast_node_integer_literal integer_literal;
    };
} ast_node;


struct pinapl_parser
{
    char const *filename;
    struct allocator *ast_allocator;
    struct allocator *err_allocator;
    struct pinapl_lexer lexer;
};


struct string pinapl_parser_get_error_string(struct pinapl_parser *p);

ast_node *pinapl_parse_expression(struct pinapl_parser *p, int precedence);
ast_node *pinapl_parse_variable_declaration(struct pinapl_parser *p);
ast_node *pinapl_parse_function_definition(struct pinapl_parser *p);
ast_node *pinapl_parse_statement(struct pinapl_parser *p);
ast_node *pinapl_parse_statement_list(struct pinapl_parser *p);
ast_node *pinapl_parse_global_declaration(struct pinapl_parser *p);
ast_node *pinapl_parse_global_declaration_list(struct pinapl_parser *p);


struct pinapl_rename_stage
{
    usize global_variable_counter;
};


usize pinapl_allocate_new_variable(struct pinapl_rename_stage *stage);
void pinapl_enumerate_variables(struct pinapl_rename_stage *rename_stage, struct ast_node *ast);


#endif // LEXER_H

