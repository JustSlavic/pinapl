#include "parser.hpp"
#include <hash.hpp>


namespace pinapl {


ast_node ast_node::make_type(type_id_t id)
{
    ast_node result = {};
    result.kind = AST_NODE__TYPE;
    result.m_type.index = id;

    return result;
}

ast_node ast_node::make_variable(string_view name)
{
    ast_node result = {};
    result.kind = AST_NODE__VARIABLE;
    result.m_variable.name = name;

    return result;
}

ast_node ast_node::make_int_literal(int64 n)
{
    ast_node result = {};
    result.kind = AST_NODE__INT_LIT;
    result.m_int_lit.n = n;

    return result;
}


type_id_t parser::reg_type(type t)
{
    uint32 hash = t.count == 1 ? t.hash[0]
        : hash_djb2((byte *) t.hash, sizeof(t.hash));

    for (uint32 offset = 0; offset < TYPES_HASH_TABLE_SIZE; offset++)
    {
        uint32 index = ((hash + offset) % TYPES_HASH_TABLE_SIZE);
        if (types_hash_table__hash[index] == 0)
        {
            types_hash_table__hash[index] = hash;
            types_hash_table__type[index] = t;

            return index;
        }
        else
        {
            if (types_hash_table__type[index] == t)
            {
                return index;
            }
        }
    }

    ASSERT_FAIL("Hash table is full. Make bigger hash table.");
    return 0;
}

type parser::get_type(type_id_t index)
{
    return types_hash_table__type[index];
}

type parser::get_type(ast_node *node)
{
    ASSERT(node->kind == AST_NODE__TYPE);
    return types_hash_table__type[node->m_type.index];
}

ast_node *parser::parse_type(lexer *lex)
{
    auto t = lex->get_token();
    if (t.kind == TOKEN_IDENTIFIER ||
        t.kind == TOKEN_KW_INT ||
        t.kind == TOKEN_KW_BOOL)
    {
        lex->eat_token();

        type type_s = {};
        type_s.hash[0] = hash_djb2((byte *) t.span.data, t.span.size);
        type_s.count = 1;
        type_s.name = t.span;

        auto type_id = reg_type(type_s);
        ast.push_back(ast_node::make_type(type_id));
        return ast.end() - 1;
    }
    else if (t.kind == '(')
    {
        lex->eat_token();

        type type_s = {};

        auto close_paren = lex->get_token();
        if (close_paren.kind == ')')
        {
            lex->eat_token();
        }
        else
        {
            ast_node *type_node = NULL;
            for (int i = 0; i < ARRAY_COUNT(type::hash); i++)
            {
                type_node = parse_type(lex);
                if (type_node)
                {
                    type_s.hash[i] = type_node->m_type.index;
                    type_s.count += 1;

                    auto comma = lex->get_token();
                    if (comma.kind == ',')
                    {
                        lex->eat_token();
                    }
                    else if (comma.kind == ')')
                    {
                        lex->eat_token();
                        break;
                    }
                }
                else
                {
                    ASSERT_FAIL("Runtime error");
                }
            }

            if (type_s.count == 1)
            {
                return type_node;
            }
        }

        auto type_id = reg_type(type_s);
        ast.push_back(ast_node::make_type(type_id));
        return ast.end() - 1;
    }
    return NULL;
}

ast_node *parser::parse_variable(lexer *lex)
{
    auto t = lex->get_token();
    if (t.kind == TOKEN_IDENTIFIER)
    {
        lex->eat_token();
        ast.push_back(ast_node::make_variable(t.span));
        return ast.end();
    }
    return NULL;
}

ast_node *parser::parse_int_literal(lexer *lex)
{
    auto t = lex->get_token();
    if (t.kind == TOKEN_LITERAL_INT)
    {
        lex->eat_token();
        ast.push_back(ast_node::make_int_literal(t.integer_value));
        return ast.end();
    }
    return NULL;
}

void debug_print_ast_type(parser *p, type t)
{
    if (t.kind == TYPE__TUPLE)
    {
        printf("t:");
        if (t.count == 0)
        {
            printf("() index=%d;", 0);
        }
        else if (t.count == 1)
        {
            uint32 index = t.hash[0] % TYPES_HASH_TABLE_SIZE;
            printf("(%.*s index=%d);", (int) t.name.size, t.name.data, index);
        }
        else
        {
            printf("(");
            for (int i = 0; i < t.count; i++)
            {
                auto t0 = p->get_type(t.hash[i] % TYPES_HASH_TABLE_SIZE);
                debug_print_ast_type(p, t0);
                if (i < t.count - 1) printf(", ");
            }
            printf(");");
        }
    }
}

void debug_print_ast_type_node(parser *p, ast_node *node)
{
    auto t = p->get_type(node);
    debug_print_ast_type(p, t);
}

void debug_print_ast_variable(parser *p, ast_node *node)
{
    auto name = node->m_variable.name;
    printf("v:%.*s;", (int) name.size, name.data);
}

void debug_print_ast_int_literal(parser *p, ast_node *node)
{
    printf("l:%lld;", node->m_int_lit.n);
}


void parser::debug_print_ast()
{
    for (int i = 0; i < ast.size(); i++)
    {
        ast_node *node = ast.data() + i;
        switch (node->kind)
        {
        case AST_NODE__TYPE: debug_print_ast_type_node(this, node);
            break;
        case AST_NODE__VARIABLE: debug_print_ast_variable(this, node);
            break;
        case AST_NODE__INT_LIT: debug_print_ast_int_literal(this, node);
            break;
        default:
            printf("<error>");
        }
        printf("\n");
    }
}



} // namespace pinapl








