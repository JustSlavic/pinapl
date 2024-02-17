#ifndef PINAPL__LEXER_H
#define PINAPL__LEXER_H

#include <base.h>
#include <string_view.h>


enum token_type
{
    TOKEN_INVALID = 0,

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_EQUALS = '=',
    TOKEN_COLON = ':',
    TOKEN_SEMICOLON = ';',
    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_ASTERICS = '*',
    TOKEN_SLASH = '/',
    TOKEN_COMMA = ',',

    TOKEN_IDENTIFIER = 256,
    TOKEN_LITERAL_INT = 257,

    TOKEN_ARROW_RIGHT = 271,
    
    TOKEN_KW_RETURN = 300,

    TOKEN_KW_BOOL,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,

    TOKEN_EOF = 500,
};

struct token
{
    enum token_type kind;

    int line;
    int column;

    union
    {
        int32 boolean_value;
        int64 integer_value;
        float floating_value;
    };

    struct string_view span;
};
typedef struct token token;

FORCE_INLINE token token__invalid()
{
    token result;
    result.kind = TOKEN_INVALID;
    return result;
}

struct token_stream
{
    token *tokens;
    usize count;
    usize capacity;
};

struct lexer
{
    char const *buffer;
    usize buffer_size;

    usize cursor;

    usize line;
    usize column;

    struct token_stream stream;

    struct logger *logger;
};


void lexer__tokenize(struct lexer *lexer);
int get_precedence(struct token operator);
void debug_print_token_stream(struct lexer *lexer);


#endif // PINAPL__LEXER_H
