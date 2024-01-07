#ifndef PINAPL__BACKEND_C_H
#define PINAPL__BACKEND_C_H

#include <base.h>
#include <string_builder.h>
#include "parser.h"


void translate_to_c(struct ast_node *ast, string_builder *output, int depth);


#endif // PINAPL__BACKEND_C_H
