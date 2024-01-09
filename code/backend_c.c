#include "backend_c.h"


void translate_type_to_c(struct translator_to_c *translator, struct type_entry *type)
{
    string_builder *output = translator->output;
    switch (type->kind)
    {
        case TYPE__VOID:
            string_builder__append_format(output, "void");
        break;

        case TYPE__NAME:
        {
            string_builder__append_format(output, "%.*s", (int) type->name.size, type->name.data);
        }
        break;

        case TYPE__TUPLE:
        {
            string_builder__append_format(output, "__TUPLE_");
            for (int i = 0; i < type->tuple_count; i++)
            {
                translate_type_to_c(translator, type->tuple_types[i]);
                string_builder__append_format(output, "_");
            }
        }
        break;

        case TYPE__FUNCTION:
        {
            string_builder__append_format(output, "__FUNCTION_TYPE__");
        }
        break;
    }
}

void translator_c__predeclare_types(struct translator_to_c *translator, struct type_registry *registry)
{
    string_builder *output = translator->output;
    for (int i = 0; i < registry->count; i++)
    {
        struct type_entry *entry = registry->entries + i;
        string_builder__append_format(output, "define ");
        translate_type_to_c(translator, entry);
        string_builder__append_format(output, "\n");
    }
}

void translate_to_c(struct translator_to_c *translator, struct ast_node *ast, int depth)
{
    char spaces[] = "                                                 ";
    string_builder *output = translator->output;

    switch (ast->kind)
    {
        case AST__BLOCK:
        {
            string_builder__append_format(output, "{\n");
            translate_to_c(translator, ast->block.statements, depth + 1);
            string_builder__append_format(output, "%.*s}\n", depth * 4, spaces);
        }
        break;

        case AST__STATEMENT:
        {
            string_builder__append_format(output, "%.*s", depth * 4, spaces);
            translate_to_c(translator, ast->statement.stmt, depth);
            if (ast->statement.stmt->kind != AST__BLOCK)
                string_builder__append_string(output, ";\n");
            if (ast->statement.next != NULL)
            {
                translate_to_c(translator, ast->statement.next, depth);
            }
        }
        break;

        case AST__DECLARATION:
        {
            // @warn: This if is temporary, declaration.type should be always present
            // after the type infer stage
            if (ast->declaration.type)
                translate_type_to_c(translator, ast->declaration.type);
            else
                string_builder__append_format(output, "__TYPE_INFER__");
            // string_builder__append_format(output, " %.*s",
            //     (int) ast->declaration.name.size, ast->declaration.name.data);
            if (ast->declaration.init != NULL)
            {
                string_builder__append_string(output, " = ");
                translate_to_c(translator, ast->declaration.init, depth);
            }
        }
        break;

        case AST__BINARY_OPERATOR:
        {
            translate_to_c(translator, ast->binary_operator.lhs, depth);
            string_builder__append_format(output, "%c", ast->binary_operator.operator);
            translate_to_c(translator, ast->binary_operator.rhs, depth);
        }
        break;

        case AST__LITERAL_INT:
        {
            string_builder__append_format(output, "%lld", ast->literal_int.value);
        }
        break;

        case AST__VARIABLE:
        {
            string_builder__append_format(output, "%.*s", (int) ast->variable.name.size, ast->variable.name.data);
        }
        break;

        case AST__FUNCTION_CALL:
        {
            string_builder__append_format(output, "%.*s(", (int) ast->function_call.name.size, ast->function_call.name.data);
            // translate_to_c(translator, ast->function_call.arg1, depth);
            string_builder__append_string(output, ")");
        }
        break;

        case AST__TUPLE:
        {
#if 0
            string_builder__append_format(output, "(");
            struct ast_node *tuple = ast;
            if (tuple->tuple.value)
                translate_to_c(translator, tuple->tuple.value, depth);
            else
                string_builder__append_format(output, "void");

            tuple = tuple->tuple.next;

            while (true)
            {
                if (tuple->kind == AST__TUPLE)
                {
                    string_builder__append_format(output, ", ");
                    if (tuple->tuple.value)
                        translate_to_c(translator, tuple->tuple.value, depth);
                    else
                        string_builder__append_format(output, "void");
                    tuple = tuple->tuple.next;
                }
                else
                {
                    string_builder__append_format(output, ", ");
                    translate_to_c(translator, tuple, depth);
                    break;
                }
            }
            string_builder__append_format(output, ")");
#endif
        }
        break;

        case AST__FUNCTION:
        {
            string_builder__append_format(output, "__FUNCTION__");
        }
        break;

        default:
        break;
    }
}