// struct type *register_type__name(struct parser *parser, string_view name);


// token get_token(struct parser *parser)
// {
//     token result = token__invalid();
//     if (parser->token_cursor < parser->token_stream.count)
//     {
//         result = parser->token_stream.tokens[parser->token_cursor];
//     }
//     return result;
// }

// void eat_token(struct parser *parser)
// {
//     parser->token_cursor += 1;
// }

// token eat_token_t(struct parser *parser)
// {
//     token result = get_token(parser);
//     parser->token_cursor += 1;
//     return result;
// }


// ast_node *make_new_ast_node(struct parser *parser)
// {
//     ast_node *result = NULL;
//     if (parser->ast_count < parser->ast_capacity)
//     {
//         result = parser->ast + parser->ast_count;
//         parser->ast_count += 1;
//     }
//     return result;
// }

// void parser__save_position(struct parser *parser)
// {
//     ASSERT(parser->rollback_buffer_count < ARRAY_COUNT(parser->rollback_buffer));
//     if (parser->rollback_buffer_count < ARRAY_COUNT(parser->rollback_buffer))
//     {
//         usize index = parser->rollback_buffer_count++;
//         parser->rollback_buffer[index].tok_stream_position = parser->token_cursor;
//         parser->rollback_buffer[index].ast_buffer_position = parser->ast_count;
//     }
// }

// void parser__rollback(struct parser *parser)
// {
//     ASSERT(parser->rollback_buffer_count > 0);
//     if (parser->rollback_buffer_count > 0)
//     {
//         usize index = parser->rollback_buffer_count;
//         parser->token_cursor = parser->rollback_buffer[index].tok_stream_position;
//         parser->ast_count = parser->rollback_buffer[index].ast_buffer_position;
//         parser->rollback_buffer_count--;
//     }
// }


// ast_node *parse_expression_operand(struct parser *parser)
// {
//     LOGGER(parser);
//     ast_node *result = NULL;

//     token t1 = eat_token_t(parser);
//     if (t1.kind == TOKEN_IDENTIFIER)
//     {
//         token t2 = get_token(parser);
//         if (t2.kind == '(') // IDENT OPEN_PAREN (it's the function call)
//         {
//             // @todo: function calls with setting argument names
//         }
//         else // IDENT not_OPEN_PAREN  (it's just a variable)
//         {
//             result = make_new_ast_node(parser);
//             if (result) ast__set_variable(result, t1.span);
//         }
//     }
//     else if (t1.kind == TOKEN_KW_TRUE)
//     {
//         result = make_new_ast_node(parser);
//         if (result) ast__set_boolean(result, true);
//     }
//     else if (t1.kind == TOKEN_KW_FALSE)
//     {
//         result = make_new_ast_node(parser);
//         if (result) ast__set_boolean(result, false);
//     }
//     else if (t1.kind == TOKEN_LITERAL_INT)
//     {
//         result = make_new_ast_node(parser);
//         if (result) ast__set_integer(result, t1.integer_value);
//     }
//     else
//     {
//         LOG("Could not recognize language at %d:%d!\n", t1.line, t1.column);
//         LOG("Expected (id|true|false|int lit) at %d:%d\n", t1.line, t1.column);
//     }

//     return result;
// }


// ast_node *parse_expression(struct parser *parser, int precedence)
// {
//     LOGGER(parser);
//     ast_node *lhs = NULL;

//     token t1 = get_token(parser);
//     if (t1.kind == '(') // OPEN_PAREN (could be a tuple OR a parenthesized expression)
//     {
//         eat_token(parser);

//         token t3 = get_token(parser);
//         if (t3.kind == ')') // OPEN_PAREN CLOSE_PAREN (this is empty tuple == void)
//         {
//             eat_token(parser);
//             lhs = make_new_ast_node(parser);
//             lhs->kind = AST__TUPLE;
//             if (lhs == NULL)
//             {
//                 LOG("Could not allocate new ast node\n");
//                 return NULL;
//             }
//         }
//         else
//         {
//             ast_node *expr1 = parse_expression(parser, 0);
//             if (expr1)
//             {
//                 token t3 = get_token(parser);
//                 if (t3.kind == ',') // OPEN_PAREN EXPR COMMA (It's a tuple)
//                 {
//                     ast_node *tuple = make_new_ast_node(parser);
//                     ast_tuple__push(tuple, expr1);

