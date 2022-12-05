#include "static_checks.h"
#include <parser.h>
#include <primes.h>


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


void pinapl_push_nested_scope(pinapl_scope *scope, pinapl_scope *nested)
{
    if (scope->nested_scopes_count < ARRAY_COUNT(scope->nested_scopes))
    {
        scope->nested_scopes[scope->nested_scopes_count++] = nested;
    }
}


pinapl_scope_entry *pinapl_get_scope_entry_slot(pinapl_scope *scope, u32 hash)
{     
    pinapl_scope_entry *result = NULL;
    for (int offset = 0; offset < ARRAY_COUNT(scope->hash_table); offset++)
    {
        int index = (hash + offset) % ARRAY_COUNT(scope->hash_table);
        pinapl_scope_entry *entry = scope->hash_table + index;
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


pinapl_scope_entry *pinapl_get_scope_entry(pinapl_scope *scope, char *string, usize string_size)
{
    pinapl_scope_entry *result = NULL;

    u32 hash = pinapl_hash_string(string, string_size);
    pinapl_scope_entry *slot = pinapl_get_scope_entry_slot(scope, hash);
    if (slot->hash != 0)
    {
        result = slot;
    }
    return result;
}


pinapl_scope_entry *pinapl_push_scope_entry(pinapl_scope *scope, char *string, usize string_size)
{
    u32 hash = pinapl_hash_string(string, string_size);
    pinapl_scope_entry *result = pinapl_get_scope_entry_slot(scope, hash);
    if (result->hash == 0)
    {
        result->hash = hash;
        result->entry_name = string;
        result->entry_name_size = string_size;
    }
    return result;
}


b32 pinapl_check_scopes(allocator *a, ast_node *node, pinapl_scope *scope)
{
    b32 result = true;

    switch (node->type)
    {
        case AST_NODE_GLOBAL_DECLARATION_LIST:
        {
            result = pinapl_check_scopes(a, node->declaration, scope);
            if (node->next_declaration)
            {
                result = result && pinapl_check_scopes(a, node->next_declaration, scope);
            }
        }
        break;

        case AST_NODE_STATEMENT_LIST:
        {
            result = pinapl_check_scopes(a, node->statement, scope);
            if (node->next_statement)
            {
                result = result && pinapl_check_scopes(a, node->next_statement, scope);
            }
        }
        break;

        case AST_NODE_BLOCK:
        {
            pinapl_scope *inner_scope = ALLOCATE(a, pinapl_scope);
            pinapl_push_nested_scope(scope, inner_scope);

            result = pinapl_check_scopes(a, node->statement_list, inner_scope);
        }
        break;
        
        case AST_NODE_VARIABLE_DECLARATION:
        case AST_NODE_CONSTANT_DECLARATION:
        {
            /*token var_name;
            token var_type;
            // for compound types:
            // struct ast_node *var_type;
            b32   is_constant;
            struct ast_node *init;
            */  
            if (node->init)
            {
                result = pinapl_check_scopes(a, node->init, scope);
            }

            if (result)
            {
                u32 hash = pinapl_hash_string(node->var_name.span, node->var_name.span_size);
                pinapl_scope_entry *slot = pinapl_get_scope_entry_slot(scope, hash);
                if (slot->hash == 0)
                {
                    slot->hash = hash;
                    slot->entry_name = node->var_name.span;
                    slot->entry_name_size = node->var_name.span_size;
                }
                else
                {
                    // Error
                    result = false;
                }
            }
        }
        break;

        case AST_NODE_FUNCTION_DEFINITION:
        {
        /* struct  // function definition
        {
            struct ast_node *parameter_list;
            struct ast_node *return_type;
            struct ast_node *statement_list;
        };*/
            if (node->block)
            {
                result = result && pinapl_check_scopes(a, node->block, scope);
            }
        }
        break;

        case AST_NODE_BINARY_OPERATOR:
        {
            result = result && pinapl_check_scopes(a, node->lhs, scope);
            result = result && pinapl_check_scopes(a, node->rhs, scope);
        }
        break;

        case AST_NODE_LITERAL_INT:
        break;

        case AST_NODE_VARIABLE:
        {
            /*
        struct  // variable/constant usage
        {
            char *var_span;
            usize var_span_size;
        };
            */

            pinapl_scope_entry *entry = pinapl_get_scope_entry(scope, node->var_span, node->var_span_size);
            if (entry == NULL)
            {
                // Error! used non-defined variable!
                result = false;
            }
        }
        break;

        case AST_NODE_FUNCTION_CALL:
        {
        /*
        struct  // function call
        {
            token function_name;
            struct ast_node *argument_list;
        };
        */
        }
        break;

        case AST_NODE_INVALID:
        {
            result = false;
        }
        break;
    }

    return result;
}

