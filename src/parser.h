#ifndef PARSER_H
#define PARSER_H

#include <base.h>
#include <allocator.h>
#include <string_id.h>


typedef bool32 predicate(char);


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
    bool32 next_token_valid;

    // @todo: figure out better solution
    struct pinapl_line_info lines[4096]; // Did you ever see a programm more than 4096 lines? Me neither. 
    usize lines_count;
};


bool32 is_ascii_space(char c);
bool32 is_ascii_whitespace(char c);
bool32 is_ascii_alpha(char c);
bool32 is_ascii_digit(char c);
bool32 is_valid_identifier_head(char c);
bool32 is_valid_identifier_body(char c);

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
    AST_NODE_RETURN_STATEMENT,

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
    struct string_id name;
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

struct ast_node_return_statement
{
    struct ast_node *expression;
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
        struct ast_node_return_statement     return_statement;
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
    uint32   hash;
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


bool32 pinapl_check_and_rename_variables(struct pinapl_rename_stage *stage, ast_node *ast, struct pinapl_scope *scope);


enum pinapl_tac_type
{
    TAC_NOP     = 0x00000001,
    TAC_LABEL   = 0x00000002,
    TAC_MOV     = 0x00000004,
    TAC_ADD     = 0x00000008,
    TAC_SUB     = 0x00000010,
    TAC_MUL     = 0x00000020,
    TAC_DIV     = 0x00000040,
    TAC_RET     = 0x00000080,

    TAC_INSTR_MASK = 0x00001111, 
    TAC_LHS_MASK   = 0x00110000,
    TAC_RHS_MASK   = 0x11000000,

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
    uint32 type;
    uint32 dst; // 'register' index OR instruction index of label
    union
    {
        struct
        {
            uint32 lhs; // 'register' index OR integer number
            uint32 rhs; // 'register' index OR integer number
        };
        struct string_id label;
    };
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
    FLATTEN_RESULT_LABEL,
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

struct pinapl_liveness_table
{
    struct allocator *allocator;
    int *table;
    int table_width;
    int table_length;
};

struct pinapl_liveness_table pinapl_make_liveness_table(struct allocator *allocator, struct pinapl_flatten_stage *flatten);
void pinapl_print_liveness_table(struct pinapl_liveness_table *table);

struct pinapl_graph_edge
{
    int from;
    int to;
};

struct pinapl_connectivity_graph
{
    int colors[32];
    struct pinapl_graph_edge edges[64];
    int edge_count;
};

struct pinapl_connectivity_graph pinapl_make_connectivity_graph(struct pinapl_liveness_table *liveness);
void pinapl_print_connectivity_graph(struct pinapl_connectivity_graph *graph);

enum pinapl_arm_instruction
{
    ARM_INVALID_INSTRUCTION,

    ARM_INSTRUCTION_MASK = 0x00ffffff,
    ARM_SUFFIX_MASK      = 0x7f000000,
    ARM_S  = 0x80000000,  // Set condition codes
    
    ARM_LABEL    = 0x1,
    ARM_SECTION  = 0x2,
    ARM_GLOBAL   = 0x3,

    ARM_NOP      = 0x4,
    ARM_MOV,
    ARM_ADD,
    ARM_SUB,
    ARM_MUL,
    ARM_DIV,
    ARM_B,
    ARM_BX,
    ARM_BL,
    ARM_LDR,
    ARM_STR,
    ARM_MLA,
    ARM_SVC,

    ARM_EQ = 0x01000000,  // Equal
    ARM_NE = 0x02000000,  // Not equal
    ARM_CS = 0x03000000,  // Carry set (identical to HS)
    ARM_HS = 0x04000000,  // Unsigned higher or same (identical to CS)
    ARM_CC = 0x05000000,  // Carry clear (identical to LO)
    ARM_LO = 0x06000000,  // Unsigned lower (identical to CC)
    ARM_MI = 0x07000000,  // Minus or negative result
    ARM_PL = 0x08000000,  // Positive or zero result
    ARM_VS = 0x09000000, // Overflow
    ARM_VC = 0x0a000000, // No overflow
    ARM_HI = 0x0b000000, // Unsigned higher
    ARM_LS = 0x0c000000, // Unsigned lower or same
    ARM_GE = 0x0d000000, // Signed greater than or equal
    ARM_LT = 0x0e000000, // Signed less than
    ARM_GT = 0x0f000000, // Signed greater than
    ARM_LE = 0x10000000, // Signed less than or equal
    ARM_AL = 0x11000000, // Always (this is the default)

