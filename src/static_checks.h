#ifndef STATIC_CHECKS_H
#define STATIC_CHECKS_H

#include <base.h>
#include <allocator.h>
#include <parser.h>


typedef struct pinapl_scope_entry
{
    char *entry_name;
    usize entry_name_size;
    u32   hash;
} pinapl_scope_entry;


typedef struct pinapl_scope
{
    pinapl_scope_entry hash_table[64];

    ast_node *parent_ast_node;
    struct pinapl_scope *next_scope;
    struct pinapl_scope *nested_scopes[8];
    usize  nested_scopes_count;
} pinapl_scope;


void pinapl_push_nested_scope(pinapl_scope *scope, pinapl_scope *nested);
b32 pinapl_check_scopes(struct allocator *a, ast_node *node, pinapl_scope *scope);


#endif // STATIC_CHECKS_H

