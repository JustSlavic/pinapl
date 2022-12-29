#ifndef PARSER_H
#define PARSER_H

#include <base.h>
#include <allocator.h>


typedef b32 predicate(char);


struct pinapl_lexer;
struct pinapl_parser;
struct ast_node;


enum pinapl_token_type
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
};


char *token_type_to_cstring(enum pinapl_token_type);


typedef struct
{
    enum pinapl_token_type type;

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

    AST_NODE_LIST,
    AST_NODE_EMPTY_LIST,
    AST_NODE_GLOBAL_DECLARATION_LIST,
    AST_NODE_STATEMENT_LIST,
    AST_NODE_BLOCK,

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
    usize symbol_id;
    struct ast_node *argument_list;
} ast_node_function_call;


typedef struct ast_node_variable_declaration
{
    token var_name;
    token var_type;
    usize symbol_id;
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
    usize symbol_id;
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
        struct ast_node_list                 list;
        struct ast_node_block                block;
        struct ast_node_function_definition  function_definition;
        struct ast_node_function_call        function_call;
        struct ast_node_variable_declaration variable_declaration;
        struct ast_node_binary_operator      binary_operator;
        struct ast_node_variable             variable;
        struct ast_node_integer_literal      integer_literal;
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

ast_node *pinapl_parse_expression              (struct pinapl_parser *p, int precedence);
ast_node *pinapl_parse_variable_declaration    (struct pinapl_parser *p);
ast_node *pinapl_parse_function_definition     (struct pinapl_parser *p);
ast_node *pinapl_parse_statement               (struct pinapl_parser *p);
ast_node *pinapl_parse_statement_list          (struct pinapl_parser *p);
ast_node *pinapl_parse_global_declaration      (struct pinapl_parser *p);
ast_node *pinapl_parse_global_declaration_list (struct pinapl_parser *p);


struct pinapl_scope_entry
{
    char *entry_name;
    usize entry_name_size;
    ast_node *declaration_node;
    u32   hash;
};


struct pinapl_scope
{
    struct pinapl_scope *parent_scope;
    struct pinapl_scope *next_scope;
    struct pinapl_scope *nested_scope;
    
    struct pinapl_scope_entry hash_table[64];
};


struct pinapl_rename_stage
{
    usize global_variable_counter;
    struct allocator *scope_allocator;
    struct allocator *err_allocator;
};


b32 pinapl_check_and_rename_variables(struct pinapl_rename_stage *stage, ast_node *ast, struct pinapl_scope *scope);


enum pinapl_tac_type
{
    TAC_NOP     = 0x00000001,
    TAC_LABEL   = 0x00000002,
    TAC_MOV     = 0x00000004,
    TAC_ADD     = 0x00000008,
    TAC_SUB     = 0x00000010,
    TAC_MUL     = 0x00000020,
    TAC_DIV     = 0x00000040,
   
    TAC_NO_LHS  = 0x00010000, // 2^16
    TAC_LHS_INT = 0x00020000, // 2^17
    TAC_LHS_REG = 0x00040000, // 2^18

    TAC_NO_RHS  = 0x01000000, // 2^24
    TAC_RHS_INT = 0x02000000, // 2^25
    TAC_RHS_REG = 0x04000000, // 2^26

    TAC_REG     = TAC_LHS_REG | TAC_NO_RHS,
    TAC_INT     = TAC_LHS_INT | TAC_NO_RHS,
    TAC_REG_REG = TAC_LHS_REG | TAC_RHS_REG,
    TAC_REG_INT = TAC_LHS_REG | TAC_RHS_INT,
    TAC_INT_REG = TAC_LHS_INT | TAC_RHS_REG,
};


struct pinapl_tac
{
    u32 type;
    usize dst; // 'register' index OR instruction index of label
    usize lhs; // 'register' index OR integer number
    usize rhs; // 'register' index OR integer number
};


struct pinapl_flatten_stage
{
    usize global_variable_counter;

    struct pinapl_tac *codes;
    usize codes_size;
    usize code_count;

    usize *labels;
    usize labels_size;
    usize label_count;
};


enum flatten_result_type
{
    FLATTEN_RESULT_INVALID = 0,
    FLATTEN_RESULT_INSTRUCTION,
    FLATTEN_RESULT_VARIABLE,
    FLATTEN_RESULT_INTEGER,
};


struct flatten_result
{
    enum flatten_result_type type;
    union
    {
        struct pinapl_tac *instruction;
        int variable_id;
        int integer_value;
    };
};


struct flatten_result pinapl_flatten_ast(struct pinapl_flatten_stage *stage, ast_node *node);


#endif // PARSER_H