//                     while (true)
//                     {
//                         token t3 = get_token(parser);
//                         if (t3.kind == ',')
//                         {
//                             eat_token(parser);
//                             ast_node *expr2 = parse_expression(parser, 0);
//                             ast_tuple__push(tuple, expr2);
//                         }
//                         else if (t3.kind == ')')
//                         {
//                             eat_token(parser);
//                             break;
//                         }
//                         else
//                         {
//                             LOG("Error while parsing tuple of expressions at %d:%d\n", t3.line, t3.column);
//                             return NULL;
//                         }
//                     }

//                     lhs = tuple;
//                 }
//                 else if (t3.kind == ')') // OPEN_PAREN EXPR CLOSE_PAREN (It's a parenth expression)
//                 {
//                     eat_token(parser);
//                     lhs = expr1;
//                 }
//                 else
//                 {
//                     LOG("Error while parsing tuple of expressions OR parenth expression\n");
//                 }
//             }
//             else
//             {
//                 LOG("Could not parse after '('\n");
//             }
//         }
//     }
//     else
//     {
//         lhs = parse_expression_operand(parser);
//         if (lhs == NULL)
//         {
//             LOG("Could not parse expression\n");
//             return NULL;
//         }
//     }

//     while (true)
//     {
//         token operator = get_token(parser);
//         if ((operator.kind != '+') &&
//             (operator.kind != '-') &&
//             (operator.kind != '*') &&
//             (operator.kind != '/') &&
//             (operator.kind != '=')) { break; }

//         int operator_precedence = get_precedence(operator);
//         if (operator_precedence < precedence) { break; }

//         eat_token(parser);

//         // +1 for left associativity, +0 for right associativity
//         ast_node *rhs = parse_expression(parser, operator_precedence + 1);
//         if (rhs == NULL)
//         {
//             LOG("Could not parse expression\n");
//             return NULL;
//         }

//         ast_node *binary_operator = make_new_ast_node(parser);
//         if (binary_operator) ast__set_binary_operator(binary_operator, operator.span, lhs, rhs);

//         lhs = binary_operator;
//     }

//     return lhs;
// }

// struct type *parse_type(struct parser *parser)
// {
//     LOGGER(parser);
//     struct type *result = NULL;

//     token t1 = get_token(parser);
//     if ((t1.kind == TOKEN_KW_BOOL) ||
//         (t1.kind == TOKEN_IDENTIFIER))
//     {
//         eat_token(parser);
//         result = register_type__name(parser, t1.span);
//     }
//     else if (t1.kind == '(')
//     {
//         eat_token(parser);
//         struct type *type1 = parse_type(parser);
//         if (type1)
//         {
//             struct type tuple = {
//                 .kind = TYPE__TUPLE,
//                 .t.types[0] = type1,
//                 .t.count = 1,
//             };

//             while (true)
//             {
//                 token t2 = get_token(parser);
//                 if (t2.kind == ',')
//                 {
//                     eat_token(parser);
//                     struct type *type2 = parse_type(parser);
//                     tuple.t.types[tuple.t.count++] = type2;
//                 }
//                 else if (t2.kind == ')')
//                 {
//                     eat_token(parser);
//                     break;
//                 }
//                 else
//                 {
//                     break;
//                 }
//             }

//             result = register_type(parser, &tuple);
//         }
//         else
//         {
//             LOG("Could not parse tuple type at %d:%d\n", t1.line, t1.column);
//         }
//     }
//     else
//     {
//         LOG("Could not recognize type at %d:%d\n", t1.line, t1.column);
//     }

//     return result;
// }

// struct type *registry__push_type(struct type_registry *registry, struct type *type)
// {
//     struct type *result = NULL;
//     if (registry->count < registry->capacity)
//     {
//         registry->types[registry->count] = *type;
//         result = registry->types + registry->count++;
//     }
//     return result;
// }

// bool32 types_equal(struct type *type1, struct type *type2)
// {
//     bool32 equal = true;
//     if (type1->kind != type2->kind)
//     {
//         equal = false;
//     }
//     else
//     {
//         if (type1->kind == TYPE__NAME)
//         {
//             equal = (string_view__compare(type1->n.name, type2->n.name) == 0);
//         }
//         else if (type1->kind == TYPE__TUPLE)
//         {
//             if (type1->t.count != type2->t.count)
//             {
//                 equal = false;
//             }
//             else
//             {
//                 for (int i = 0; i < type1->t.count; i++)
//                 {
//                     equal = types_equal(type1->t.types[i], type2->t.types[i]);
//                     if (!equal) break;
//                 }
//             }
//         }
//     }
//     return equal;
// }

// struct type *register_type(struct parser *parser, struct type *type1)
// {
//     LOGGER(parser);

//     struct type *result = NULL;
//     for (int type_index = 0; type_index < parser->type_registry->count; type_index++)
//     {
//         struct type *type2 = parser->type_registry->types + type_index;

//         if (types_equal(type1, type2))
//         {
//             result = type2;
//         }
//     }

//     if (result == NULL)
//     {
//         result = registry__push_type(parser->type_registry, type1);
//         if (result == NULL)
//         {
//             LOG("Could not push type into registry\n");
//         }
//     }

//     return result;
// }

