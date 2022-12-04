#ifndef STATIC_CHECKS_H
#define STATIC_CHECKS_H

#include <base.h>
#include <allocator.h>
#include <parser.h>


typedef struct pinapl_scope_entry
{
    char *entry_name;
    u32   entry_name_size;
    u32   hash;
} pinapl_scope_entry;


typedef struct pinapl_scope
{
    pinapl_scope_entry hash_table[128];
} pinapl_scope;


b32 pinapl_check_scopes(allocator *a, ast_node *node, pinapl_scope *scope);


#endif // STATIC_CHECKS_H

