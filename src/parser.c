#include "parser.h"
#include <string>
#include <primes.h>
#include <print.h>
#include <elf.h>
#include <syscall.h>


GLOBAL char const *spaces = "                                             ";
GLOBAL char const *carets = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
GLOBAL char const *keywords[] =
{
    "return",
};
GLOBAL enum pinapl_token_type keyword_types[] =
{
    TOKEN_KW_RETURN,
};


char *token_type_to_cstring(enum pinapl_token_type t)
{
    switch (t)
    {
        case TOKEN_INVALID: return "TOKEN_INVALID";
        case TOKEN_PAREN_OPEN: return "TOKEN_PAREN_OPEN";
        case TOKEN_PAREN_CLOSE: return "TOKEN_PAREN_CLOSE";
        case TOKEN_BRACKET_OPEN: return "TOKEN_BRACKET_OPEN";
        case TOKEN_BRACKET_CLOSE: return "TOKEN_BRACKET_CLOSE";
        case TOKEN_BRACE_OPEN: return "TOKEN_BRACE_OPEN";
        case TOKEN_BRACE_CLOSE: return "TOKEN_BRACE_CLOSE";
        case TOKEN_EQUALS: return "TOKEN_EQUALS";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";
        case TOKEN_ARROW_RIGHT: return "TOKEN_ARROW_RIGHT";
        case TOKEN_KW_RETURN: return "TOKEN_KW_RETURN";
        case TOKEN_EOF: return "TOKEN_EOF";
        
        default:
            return "<token_type>";
    }
    return 0;
}

b32 is_ascii_space(char c)
{
    return (c == ' ');
}

b32 is_ascii_whitespace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