    ARM_MOVS = ARM_MOV | ARM_S,
};

enum pinapl_arm_instruction_operand_type
{
    ARM_OPERAND_NONE,
    ARM_OPERAND_REGISTER,
    ARM_OPERAND_IMMEDIATE_VALUE,
    ARM_OPERAND_DEREFERENCE,
    ARM_OPERAND_LABEL,
    ARM_OPERAND_DECLARATION,
};

enum pinapl_arm_register
{
    ARM_R0    = 0,
    ARM_R1    = 1,
    ARM_R2    = 2,
    ARM_R3    = 3,
    ARM_R4    = 4,
    ARM_R5    = 5,
    ARM_R6    = 6,
    ARM_R7    = 7,
    ARM_R8    = 8,
    ARM_R9    = 9,
    ARM_R10   = 10,
    ARM_R11   = 11,
    ARM_R12   = 12,
    ARM_SP    = 13,
    ARM_LR    = 14,
    ARM_PC    = 15,
    ARM_CPSR  = 16,
    ARM_FPSCR = 17,

    ARM_REGISTER_COUNT = 18,
};

struct pinapl_arm_instruction_operand
{
    uint32 type;
    union
    {
        uint32 value;
        struct string_id label;
        struct string_id strid;
    };
};


// @todo: reduce the size of the intruction by implementing the variable size entry contiguous array/allocator
struct pinapl_instruction
{
    // 1  bit  condition code
    // 7  bits suffix
    // 24 bits     
    uint32 arm;
    struct pinapl_arm_instruction_operand op1;
    struct pinapl_arm_instruction_operand op2;
    struct pinapl_arm_instruction_operand op3;
    struct pinapl_arm_instruction_operand op4;
};

struct pinapl_instruction_stream
{
    struct allocator *allocator;
    struct pinapl_instruction *instructions;
    usize instruction_count;
    usize instruction_capacity;

    // @todo: hash table with string_id as an index/hash value, and
    // values correspond to the index of the instruction which the
    // label is pointing to
    struct string_id labels[32];
    uint32 label_instruction_index[32];
    uint32 label_count;
};

struct pinapl_instruction_stream pinapl_make_instruction_stream(struct allocator *allocator);

void pinapl_arm_push_instruction(struct pinapl_instruction_stream *stream, struct pinapl_instruction instruction);
void pinapl_arm_push_instructions_from_flatten_stage(struct pinapl_instruction_stream *stream, struct pinapl_flatten_stage *flatten, struct pinapl_connectivity_graph *graph);

void pinapl_arm_push_section(struct pinapl_instruction_stream *stream,
                             struct string_id section);
void pinapl_arm_push_global(struct pinapl_instruction_stream *stream,
                            struct string_id decl);
void pinapl_arm_push_label(struct pinapl_instruction_stream *stream, 
                           struct string_id label);
void pinapl_arm_push_l    (struct pinapl_instruction_stream *stream,
                           enum pinapl_arm_instruction instruction,
                           struct string_id label);
void pinapl_arm_push_r    (struct pinapl_instruction_stream *stream,
                           uint32 instruction,
                           enum pinapl_arm_register arg);
void pinapl_arm_push_i    (struct pinapl_instruction_stream *stream,
                           enum pinapl_arm_instruction instruction,
                           int immediate_value);
void pinapl_arm_push_ri   (struct pinapl_instruction_stream *stream, 
                           enum pinapl_arm_instruction instruction,
                           enum pinapl_arm_register dst,
                           int immediate_value);
void pinapl_arm_push_rr   (struct pinapl_instruction_stream *stream,
                           uint32 instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register src);
void pinapl_arm_push_rd   (struct pinapl_instruction_stream *stream,
                           uint32 instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register lhs);
void pinapl_arm_push_rri  (struct pinapl_instruction_stream *stream,
                           uint32 instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register lhs,
                           int immediate_value);
void pinapl_arm_push_rrrr (struct pinapl_instruction_stream *stream,
                           uint32 instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register s1,
                           enum pinapl_arm_register s2,
                           enum pinapl_arm_register s3);

void pinapl_arm_print_instruction_stream(struct pinapl_instruction_stream *stream);
void pinapl_arm_print_entry_point(void);

void pinapl_arm_dump_elf(char const *filename, struct pinapl_instruction_stream *stream, struct allocator *allocator);

#endif // PARSER_H

