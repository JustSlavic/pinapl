#include "parser.h"


struct ast_node *parse_expression_operand(struct parser *parser);
struct ast_node *parse_expression(struct parser *parser, int precedence);
struct ast_node *parse_type(struct parser *parser);
struct ast_node *parse_declaration(struct parser *parser);
struct ast_node *parse_statement(struct parser *parser);
struct ast_node *parse_statements(struct parser *parser);
struct ast_node *parse_block(struct parser *parser);


struct token get_token(struct parser *parser)
{
    struct token result = {};
    if (parser->cursor < parser->token_count)
    {
        result = parser->token_stream[parser->cursor];
    }
    return result;
}

struct token eat_token(struct parser *parser)
{
    struct token result = get_token(parser);
    parser->cursor += 1;
    return result;
}

struct ast_node *make_new_ast_node(struct parser *parser)
{
    struct ast_node *result = NULL;
    if (parser->ast_node_count < parser->ast_buffer_size)
    {
        result = parser->ast + parser->ast_node_count;
        parser->ast_node_count += 1;
    }
    return result;
}

struct ast_node *parse_expression_operand(struct parser *parser)
{
    struct ast_node *result = NULL;

    struct token t = get_token(parser);
    if (t.type == TOKEN_IDENTIFIER)
    {
        eat_token(parser);

        struct token open_paren = get_token(parser);
        if (open_paren.type == '(')
        {
            // Function call
            eat_token(parser);

            struct token close_paren = get_token(parser);
            if (close_paren.type == ')')
            {
                eat_token(parser);

                result = make_new_ast_node(parser);
                if (result)
                {
                    result->kind = AST__FUNCTION_CALL;
                    result->function_call.name = t.span;
                }
            }
            else
            {
                struct ast_node *arg1 = parse_expression(parser, 0);
                if (arg1 != NULL)
                {
                    close_paren = get_token(parser);
                    if (close_paren.type == ')')
                    {
                        eat_token(parser);

                        result = make_new_ast_node(parser);
                        if (result)
                        {
                            result->kind = AST__FUNCTION_CALL;
                            result->function_call.name = t.span;
                            result->function_call.arg1 = arg1;
                        }
                    }
                }
            }
        }
        else
        {
            // Variable
            result = make_new_ast_node(parser);
            if (result)
            {
                result->kind = AST__VARIABLE;
                result->variable.name = t.span;
            }
        }
    }
    else if (t.type == TOKEN_LITERAL_INT)
    {
        eat_token(parser);

        result = make_new_ast_node(parser);
        if (result)
        {
            result->kind = AST__LITERAL_INT;
            result->literal_int.value = t.integer_value;
        }
    }
    else
    {
        // pinapl_report_compiler_error_cstring(parser, "Error 1.\n");
        // pinapl_report_highlight_token(parser, t);
    }

    return result;
}


struct ast_node *parse_expression(struct parser *parser, int precedence)
{
    struct ast_node *left_operand = NULL;

    struct token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);
        left_operand = parse_expression(parser, 0);
        if (left_operand == NULL) { return NULL; }

        struct token close_paren = get_token(parser);
        if (close_paren.type == ')') { eat_token(parser); }
        else { return NULL; }
    }
    else
    {
        left_operand = parse_expression_operand(parser);
        if (left_operand == NULL) { return NULL; }
    }

    while (true)
    {
        struct token operator = get_token(parser);
        if ((operator.type != '+') &&
            (operator.type != '-') &&
            (operator.type != '*') &&
            (operator.type != '/') &&
            (operator.type != '=')) { break; }

        int operator_precedence = get_precedence(operator);
        if (operator_precedence < precedence) { break; }

        eat_token(parser);

        // +1 for left associativity, +0 for right associativity
        struct ast_node *right_operand = parse_expression(parser, operator_precedence + 1);
        if (right_operand == NULL) { return NULL; }

        struct ast_node *bop_node = make_new_ast_node(parser);
        bop_node->kind = AST__BINARY_OPERATOR;
        bop_node->binary_operator.operator = operator.type;
        bop_node->binary_operator.lhs      = left_operand;
        bop_node->binary_operator.rhs      = right_operand;

        left_operand = bop_node;
    }

    return left_operand;
}

struct ast_node *parse_type(struct parser *parser)
{
    struct ast_node *result = NULL;

    struct token t = get_token(parser);
    if (t.type == TOKEN_IDENTIFIER)
    {
        eat_token(parser);
        result = make_new_ast_node(parser);
        result->kind = AST__TYPE;
    }

    return result;
}

struct ast_node *parse_declaration(struct parser *parser)
{
    // x :: <expr>;
    // x := <expr>;
    // x : int;
    // x : int : <expr>;
    // x : int = <expr>;
    struct ast_node *result = NULL;

