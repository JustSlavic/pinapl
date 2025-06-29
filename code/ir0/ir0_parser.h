#ifndef IR0_PARSER_H
#define IR0_PARSER_H

#include "../common.h"
#include "ir0_stream.h"
#include "ir0_lexer.h"


ir0_stream ir0_parse_text(char const *source, int32 source_size);


#endif // IR0_PARSER_H