b32 is_ascii_alpha(char c)
{
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

b32 is_ascii_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

b32 is_valid_identifier_head(char c)
{
    return (c == '_') || is_ascii_alpha(c);
}

b32 is_valid_identifier_body(char c)
{
    return (c == '_') || is_ascii_alpha(c) || is_ascii_digit(c);
}


struct pinapl_lexer pinapl_init_lexer_with_buffer(char *buffer, usize size)
{
    struct pinapl_lexer result = {0};
    result.buffer = buffer;
    result.buffer_size = size;
    result.line = 1;
    result.column = 1;

    return result;
}

struct pinapl_parser pinapl_init_parser(struct allocator *ast_allocator, struct allocator *err_allocator, char const *filename, char *buffer, usize size)
{
    struct pinapl_parser result = {0};
    result.filename = filename;
    result.lexer = pinapl_init_lexer_with_buffer(buffer, size);
    result.ast_allocator = ast_allocator;
    result.err_allocator = err_allocator;

    return result;
}

char pinapl_get_char_at(struct pinapl_lexer *l, usize index)
{
    char c = 0;
    if (index < l->buffer_size)
    {
        c = l->buffer[index];
    }
    return c;
}

char pinapl_get_char(struct pinapl_lexer *l)
{
    char c = pinapl_get_char_at(l, l->index);
    return c;
}

char pinapl_eat_char(struct pinapl_lexer *l)
{
    char c = pinapl_get_char(l);
    l->index += 1;
    char c1 = pinapl_get_char(l);

    if ((c == '\n') || (c == '\r'))
    {
        if (((c == '\n') && (c1 == '\r')) || ((c == '\r') && (c1 == '\n')))
        {
            l->index += 1;
        }

        struct pinapl_line_info info;
        info.start_index_in_buffer = l->index - l->column;
        info.length = l->column;

        l->lines[l->lines_count++] = info;

        l->line += 1;
        l->column = 0;
    }
    l->column += 1;
    return c;
}

void consume_while(struct pinapl_lexer *l, predicate *p)
{
    char c = pinapl_get_char(l);
    while (p(c))
    {
        pinapl_eat_char(l);
        c = pinapl_get_char(l);
    }
}

void consume_until(struct pinapl_lexer *l, predicate *p)
{
    char c = pinapl_get_char(l);
    while (!p(c))
    {
        pinapl_eat_char(l);
        c = pinapl_get_char(l);
    }
}

token pinapl_get_token(struct pinapl_parser *parser)
{
    // @todo: use normal name for the variable
    struct pinapl_lexer *l = &parser->lexer;

    if (!l->next_token_valid)
    {
        consume_while(l, is_ascii_whitespace);
       
        token t = {0};

        char c = pinapl_get_char(l);
        if (c == 0)
        {
            t.type = TOKEN_EOF;
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;
            t.span_size = 1;
        }
        else if (is_valid_identifier_head(c))
        {
            t.type = TOKEN_IDENTIFIER;
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;

            pinapl_eat_char(l);
            c = pinapl_get_char(l);
            int span_size = 1;

            while (is_valid_identifier_body(c))
            {
                pinapl_eat_char(l);
                c = pinapl_get_char(l);
                span_size += 1;
            }
            
            t.span_size = span_size;

            // @todo @efficiency
            // Maybe precompute hashes of keywrods
            for (int keyword_index = 0; keyword_index < ARRAY_COUNT(keywords); keyword_index++)
            {
                struct string keyword = make_string_from_cstring(keywords[keyword_index]);
                struct string testing = { .data = t.span, .size = t.span_size };
                if (strings_equal(keyword, testing))
                {
                    t.type = keyword_types[keyword_index];
                    break;
                }
            }
        }
        else if (is_ascii_digit(c))
        {
            t.type = TOKEN_LITERAL_INT;
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;
            t.span_size = 0;

            int integer = 0;

            while (is_ascii_digit(c))
            {
                integer *= 10;
                integer += (c - '0');
                pinapl_eat_char(l);
                c = pinapl_get_char(l);
                t.span_size += 1;
            }

            t.integer_value = integer;
        }
        else
        {
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;

            if (c == '-')
            {
                pinapl_eat_char(l);
                c = pinapl_get_char(l);
                if (c == '>')
                {
                    t.type = TOKEN_ARROW_RIGHT;
                    pinapl_eat_char(l);
                    t.span_size = 2;
                }
                else
                {
                    t.type = TOKEN_MINUS;
                    t.span_size = 1;
                }
            }
            else
            {
                pinapl_eat_char(l);
                t.type = (enum pinapl_token_type) c;
                t.span_size = 1;
            }
        }

        l->next_token = t;
        l->next_token_valid = true;
    }

    return l->next_token;
}

token pinapl_eat_token(struct pinapl_parser *parser)
{
    token result = pinapl_get_token(parser);
    parser->lexer.next_token_valid = false;
    return result;
}

int parser_get_operator_precedence(token t)
{
    switch (t.type)
    {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return 0;
    }
    return 0;
}

void pinapl_report_compiler_error(struct pinapl_parser *parser, char const *data, usize size)
{
    char *buffer = ALLOCATE_BUFFER_(parser->err_allocator, size);
    memcpy(buffer, data, size);
}

void pinapl_report_compiler_error_cstring(struct pinapl_parser *parser, char const *c_string)
{
    usize size_no0 = cstring_size_no0(c_string);
    pinapl_report_compiler_error(parser, c_string, size_no0);
}

void pinapl_report_compiler_error_string(struct pinapl_parser *parser, struct string string)
{
    pinapl_report_compiler_error(parser, string.data, string.size);
}

void pinapl_report_highlight_token(struct pinapl_parser *parser, token t)
{
    int line_number_width = 3;
    int skip_lines_up = 1;
    int skip_lines_down = 0; // @todo: decide if I need post lines to show up
    UNUSED(skip_lines_down);

    pinapl_report_compiler_error_cstring(parser, "In file '");
    pinapl_report_compiler_error_cstring(parser, parser->filename);
    pinapl_report_compiler_error_cstring(parser, "'\n");

    int line_index = (t.line - 1) - skip_lines_up;
    while (skip_lines_up)
    {
        if (0 <= line_index && line_index < parser->lexer.lines_count)
        {
            struct pinapl_line_info info = parser->lexer.lines[line_index];

            char *data = parser->lexer.buffer + info.start_index_in_buffer;
            usize size = info.length;

            char number_buffer[8] = {0};
            int number_index = ARRAY_COUNT(number_buffer);

            int n = line_index + 1;
            while (n)
            {
                number_index -= 1;
                number_buffer[number_index] = '0' + (n % 10);
                n = n / 10;
            }
            int count = ARRAY_COUNT(number_buffer) - number_index;

            pinapl_report_compiler_error(parser, number_buffer + number_index, count);
            if (line_number_width - count > 0) pinapl_report_compiler_error(parser, spaces, line_number_width - count);
            pinapl_report_compiler_error(parser, "| ", 2);
            pinapl_report_compiler_error(parser, data, size);
        }

        skip_lines_up -= 1;
        line_index += 1;
    }

    char *line_start = t.span - (t.column - 1);
    usize line_length = (t.column - 1) + t.span_size;
    char *p = t.span + t.span_size;
    while (*p)
    {
        char c = *p;
        if (c == '\n' || c == '\r')
        {
            line_length += 1;
            char c1 = *(p + 1);
            if (((c == '\n') && (c1 == '\r')) || ((c == '\r') && (c1 == '\n')))
            {
                line_length += 1;
            }

            break;
        }

        p++;
        line_length += 1;
    }

    char number_buffer[8] = {0};
    int number_index = ARRAY_COUNT(number_buffer);

    int n = t.line;
    while (n)
    {
        number_index -= 1;
        number_buffer[number_index] = '0' + (n % 10);
        n = n / 10;
    }
    int count = ARRAY_COUNT(number_buffer) - number_index;

    pinapl_report_compiler_error(parser, number_buffer + number_index, count);
    if (line_number_width - count > 0) pinapl_report_compiler_error(parser, spaces, line_number_width - count);
    pinapl_report_compiler_error(parser, "| ", 2);

    pinapl_report_compiler_error(parser, line_start, line_length);
    pinapl_report_compiler_error(parser, spaces, (t.column - 1) + (line_number_width + 2));
    pinapl_report_compiler_error(parser, carets, t.span_size);
    pinapl_report_compiler_error(parser, "\n", 1);
}

struct string pinapl_parser_get_error_string(struct pinapl_parser *parser)
{
    struct string result = {0};

    result.data = parser->err_allocator->memory;
    result.size = parser->err_allocator->used;

    return result;
}

ast_node *pinapl_parse_expression_operand(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    token t = pinapl_get_token(parser);
    if (t.type == TOKEN_IDENTIFIER)
    {
        pinapl_eat_token(parser);

        token paren_open = pinapl_get_token(parser);
        if (paren_open.type == '(')
        {
            pinapl_eat_token(parser);

            // @todo: parse argument_list

            token s = pinapl_get_token(parser);
            if (s.type == ')')
            {
                pinapl_eat_token(parser);

                result = ALLOCATE(parser->ast_allocator, ast_node);
                result->type = AST_NODE_FUNCTION_CALL;
                result->function_call.name = t;
                result->function_call.argument_list = NULL;
            }
        }
        else
        {
            result = ALLOCATE(parser->ast_allocator, ast_node);
            result->type = AST_NODE_VARIABLE;
            result->variable.span = t.span;
            result->variable.span_size = t.span_size;
        }
    }
    else if (t.type == TOKEN_LITERAL_INT)
    {
        pinapl_eat_token(parser);

        result = ALLOCATE(parser->ast_allocator, ast_node);
        result->type = AST_NODE_LITERAL_INT;
        result->integer_literal.span = t.span;
        result->integer_literal.span_size = t.span_size;
        result->integer_literal.integer_value = t.integer_value;
    }
    else
    {
        pinapl_report_compiler_error_cstring(parser, "Error 1.\n");
        pinapl_report_highlight_token(parser, t);
    }

    return result;
}

ast_node *pinapl_parse_expression(struct pinapl_parser *parser, int precedence)
{
    ast_node *left_operand = NULL;

    token open_paren = pinapl_get_token(parser);
    if (open_paren.type == '(')
    {
        pinapl_eat_token(parser);
        left_operand = pinapl_parse_expression(parser, 0);
        if (left_operand == NULL)
        {
            return NULL;
        }

        token close_paren = pinapl_get_token(parser);
        if (close_paren.type == ')')
        {
            pinapl_eat_token(parser);
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error: expected ')'\n");
            return NULL; // @leak left_operand
        }
    }
    else
    {
        left_operand = pinapl_parse_expression_operand(parser);
        if (left_operand == NULL)
        {
            return NULL;
        }
    }

    while (true)
    {
        token operator = pinapl_get_token(parser);
        if ((operator.type != '+') &&
            (operator.type != '-') &&
            (operator.type != '*') &&
            (operator.type != '/') &&
            (operator.type != '='))
        {
            break;
        }

        int operator_precedence = parser_get_operator_precedence(operator);
        if (operator_precedence < precedence) break;

        pinapl_eat_token(parser);
        
        // +1 for left associativity, +0 for right ass
        ast_node *right_operand = pinapl_parse_expression(parser, operator_precedence + 1);
        if (right_operand == NULL)
        {
            pinapl_report_compiler_error_cstring(parser, "Error 2.\n");
            return NULL;
        }

        ast_node *binary_operator = ALLOCATE(parser->ast_allocator, ast_node);
        binary_operator->type = AST_NODE_BINARY_OPERATOR;
        binary_operator->binary_operator.op   = operator;
        binary_operator->binary_operator.lhs  = left_operand;
        binary_operator->binary_operator.rhs  = right_operand;

        left_operand = binary_operator;
    }

    return left_operand;
}

ast_node *pinapl_parse_variable_declaration(struct pinapl_parser *parser)
{
    //
    // x :: <expr>;
    // x := <expr>;
    // x : int;
    // x : int : <expr>;
    // x : int = <expr>;
    //

    ast_node *result = NULL;

    token var_name = pinapl_eat_token(parser);
    if (var_name.type == TOKEN_IDENTIFIER)
    {
        token colon = pinapl_eat_token(parser);
        if (colon.type == ':')
        {
            token type = {0};
            b32 is_constant = false;
            b32 should_init = true;

            token t = pinapl_get_token(parser);
            if (t.type == '=')
            {
                //
                // <ident> := <expr>
                //
                pinapl_eat_token(parser);
                is_constant = false;
            }
            else if (t.type == ':')
            {
                //
                // <ident> :: <expr>
                //
                pinapl_eat_token(parser);
                is_constant = true;
            }
            else if (t.type == TOKEN_IDENTIFIER)
            {
                pinapl_eat_token(parser);
                if ((t.span_size == 3) &&
                    (t.span[0] == 'i') &&
                    (t.span[1] == 'n') &&
                    (t.span[2] == 't'))
                {
                    token t2 = pinapl_get_token(parser);
                    if (t2.type == '=')
                    {
                        //
                        // <ident> : int = <expr>
                        //
                        pinapl_eat_token(parser);
                        type = t;
                        is_constant = false;
                    }
                    else if (t2.type == ':')
                    {
                        //
                        // <ident> : int : <expr>
                        //
                        pinapl_eat_token(parser);
                        type = t;
                        is_constant = true;
                    }
                    else if (t2.type == ';')
                    {
                        // x : int
                        type = t;
                        is_constant = false;
                        should_init = false;

                        result = ALLOCATE(parser->ast_allocator, ast_node);
                        result->type = AST_NODE_VARIABLE_DECLARATION;
                        result->variable_declaration.name = make_string_id(var_name.span, var_name.span_size);
                        result->variable_declaration.var_type = type;
                        result->variable_declaration.init = NULL;
                    }
                    else
                    {
                        pinapl_report_compiler_error_cstring(parser, "Error: expected ':', '=', or ';'\nIn file: '");
                        pinapl_report_compiler_error_cstring(parser, parser->filename);
                        pinapl_report_compiler_error(parser, "'\n", 2);
                        pinapl_report_highlight_token(parser, t2);
                    }
                }
                else
                {
                    pinapl_report_compiler_error_cstring(parser, "Error: I do not allow other types other than 'int' YET.\n");
                }
            }
            else
            {
                should_init = false;
                pinapl_report_compiler_error_cstring(parser, "Error: expected '=', ':', or a type.\n");
                pinapl_report_highlight_token(parser, t);
            }

            if (should_init)
            {
                struct pinapl_lexer checkpoint = parser->lexer;
                ast_node *initializer = pinapl_parse_expression(parser, 0);
                if (!initializer)
                {
                    parser->err_allocator->used = 0; // Clear up error buffer
                    parser->lexer = checkpoint;
                    initializer = pinapl_parse_function_definition(parser);
                }

                if (initializer)
                {
                    result = ALLOCATE(parser->ast_allocator, ast_node);
                    result->type = is_constant 
                        ? AST_NODE_CONSTANT_DECLARATION
                        : AST_NODE_VARIABLE_DECLARATION;
                    result->variable_declaration.name = make_string_id(var_name.span, var_name.span_size);
                    result->variable_declaration.var_type = type;
                    result->variable_declaration.init = initializer;
                }
                else
                {
                    pinapl_report_compiler_error_cstring(parser, "Error: expression expected, or function declaration.\n");
                }
            }
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error: token ':' expected.\n");
        }
    }
    else
    {
        pinapl_report_compiler_error_cstring(parser, "Error: variable declaration is not started with identifier.\n");
    }

    return result;
}

ast_node *pinapl_parse_block(struct pinapl_parser *parser)
{
    ast_node *result = NULL;
    //
    // block ::= { <statement-list> }
    //
    token open_brace = pinapl_eat_token(parser);
    if (open_brace.type == '{')
    {
        struct pinapl_lexer checkpoint = parser->lexer;
        ast_node *statement_list = pinapl_parse_statement_list(parser);
        if (!statement_list)
        {
            parser->lexer = checkpoint;
        }
        
        token close_brace = pinapl_eat_token(parser);
        if (close_brace.type == '}')
        {
            result = ALLOCATE(parser->ast_allocator, ast_node);
            result->type = AST_NODE_BLOCK;
            result->block.statement_list = statement_list;
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error: expected '}'\n");
        }
    }
    else
    {
        pinapl_report_compiler_error_cstring(parser, "Error: expected '{'\n");
    }

    return result;
}

ast_node *pinapl_parse_function_definition(struct pinapl_parser *parser)
{
    //
    // function-definition ::= 
    //   (<argument-list>) <block>
    //   (<argument-list>) -> <return-type> <block>
    //

    ast_node *result = NULL;

    token open_paren = pinapl_eat_token(parser);
    if (open_paren.type == '(')
    {
        // @todo: argument parsing here

        token close_paren = pinapl_eat_token(parser);
        if (close_paren.type == ')')
        {
            ast_node *block = pinapl_parse_block(parser);
            if (block)
            {
                result = ALLOCATE(parser->ast_allocator, ast_node);
                result->type = AST_NODE_FUNCTION_DEFINITION;
                result->function_definition.parameter_list = NULL; 
                result->function_definition.return_type = NULL;
                result->function_definition.block = block; 
            }
            else
            {
                pinapl_report_compiler_error_cstring(parser, "Do I allow forward-declarations for functions in pinapl ?\n");
            }
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error: expected ')'\n");
        }
    }
    else
    {
        pinapl_report_compiler_error_cstring(parser, "Error: expected '('\n");
    }

    return result;
}

ast_node *pinapl_parse_return_statement(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    token keyword = pinapl_get_token(parser);
    if (keyword.type == TOKEN_KW_RETURN)
    {
        pinapl_eat_token(parser);
        ast_node *return_expression = pinapl_parse_expression(parser, 0);

        if (return_expression)
        {
            result = ALLOCATE(parser->ast_allocator, ast_node);
            result->type = AST_NODE_RETURN_STATEMENT;
            result->return_statement.expression = return_expression;
        }
    }
    else
    {
        pinapl_report_compiler_error_cstring(parser, "Error: expected 'return' keyword\n");
    }

    return result;
}

ast_node *pinapl_parse_statement(struct pinapl_parser *parser)
{
    ast_node *result = NULL;
    struct pinapl_lexer checkpoint = parser->lexer;
    b32 have_to_end_with_semicolon = true;

    ast_node *variable_declaration = pinapl_parse_variable_declaration(parser);
    if (variable_declaration)
    {
        result = variable_declaration;
    }
    else
    {
        parser->err_allocator->used = 0; // Clear up error buffer
        parser->lexer = checkpoint;

        ast_node *block = pinapl_parse_block(parser);
        if (block)
        {
            result = block;
            have_to_end_with_semicolon = false; // Exception for blocks @todo better algorithm? 
        }
        else
        {
            parser->err_allocator->used = 0;
            parser->lexer = checkpoint;

            ast_node *expression = pinapl_parse_expression(parser, 0);
            if (expression)
            {
                result = expression;
            }
            else
            {
                parser->err_allocator->used = 0;
                parser->lexer = checkpoint;

                ast_node *return_statement = pinapl_parse_return_statement(parser);
                if (return_statement)
                {
                    result = return_statement;
                }
                else
                {
                    pinapl_report_compiler_error_cstring(parser, "Error!!! I don't know what it is!\n");
                }
            }
        }
    }

    if (have_to_end_with_semicolon)
    {
        token semicolon = pinapl_get_token(parser);
        if (semicolon.type == ';')
        {
            pinapl_eat_token(parser);
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error! statement should end with a semicolon!\n");
            result = NULL; // @leak
        }
    }

    return result;
}

ast_node *pinapl_parse_statement_list(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    token close_brace = pinapl_get_token(parser);
    if (close_brace.type == '}')
    {
        result = ALLOCATE(parser->ast_allocator, ast_node);
        result->type = AST_NODE_EMPTY_LIST;
    }
    else
    {
        ast_node *first_statement = pinapl_parse_statement(parser);
        if (first_statement)
        {
            result = ALLOCATE(parser->ast_allocator, ast_node);
            result->type = AST_NODE_STATEMENT_LIST;
            result->list.node = first_statement;
            result->list.next = NULL;

            ast_node *last_list_node = result;
        
            while (true)
            {
                close_brace = pinapl_get_token(parser);
                if (close_brace.type == '}')
                {
                    // Ok. This is the end of the list
                    break;
                }
                else
                {
                    ast_node *statement = pinapl_parse_statement(parser);
                    if (statement)
                    {
                        ast_node *new_list_node = ALLOCATE(parser->ast_allocator, ast_node);
                        new_list_node->type = AST_NODE_STATEMENT_LIST;
                        new_list_node->list.node = statement;
                        new_list_node->list.next = NULL;

                        last_list_node->list.next = new_list_node;
                        last_list_node = new_list_node;
                    }
                    else
                    {
                        // Not ok! Statement not parsed, something went wrong!
                        result = NULL; // @leak!
                        break;
                    }
                }
            }
        }
    }

    return result;
}

ast_node *pinapl_parse_global_declaration(struct pinapl_parser *parser)
{
    ast_node *result = pinapl_parse_variable_declaration(parser);
    return result;
}

ast_node *pinapl_parse_global_declaration_list(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    ast_node *first_declaration = pinapl_parse_global_declaration(parser);
    if (first_declaration)
    {
        result = ALLOCATE(parser->ast_allocator, ast_node);
        result->type = AST_NODE_GLOBAL_DECLARATION_LIST;
        result->list.node = first_declaration;
        result->list.next = NULL;

        ast_node *last_list_node = result;
        while (true)
        {
            ast_node *declaration = pinapl_parse_global_declaration(parser);
            if (declaration)
            {
                ast_node *next_list_node = ALLOCATE(parser->ast_allocator, ast_node);
                next_list_node->type = AST_NODE_GLOBAL_DECLARATION_LIST;
                next_list_node->list.node = declaration;
                next_list_node->list.next = NULL;

                last_list_node->list.next = next_list_node;
                last_list_node = next_list_node;
            }
            else
            {
                break;
            }
        }
    }

    return result;
}


u32 pinapl_hash_string(char *string, usize string_size)
{
    // @todo: better hash function!
    u32 hash = 0;
    for (int index = 0; (index < string_size) && string[index] != 0; index++)
    {
        hash += string[index] * primes[index % ARRAY_COUNT(primes)];
    }
    return hash;
}


void pinapl_push_nested_scope(struct pinapl_scope *parent, struct pinapl_scope *nested)
{
    struct pinapl_scope *scope = parent->nested_scope;

    nested->parent_scope = parent;
    nested->next_scope = scope;
    parent->nested_scope = nested;
}


struct pinapl_scope_entry *pinapl_get_scope_entry_slot(struct pinapl_scope *scope, u32 hash)
{     
    struct pinapl_scope_entry *result = NULL;
    for (int offset = 0; offset < ARRAY_COUNT(scope->hash_table); offset++)
    {
        int index = (hash + offset) % ARRAY_COUNT(scope->hash_table);
        struct pinapl_scope_entry *entry = scope->hash_table + index;
        if (entry->hash == hash)
        {
            result = entry;
            break;
        }
        else if (entry->hash == 0 && entry->entry_name == NULL)
        {
            result = entry;
            break;
        }
    }
    return result;
}


struct pinapl_scope_entry *pinapl_is_variable_declared_in_scope(struct pinapl_scope *scope, char *string, usize string_size)
{
    u32 hash = pinapl_hash_string(string, string_size);
    struct pinapl_scope_entry *slot = pinapl_get_scope_entry_slot(scope, hash);
    struct pinapl_scope_entry *result = NULL;
    if (slot && slot->hash)
    {
        result = slot;
    }
    return result;
}


struct pinapl_scope_entry *pinapl_is_variable_declared(struct pinapl_scope *scope, char *string, usize string_size)
{
    u32 hash = pinapl_hash_string(string, string_size);

    struct pinapl_scope_entry *result = NULL;
    while (scope)
    {
        struct pinapl_scope_entry *slot = pinapl_get_scope_entry_slot(scope, hash);
        if (slot && slot->hash)
        {
            result = slot;
            break;
        }
        scope = scope->parent_scope;
    }

    return result;
}


struct pinapl_scope_entry *pinapl_declare_variable_in_scope(struct pinapl_scope *scope, ast_node *node)
{
    struct string string = get_string_by_id(node->variable_declaration.name);

    u32 hash = pinapl_hash_string(string.data, string.size);
    struct pinapl_scope_entry *slot = pinapl_get_scope_entry_slot(scope, hash);
    if (slot && slot->hash == 0)
    {
        slot->hash = hash;
        slot->entry_name = string.data;
        slot->entry_name_size = string.size;
        slot->declaration_node = node;
    }
    return slot;
}


b32 pinapl_check_and_rename_variables(struct pinapl_rename_stage *stage, ast_node *node, struct pinapl_scope *scope)
{
    b32 is_ok = true;

    switch (node->type)
    {
        case AST_NODE_EMPTY_LIST:
        break;

        case AST_NODE_GLOBAL_DECLARATION_LIST:
        case AST_NODE_STATEMENT_LIST:
        case AST_NODE_LIST:
        {
            is_ok = pinapl_check_and_rename_variables(stage, node->list.node, scope);
            if (is_ok && node->list.next)
            {
                is_ok = pinapl_check_and_rename_variables(stage, node->list.next, scope);
            }
        }
        break;

        case AST_NODE_RETURN_STATEMENT:
        {
            pinapl_check_and_rename_variables(stage, node->return_statement.expression, scope);
        }
        break;

        case AST_NODE_BLOCK:
        {
            struct pinapl_scope *inner_scope = ALLOCATE(stage->scope_allocator, struct pinapl_scope);
            pinapl_push_nested_scope(scope, inner_scope);

            is_ok = pinapl_check_and_rename_variables(stage, node->block.statement_list, inner_scope);
        }
        break;
        
        case AST_NODE_VARIABLE_DECLARATION:
        case AST_NODE_CONSTANT_DECLARATION:
        {
            ast_node_variable_declaration *var = &node->variable_declaration;
            struct string name = get_string_by_id(var->name);

            struct pinapl_scope_entry *entry = pinapl_is_variable_declared_in_scope(scope, name.data, name.size);
            if (entry == NULL) // i.e. not declared
            {
                if (var->init->type == AST_NODE_FUNCTION_DEFINITION)
                {
                    var->symbol_id = stage->global_variable_counter++;
                    entry = pinapl_declare_variable_in_scope(scope, node);
                }

                is_ok = pinapl_check_and_rename_variables(stage, var->init, scope);

                if (is_ok)
                {
                    // Init expression is ok
                    if (entry == NULL) // i.e. not declared
                    {
                        var->symbol_id = stage->global_variable_counter++;
                        entry = pinapl_declare_variable_in_scope(scope, node);
                    }
                }
                else
                {
                    // Pass error from check_scopes up
                    is_ok = false;
                }
            }
            else
            {
                // Error: variable is already declared
                is_ok = false;
            }
        }
        break;

        case AST_NODE_FUNCTION_DEFINITION:
        {
            if (node->function_definition.block)
            {
                is_ok = pinapl_check_and_rename_variables(stage, node->function_definition.block, scope);
            }
        }
        break;

        case AST_NODE_BINARY_OPERATOR:
        {
            is_ok = pinapl_check_and_rename_variables(stage, node->binary_operator.lhs, scope);
            if (is_ok)
            {
                is_ok = pinapl_check_and_rename_variables(stage, node->binary_operator.rhs, scope);
            }
        }
        break;

        case AST_NODE_LITERAL_INT:
        break;

        case AST_NODE_VARIABLE:
        {
            char *name = node->variable.span;
            usize name_size = node->variable.span_size;
            struct pinapl_scope_entry *entry = pinapl_is_variable_declared(scope, name, name_size);
            if (entry) // i.e. is declared
            {
                node->variable.symbol_id = entry->declaration_node->variable_declaration.symbol_id;
            }
            else // i.e. not declared
            {
                // Error! used non-defined variable!
                is_ok = false;
            }
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        {
            char *name = node->function_call.name.span;
            usize name_size = node->function_call.name.span_size;
            struct pinapl_scope_entry *entry = pinapl_is_variable_declared(scope, name, name_size);
            if (entry == NULL) // i.e. not declared
            {
                // Error! used non-defined function!
                is_ok = false;
            }
        }
        break;

        case AST_NODE_INVALID:
        {
            is_ok = false;
        }
        break;
    }

    return is_ok;
}


struct pinapl_tac *pinapl_push_tac(struct pinapl_flatten_stage *stage, struct pinapl_tac code)
{
    struct pinapl_tac *result = stage->codes + stage->code_count++;
    *result = code;
    return result;
}




struct flatten_result pinapl_flatten_ast(struct pinapl_flatten_stage *stage, ast_node *node)
{
    struct flatten_result result = {0};

    switch (node->type)
    {
        case AST_NODE_INVALID:
        case AST_NODE_EMPTY_LIST:
        break;

        case AST_NODE_LIST:
        case AST_NODE_GLOBAL_DECLARATION_LIST:
        case AST_NODE_STATEMENT_LIST:
        {
            result = pinapl_flatten_ast(stage, node->list.node);
            if (node->list.next)
            {
                result = pinapl_flatten_ast(stage, node->list.next);
            }
        }
        break;

        case AST_NODE_RETURN_STATEMENT:
        {
            struct flatten_result ret_expr = pinapl_flatten_ast(stage, node->return_statement.expression);

            struct pinapl_tac code;
            code.type = TAC_MOV;
            code.dst  = 0;
            UNUSED(code.rhs);

            if (ret_expr.type == FLATTEN_RESULT_INTEGER)
            {
                code.type |= TAC_LHS_INT;
                code.lhs = ret_expr.integer_value;
            }
            else if (ret_expr.type == FLATTEN_RESULT_VARIABLE)
            {
                code.type |= TAC_LHS_REG;
                code.lhs = ret_expr.variable_id;
            }
            else if (ret_expr.type == FLATTEN_RESULT_INSTRUCTION)
            {
                code.type |= TAC_LHS_REG;
                code.lhs = ret_expr.instruction->dst;
            }
            else
            {
                ASSERT_FAIL();
            }
                
            pinapl_push_tac(stage, code);

            struct pinapl_tac ret_code;
            ret_code.type = TAC_RET;
            ret_code.dst = 0;

            result.type = FLATTEN_RESULT_INSTRUCTION;
            result.instruction = pinapl_push_tac(stage, ret_code);
        }
        break;

        case AST_NODE_BLOCK:
        {
            result = pinapl_flatten_ast(stage, node->block.statement_list);
        }
        break;

        case AST_NODE_VARIABLE_DECLARATION:
        case AST_NODE_CONSTANT_DECLARATION:
        {
            if (node->variable_declaration.init)
            {
                struct flatten_result init_result = pinapl_flatten_ast(stage, node->variable_declaration.init);
                
                if (init_result.type == FLATTEN_RESULT_INTEGER)
                {
                    // mov %dst #int
                    struct pinapl_tac code;
                    code.type = TAC_MOV | TAC_LHS_INT | TAC_NO_RHS;
                    code.dst  = node->variable_declaration.symbol_id;
                    code.lhs  = init_result.integer_value;
                    UNUSED(code.rhs);

                    result.type = FLATTEN_RESULT_INSTRUCTION;
                    result.instruction = pinapl_push_tac(stage, code);
                }
                else if (init_result.type == FLATTEN_RESULT_INSTRUCTION)
                {
                    // patch instruction so that it's dst becomes ours
                    init_result.instruction->dst = node->variable_declaration.symbol_id;
                }
                else if (init_result.type == FLATTEN_RESULT_LABEL)
                {
                    init_result.instruction->label = node->variable_declaration.name;
                }
                else if (init_result.type == FLATTEN_RESULT_VARIABLE)
                {
                    // mov %dst %src
                    struct pinapl_tac code;
                    code.type = TAC_MOV | TAC_LHS_REG | TAC_NO_RHS;
                    code.dst  = node->variable_declaration.symbol_id;
                    code.lhs  = init_result.variable_id;
                    UNUSED(code.rhs);

                    result.type = FLATTEN_RESULT_INSTRUCTION;
                    result.instruction = pinapl_push_tac(stage, code);
                }
            }
            else
            {
                struct pinapl_tac code;
                code.type = TAC_MOV | TAC_LHS_INT;
                code.dst  = node->variable_declaration.symbol_id;
                code.lhs  = 0;
                UNUSED(code.rhs);

                result.type = FLATTEN_RESULT_INSTRUCTION;
                result.instruction = pinapl_push_tac(stage, code);
            }
        }
        break; 

        case AST_NODE_FUNCTION_DEFINITION:
        {
            usize label_to = stage->code_count;

            struct pinapl_tac code;
            code.type = TAC_LABEL;
            code.dst  = label_to;
            UNUSED(code.lhs);
            UNUSED(code.rhs);

            struct pinapl_tac *instruction = pinapl_push_tac(stage, code);

            pinapl_flatten_ast(stage, node->function_definition.block);

            ASSERT(stage->label_count < stage->labels_size);
            stage->labels[stage->label_count++] = label_to;

            result.type = FLATTEN_RESULT_LABEL;
            result.instruction = instruction;
        }
        break;

        case AST_NODE_BINARY_OPERATOR:
        {
            ast_node *lhs_node = node->binary_operator.lhs;
            struct flatten_result lhs_result = pinapl_flatten_ast(stage, lhs_node);

            ast_node *rhs_node = node->binary_operator.rhs;
            struct flatten_result rhs_result = pinapl_flatten_ast(stage, rhs_node);

            struct pinapl_tac code;

            int code_lhs_type = 0;
            int code_rhs_type = 0;

            if (lhs_result.type == FLATTEN_RESULT_INTEGER)
            {
                code_lhs_type = TAC_LHS_INT;
                code.lhs = lhs_result.integer_value;
            }
            else if (lhs_result.type == FLATTEN_RESULT_VARIABLE)
            {
                code_lhs_type = TAC_LHS_REG;
                code.lhs = lhs_result.variable_id;
            }
            else if (lhs_result.type == FLATTEN_RESULT_INSTRUCTION)
            {
                code_lhs_type = TAC_LHS_REG;
                code.lhs = lhs_result.instruction->dst;
            }
            else
            {
                ASSERT_FAIL();
            }
            
            if (rhs_result.type == FLATTEN_RESULT_INTEGER)
            {
                code_rhs_type = TAC_RHS_INT;
                code.rhs = rhs_result.integer_value;
            }
            else if (rhs_result.type == FLATTEN_RESULT_VARIABLE)
            {
                code_rhs_type = TAC_RHS_REG;
                code.rhs = rhs_result.variable_id;
            }
            else if (rhs_result.type == FLATTEN_RESULT_INSTRUCTION)
            {
                code_rhs_type = TAC_RHS_REG;
                code.rhs = rhs_result.instruction->dst;
            }
            else
            {
                ASSERT_FAIL();
            }
            
            enum pinapl_token_type op_type = node->binary_operator.op.type;
            switch (op_type)
            {
                case TOKEN_PLUS:
                    code.type = TAC_ADD | code_lhs_type | code_rhs_type;
                    code.dst  = stage->global_variable_counter++;
                break;
                
                case TOKEN_MINUS:
                    code.type = TAC_SUB | code_lhs_type | code_rhs_type;
                    code.dst  = stage->global_variable_counter++;
                break;
                
                case TOKEN_ASTERICS:
                    code.type = TAC_MUL | code_lhs_type | code_rhs_type;
                    code.dst  = stage->global_variable_counter++;
                break;
                
                case TOKEN_SLASH:
                    code.type = TAC_DIV | code_lhs_type | code_rhs_type;
                    code.dst  = stage->global_variable_counter++;
                break;

                case TOKEN_EQUALS:
                    code.type = TAC_MOV | (code_rhs_type >> 8) | TAC_NO_RHS; // SHR go from RHS to LHS
                    code.dst = code.lhs;
                    code.lhs = code.rhs;
                    code.rhs = 0;
                break;

                default:
                break;
            }

            result.type = FLATTEN_RESULT_INSTRUCTION;
            result.instruction = pinapl_push_tac(stage, code);
        }
        break;

        case AST_NODE_LITERAL_INT:
        {
            result.type = FLATTEN_RESULT_INTEGER;
            result.integer_value = node->integer_literal.integer_value;
        }
        break;

        case AST_NODE_VARIABLE:
        {
            result.type = FLATTEN_RESULT_VARIABLE;
            result.variable_id = node->variable.symbol_id;
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        break;
   }

   return result;
}

struct pinapl_liveness_table pinapl_make_liveness_table(struct allocator *allocator, struct pinapl_flatten_stage *flatten)
{
    struct pinapl_liveness_table liveness;
    liveness.allocator = allocator;
    liveness.table = ALLOCATE_BUFFER(allocator, flatten->code_count * flatten->global_variable_counter * sizeof(int));
    liveness.table_width = flatten->global_variable_counter;
    liveness.table_length = flatten->code_count;
    
    int *row = liveness.table + liveness.table_width * (liveness.table_length - 1);
    int code_index = flatten->code_count;
    int index_to_set_0 = 0;
    while (code_index-->1)
    {
        int *next_row = row;
        row -= liveness.table_width;
        memcpy(row, next_row, sizeof(int) * liveness.table_width); 
        row[index_to_set_0] = 0;

        struct pinapl_tac code = flatten->codes[code_index];
        
        // dst
        if ((code.type & TAC_LABEL) == 0) // NOT A LABEL
        {
            row[code.dst] = 1;
            index_to_set_0 = code.dst;
        }
        // lhs
        if (code.type & TAC_LHS_REG)
        {
            row[code.lhs] = 1; 
        }
        // rhs
        if (code.type & TAC_RHS_REG)
        {
           row[code.rhs] = 1; 
        }
    }

    return liveness;
}

void pinapl_print_liveness_table(struct pinapl_liveness_table *table)
{
    for (int row = 0; row < table->table_length; row++) // code count
    {
        print("code %d: \n", row);
        for (int column = 0; column < table->table_width; column++) // var count
        {
            int live = table->table[column + row * table->table_width];
            print("%d ", live);
        }
        print("\n");
    }
}

void pinapl_push_edge(struct pinapl_connectivity_graph *graph, int from, int to)
{
    if (graph->edge_count < ARRAY_COUNT(graph->edges))
    {
        struct pinapl_graph_edge *edge = graph->edges + graph->edge_count++;
        edge->from = from;
        edge->to = to;
    }
}

struct pinapl_connectivity_graph pinapl_make_connectivity_graph(struct pinapl_liveness_table *liveness)
{
    struct pinapl_connectivity_graph graph = {0};
    for (int i = 0; i < ARRAY_COUNT(graph.colors); i++)
    {
        graph.colors[i] = -1;
    }

    for (int row = 0; row < liveness->table_length; row++)
    {
        for (int var1 = 0; var1 < liveness->table_width; var1++)
        {
            int var1_live = liveness->table[var1 + row * liveness->table_width];
            
            if (var1_live)
            {
                for (int var2 = var1 + 1; var2 < liveness->table_width; var2++)
                {
                    int var2_live = liveness->table[var2 + row * liveness->table_width];

                    if (var2_live)
                    {
                        b32 edge_found = false;
                        for (int edge_index = 0; edge_index < graph.edge_count; edge_index++)
                        {
                            struct pinapl_graph_edge edge = graph.edges[edge_index];
                            if ((var1 == edge.from && var2 == edge.to) ||
                                (var2 == edge.from && var1 == edge.to))
                            {
                                edge_found = true;
                                break;
                            }
                        }

                        if (!edge_found)
                        {
                            pinapl_push_edge(&graph, var1, var2);
                        }
                    }
                }
            }
        }
    }

    for (int var = 0; var < 32; var++)
    {
        b32 color_is_taken[32] = {0};

        for (int edge_index = 0; edge_index < graph.edge_count; edge_index++)
        {
            struct pinapl_graph_edge edge = graph.edges[edge_index];

            if (edge.to == var && edge.from != var)
            {
                int neighbour_color = graph.colors[edge.from];
                if (0 <= neighbour_color && neighbour_color < ARRAY_COUNT(color_is_taken))
                {
                    color_is_taken[neighbour_color] = true;
                }
            }
            else if (edge.to != var && edge.from == var)
            {
                int neighbour_color = graph.colors[edge.to];
                if (0 <= neighbour_color && neighbour_color < ARRAY_COUNT(color_is_taken))
                {
                    color_is_taken[neighbour_color] = true;
                }
            }
        }

        int minimal_color = INT32_MAX;
        for (int color_index = 0; color_index < ARRAY_COUNT(color_is_taken); color_index++)
        {
            if (!color_is_taken[color_index])
            {
                minimal_color = color_index;
                break;
            }
        }

        ASSERT(minimal_color < INT32_MAX);
        graph.colors[var] = minimal_color;
    }

    return graph;
}

void pinapl_print_connectivity_graph(struct pinapl_connectivity_graph *graph)
{
    print("variable => color;\n");
    for (int node = 0; node < 32; node++)
    {
        print("       %d => %d;\n", node, graph->colors[node]);
    }

    print("\nGRAPH: (from, to)\n");

    for (int edge_index = 0; edge_index < graph->edge_count; edge_index++)
    {
        struct pinapl_graph_edge edge = graph->edges[edge_index];
        print("(%d, %d)\n", edge.from, edge.to);
    }
}

void pinapl_arm_print_register(int reg)
{
    if (reg == ARM_LR)
        print("lr");
    else if (reg == ARM_SP)
        print("sp");
    else if (reg == ARM_PC)
        print("pc");
    else
        print("r%d", reg); 
}

void
pinapl_arm_print_instruction_operand(struct pinapl_arm_instruction_operand op)
{
    switch (op.type)
    {
        case ARM_OPERAND_REGISTER:
            pinapl_arm_print_register(op.value);
            break;

        case ARM_OPERAND_IMMEDIATE_VALUE:
            print("#%d", op.value);
            break;

        case ARM_OPERAND_DEREFERENCE:
            print("[");
            pinapl_arm_print_register(op.value);
            print("]");
            break;

        case ARM_OPERAND_LABEL:
        case ARM_OPERAND_DECLARATION:
            print_string(get_string_by_id(op.strid));
            break;

        case ARM_OPERAND_NONE:
            print("_");
            break;

        default:
            print("[UNKNOWN OPERAND TYPE]");
    }
};

void
pinapl_arm_print_instruction(struct pinapl_instruction *instruction)
{
    enum pinapl_arm_instruction instr = instruction->arm & ARM_INSTRUCTION_MASK;
    switch (instr)
    {
        case ARM_LABEL:
        case ARM_SECTION:
        case ARM_GLOBAL: 
            break;

        case ARM_MOV:  print("    mov    "); break;
        case ARM_MOVS: print("    movs   "); break;
        case ARM_ADD:  print("    add    "); break;
        case ARM_SUB:  print("    sub    "); break;
        case ARM_MUL:  print("    mul    "); break;
        case ARM_DIV:  print("    div    "); break;
        case ARM_B:    print("    b      "); break;
        case ARM_BX:   print("    bx     "); break;
        case ARM_BL:   print("    bl     "); break;
        case ARM_LDR:  print("    ldr    "); break;
        case ARM_STR:  print("    str    "); break;
        case ARM_MLA:  print("    mla    "); break;
        case ARM_SVC:  print("    svc    "); break;

        default:
            print("    [UNKNOWN INSTRUCTION]");
    }

    b32 printed = false;
    if (instruction->op1.type != ARM_OPERAND_NONE)
    {
        if (instr == ARM_GLOBAL)
        {
            print(".global ");
        }
        
        // Do not print dst operand is equal to lhs operand
        if ((instruction->op1.type  != instruction->op2.type) ||
            (instruction->op1.value != instruction->op2.value))
        {
            pinapl_arm_print_instruction_operand(instruction->op1);
            printed = true;
        }

        if (instr == ARM_LABEL)
        {
            print(":");
        }
    }

    if (instruction->op2.type != ARM_OPERAND_NONE)
    {
        if (printed) print(", ");
        pinapl_arm_print_instruction_operand(instruction->op2);
        printed = true;
    }

    if (instruction->op3.type != ARM_OPERAND_NONE)
    {
        if (printed) print(", ");
        pinapl_arm_print_instruction_operand(instruction->op3);
        printed = true;
    }

    if (instruction->op4.type != ARM_OPERAND_NONE)
    {
        if (printed) print(", ");
        pinapl_arm_print_instruction_operand(instruction->op4);
        printed = true;
    }
    
    print("\n");
}

void
pinapl_arm_print_instruction_stream(struct pinapl_instruction_stream *stream)
{
    for (int instruction_index = 0; instruction_index < stream->instruction_count; instruction_index++)
    {
        struct pinapl_instruction *instruction = stream->instructions + instruction_index;
        pinapl_arm_print_instruction(instruction);
    }
}

struct pinapl_instruction_stream pinapl_make_instruction_stream(struct allocator *allocator)
{
    struct pinapl_instruction_stream stream;
    stream.allocator = allocator;
    stream.instruction_count = 0;
    stream.instruction_capacity = 100;
    stream.instructions = ALLOCATE(allocator, stream.instruction_capacity * sizeof(struct pinapl_instruction));

    memset(stream.labels, 0, sizeof(stream.labels));
    memset(stream.label_instruction_index, 0, sizeof(stream.label_instruction_index));
    stream.label_count = 0;

    return stream;
}

void pinapl_arm_push_instruction(struct pinapl_instruction_stream *stream, struct pinapl_instruction instruction)
{
    if (stream->instruction_count < stream->instruction_capacity)
    {
        stream->instructions[stream->instruction_count++] = instruction;
    }
    else
    {
        ASSERT(false);
    }
}

void pinapl_arm_push_instructions_from_flatten_stage(struct pinapl_instruction_stream *stream, struct pinapl_flatten_stage *flatten, struct pinapl_connectivity_graph *graph)
{
    for (int code_index = 0; code_index < flatten->code_count; code_index++)
    {
        struct pinapl_tac code = flatten->codes[code_index];
        struct pinapl_instruction instruction = {0};

        if (code.type & TAC_MOV)
        {
            instruction.arm = ARM_MOV;
        }
        else if (code.type & TAC_ADD)
        {
            instruction.arm = ARM_ADD;
        }
        else if (code.type & TAC_SUB)
        {
            instruction.arm = ARM_SUB;
        }
        else if (code.type & TAC_MUL)
        {
            instruction.arm = ARM_MUL;
        }
        else if (code.type & TAC_DIV)
        {
            instruction.arm = ARM_DIV;
        }
        else if (code.type & TAC_RET)
        {
            if (graph->colors[code.lhs] != ARM_R0)
                pinapl_arm_push_rr(stream, ARM_MOV, ARM_R0, graph->colors[code.lhs]);
            pinapl_arm_push_r(stream, ARM_BX, ARM_LR);
            continue;
        }
        else
        {
        }

        if ((code.type & TAC_LABEL) == 0) // NOT A LABEL
        {
            instruction.op1.type = ARM_OPERAND_REGISTER;
            instruction.op1.value = graph->colors[code.dst];
        }
        else
        {
            instruction.arm = ARM_LABEL;
            instruction.op1.type = ARM_OPERAND_LABEL;
            instruction.op1.label = code.label;
        }

        if (code.type & TAC_LHS_REG)
        {
            instruction.op2.type = ARM_OPERAND_REGISTER;
            instruction.op2.value = graph->colors[code.lhs];
        }
        else if (code.type & TAC_LHS_INT)
        {
            instruction.op2.type = ARM_OPERAND_IMMEDIATE_VALUE;
            instruction.op2.value = code.lhs;
        }

        if (code.type & TAC_RHS_REG)
        {
           instruction.op3.type = ARM_OPERAND_REGISTER;
           instruction.op3.value = graph->colors[code.rhs]; 
        }
        else if (code.type & TAC_RHS_INT)
        {
            instruction.op3.type = ARM_OPERAND_IMMEDIATE_VALUE;
            instruction.op3.value = code.rhs;
        }
        pinapl_arm_push_instruction(stream, instruction);
    }
}

void pinapl_arm_push_label(struct pinapl_instruction_stream *stream, 
                           struct string_id label)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = ARM_LABEL;
    instruction.op1.type = ARM_OPERAND_LABEL;
    instruction.op1.label = label;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_l(struct pinapl_instruction_stream *stream,
                       enum pinapl_arm_instruction arm_instruction,
                       struct string_id label)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_LABEL;
    instruction.op1.label = label;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_r(struct pinapl_instruction_stream *stream,
                       enum pinapl_arm_instruction arm_instruction,
                       enum pinapl_arm_register arg)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = arg;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_i(struct pinapl_instruction_stream *stream,
                       enum pinapl_arm_instruction arm_instruction,
                       int immediate_value)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_IMMEDIATE_VALUE;
    instruction.op1.value = immediate_value;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_ri(struct pinapl_instruction_stream *stream, 
                        enum pinapl_arm_instruction arm_instruction,
                        enum pinapl_arm_register dst,
                        int immediate_value)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = dst;
    instruction.op2.type = ARM_OPERAND_IMMEDIATE_VALUE;
    instruction.op2.value = immediate_value;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_rr   (struct pinapl_instruction_stream *stream,
                           enum pinapl_arm_instruction arm_instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register src)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = dst;
    instruction.op2.type = ARM_OPERAND_REGISTER;
    instruction.op2.value = src;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_rd(struct pinapl_instruction_stream *stream,
                        enum pinapl_arm_instruction arm_instruction,
                        enum pinapl_arm_register dst,
                        enum pinapl_arm_register src1)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = dst;
    instruction.op2.type = ARM_OPERAND_DEREFERENCE;
    instruction.op2.value = src1;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_rri(struct pinapl_instruction_stream *stream,
                         enum pinapl_arm_instruction arm_instruction,
                         enum pinapl_arm_register dst,
                         enum pinapl_arm_register src1,
                         int immediate_value)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = dst;
    instruction.op2.type = ARM_OPERAND_REGISTER;
    instruction.op2.value = src1;
    instruction.op3.type = ARM_OPERAND_IMMEDIATE_VALUE;
    instruction.op3.value = immediate_value;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_rrrr (struct pinapl_instruction_stream *stream,
                           enum pinapl_arm_instruction arm_instruction,
                           enum pinapl_arm_register dst,
                           enum pinapl_arm_register s1,
                           enum pinapl_arm_register s2,
                           enum pinapl_arm_register s3)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = arm_instruction;
    instruction.op1.type = ARM_OPERAND_REGISTER;
    instruction.op1.value = dst;
    instruction.op2.type = ARM_OPERAND_REGISTER;
    instruction.op2.value = s1;
    instruction.op3.type = ARM_OPERAND_REGISTER;
    instruction.op3.value = s2;
    instruction.op4.type = ARM_OPERAND_REGISTER;
    instruction.op4.value = s3;

    pinapl_arm_push_instruction(stream, instruction);
}

void pinapl_arm_push_section(struct pinapl_instruction_stream *stream,
                             struct string_id section)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = ARM_SECTION;
    instruction.op1.type = ARM_OPERAND_DECLARATION;
    instruction.op1.strid = section;

    pinapl_arm_push_instruction(stream, instruction);

}

void pinapl_arm_push_global(struct pinapl_instruction_stream *stream,
                            struct string_id decl)
{
    struct pinapl_instruction instruction = {0};
    instruction.arm = ARM_GLOBAL;
    instruction.op1.type = ARM_OPERAND_DECLARATION;
    instruction.op1.strid = decl;

    pinapl_arm_push_instruction(stream, instruction);

}

uint32 pinapl_arm_data_processing_instruction_to_binary(struct pinapl_instruction *instruction, uint32 opcode)
{
    uint32 binary = 0;

    b32 immediate_operand = (instruction->op3.type == ARM_OPERAND_IMMEDIATE_VALUE);
    b32 set_condition_codes = instruction->arm & ARM_S;

    binary |= (immediate_operand << 25); 
    binary |= (opcode << 21);
    binary |= (set_condition_codes << 20);
    binary |= (instruction->op2.value << 16);
    binary |= (instruction->op1.value << 12);

    if (immediate_operand)
    {
        uint32 rot = 0;
        uint32 imm = instruction->op3.value;
        binary |= ((rot & 0xf) << 8);
        binary |= (imm & 0xff);
    }
    else
    {
        uint32 shift = 0;
        uint32 rm = instruction->op3.value;
        binary |= ((shift & 0xff) << 4);
        binary |= (rm & 0xf);
    }

    return binary;
}

uint32 pinapl_arm_instruction_to_binary(struct pinapl_instruction_stream *stream, int instruction_index_, struct pinapl_instruction *instruction)
{
    uint32 binary = 0;
    uint32 condition = 0xe0000000;

    enum pinapl_arm_instruction instr = instruction->arm & ARM_INSTRUCTION_MASK;
    switch (instr)
    {
        case ARM_INVALID_INSTRUCTION:
            break;

        case ARM_LABEL:
        case ARM_SECTION:
        case ARM_GLOBAL:
        break;

        case ARM_NOP:
        break;

        case ARM_MOV:
        {
            // Patch mov instruction
            instruction->op3 = instruction->op2;
            SET_ZERO(instruction->op2);

            u32 opcode = 0xd;
            binary = condition | pinapl_arm_data_processing_instruction_to_binary(instruction, opcode);
        }
        break;

        case ARM_ADD:
        {
            u32 opcode = 0x4;
            binary = condition | pinapl_arm_data_processing_instruction_to_binary(instruction, opcode);
        }
        break;

        case ARM_SUB:
        {
            u32 opcode = 0x2;
            binary = condition | pinapl_arm_data_processing_instruction_to_binary(instruction, opcode);
        }
        break;

        case ARM_MUL:
        case ARM_MLA:
        {
            //
            // Rd := Rm * Rs + Rn
            //

            b32 accumulate = (instr == ARM_MLA);
            b32 set_condition_code = 0;
            
            binary |= condition;
            binary |= (accumulate << 21);
            binary |= (set_condition_code << 20);
            binary |= (instruction->op1.value << 16); // Rd - destination register
            binary |= (instruction->op4.value << 12); // Rn - Operand register
            binary |= (instruction->op3.value << 8);  // Rs - Operand register
            binary |= (0x9 << 4);
            binary |= (instruction->op2.value);       // Rm - Operand register
        }
        break;
        
        case ARM_DIV:
        {
        }
        break;

        case ARM_B:
        case ARM_BL:
        {
            b32 link_bit = (instr == ARM_BL);

            int32 instruction_index = 0;
            for (int idx = 0; idx < stream->label_count; idx++)
            {
                if (stream->labels[idx].id == instruction->op1.label.id)
                {
                    instruction_index = idx;
                    break;
                }
            }
#define ARM_PC_PREFETCH_COUNT 2
            int32 offset = instruction_index - instruction_index_ - ARM_PC_PREFETCH_COUNT;

            binary |= condition;
            binary |= (0x5 << 25);
            binary |= (link_bit << 24);
            binary |= (0x00ffffff & offset);
        }
        break;

        case ARM_BX:
        {
            binary = condition;
            binary |= (0x12fff1 << 4);
            binary |= (instruction->op1.value);
        }
        break;
        
        case ARM_LDR:
        case ARM_STR:
        {
            b32 is_load = (instr == ARM_LDR);
            b32 offset_is_a_register = 0;
            b32 pre_indexing_bit = 1;
            b32 up_bit = 1;
            b32 byte_bit = 0;
            b32 write_back_bit = 0;
            
            binary = condition;
            binary |= (1 << 26);
            binary |= (offset_is_a_register << 25);
            binary |= (pre_indexing_bit << 24);
            binary |= (up_bit << 23);
            binary |= (byte_bit << 22);
            binary |= (write_back_bit << 21);
            binary |= (is_load << 20);
            binary |= (instruction->op2.value << 16);
            binary |= (instruction->op1.value << 12);
        }
        break;

        case ARM_SVC:
        {
            if (instruction->op1.type == ARM_OPERAND_IMMEDIATE_VALUE)
            {
                binary = condition | 0x0f000000 | (instruction->op1.value & 0x00ffffff);
            }
        }
        break;

        default:
        break;
    }

    return binary;
}

void pinapl_arm_dump_elf(char const *filename,
                         struct pinapl_instruction_stream *stream,
                         struct allocator *allocator)
{
    struct elf_header *header = ALLOCATE(allocator, struct elf_header);
    
    header->e_ident[ELF_IDENT_MAGIC0]  = 0x7f;
    header->e_ident[ELF_IDENT_MAGIC1]  = 'E';
    header->e_ident[ELF_IDENT_MAGIC2]  = 'L';
    header->e_ident[ELF_IDENT_MAGIC3]  = 'F';
    header->e_ident[ELF_IDENT_CLASS]   = ELF_CLASS_32;
    header->e_ident[ELF_IDENT_DATA]    = ELF_DATA_2LSB;
    header->e_ident[ELF_IDENT_VERSION] = 1;
    header->e_ident[ELF_IDENT_PAD]     = 0;

    header->e_type = ET_EXEC;
    header->e_machine = EM_ARM;
    header->e_version = EV_CURRENT;
    header->e_entry = 0;
    header->e_phoff = sizeof(struct elf_header);
    header->e_shoff = 0;
    header->e_flags = EF_ARM_ABI_CURRENT_VERSION | EF_ARM_ABI_FLOAT_SOFT;
    header->e_ehsize = sizeof(struct elf_header);
    header->e_phentsize = sizeof(struct elf_program_header);
    header->e_phnum = 1;
    header->e_shentsize = sizeof(struct elf_section_header);
    header->e_shnum = 3;
    header->e_shstrndx = 2;

    struct elf_program_header *program_header = ALLOCATE(allocator, struct elf_program_header);
    program_header->p_type = ELF_PT_LOAD;
    program_header->p_offset = 0;
    program_header->p_vaddr = 0x10000;
    program_header->p_paddr = 0x10000;
    program_header->p_filesz = 0;
    program_header->p_memsz = 0;
    program_header->p_flags = ELF_PF_X | ELF_PF_R;
    program_header->p_align = 0x10000;

    usize code_offset = allocator->used;

    uint32 instructions_pushed = 0;
    for (int instruction_index = 0; instruction_index < stream->instruction_count; instruction_index++)
    {
        struct pinapl_instruction *instruction = stream->instructions + instruction_index;

        if (instruction->arm == ARM_LABEL)
        {
            if (instruction->op1.label.id == STRID("_start").id)
            {
                header->e_entry = program_header->p_vaddr + allocator->used;
            }

            stream->labels[stream->label_count] = instruction->op1.label;
            stream->label_instruction_index[stream->label_count] = instructions_pushed;
            stream->label_count += 1;

            continue;
        }

        if (instruction->arm == ARM_SECTION ||
            instruction->arm == ARM_GLOBAL)
        {
            continue;
        }

        uint32 binary = pinapl_arm_instruction_to_binary(stream, instructions_pushed, instruction);
        uint32 *code = ALLOCATE(allocator, uint32);
        *code = binary;
        instructions_pushed += 1;
    }

    usize code_size = allocator->used - code_offset;

    char buffer[] = "\0.text\0.shstrtab\0";
    usize strbuf_size = sizeof(buffer);
    usize strbuf_offset = allocator->used;

    char *string_buffer = ALLOCATE_BUFFER(allocator, sizeof(buffer));
    memcpy(string_buffer, buffer, sizeof(buffer));

    program_header->p_filesz = allocator->used;
    program_header->p_memsz  = allocator->used;

    struct elf_section_header *section_header_zero = ALLOCATE(allocator, struct elf_section_header);
    UNUSED(section_header_zero);
    
    usize sh_offset = (usize) allocator->used - sizeof(struct elf_section_header);
    header->e_shoff = sh_offset;
    
    struct elf_section_header *section_header_text = ALLOCATE(allocator, struct elf_section_header);
    section_header_text->sh_name = 1;
    section_header_text->sh_type = ELF_SHT_PROGBITS;
    section_header_text->sh_flags = ELF_SHF_ALLOC | ELF_SHF_EXECINSTR;
    section_header_text->sh_addr = 0x10000 + code_offset;
    section_header_text->sh_offset = code_offset;
    section_header_text->sh_size = code_size;
    section_header_text->sh_link = 0;
    section_header_text->sh_info = 0;
    section_header_text->sh_addralign = 4;
    section_header_text->sh_entsize = 0;

    struct elf_section_header *section_header_shstr = ALLOCATE(allocator, struct elf_section_header);
    section_header_shstr->sh_name = 7;
    section_header_shstr->sh_type = ELF_SHT_STRTAB;
    section_header_shstr->sh_flags = 0;
    section_header_shstr->sh_addr = 0;
    section_header_shstr->sh_offset = strbuf_offset;
    section_header_shstr->sh_size = strbuf_size;
    section_header_shstr->sh_link = 0;
    section_header_shstr->sh_info = 0;
    section_header_shstr->sh_addralign = 0;
    section_header_shstr->sh_entsize = 0;

    int fd = open(filename, O_CREAT | O_WRONLY, 0755);
    write(fd, allocator->memory, allocator->used);
    close(fd);
}

/*
enum elf_section_header_flags
{
    ELF_SHF_WRITE     = 0x1, // The section contains the data that should be writable during process execution
    ELF_SHF_ALLOC     = 0x2, // The section occupies memory during process execution
    ELF_SHF_EXECINSTR = 0x4, // The section contains executable machine instructions
    ELF_SHF_MASKPROC  = 0xf0000000, // All bits included in this mask are reserved for processor-specific semantics
};
*/
//
// The two members of the section header, 'sh_link' and 'sh_info' hold special information
// depending on section type:
//
// if sh_type == ELF_SHT_DYNAMIC
//    sh_link = The section header index of the string table used by entries in the section.
//    sh_info = 0;
//
// if sh_type == ELF_SHT_HASH
//    sh_link = The section header index of the symbol table to which the hash table applies.
//    sh_info = 0;
//
// if sh_type == ELF_SHT_REL || sh_type == ELF_SHT_RELA
//    sh_link = The section header index of the associated symbol table.
//    sh_info = The section header index of the section to which the relocation applies.
// 
// if sh_type == ELF_SHT_SYMTAB || sh_type == ELF_SHT_DYNSYM
//    sh_link = The section header index of the associated string table.
//    sh_info = One greater than the symbol table index of the last local symbol (binding STB_LOCAL)
//
// else
//    sh_link = ELF_SHN_UNDEF
//    sh_info = 0
//
    // struct elf_section_header section_header_text;
    

