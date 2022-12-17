#ifndef STATIC_CHECKS_H
#define STATIC_CHECKS_H

#include <base.h>
#include <allocator.h>
#include <parser.h>


struct pinapl_scope_entry
{
    char *entry_name;
    usize entry_name_size;
    u32   hash;
};


struct pinapl_scope
{
    struct pinapl_scope *parent_scope;
    struct pinapl_scope *next_scope;
    struct pinapl_scope *nested_scope;
    
    struct pinapl_scope_entry hash_table[64];
};


void pinapl_push_nested_scope(struct pinapl_scope *scope, struct pinapl_scope *nested);
b32 pinapl_check_scopes(struct allocator *a, ast_node *node, struct pinapl_scope *scope);


#endif // STATIC_CHECKS_H

