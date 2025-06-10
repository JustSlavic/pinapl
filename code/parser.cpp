#include "parser.hpp"
#include <hash.hpp>


namespace pinapl {


static char spaces[] = "                                                        ";



ast_node ast_node::make_variable(string_view name)
{
    ast_node result = {};
    result.kind = AST_NODE__VARIABLE;
    result.m_variable = name;

    return result;
}

ast_node ast_node::make_int_literal(int64 n)
{
    ast_node result = {};
    result.kind = AST_NODE__INT_LIT;
    result.m_int_literal = n;

    return result;
}

ast_node ast_node::make_expr_tuple()
{
    ast_node result = {};
    result.kind = AST_NODE__EXPR_TUPLE;
    return result;
}



ast_node *parser::parse_variable(lexer *lex)
{
    auto checkpoint = *lex;

    auto t = lex->get_token();
    if (t.kind == TOKEN_IDENTIFIER)
    {
        lex->eat_token();
        ast.push_back(ast_node::make_variable(t.span));
        return ast.end() - 1;
    }

    *lex = checkpoint;
    return NULL;
}

ast_node *parser::parse_int_literal(lexer *lex)
{
    auto checkpoint = *lex;

    auto t = lex->get_token();
    if (t.kind == TOKEN_LITERAL_INT)
    {
        lex->eat_token();
        ast.push_back(ast_node::make_int_literal(t.integer_value));
        return ast.end() - 1;
    }
    *lex = checkpoint;
    return NULL;
}

ast_node *parser::parse_expr_tuple(lexer *lex)
{
    auto checkpoint = *lex;

    auto t = lex->get_token();
    if (t.kind == '(')
    {
        lex->eat_token();

        auto tuple_result = ast_node::make_expr_tuple();

        do
        {
            t = lex->get_token();
            if (t.kind == ')')
            {
                lex->eat_token();
                break;
            }

            auto expr = parse_expression(lex, 0);
            if (!expr) goto parse_expr_tuple_exit;

            tuple_result.m_expr_tuple.push_back(expr);

            t = lex->get_token();
            if (t.kind == ',')
            {
                lex->eat_token();
            }
        } while (true);

        ast.push_back(tuple_result);
        return ast.end() - 1;
    }

parse_expr_tuple_exit:
    *lex = checkpoint;
    return NULL;
}

ast_node *parser::parse_expression_operand(lexer *lex)
{
    ast_node *tuple = parse_expr_tuple(lex);
    if (tuple) { return tuple; }

    ast_node *variable = parse_variable(lex);
    if (variable) { return variable; }

    ast_node *int_literal = parse_int_literal(lex);
    if (int_literal) { return int_literal; }

    return NULL;
}

ast_node *parser::parse_expression(lexer *lex, int precedence)
{
    ast_node *lhs = parse_expression_operand(lex);
    if (lhs == NULL)
    {
        return NULL;
    }

    // while (true)
    // {
    //     auto op = lex->get_token();
    //     if (op.kind != '+' &&
    //         op.kind != '-')
    //     {
    //         break;
    //     }

    //     int operator_precedence = 1;
    //     if (operator_precedence < precedence) break;

    //     lex->eat_token();

    //     ast_node *rhs = parse_expression(lex, operator_precedence + 1);
    //     if (rhs == NULL)
    //     {
    //         return NULL;
    //     }

    //     ast.push_back(ast_node::make_binary_op(op.span, lhs, rhs));
    //     lhs = ast.end() - 1;
    // }

    return lhs;
}


void debug_print_ast(ast_node *w, int depth)
{
    switch (w->kind)
    {
        case AST_NODE__NONE:
        {
            printf("AST_NODE__NONE\n");
        }
        break;

        case AST_NODE__VARIABLE:
        {
            printf("%.*s", (int) w->m_variable.size, w->m_variable.data);
        }
        break;

        case AST_NODE__INT_LIT:
        {
            printf("AST_NODE__INT_LIT\n");
        }
        break;

        case AST_NODE__EXPR_TUPLE:
        {
            printf("(");
            for (auto e : w->m_expr_tuple) {
                debug_print_ast(e, depth + 1);
                printf(", ");
            }
            printf(")");
        }
        break;
    }
}


#if 0
ast_node ast_node::make_binary_op(string_view op, ast_node *lhs, ast_node *rhs)
{
    ast_node result = {};
    result.kind = AST_NODE__BINARY_OP;
    result.m_bin_op.op = op;
    result.m_bin_op.lhs = lhs;
    result.m_bin_op.rhs = rhs;

    return result;
}


ast_node *parser::parse_tuple_decl(lexer *lex)
{
    ast_node result = {};

    auto t = lex->get_token();
    if (t.kind == TOKEN_EOF)
    {
        return NULL;
    }
    else if (t.kind == TOKEN_IDENTIFIER ||
             t.kind == TOKEN_KW_INT ||
             t.kind == TOKEN_KW_BOOL)
    {
        lex->eat_token();

        auto colon = lex->get_token();
        if (colon.kind == ':')
        {
            lex->eat_token();
            ast_node *parsed = parse_tuple_decl(lex);
            if (!parsed)
            {
                return NULL;
            }
            else
            {
                result.kind = AST_NODE__TUPLE_DECL;
                result.m_tuple_decl.names[0] = t.span;
                result.m_tuple_decl.elements[0] = parsed;
                result.m_tuple_decl.count = 1;
            }
        }
        else
        {
            result.kind = AST_NODE__TUPLE_DECL;
            result.m_tuple_decl.names[0] = t.span;
            result.m_tuple_decl.count = 1;
        }
    }
    else if (t.kind == '(')
    {
        result.kind = AST_NODE__TUPLE_DECL;

        lex->eat_token();
        auto paren = lex->get_token();
        if (paren.kind == ')')
        {
            lex->eat_token();
        }
        else
        {
            ast_node *parsed = parse_tuple_decl(lex);
            if (!parsed)
            {
                return NULL;
            }

            result.m_tuple_decl.elements[0] = parsed;
            result.m_tuple_decl.count = 1;

            while (true)
            {
                auto comma_or_paren = lex->get_token();
                if (comma_or_paren.kind == TOKEN_EOF)
                {
                    return NULL;
                }
                else if (comma_or_paren.kind == ',')
                {
                    lex->eat_token();
                }
                else if (comma_or_paren.kind == ')')
                {
                    lex->eat_token();
                    break;
                }

                ast_node *parsed = parse_tuple_decl(lex);
                if (!parsed)
                {
                    return NULL;
                }

                result.m_tuple_decl.elements[result.m_tuple_decl.count] = parsed;
                result.m_tuple_decl.count += 1;
            }

            if (result.m_tuple_decl.count == 1)
            {
                return result.m_tuple_decl.elements[0];
            }
        }
    }
    else
    {
        return NULL;
    }

    ast.push_back(result);
    return ast.end() - 1;
}

ast_node *parser::parse_func_decl(lexer *lex)
{
    ast_node result = {};

    auto t = lex->get_token();
    if (t.kind == TOKEN_EOF)
    {
        return NULL;
    }
    else if (t.kind == '(')
    {
        ast_node *argument = parse_tuple_decl(lex);
        ast_node *returns  = NULL;

        if (!argument)
        {
            return NULL;
        }

        auto arrow = lex->get_token();
        if (arrow.kind == TOKEN_ARROW_RIGHT)
        {
            lex->eat_token();
            returns = parse_tuple_decl(lex);

            if (!returns)
            {
                return NULL;
            }
        }

        result.kind = AST_NODE__FUNC_DECL;
        result.m_func_decl.argument = argument;
        result.m_func_decl.returns  = returns;
    }
    else
    {
        return NULL;
    }

    ast.push_back(result);
    return ast.end() - 1;
}




ast_node *parser::parse_var_decl(lexer *lex)
{
    // x :: <expr>;
    // x := <expr>;
    // x : int;
    // x : int : <expr>;
    // x : int = <expr>;
    ast_node result = {};
    UNUSED(result);

    auto name = lex->get_token();
    if (name.kind == TOKEN_IDENTIFIER)
    {
        lex->eat_token();

        // lhs = make_new_ast_node(parser);
        // lhs->kind = AST__VARIABLE;
        // lhs->variable.name = name.span;
    }
    else
    {
        if (name.kind == '(') // Allow tuple of identifiers
        {
            // lhs = parse_tuple_of_variables(parser);
        }
        else
        {
            // @todo: report error
        }
    }

    // {
    //     struct token colon = eat_token(parser);
    //     if (colon.type != ':')
    //     {
    //         // @todo: report error
    //     }
    //     else
    //     {
    //         bool32 is_constant = false;
    //         bool32 should_init = false;

    //         struct ast_node *initializer = NULL;
    //         struct type_entry *type = NULL;

    //         struct token t = get_token(parser);
    //         if (t.type == '=') // <ident> := <expr>
    //         {
    //             eat_token(parser);
    //             is_constant = false;
    //             should_init = true;
    //         }
    //         else if (t.type == ':') // <ident> :: <expr>
    //         {
    //             eat_token(parser);
    //             is_constant = true;
    //             should_init = true;
    //         }
    //         else
    //         {
    //             type = parse_type(parser);
    //             if (type != NULL)
    //             {
    //                 struct token t2 = get_token(parser);
    //                 if (t2.type == '=') // <ident> : int = <expr>
    //                 {
    //                     eat_token(parser);
    //                     is_constant = false;
    //                     should_init = true;
    //                 }
    //                 else if (t2.type == ':') // <ident> : int : <expr>
    //                 {
    //                     eat_token(parser);
    //                     is_constant = true;
    //                     should_init = true;
    //                 }
    //                 else if (t2.type == ';') // x : int
    //                 {
    //                     // @note: do not eat semicolon,
    //                     // because it is going to be eaten later
    //                     is_constant = false;
    //                     should_init = false;
    //                 }
    //                 else
    //                 {
    //                     // @todo: report error
    //                 }
    //             }
    //         }

    //         if (should_init)
    //         {
    //             struct token open_paren = get_token(parser);
    //             if (open_paren.type == '(')
    //             {
    //                 usize saved_cursor = parser->cursor;
    //                 initializer = parse_function(parser);
    //                 if (initializer == NULL) parser->cursor = saved_cursor;
    //             }
    //             if (!initializer)
    //             {
    //                 initializer = parse_expression(parser, 0);

    //                 if (!ignore_semicolon)
    //                 {
    //                     struct token semicolon = get_token(parser);
    //                     if (semicolon.type == ';')
    //                     {
    //                         eat_token(parser);
    //                     }
    //                     else
    //                     {
    //                         // @todo: report error
    //                     }
    //                 }
    //             }
    //         }

    //         result = make_new_ast_node(parser);
    //         result->kind = AST__DECLARATION;
    //         result->declaration.is_constant = is_constant;
    //         result->declaration.decl_type = type;
    //         result->declaration.lhs  = lhs;
    //         result->declaration.init = initializer;
    //     }
    // }

    return NULL;
}


void debug_print_ast_int_literal(ast_node *node)
{
}

void debug_print_ast__tuple_decl(ast_node *node, int depth = 0)
{
    auto *decl = &node->m_tuple_decl;
    if (decl->count == 0)
    {
        printf("()");
    }
    else if (decl->count == 1)
    {
        if (decl->elements[0] == NULL)
        {
            printf("(%.*s)", (int) decl->names[0].size, decl->names[0].data);
        }
        else
        {
            printf("(%.*s:", (int) decl->names[0].size, decl->names[0].data);
            debug_print_ast__tuple_decl(decl->elements[0]);
            printf(")");
        }
    }
    else
    {
        printf("(");
        debug_print_ast__tuple_decl(decl->elements[0]);
        for (int i = 1; i < decl->count; i++)
        {
            printf(", ");
            debug_print_ast__tuple_decl(decl->elements[i]);
        }
        printf(")");
    }
}

void debug_print_ast__func_decl(ast_node *node, int depth = 0)
{
    auto *decl = &node->m_func_decl;
    debug_print_ast__tuple_decl(decl->argument);
    printf(" -> ");
    if (decl->returns)
    {
        debug_print_ast__tuple_decl(decl->returns);
    }
    else
    {
        printf("()");
    }
}

void debug_print_ast__binary_op(ast_node *node, int depth = 0)
{
    auto *binop = &node->m_bin_op;

    printf("(%.*s)---", (int) binop->op.size, binop->op.data);
    debug_print_ast(binop->lhs, depth + 1);
    printf("%.*s \\---", (depth) * 7, spaces);
    debug_print_ast(binop->rhs, depth + 1);
}

void debug_print_ast(ast_node *node, int depth)
{
    switch (node->kind)
    {
    case AST_NODE__VARIABLE:
        {
            auto name = node->m_variable.name;
            printf("v:%.*s;", (int) name.size, name.data);
        }
        break;
    case AST_NODE__INT_LIT:
        {
            printf("l:%lld;", node->m_int_lit.n);
        }
        break;
    case AST_NODE__TUPLE_DECL:
        debug_print_ast__tuple_decl(node);
        break;
    case AST_NODE__FUNC_DECL:
        debug_print_ast__func_decl(node);
        break;
    case AST_NODE__BINARY_OP:
        debug_print_ast__binary_op(node, depth);
        break;
    default:
        printf("<error>");
    }
    printf("\n");
}

#endif

} // namespace pinapl

