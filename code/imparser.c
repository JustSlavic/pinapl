#include "imparser.h"
#include <lexer.h>


static lexer *global_imparser_lexer;


void imparser_init(lexer *l)
{
    global_imparser_lexer = l;
}

static bool imparser_token(int token_tag, token *result)
{

    token t = lexer_get_token(global_imparser_lexer);
    if (t.tag != token_tag) return false;
    lexer_eat_token(global_imparser_lexer);
    *result = t;
    return true;
}

bool imparser_paren_open(token *result) { return imparser_token('(', result); }
bool imparser_paren_close(token *result) { return imparser_token(')', result); }
bool imparser_bracket_open(token *result) { return imparser_token('[', result); }
bool imparser_bracket_close(token *result) { return imparser_token(']', result); }
bool imparser_brace_open(token *result) { return imparser_token('{', result); }
bool imparser_brace_close(token *result) { return imparser_token('}', result); }
bool imparser_colon(token *result) { return imparser_token(':', result); }
bool imparser_semicolon(token *result) { return imparser_token(';', result); }
bool imparser_plus(token *result) { return imparser_token('+', result); }
bool imparser_minus(token *result) { return imparser_token('-', result); }
bool imparser_asterics(token *result) { return imparser_token('*', result); }
bool imparser_slash(token *result) { return imparser_token('/', result); }
bool imparser_comma(token *result) { return imparser_token(',', result); }
bool imparser_period(token *result) { return imparser_token('.', result); }
bool imparser_equals(token *result) { return imparser_token('=', result); }
bool imparser_more(token *result) { return imparser_token('>', result); }
bool imparser_less(token *result) { return imparser_token('<', result); }

bool imparser_identifier(token *result)
{
    token t = lexer_get_token(global_imparser_lexer);
    if (t.tag != TOKEN_IDENTIFIER) return false;
    lexer_eat_token(global_imparser_lexer);
    *result = t;
    return true;
}

bool imparser_keyword(token *result)
{
    token t = lexer_get_token(global_imparser_lexer);
    if (t.tag & TOKEN_KEYWORD) return false;
    lexer_eat_token(global_imparser_lexer);
    *result = t;
    return true;
}

bool imparser_integer(token *result)
{
    token t = lexer_get_token(global_imparser_lexer);
    if (t.tag != TOKEN_LITERAL_INTEGER) return false;
    lexer_eat_token(global_imparser_lexer);
    *result = t;
    return true;
}