// struct type *register_type__name(struct parser *parser, string_view name)
// {
//     struct type type = {
//         .kind = TYPE__NAME,
//         .n.name = name,
//     };
//     return register_type(parser, &type);
// }

#if 0


struct type_entry *register_type_entry(struct type_registry *registry, struct type_entry *entry_to_register)
{
    struct type_entry *entry = NULL;
    for (int i = 0; i < registry->entry_count; i++)
    {
        if (type_entries_equal(entry_to_register, registry->entries + i))
        {
            entry = registry->entries + i;
            break;
        }
    }
    if (entry == NULL)
    {
        if (registry->entry_count < ARRAY_COUNT(registry->entries))
        {
            entry = registry->entries + registry->entry_count++;
            *entry = *entry_to_register;
        }
    }

    return entry;
}


struct ast_node *parse_expression_operand(struct parser *parser);
struct ast_node *parse_expression(struct parser *parser, int precedence);
struct ast_node *parse_declaration_without_semicolon(struct parser *parser);
struct ast_node *parse_declaration_with_semicolon(struct parser *parser);
struct ast_node *parse_statement(struct parser *parser);
struct ast_node *parse_statements(struct parser *parser, struct scope *scope);
struct ast_node *parse_block(struct parser *parser);
struct ast_node *parse_function(struct parser *parser);
struct type_entry *parse_type(struct parser *parser);






struct scope *get_new_scope(struct parser *parser)
{
    struct scope *result = NULL;
    if (parser->scopes.count < ARRAY_COUNT(parser->scopes.scopes))
    {
        result = parser->scopes.scopes + parser->scopes.count;
        parser->scopes.count += 1;
    }
    return result;
}

void scope__push_variable(struct scope *scope, string_view name, struct type_entry *type)
{
    if (scope->variable_count < ARRAY_COUNT(scope->variable_names))
    {
        scope->variable_names[scope->variable_count] = name;
        scope->variable_types[scope->variable_count] = type;
        scope->variable_count += 1;
    }
}

void pop_ast_node(struct parser *parser)
{
    if (parser->ast_node_count > 0)
    {
        parser->ast_node_count -= 1;
    }
}

struct ast_node *parse_tuple_of_variables(struct parser *parser)
{
    ast_node *result = NULL;

