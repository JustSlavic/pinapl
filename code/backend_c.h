#ifndef PINAPL__BACKEND_C_H
#define PINAPL__BACKEND_C_H

#include <base.h>
#include <string_builder.h>
#include "parser.h"


struct translator_to_c
{
    struct string_builder *output;
    struct ast_node *tuples_to_predefine[32];
    uint32 tuples_to_predefine_count;
};

void translator_c__predeclare_types(struct translator_to_c *translator, struct type_registry *registry);
void translate_to_c(struct translator_to_c *translator, struct ast_node *ast,int depth);


#endif // PINAPL__BACKEND_C_H
