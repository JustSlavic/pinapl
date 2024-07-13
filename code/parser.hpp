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

typedef uint32 type_id_t;

enum type_kind
{
    TYPE__TUPLE,     // (), (a, b), (a, b, c), (a, b, c, d)
    TYPE__FUNCTION,
};

struct type
{
    type_kind kind;
    // if unit - all hashes are zero
    // if single - only hash[0] is valid
    // if tuple - all hashes are type_id_t-s pointing to sub-types
    // if function - first is return type, second is argument type
    uint32 hash[4];
    uint32 count; // only for tuples
    string_view name; // only for single types
};

FORCE_INLINE bool operator == (type lhs, type rhs)
{
    if (lhs.kind != rhs.kind) return false;
    for (int i = 0; i < ARRAY_COUNT(lhs.hash); i++)
        if (lhs.hash[i] != rhs.hash[i])
            return false;
    return true;
}

enum ast_node_kind
{
    AST_NODE__NONE,

    AST_NODE__TYPE,
    AST_NODE__VARIABLE,
    AST_NODE__INT_LIT,
};

struct ast_node__type
{
    type_id_t index;
};

struct ast_node__variable
{
    string_view name;
};

struct ast_node__int_literal
{
    int64 n;
};

struct ast_node
{
    ast_node_kind kind;
    union
    {
        ast_node__type         m_type;
        ast_node__variable     m_variable;
        ast_node__int_literal  m_int_lit;
    };

    static ast_node make_type(type_id_t id);
    static ast_node make_variable(string_view name);
    static ast_node make_int_literal(int64 n);
};

struct parser
{
    static_array<ast_node, 32> ast;

    #define TYPES_HASH_TABLE_SIZE 32
    uint32 types_hash_table__hash[TYPES_HASH_TABLE_SIZE];
    type   types_hash_table__type[TYPES_HASH_TABLE_SIZE];

    type_id_t reg_type(type);
    type get_type(type_id_t index);
    type get_type(ast_node *node);

    void push_ast_node(ast_node node);

    ast_node *parse_type(lexer *lex);
    ast_node *parse_variable(lexer *lex);
    ast_node *parse_int_literal(lexer *lex);

    void debug_print_ast();
};



} // namespace pinapl


// struct ast_node;
// typedef struct ast_node ast_node;
// struct type;
// struct type_registry;


// struct parser
// {
//     struct token_stream token_stream;
//     usize token_cursor;

//     ast_node *ast;
//     usize ast_capacity;
//     usize ast_count;

//     struct {
//         usize tok_stream_position;
//         usize ast_buffer_position;
//     } rollback_buffer[32];
//     usize rollback_buffer_count;

//     struct type_registry *type_registry;

//     struct logger *logger;
// };

// ast_node *parse_expression(struct parser *parser, int precedence);
// struct type *parse_type(struct parser *parser);
// struct type *register_type(struct parser *parser, struct type *type1);
// bool32 debug__print_ast(ast_node *ast, int depth);
// void debug__print_type(struct type *type);
// void debug__print_type_registry(struct type_registry *type_registry);


// enum ast_node_kind
// {
//     AST__INVALID = 0,

//     // Expressions
//     AST__VARIABLE,
//     AST__LITERAL_BOOL,
//     AST__LITERAL_INT,
//     AST__BINARY_OPERATOR,
//     AST__TUPLE,
// };
// typedef enum ast_node_kind ast_node_kind;

// struct ast__variable
// {
//     string_view name;
// };

// struct ast__literal_bool
// {
//     bool32 value;
// };

// struct ast__literal_int
// {
//     int64 value;
// };

// struct ast__binary_operator
// {
//     string_view name;
//     ast_node *lhs;
//     ast_node *rhs;
// };

// struct ast__tuple
// {
//     ast_node *expressions[16];
//     usize count;
// };

// struct ast_node
// {
//     ast_node_kind kind;

//     union
//     {
//         struct ast__variable        v;
//         struct ast__literal_bool    b;
//         struct ast__literal_int     i;
//         struct ast__binary_operator bo;
//         struct ast__tuple           t;
//     };
// };

// void ast__set_variable(ast_node *node, string_view name);
// void ast__set_boolean(ast_node *node, bool32 value);
// void ast__set_integer(ast_node *node, int64 value);
// void ast__set_binary_operator(ast_node *node, string_view op, ast_node *lhs, ast_node *rhs);
// void ast_tuple__push(ast_node *node, ast_node *expr);

// string_view ast__get_name(ast_node *node);
// bool32 ast__get_boolean(ast_node *node);
// int64 ast__get_integer(ast_node *node);


// enum type__kind
// {
//     TYPE__INVALID,
//     TYPE__NAME,
//     TYPE__TUPLE,
// };
// typedef enum type__kind type__kind;


// struct type__name
// {
//     string_view name;
// };

// struct type__tuple
// {
//     uint32 count;
//     struct type *types[16];
// };

// struct type
// {
//     type__kind kind;
//     union
//     {
//         struct type__name  n;
//         struct type__tuple t;
//     };
// };

// struct type_registry
// {
//     struct type *types;
//     usize count;
//     usize capacity;
// };


