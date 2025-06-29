#include "ir0_lexer.h"
#include "../ascii.h"


bool32 ir0_is_valid_identifier_head(char c)
{
    return is_ascii_alpha(c) || (c == '.') || (c == '_');
}

bool32 ir0_is_valid_identifier_body(char c)
{
    return is_ascii_alpha(c) || is_ascii_digit(c) || (c == '_');
}
