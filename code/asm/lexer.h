#ifndef LEXER_H
#define LEXER_H

#include "common.h"
#include "string_view.h"

enum
{
    Token_Invalid = -1,
    Token_Eof = 0,

    Token_ParenOpen = '(',
    Token_ParenClose = ')',

    Token_BracketOpen = '[',
    Token_BracketClose = ']',

    Token_BraceOpen = '{',
    Token_BraceClose = '}',

    Token_Colon = ':',
    Token_Semicolon = ';',

    Token_Plus = '+',
    Token_Minus = '-',
    Token_Asterics = '*',
    Token_Slash = '/',
    Token_Comma = ',',
    Token_Period = '.',

    Token_Equals = '=',
    Token_More = '>',
    Token_Less = '<',

    Token_Identifier = 0x100, // 256

    Token_LiteralInteger,
    Token_LiteralFloat,
    Token_LiteralString,

    Token_Count,
};

typedef struct token
{
    int32 tag;
    int32 line, column;

    union
    {
        int32 boolean_value;
        int64 integer_value;
        double  float_value;
    };

        string_view span;
} token;

typedef bool32 predicate_t(char);

typedef struct lexer
{
    uint8 *data;
    uint32 size;

    uint32 cursor;
    uint32 line;
    uint32 column;

    token  current_token;
    bool32 current_token_ok;

    string_view *keywords;
    int32       *keyword_tags;
    uint32       keyword_count;

    predicate_t *is_valid_identifier_head;
    predicate_t *is_valid_identifier_body;
} lexer;

char get_char(lexer *);
char eat_char(lexer *);
void eat_crlf(lexer *);
int32 consume_while(lexer *, predicate_t *p);
int32 consume_until(lexer *, predicate_t *p);
int eat_string(lexer *, const char *, uint32);
token get_token(lexer *);
token eat_token(lexer *);

bool32 is_valid_identifier_head(char c);
bool32 is_valid_identifier_body(char c);

#endif // LEXER_H