// FORCE_INLINE struct type_registry *make_type_registry(int capacity)
// {
//     struct type_registry *result = malloc(sizeof(struct type_registry));
//     result->types = malloc(sizeof(struct type) * capacity);
//     result->count = 0;
//     result->capacity = capacity;

//     return result;
// }

// struct type *reg_type(struct parser *parser, struct type *type1);



// #if 0
// enum type_entry_kind
// {
//     TYPE__VOID,
//     TYPE__NAME,
//     TYPE__TUPLE,
//     TYPE__FUNCTION,
// };

// struct type_entry;
// typedef struct type_entry type_entry;

// struct type__tuple // ... : (int, int) = ...
// {
//     type_entry *types[16];
//     usize type_count;
// };

// struct type_entry
// {
//     enum type_entry_kind kind;
//     union
//     {
//         struct
//         {
//             string_view type_name;
//         };
//         struct type__tuple tuple;
//         struct
//         {
//             struct type_entry *return_type;
//             struct type_entry *arguments;
//         };
//     };
// };

// typedef struct type_entry type_entry;

// struct type_registry
// {
//     type_entry entries[32];
//     usize      entry_count;
// };

// bool32 type_entries_equal(type_entry *e1, type_entry *e2);
// type_entry *register_type_entry(struct type_registry *registry, type_entry *entry_to_register);


// struct scope
// {
//     string_view variable_names[32];
//     type_entry *variable_types[32];
//     usize       variable_count;
// };

// struct scope_registry
// {
//     struct scope scopes[32];
//     uint32 count;
// };


// enum ast_node_kind
// {
//     AST__INVALID = 0,

//     // Statements
//     AST__STATEMENT,
//     AST__DECLARATION,
//     AST__BLOCK,
//     AST__FUNCTION,
//     AST__RETURN,

//     // Expressions
//     AST__BINARY_OPERATOR,
//     AST__LITERAL_INT,
//     AST__VARIABLE,
//     AST__FUNCTION_CALL,
//     AST__TUPLE,

//     AST__TUPLE_OF_VARIABLES,
//     AST__TUPLE_OF_EXPRESSIONS,
//     AST__TUPLE_OF_ARGUMENTS,
//     AST__TUPLE_OF_RETURNS,
// };

// struct ast_node;
// typedef struct ast_node ast_node;

// struct ast_type_tuple
// {
//     struct ast_node *next;
// };

// struct ast_tuple
// {
//     struct ast_node *values[16];
//     usize value_count;
// };

// struct ast_binary_operator
// {
//     char operator;
//     struct ast_node *lhs;
//     struct ast_node *rhs;
// };

// struct ast_literal_int
// {
//     int64 value;
// };

// struct ast_variable
// {
//     string_view name;
//     type_entry *type;
// };

// struct ast_function_call
// {
//     string_view name;
//     struct ast_node *args[16];
//     uint32 arg_count;
// };

// struct ast_declaration
// {
//     bool32 is_constant;
//     type_entry *decl_type;
//     struct ast_node *lhs;
//     struct ast_node *init;
// };

// struct ast_statement
// {
//     struct ast_node *stmt;
//     struct ast_node *next;
// };

// struct ast_block
// {
//     struct ast_node *statements;
//     struct scope *scope;
// };

// struct ast_function
// {
//     struct ast_node *arguments;
//     struct ast_node *returns;
//     struct ast_node *body;
// };

// struct ast_return
// {
//     struct ast_node *return_expression;
// };

// struct ast__tuple
// {
//     usize count;
//     union
//     {
//         struct
//         {
//             type_entry *type;
//             ast_node *expressions[16];
//         };
//         struct
//         {
//             type_entry *type_; // @note: should be the same as 'type'
//             string_view names[16];
//         };
//         struct
//         {
//             type_entry *arg_types[16];
//             string_view arg_names[16];
//         };
//     };
// };

// struct ast_node
// {
//     enum ast_node_kind kind;
//     union
//     {
//         struct ast_binary_operator binary_operator;
//         struct ast_literal_int     literal_int;
//         struct ast_variable        variable;
//         struct ast_function_call   function_call;
//         struct ast_declaration     declaration;
//         struct ast_statement       statement;
//         struct ast_block           block;
//         struct ast__tuple          tuple;
//         struct ast_function        function;
//         struct ast_return          return_;
//     };
// };

// typedef struct ast_node ast_node;

// struct parser
// {
//     struct token *token_stream;
//     usize token_count;

//     usize cursor;

//     struct ast_node *ast;
//     usize ast_node_count;
//     usize ast_buffer_size;

//     struct {
//         usize tok_stream_position;
//         usize ast_buffer_position;
//     } rollback_buffer[32];
//     usize rollback_buffer_count;

//     struct scope *global_scope;

//     struct type_registry types;
//     struct scope_registry scopes;
// };

// void parser__save_position(struct parser *parser);
// void parser__rollback(struct parser *parser);

// struct ast_node *parse_statements(struct parser *p, struct scope *scope);

// void debug_print_type(struct type_entry *type);
// bool32 debug_print_ast(struct ast_node *ast, int depth);

// type_entry *parse_tuple_of_types(struct parser *parser);
// ast_node *parse_tuple_of_variables(struct parser *parser);
// ast_node *parse_tuple_of_expressions(struct parser *parser);
// ast_node *parse_function_arguments(struct parser *parser);
// #endif

#endif // PINAPL__PARSER_H
