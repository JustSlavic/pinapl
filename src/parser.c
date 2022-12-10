#include "parser.h"


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


char pinapl_get_char(struct pinapl_lexer *l)
{
    char c = 0;
    if (l->index < l->buffer_size)
    {
        c = l->buffer[l->index];
    }
    return c;
}

char pinapl_eat_char(struct pinapl_lexer *l)
{
    char c = pinapl_get_char(l);
    l->index += 1;
    if (c == '\n')
    {
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

                result = ALLOCATE(&parser->ast_allocator, ast_node);
                result->type = AST_NODE_FUNCTION_CALL;
                result->function_call.name = t;
                result->function_call.argument_list = NULL;
            }
        }
        else
        {
            result = ALLOCATE(&parser->ast_allocator, ast_node);
            result->type = AST_NODE_VARIABLE;
            result->var_span = t.span;
            result->var_span_size = t.span_size;
        }
    }
    else if (t.type == TOKEN_LITERAL_INT)
    {
        pinapl_eat_token(parser);

        result = ALLOCATE(&parser->ast_allocator, ast_node);
        result->type = AST_NODE_LITERAL_INT;
        result->literal_span = t.span;
        result->literal_span_size = t.span_size;
        result->integer_value = t.integer_value;
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
            // @leak left_operand
            return NULL;
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
            return NULL;
        }

        ast_node *binary_operator = ALLOCATE(&parser->ast_allocator, ast_node);
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

                        result = ALLOCATE(&parser->ast_allocator, ast_node);
                        result->type = AST_NODE_VARIABLE_DECLARATION;
                        result->variable_declaration.var_name = var_name;
                        result->variable_declaration.var_type = type;
                        result->variable_declaration.init = NULL;
                    }
                    else
                    {
                        // error: expected ':', '=', or ';'
                    }
                }
                else
                {
                    // error: I do not allow other types other than 'int' YET
                }
            }
            else
            {
                should_init = false;
                // error: expected '-', ':', or a type
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
                    result = ALLOCATE(&parser->ast_allocator, ast_node);
                    result->type = is_constant 
                        ? AST_NODE_CONSTANT_DECLARATION
                        : AST_NODE_VARIABLE_DECLARATION;
                    result->variable_declaration.var_name = var_name;
                    result->variable_declaration.var_type = type;
                    result->variable_declaration.init = initializer;
                }
                else
                {
                    // error: expression expected (probably should print error, that comes from that call
                }
            }
        }
        else
        {
            // error: token ':' expected
        }
    }
    else
    {
        // error: variable declaration is not started with identifier
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
            result = ALLOCATE(&parser->ast_allocator, ast_node);
            result->type = AST_NODE_BLOCK;
            result->block.statement_list = statement_list;
        }
        else
        {
            // error: expected '}'
        }
    }
    else
    {
        // error: expected '{'
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
                result = ALLOCATE(&parser->ast_allocator, ast_node);
                result->type = AST_NODE_FUNCTION_DEFINITION;
                result->function_definition.parameter_list = NULL; 
                result->function_definition.return_type = NULL;
                result->function_definition.block = block; 
            }
            else
            {
                // Do I allow forward-declarations for functions in pinapl ?
            }
        }
        else
        {
            // Error: expected ')'
        }
    }
    else
    {
        // Error: expected '('
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
            // @error!!! I don't know what it is!
        }
    }

    token semicolon = pinapl_get_token(parser);
    if (semicolon.type == ';')
    {
        pinapl_eat_token(parser);
    }
    else
    {
        // Error! statement should end with a semicolon!
        // @leak
        result = NULL;
    }

    return result;
}

ast_node *pinapl_parse_statement_list(struct pinapl_parser *parser)
{
    ast_node *result = NULL;

    ast_node *first_statement = pinapl_parse_statement(parser);
    if (first_statement)
    {
        result = ALLOCATE(&parser->ast_allocator, ast_node);
        result->type = AST_NODE_STATEMENT_LIST;
        result->statement_list.node = first_statement;
        result->statement_list.next = NULL;

        ast_node *last_list_node = result;
        
        while (true)
        {
            ast_node *statement = pinapl_parse_statement(parser);
            if (statement)
            {
                ast_node *new_list_node = ALLOCATE(&parser->ast_allocator, ast_node);
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
        result = ALLOCATE(&parser->ast_allocator, ast_node);
        result->type = AST_NODE_GLOBAL_DECLARATION_LIST;
        result->global_list.node = first_declaration;
        result->global_list.next = NULL;

        ast_node *last_list_node = result;
        while (true)
        {
            ast_node *declaration = pinapl_parse_global_declaration(parser);
            if (declaration)
            {
                ast_node *next_list_node = ALLOCATE(&parser->ast_allocator, ast_node);
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

