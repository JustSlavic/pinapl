#include "parser.h"
#include <string.h>


GLOBAL char const *spaces = "                                             ";
GLOBAL char const *carets = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";


char *token_type_to_cstring(token_type t)
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
                t.type = (token_type) c;
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
        left_operand = pinapl_parse_expression(parser, precedence);
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
                        result->variable_declaration.var_name = var_name;
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
                    parser->lexer = checkpoint;
                    initializer = pinapl_parse_function_definition(parser);
                }

                if (initializer)
                {
                    result = ALLOCATE(parser->ast_allocator, ast_node);
                    result->type = is_constant 
                        ? AST_NODE_CONSTANT_DECLARATION
                        : AST_NODE_VARIABLE_DECLARATION;
                    result->variable_declaration.var_name = var_name;
                    result->variable_declaration.var_type = type;
                    result->variable_declaration.init = initializer;
                }
                else
                {
                    pinapl_report_compiler_error_cstring(parser, "Error: expression expected (probably should print error, that comes from that call.\n");
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

ast_node *pinapl_parse_statement(struct pinapl_parser *parser)
{
    ast_node *result = NULL;
    struct pinapl_lexer checkpoint = parser->lexer;

    ast_node *variable_declaration = pinapl_parse_variable_declaration(parser);
    if (variable_declaration)
    {
        result = variable_declaration;
    }
    else
    {
        parser->lexer = checkpoint;
        ast_node *expression = pinapl_parse_expression(parser, 0);
        if (expression)
        {
            result = expression;
        }
        else
        {
            pinapl_report_compiler_error_cstring(parser, "Error!!! I don't know what it is!\n");
        }
    }

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

    return result;
}

ast_node *pinapl_parse_statement_list(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    ast_node *first_statement = pinapl_parse_statement(parser);
    if (first_statement)
    {
        result = ALLOCATE(parser->ast_allocator, ast_node);
        result->type = AST_NODE_STATEMENT_LIST;
        result->statement_list.node = first_statement;
        result->statement_list.next = NULL;

        ast_node *last_list_node = result;
        
        while (true)
        {
            ast_node *statement = pinapl_parse_statement(parser);
            if (statement)
            {
                ast_node *new_list_node = ALLOCATE(parser->ast_allocator, ast_node);
                new_list_node->type = AST_NODE_STATEMENT_LIST;
                new_list_node->statement_list.node = statement;
                new_list_node->statement_list.next = NULL;

                last_list_node->statement_list.next = new_list_node;
                last_list_node = new_list_node;
            }
            else
            {
                break;
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
        result->global_list.node = first_declaration;
        result->global_list.next = NULL;

        ast_node *last_list_node = result;
        while (true)
        {
            ast_node *declaration = pinapl_parse_global_declaration(parser);
            if (declaration)
            {
                ast_node *next_list_node = ALLOCATE(parser->ast_allocator, ast_node);
                next_list_node->type = AST_NODE_GLOBAL_DECLARATION_LIST;
                next_list_node->global_list.node = declaration;
                next_list_node->global_list.next = NULL;

                last_list_node->global_list.next = next_list_node;
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

