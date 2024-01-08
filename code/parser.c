#include "parser.h"


bool32 type_entries_equal(struct type_registry_entry *e1, struct type_registry_entry *e2)
{
    bool32 result = (e1->kind == e2->kind);
    if (result && (e1->kind == TYPE__NAME))
    {
        if (e1->name.size == e2->name.size)
        {
            for (int i = 0; i < e1->name.size; i++)
            {
                if (e1->name.data[i] != e2->name.data[i])
                {
                    result = false;
                    break;
                }
            }
        }
        else
        {
            result = false;
        }
    }
    else if (result && (e1->kind == TYPE__TUPLE))
    {
        if (e1->tuple_count == e2->tuple_count)
        {
            for (int i = 0; i < e1->tuple_count; i++)
            {
                result = result && type_entries_equal(e1->tuple[i], e2->tuple[i]);
                if (result == false) break;
            }
        }
        else
        {
            result = false;
        }
    }

    return result;
}

struct type_registry_entry *register_type_entry(struct type_registry *registry, struct type_registry_entry *entry_to_register)
{
    struct type_registry_entry *entry = NULL;
    for (int i = 0; i < registry->count; i++)
    {
        if (type_entries_equal(entry_to_register, registry->entries + i))
        {
            entry = registry->entries + i;
            break;
        }
    }
    if (entry == NULL)
    {
        if (registry->count < ARRAY_COUNT(registry->entries))
        {
            entry = registry->entries + registry->count++;
            *entry = *entry_to_register;
        }
    }

    return entry;
}


struct ast_node *parse_expression_operand(struct parser *parser);
struct ast_node *parse_expression(struct parser *parser, int precedence);
struct ast_node *parse_declaration(struct parser *parser);
struct ast_node *parse_statement(struct parser *parser);
struct ast_node *parse_statements(struct parser *parser);
struct ast_node *parse_block(struct parser *parser);
struct ast_node *parse_function(struct parser *parser);
struct type_registry_entry *parse_type(struct parser *parser);


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

struct ast_node *parse_tuple(struct parser *parser)
{
    struct ast_node *result = NULL;

    struct token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);

        struct ast_node *value1 = parse_expression(parser, 0);

        struct token comma = get_token(parser);
        if (comma.type == ')')
        {
            eat_token(parser);
            result = value1;
        }
        else if (comma.type == ',')
        {
            struct ast_node *tuple1 = make_new_ast_node(parser);
            tuple1->kind = AST__TUPLE;
            tuple1->tuple.value = value1;
            tuple1->tuple.next = NULL;

            struct ast_node *tuple_last = tuple1;
            while (true)
            {
                comma = get_token(parser);
                if (comma.type == ',')
                {
                    eat_token(parser);

                    struct ast_node *value2 = parse_expression(parser, 0);

                    struct token possible_close_paren = get_token(parser);
                    if (possible_close_paren.type == ')')
                    {
                        // Last expression in the tuple, I can skip 1 node
                        tuple_last->tuple.next = value2;
                    }
                    else
                    {
                        struct ast_node *tuple2 = make_new_ast_node(parser);
                        tuple2->kind = AST__TUPLE;
                        tuple2->tuple.value = value2;
                        tuple2->tuple.next = NULL;

                        tuple_last->tuple.next = tuple2;
                        tuple_last = tuple2;
                    }
                }
                else if (comma.type == ')')
                {
                    eat_token(parser);
                    break;
                }
                else
                {
                    break;
                }
            }

            result = tuple1;
        }
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
        // Try to parse tuple
        usize saved = parser->cursor;
        struct ast_node *tuple = parse_tuple(parser);
        if (tuple == NULL)
        {
            parser->cursor = saved;
            eat_token(parser);
            left_operand = parse_expression(parser, 0);
            if (left_operand == NULL) { return NULL; }

            struct token close_paren = get_token(parser);
            if (close_paren.type == ')') { eat_token(parser); }
            else { return NULL; }
        }
        else
        {
            left_operand = tuple;
        }
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

struct type_registry_entry *parse_type(struct parser *parser)
{
    struct type_registry_entry *result = NULL;

