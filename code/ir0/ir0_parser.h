#ifndef PINAPL_IR0_PARSER_H_
#define PINAPL_IR0_PARSER_H_

#include <base.h>
#include <string_view.h>
#include <lexer.h>


char const *ir0_token_tag_to_cstring(int tag);
uint32 ir0_get_keywords(string_view **kst, int **kvt);
bool32 ir0_parse_instruction(lexer *l);


#endif /* PINAPL_IR0_PARSER_H_ */