    token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);
        token ident1 = get_token(parser);
        if (ident1.type == TOKEN_IDENTIFIER)
        {
            eat_token(parser);

            token comma = get_token(parser);
            if (comma.type == ')')
            {
                // Just a variable
                eat_token(parser);

                ast_node *variable = make_new_ast_node(parser);
                variable->kind = AST__VARIABLE;
                variable->variable.name = ident1.span;
                variable->variable.type = NULL;

                result = variable;
            }
            else if (comma.type == ',')
            {
                // A tuple
                ast_node *tuple = make_new_ast_node(parser);
                tuple->kind = AST__TUPLE_OF_VARIABLES;
                tuple->tuple.names[0] = ident1.span;
                tuple->tuple.count = 1;
                tuple->tuple.type = NULL; // infer

                while (true)
                {
                    comma = get_token(parser);
                    if (comma.type == ',')
                    {
                        eat_token(parser);

                        token ident2 = get_token(parser);
                        if (ident2.type == TOKEN_IDENTIFIER)
                        {
                            eat_token(parser);

                            tuple->tuple.names[tuple->tuple.count++] = ident2.span;
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

                result = tuple;
            }
        }
    }

    return result;
}

ast_node *parse_tuple_of_expressions(struct parser *parser)
{
    ast_node *result = NULL;

    token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);

        ast_node *expr1 = parse_expression(parser, 0);
        if (expr1)
        {
            token comma = get_token(parser);
            if (comma.type == ')')
            {
                eat_token(parser);
                result = expr1;
            }
            else if (comma.type == ',')
            {
                // A tuple
                ast_node *tuple = make_new_ast_node(parser);
                tuple->kind = AST__TUPLE_OF_EXPRESSIONS;
                tuple->tuple.expressions[0] = expr1;
                tuple->tuple.count = 1;
                tuple->tuple.type = NULL; // infer

                while (true)
                {
                    comma = get_token(parser);
                    if (comma.type == ',')
                    {
                        eat_token(parser);

                        ast_node *expr2 = parse_expression(parser, 0);
                        if (expr2)
                        {
                            tuple->tuple.expressions[tuple->tuple.count++] = expr2;
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

                result = tuple;
            }
        }
    }

    return result;
}

ast_node *parse_function_arguments(struct parser *parser)
{
    ast_node *result = NULL;

    token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);
        token name1 = get_token(parser);
        if (name1.type == TOKEN_IDENTIFIER)
        {
            eat_token(parser);
            token colon = get_token(parser);
            if (colon.type == ':')
            {
                eat_token(parser);
                type_entry *type1 = parse_type(parser);
                if (type1)
                {
                    ast_node *tuple = make_new_ast_node(parser);
                    tuple->kind = AST__TUPLE_OF_ARGUMENTS;
                    tuple->tuple.arg_names[0] = name1.span;
                    tuple->tuple.arg_types[0] = type1;
                    tuple->tuple.count = 1;

                    while (true)
                    {
                        token comma = get_token(parser);
                        if (comma.type == ',')
                        {
                            eat_token(parser);

                            token name2 = get_token(parser);
                            if (name2.type == TOKEN_IDENTIFIER)
                            {
                                eat_token(parser);

                                token colon2 = get_token(parser);
                                if (colon2.type == ':')
                                {
                                    eat_token(parser);
                                    type_entry *type2 = parse_type(parser);
                                    if (type2)
                                    {
                                        tuple->tuple.arg_names[tuple->tuple.count] = name2.span;
                                        tuple->tuple.arg_types[tuple->tuple.count] = type2;
                                        tuple->tuple.count += 1;
                                    }
                                }
                            }
                        }
                        else if (comma.kind == ')')
                        {
                            eat_token(parser);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
        else if (name1.kind == ')')
        {
            eat_token(parser);
            ast_node *tuple = make_new_ast_node(parser);
            tuple->kind = AST__TUPLE_OF_ARGUMENTS;
            tuple->tuple.count = 0;
        }
    }

    return result;
}

ast_node *parse_function_returns(struct parser *parser)
{
    ast_node *result = NULL;

    token open_paren = get_token(parser);
    if (open_paren.type == '(')
    {
        eat_token(parser);

        token ident1 = get_token(parser);
        if (ident1.type == TOKEN_IDENTIFIER)
        {
            eat_token(parser);

            string_view name1 = {};
            type_entry *type1 = NULL;

            token colon = get_token(parser);
            if (colon.type == ':')
            {
                name1 = ident1.span;
                type1 = parse_type(parser);
            }
            else if (colon.type == ',')
            {
                type_entry type_to_register = {
                    .kind = TYPE__NAME,
                    .type_name = ident1.span,
                };
                type1 = register_type_entry(&parser->types, &type_to_register);
            }

            result = make_new_ast_node(parser);
            result->kind = AST__TUPLE_OF_RETURNS;
            result->tuple.arg_names[0] = name1;
            result->tuple.arg_types[0] = type1;
            result->tuple.count = 1;

            while (true)
            {
                token comma = get_token(parser);
                if (comma.kind == ',')
                {
                    eat_token(parser);

                    token ident2 = get_token(parser);
                    if (ident2.kind == TOKEN_IDENTIFIER)
                    {
                        string_view name2 = {};
                        type_entry *type2 = NULL;

                        token colon = get_token(parser);
                        if (colon.type == ':')
                        {
                            name2 = ident2.span;
                            type2 = parse_type(parser);
                        }
                        else if (colon.type == ',')
                        {
                            type_entry type_to_register = {
                                .kind = TYPE__NAME,
                                .type_name = ident1.span,
                            };
                            type2 = register_type_entry(&parser->types, &type_to_register);
                        }

                        result->tuple.arg_names[result->tuple.count] = name2;
                        result->tuple.arg_types[result->tuple.count] = type2;
                        result->tuple.count += 1;
                    }
                }
                else if (comma.kind == ')')
                {
                    eat_token(parser);
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        else if (ident1.kind == ')')
        {
            result = make_new_ast_node(parser);
            result->kind = AST__TUPLE_OF_RETURNS;
            result->tuple.count = 0;
        }
    }

    return result;
}



type_entry *parse_tuple_of_types(struct parser *parser)
{
    struct type_entry *result = NULL;

    struct token open_paren = get_token(parser);
    if (open_paren.type == '(')
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
            struct type_entry *type1 = parse_type(parser);
            if (type1 != NULL)
            {
                struct token comma = get_token(parser);
                if (comma.type == ')')
                {
                    eat_token(parser);
                    result = type1;
                }
                else if (comma.type == ',')
                {
                    struct type_entry type_to_register = {
                        .kind = TYPE__TUPLE,
                        .tuple.type_count = 0,
                    };

                    type_to_register.tuple.types[0] = type1;
                    type_to_register.tuple.type_count = 1;

                    while (true)
                    {
                        struct token comma = get_token(parser);
                        if (comma.type == ',')
                        {
                            eat_token(parser);

                            struct type_entry *type2 = parse_type(parser);
                            if (type2 != NULL)
                            {
                                if (type_to_register.tuple.type_count < ARRAY_COUNT(type_to_register.tuple.types))
                                {
                                    type_to_register.tuple.types[type_to_register.tuple.type_count] = type2;
                                    type_to_register.tuple.type_count += 1;
                                }
                            }
                        }
                        else if (comma.type == ')')
                        {
                            eat_token(parser);
                            break;
                        }
                    }

                    result = register_type_entry(&parser->types, &type_to_register);
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

struct type_entry *parse_type(struct parser *parser)
{
    struct type_entry *result = NULL;

    struct token t = get_token(parser);
    if (t.type == TOKEN_IDENTIFIER)
    {
        eat_token(parser);

        // @speed
        struct type_entry entry_to_register = {
            .kind = TYPE__NAME,
            .type_name = t.span,
        };
        result = register_type_entry(&parser->types, &entry_to_register);
    }
    else if (t.type == TOKEN_KW_BOOL)
    {
        eat_token(parser);

        struct type_entry entry_to_register = {
            .kind = TYPE__NAME,
            .type_name = t.span,
        };
        result = register_type_entry(&parser->types, &entry_to_register);
    }
    else if (t.type == '(')
    {
        result = parse_tuple_of_types(parser);
    }

    return result;
}

struct ast_node *parse_declaration(struct parser *parser, bool32 ignore_semicolon)
{
    // x :: <expr>;
    // x := <expr>;
    // x : int;
    // x : int : <expr>;
    // x : int = <expr>;
    struct ast_node *result = NULL;
    struct ast_node *lhs = NULL;

    struct token name = get_token(parser);
    if (name.type == TOKEN_IDENTIFIER)
    {
        eat_token(parser);

        lhs = make_new_ast_node(parser);
        lhs->kind = AST__VARIABLE;
        lhs->variable.name = name.span;
    }
    else
    {
        if (name.type == '(') // Allow tuple of identifiers
        {
            lhs = parse_tuple_of_variables(parser);
        }
        else
        {
            // @todo: report error
        }
    }

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
            struct type_entry *type = NULL;

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
                        // because it is going to be eaten later
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
                    usize saved_cursor = parser->cursor;
                    initializer = parse_function(parser);
                    if (initializer == NULL) parser->cursor = saved_cursor;
                }
                if (!initializer)
                {
                    initializer = parse_expression(parser, 0);

                    if (!ignore_semicolon)
                    {
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
            }

            result = make_new_ast_node(parser);
            result->kind = AST__DECLARATION;
            result->declaration.is_constant = is_constant;
            result->declaration.decl_type = type;
            result->declaration.lhs  = lhs;
            result->declaration.init = initializer;
        }
    }

    return result;
}

struct ast_node *parse_declaration_without_semicolon(struct parser *parser)
{
    struct ast_node *result = parse_declaration(parser, true);
    return result;
}

struct ast_node *parse_declaration_with_semicolon(struct parser *parser)
{
    struct ast_node *result = parse_declaration(parser, false);
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
    else if (t.type == TOKEN_KW_RETURN)
    {
        eat_token(parser);

        struct ast_node *return_statement = NULL;
        struct token semicolon = get_token(parser);
        if (semicolon.type == ';')
        {
            eat_token(parser);

            return_statement = make_new_ast_node(parser);
            return_statement->kind = AST__RETURN;
            return_statement->return_.return_expression = NULL;
        }
        else
        {
            struct ast_node *return_expression = parse_expression(parser, 0);
            semicolon = get_token(parser);
            if (semicolon.type == ';')
            {
                eat_token(parser);

                return_statement = make_new_ast_node(parser);
                return_statement->kind = AST__RETURN;
                return_statement->return_.return_expression = return_expression;
            }
        }

        result = make_new_ast_node(parser);
        result->kind = AST__STATEMENT;
        result->statement.stmt = return_statement;
        result->statement.next = NULL;
    }
    else
    {
        struct ast_node *ast = parse_declaration_with_semicolon(parser);
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
        else
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

struct ast_node *parse_statements(struct parser *parser, struct scope *scope)
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

        struct scope *block_scope = get_new_scope(parser);
        struct ast_node *stmts = parse_statements(parser, block_scope);

        struct token close_brace = get_token(parser);
        if (close_brace.type == '}')
        {
            eat_token(parser);

            result = make_new_ast_node(parser);
            result->kind = AST__BLOCK;
            result->block.statements = stmts;
            result->block.scope = block_scope;
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
        struct ast_node *arguments = parse_function_arguments(parser);
        struct ast_node *returns = NULL;
        if (arguments)
        {
            struct token arrow = get_token(parser);
            if (arrow.type == TOKEN_ARROW_RIGHT)
            {
                eat_token(parser);
                returns = parse_function_returns(parser);
            }
            else
            {
                returns = make_new_ast_node(parser);
                returns->kind = AST__TUPLE_OF_RETURNS;
                returns->tuple.arg_types[0] = parser->types.entries; // void
                memory__set(&returns->tuple.arg_names[0], 0, sizeof(returns->tuple.arg_names[0]));
                returns->tuple.count = 1;
            }

            if (arguments && returns)
            {
                struct ast_node *body = parse_block(parser);

                result = make_new_ast_node(parser);
                result->kind = AST__FUNCTION;
                result->function.arguments = arguments;
                result->function.returns = returns;
                result->function.body = body;
            }
        }
    }

    return result;
}

void debug_print_type(struct type_entry *type)
{
    switch (type->kind)
    {
        case TYPE__VOID:
        {
            printf("void");
        }
        break;

        case TYPE__NAME:
        {
            printf("%.*s", (int) type->type_name.size, type->type_name.data);
        }
        break;

        case TYPE__TUPLE:
        {
            printf("(");

            if (type->tuple.type_count > 0)
            {
                // if (type->tuple_names[0].data != NULL)
                // {
                //     printf("%.*s : ", (int) type->tuple_names[0].size, type->tuple_names[0].data);
                // }
                debug_print_type(type->tuple.types[0]);
            }

            for (int i = 1; i < type->tuple.type_count; i++)
            {
                printf(", ");
                // if (type->tuple_names[i].data != NULL)
                // {
                //     printf("%.*s : ", (int) type->tuple_names[i].size, type->tuple_names[i].data);
                // }
                debug_print_type(type->tuple.types[i]);
            }
            printf(")");
        }
        break;

        case TYPE__FUNCTION:
        {
            debug_print_type(type->arguments);
            if (type->return_type)
            {
                printf(" -> ");
                debug_print_type(type->return_type);
            }
        }
        break;
    }
}


#endif








#if 0
void ast__set_variable(ast_node *node, string_view name)
{
    node->kind = AST__VARIABLE;
    node->v.name = name;
}

void ast__set_integer(ast_node *node, int64 value)
{
    node->kind = AST__LITERAL_INT;
    node->i.value = value;
}

void ast__set_boolean(ast_node *node, bool32 value)
{
    node->kind = AST__LITERAL_BOOL;
    node->b.value = value;
}

void ast__set_binary_operator(ast_node *node, string_view op, ast_node *lhs, ast_node *rhs)
{
    node->kind = AST__BINARY_OPERATOR;
    node->bo.name = op;
    node->bo.lhs = lhs;
    node->bo.rhs = rhs;
}

void ast_tuple__push(ast_node *node, ast_node *expr)
{
    node->kind = AST__TUPLE;
    if (node->t.count < ARRAY_COUNT(node->t.expressions))
    {
        node->t.expressions[node->t.count++] = expr;
    }
}


string_view ast__get_name(ast_node *node)
{
    string_view result = {};
    if (node->kind == AST__VARIABLE)
    {
        result = node->v.name;
    }
    else if (node->kind == AST__BINARY_OPERATOR)
    {
        result = node->bo.name;
    }
    return result;
}

bool32 ast__get_boolean(ast_node *node)
{
    bool32 result = false;
    if (node->kind == AST__LITERAL_BOOL)
    {
        result = node->b.value;
    }
    return result;
}

int64 ast__get_integer(ast_node *node)
{
    int64 result = 0;
    if (node->kind == AST__LITERAL_INT)
    {
        result = node->i.value;
    }
    return result;
}











void debug__print_type(struct type *type)
{
    if (type == NULL)
    {
        printf("Invalid type");
        return;
    }

    switch (type->kind)
    {
        case TYPE__NAME:
        {
            printf("%.*s", (int) type->n.name.size, type->n.name.data);
        }
        break;

        case TYPE__TUPLE:
        {
            printf("(");
            if (type->t.count > 0)
            {
                debug__print_type(type->t.types[0]);
                for (int i = 1; i < type->t.count; i++)
                {
                    printf(", ");
                    debug__print_type(type->t.types[i]);
                }
            }
            printf(")");
        }
        break;

        default:
            printf("Invalid type");
    }
}

void debug__print_type_registry(struct type_registry *type_registry)
{
    if (type_registry)
    {
        printf("Type registry:\n");
        for (int i = 0; i < type_registry->count; i++)
        {
            printf("%2d) ", i + 1);
            debug__print_type(&type_registry->types[i]);
            printf("\n");
        }
    }
}

bool32 debug__print_ast(struct ast_node *ast, int depth)
{
    if (ast == NULL)
    {
        printf("Invalid ast\n");
        return false;
    }

    char spaces[] = "                                                                                                                 ";
    char dashes[] = "----------------------";
    bool32 newlined = false;

#define DO_NEWLINE if (!newlined) { printf("\n"); newlined = true; }

    switch (ast->kind)
    {
        case AST__VARIABLE:
        {
            string_view name = ast__get_name(ast);
            printf("variable (%.*s)", (int) name.size, name.data);
        }
        break;

        case AST__LITERAL_BOOL:
        {
            printf("literal boolean (%s)", ast__get_boolean(ast) ? "true" : "false");
        }
        break;

        case AST__LITERAL_INT:
        {
            printf("literal integer (%lld)", ast__get_integer(ast));
        }
        break;

        case AST__BINARY_OPERATOR:
        {
            string_view name = ast__get_name(ast);
            printf("(%.*s)%.*s", (int) name.size, name.data,
                                 (int) (5 - name.size), dashes);
            newlined = debug__print_ast(ast->bo.rhs, depth+1);
            DO_NEWLINE
            printf("%.*s\\---", 4*(depth + 1) + 3*depth - 1, spaces);
            newlined = debug__print_ast(ast->bo.lhs, depth+1);
            DO_NEWLINE
        }
        break;

        case AST__TUPLE:
        {
            printf("tuple\n");
            for (int i = 0; i < ast->t.count; i++)
            {
                printf("%.*s\\--", 4*(depth + 1) + 3*(depth), spaces);
                newlined = debug__print_ast(ast->t.expressions[i], depth+1);
                DO_NEWLINE
            }
            DO_NEWLINE
        }
        break;

        default:
            printf("Invalid ast\n");
        break;
    }

#if 0
    {
        case AST__BINARY_OPERATOR:
        {
            printf("(%c)----", ast->binary_operator.operator);
            newlined = debug__print_ast(ast->binary_operator.rhs, depth+1);
            DO_NEWLINE
            printf("%.*s\\---", 4*(depth + 1) + 3*depth - 1, spaces);
            newlined = debug__print_ast(ast->binary_operator.lhs, depth+1);
            DO_NEWLINE
        }
        break;

        case AST__DECLARATION:
        {
            printf("%s---", ast->declaration.is_constant ? "(::)" : "(:=)");
            if (ast->declaration.decl_type != NULL)
            {
                debug_print_type(ast->declaration.decl_type);
                DO_NEWLINE
            }
            else
            {
                printf("infr\n");
                newlined = true;
            }

            printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
            if (ast->declaration.lhs)
            {
                newlined = debug__print_ast(ast->declaration.lhs, depth + 1);
                DO_NEWLINE
            }

            printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
            if (ast->declaration.init != NULL)
            {
                debug__print_ast(ast->declaration.init, depth+1);
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
            if (ast->function_call.arg_count > 0)
            {
                newlined = debug__print_ast(ast->function_call.args[0], depth + 1);
                DO_NEWLINE
                for (int i = 1; i < ast->function_call.arg_count; i++)
                {
                    printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
                    newlined = debug__print_ast(ast->function_call.args[i], depth + 1);
                    DO_NEWLINE
                }
            }
            else
            {
                printf("null");
            }
            DO_NEWLINE
        }
        break;

        case AST__STATEMENT:
        {
            printf("stmt---");
            newlined = debug__print_ast(ast->statement.stmt, depth + 1);
            if (ast->statement.next != NULL)
            {
                printf("%.*s", 7*depth, spaces);
                newlined = debug__print_ast(ast->statement.next, depth);
            }
        }
        break;

        case AST__BLOCK:
        {
            printf("block scope: ");
            for (int i = 0; i < ast->block.scope->variable_count; i++)
            {
                printf("%.*s ", (int) ast->block.scope->variable_names[i].size, ast->block.scope->variable_names[i].data);
                debug_print_type(ast->block.scope->variable_types[i]);
            }
            DO_NEWLINE
            printf("%.*s{\n", 4*(depth) + 3*(depth - 1), spaces);
            if (ast->block.statements)
            {
                printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
                newlined = debug__print_ast(ast->block.statements, depth + 1);
            }
            printf("%.*s}\n", 4*(depth) + 3*(depth - 1), spaces);
            newlined = true;
        }
        break;



        case AST__VARIABLE:
        {
            printf("var ");
        }
        break;

        case AST__TUPLE_OF_VARIABLES:
        {
            printf("tuple of variables\n");
            for (int i = 0; i < ast->tuple.count; i++)
            {
                printf("%.*s\\--", 4*(depth + 1) + 3*(depth), spaces);
                printf("%.*s\n", (int) ast->tuple.names[i].size, ast->tuple.names[i].data);
                newlined = true;
            }
        }
        break;



        case AST__TUPLE_OF_ARGUMENTS:
        {
            printf("tuple of args\n");
            for (int i = 0; i < ast->tuple.count; i++)
            {
                printf("%.*s\\-- ", 4*(depth + 1) + 3*(depth), spaces);
                printf("%.*s : ", (int) ast->tuple.arg_names[i].size, ast->tuple.arg_names[i].data);
                debug_print_type(ast->tuple.arg_types[i]);
                printf("\n");
            }
            newlined = true;
        }
        break;

        case AST__FUNCTION:
        {
            printf("func (");
            if (ast->function.arguments && ast->function.arguments->kind == AST__TUPLE_OF_ARGUMENTS)
            {
                if (ast->function.arguments->tuple.arg_names[0].size > 0)
                {
                    printf("%.*s : ", (int) ast->function.arguments->tuple.arg_names[0].size,
                                            ast->function.arguments->tuple.arg_names[0].data);
                }
                debug_print_type(ast->function.arguments->tuple.arg_types[0]);

                for (int i = 1; i < ast->function.arguments->tuple.count; i++)
                {
                    if (ast->function.arguments->tuple.arg_names[i].size > 0)
                    {
                        printf(", %.*s : ", (int) ast->function.arguments->tuple.arg_names[i].size,
                                                ast->function.arguments->tuple.arg_names[i].data);
                    }
                    debug_print_type(ast->function.arguments->tuple.arg_types[i]);
                }
            }
            printf(")");
            DO_NEWLINE
            printf("%.*s\\--", 4*(depth + 1) + 3*depth, spaces);
            newlined = debug__print_ast(ast->function.body, depth + 1);
        }
        break;

        case AST__RETURN:
        {
            printf("ret");
            if (ast->return_.return_expression)
            {
                printf("----");
                newlined = debug__print_ast(ast->return_.return_expression, depth + 1);
                DO_NEWLINE
            }
        }
        break;


    }

#endif

#undef DO_NEWLINE

    return newlined;
}
#endif

