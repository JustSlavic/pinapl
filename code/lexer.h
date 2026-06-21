#ifndef LEXER_H
#define LEXER_H

#include "base.h"
#include "string_view.h"


enum
{
    TOKEN_INVALID = -1,
    TOKEN_EOF = 0,

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_COLON = ':',
    TOKEN_SEMICOLON = ';',

    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_ASTERICS = '*',
    TOKEN_SLASH = '/',
    TOKEN_COMMA = ',',
    TOKEN_PERIOD = '.',

    TOKEN_EQUALS = '=',
    TOKEN_MORE = '>',
    TOKEN_LESS = '<',

    TOKEN_IDENTIFIER = 0x100, /* 256 */

    TOKEN_LITERAL_INTEGER,
    TOKEN_LITERAL_FLOAT,
    TOKEN_LITERAL_STRING,

    TOKEN_COUNT,
};

char const *token_tag_to_cstring(int);

typedef struct token
{
    int32 tag;
    int32 line;
    int32 column;

    union
    {
        int64   boolean_value;
        int64   integer_value;
        float64 float_value;
    };

    string_view span;
} token;

typedef bool lexer_predicate_t(char);

typedef struct lexer
{
    char const *data;
    uint32      size;

    uint32 cursor;
    uint32 line;
    uint32 column;

    token  current_token;
    bool32 current_token_ok;

    string_view *keywords;
    int32       *keyword_tags;
    uint32       keyword_count;
} lexer;

char  lexer_get_char(lexer *);
char  lexer_eat_char(lexer *);
void  lexer_eat_crlf(lexer *);
int32 lexer_consume_while(lexer *, lexer_predicate_t *p);
int32 lexer_consume_until(lexer *, lexer_predicate_t *p);
int   lexer_eat_string(lexer *, const char *, uint32);
token lexer_get_token(lexer *);
token lexer_eat_token(lexer *);

#endif /* LEXER_H */