    struct token name = eat_token(parser);
    if (name.type != TOKEN_IDENTIFIER)
    {
        // @todo: report error
    }
    else
    {
        struct token colon = eat_token(parser);
        if (colon.type != ':')
        {
            // @todo: report error
        }
        else
        {
            bool32 is_constant = false;
            bool32 should_init = false;
            struct ast_node *type = NULL;
            struct ast_node *initializer = NULL;

            struct token t = get_token(parser);
            if (t.type == '=') // <ident> := <expr>
            {
                eat_token(parser);
                is_constant = false;
                should_init = true;
            }
            else if (t.type == ':') // <ident> :: <expr>
            {
                eat_token(parser);
                is_constant = true;
                should_init = true;
            }
            else
            {
                type = parse_type(parser);
                if (type != NULL)
                {
                    struct token t2 = get_token(parser);
                    if (t2.type == '=') // <ident> : int = <expr>
                    {
                        eat_token(parser);
                        is_constant = false;
                        should_init = true;
                    }
                    else if (t2.type == ':') // <ident> : int : <expr>
                    {
                        eat_token(parser);
                        is_constant = true;
                        should_init = true;
                    }
                    else if (t2.type == ';') // x : int
                    {
                        // @note: do not eat semicolon,
                        // because it is going to be eaten
                        // in the 'parse_statement'
                        is_constant = false;
                        should_init = false;
                    }
                    else
                    {
                        // @todo: report error
                    }
                }
            }

            if (should_init)
            {
                initializer = parse_expression(parser, 0);
            }

            struct token semicolon = get_token(parser);
            if (semicolon.type == ';')
            {
                result = make_new_ast_node(parser);
                result->kind = AST__DECLARATION;
                result->declaration.is_constant = is_constant;
                result->declaration.name = name.span;
                result->declaration.type = type;
                result->declaration.init = initializer;
            }
        }
    }

    return result;
}

struct ast_node *parse_statement(struct parser *parser)
{
    usize saved = parser->cursor;
    struct ast_node *result = NULL;

    struct token t = get_token(parser);
    if (t.type == '{')
    {
        struct ast_node *block = parse_block(parser);
        if (block != NULL)
        {
            result = make_new_ast_node(parser);
            result->kind = AST__STATEMENT;
            result->statement.stmt = block;
            result->statement.next = NULL;
        }
    }
    else
    {
        struct ast_node *ast = parse_declaration(parser);
        if (ast == NULL)
        {
            parser->cursor = saved;

            ast = parse_expression(parser, 0);
            if (ast == NULL)
            {
                parser->cursor = saved;
            }
        }

        if (ast)
        {
            struct token semicolon = get_token(parser);
            if (semicolon.type == ';')
            {
                eat_token(parser);
                result = make_new_ast_node(parser);
                if (result)
                {
                    result = make_new_ast_node(parser);
                    result->kind = AST__STATEMENT;
                    result->statement.stmt = ast;
                    result->statement.next = NULL;
                }
            }
        }
    }

    return result;
}

struct ast_node *parse_statements(struct parser *parser)
{
    struct ast_node *result = parse_statement(parser);
    struct ast_node *stmt = result;
    while (stmt != NULL)
    {
        stmt->statement.next = parse_statement(parser);
        stmt = stmt->statement.next;
    }

    return result;
}

struct ast_node *parse_block(struct parser *parser)
{
    struct ast_node *result = NULL;

    struct token open_brase = get_token(parser);
    if (open_brase.type == '{')
    {
        eat_token(parser);
        struct ast_node *stmts = parse_statements(parser);

        struct token close_brace = get_token(parser);
        if (close_brace.type == '}')
        {
            eat_token(parser);

            result = make_new_ast_node(parser);
            result->kind = AST__BLOCK;
            result->block.statements = stmts;
        }
    }

    return result;
}

bool32 debug_print_ast(struct ast_node *ast, int depth)
{
    char spaces[] = "                                                 ";
    bool32 newlined = false;

    switch (ast->kind)
    {
        case AST__BINARY_OPERATOR:
        {
            printf("(%c)----", ast->binary_operator.operator);
            newlined = debug_print_ast(ast->binary_operator.rhs, depth+1);
            if (!newlined) printf("\n");
            printf("%.*s\\---", 4*depth + 3*(depth + 1), spaces);
            newlined = debug_print_ast(ast->binary_operator.lhs, depth+1);
            if (!newlined)
            {
                printf("\n");
                newlined = true;
            }
        }
        break;

        case AST__DECLARATION:
        {
            printf("%s---", ast->declaration.is_constant ? "(::)" : "(:=)");
            if (ast->declaration.init != NULL)
                debug_print_ast(ast->declaration.init, depth + 1);
            else
                printf("null");
            printf("%.*s \\---", 4*depth + 3*(depth + 1), spaces);
            if (ast->declaration.type != NULL)
                debug_print_ast(ast->declaration.type, depth + 1);
            else
                printf("null");
            if (!newlined)
            {
                printf("\n");
                newlined = true;
            }
        }
        break;

        case AST__FUNCTION_CALL:
        {
            printf("fn()---");
            if (ast->function_call.arg1 != NULL)
                newlined = debug_print_ast(ast->function_call.arg1, depth + 1);
            else
                printf("null");
            if (!newlined)
            {
                printf("\n");
                newlined = true;
            }
        }
        break;

        case AST__STATEMENT:
        {
            printf("stmt---");
            newlined = debug_print_ast(ast->statement.stmt, depth + 1);
            if (ast->statement.next != NULL)
            {
                printf("%.*s", 7*depth, spaces);
                newlined = debug_print_ast(ast->statement.next, depth);
            }
        }
        break;

        case AST__BLOCK:
        {
            printf("blk----");
            newlined = debug_print_ast(ast->block.statements, depth + 1);
        }
        break;

        case AST__LITERAL_INT:
        {
            printf("lit ");
        }
        break;

        case AST__VARIABLE:
        {
            printf("var ");
        }
        break;

        case AST__TYPE:
        {
            printf("type");
        }
        break;

        default:
            printf("Invalid ast\n");
        break;
    }

    return newlined;
}