    struct token t = get_token(parser);
    if (t.type == TOKEN_IDENTIFIER)
    {
        eat_token(parser);

        // @speed
        struct type_registry_entry entry_to_register = {
            .kind = TYPE__NAME,
            .name = t.span,
        };
        result = register_type_entry(&parser->types, &entry_to_register);
    }
    else if (t.type == '(')
    {
        eat_token(parser);

        struct token close_paren = get_token(parser);
        if (close_paren.type == ')')
        {
            eat_token(parser);
            result = &parser->types.entries[0];
        }
        else
        {
            struct type_registry_entry entry_to_register = {
                .kind = TYPE__TUPLE,
            };
            struct type_registry_entry *entry1 = parse_type(parser);
            if (entry1 != NULL)
            {
                struct token comma = get_token(parser);
                if (comma.type == ')')
                {
                    eat_token(parser);
                    result = entry1;
                }
                else if (comma.type == ',')
                {
                    entry_to_register.tuple[0] = entry1;
                    entry_to_register.tuple_count += 1;

                    while (true)
                    {
                        struct token comma = get_token(parser);
                        if (comma.type == ',')
                        {
                            eat_token(parser);

                            struct type_registry_entry *entry2 = parse_type(parser);
                            if (entry2 != NULL)
                                if (entry_to_register.tuple_count < ARRAY_COUNT(entry_to_register.tuple))
                                    entry_to_register.tuple[entry_to_register.tuple_count++] = entry2;
                        }
                        else if (comma.type == ')')
                        {
                            eat_token(parser);
                            break;
                        }
                    }

                    struct type_registry_entry *tuple_entry = register_type_entry(&parser->types, &entry_to_register);
                    result = tuple_entry;
                }
                else
                {
                    // @todo: report error
                }
            }
        }

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

            struct ast_node *initializer = NULL;
            struct type_registry_entry *type = NULL;

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
                struct token open_paren = get_token(parser);
                if (open_paren.type == '(')
                {
                    initializer = parse_function(parser);
                }
                if (!initializer)
                {
                    initializer = parse_expression(parser, 0);

                    struct token semicolon = get_token(parser);
                    if (semicolon.type == ';')
                    {
                        eat_token(parser);
                    }
                    else
                    {
                        // @todo: report error
                    }
                }
            }

            result = make_new_ast_node(parser);
            result->kind = AST__DECLARATION;
            result->declaration.is_constant = is_constant;
            result->declaration.name = name.span;
            result->declaration.type = type;
            result->declaration.init = initializer;
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
            else
            {
                struct token semicolon = get_token(parser);
                if (semicolon.type == ';')
                {
                    eat_token(parser);

                }
            }
        }

        if (ast)
        {
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

struct ast_node *parse_function(struct parser *parser)
{
    struct ast_node *result = NULL;

    struct token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);

        struct token close_paren = get_token(parser);
        if (close_paren.type == ')')
        {
            eat_token(parser);

            struct ast_node *body = parse_block(parser);

            result = make_new_ast_node(parser);
            if (result)
            {
                result->kind = AST__FUNCTION;
                result->function.body = body;
            }
        }
    }

    return result;
}

void debug_print_type(struct type_registry_entry *type)
{
    switch (type->kind)
    {
        case TYPE__VOID:
        {
            printf("type");
        }
        break;

        case TYPE__NAME:
        {
            printf("type");
        }
        break;

        case TYPE__TUPLE:
        {
            printf("type");
        }
        break;

        case TYPE__FUNCTION:
        {
            printf("type");
        }
        break;
    }
}

bool32 debug_print_ast(struct ast_node *ast, int depth)
{
    char spaces[] = "                                                                                                                 ";
    bool32 newlined = false;

#define DO_NEWLINE if (!newlined) { printf("\n"); newlined = true; }

    switch (ast->kind)
    {
        case AST__BINARY_OPERATOR:
        {
            printf("(%c)----", ast->binary_operator.operator);
            newlined = debug_print_ast(ast->binary_operator.rhs, depth+1);
            DO_NEWLINE
            printf("%.*s\\---", 4*(depth + 1) + 3*depth - 1, spaces);
            newlined = debug_print_ast(ast->binary_operator.lhs, depth+1);
            DO_NEWLINE
        }
        break;

        case AST__DECLARATION:
        {
            printf("%s---", ast->declaration.is_constant ? "(::)" : "(:=)");
            if (ast->declaration.type != NULL)
            {
                debug_print_type(ast->declaration.type);
                DO_NEWLINE
            }
            else
            {
                printf("infr\n");
                newlined = true;
            }
            printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
            if (ast->declaration.init != NULL)
            {
                debug_print_ast(ast->declaration.init, depth+1);
                printf("\n");
                newlined = true;
            }
            else
            {
                printf("null\n");
                newlined = true;
            }
            DO_NEWLINE
        }
        break;

        case AST__FUNCTION_CALL:
        {
            printf("fn()---");
            if (ast->function_call.arg1 != NULL)
                newlined = debug_print_ast(ast->function_call.arg1, depth + 1);
            else
                printf("null");
            DO_NEWLINE
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

        case AST__TUPLE:
        {
            printf("tupl");
            if (ast->tuple.next)
            {
                printf("---");
                newlined = debug_print_ast(ast->tuple.next, depth + 1);
            }
            DO_NEWLINE
            printf("%.*s\\--", 4*depth + 3*(depth + 1), spaces);
            newlined = debug_print_ast(ast->tuple.value, depth);
            DO_NEWLINE
        }
        break;

        case AST__FUNCTION:
        {
            printf("func");
            DO_NEWLINE
            printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
            newlined = debug_print_ast(ast->function.body, depth + 1);
        }
        break;

        default:
            printf("Invalid ast\n");
        break;
    }

    return newlined;
}

