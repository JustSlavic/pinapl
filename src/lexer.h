#ifndef LEXER_H
#define LEXER_H

#include <base.h>


typedef bool predicate(char);


typedef enum
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

    TOKEN_IDENTIFIER = 256,
    TOKEN_INT_LITERAL,
    
    TOKEN_KW_RETURN = 300,
    
    TOKEN_EOF = 500,
} token_type;


char *token_type_to_cstring(token_type);


typedef struct
{
    token_type type;

    int line;
    int column;

    int integer_value;

    char *span;
    int span_size;
} token;


typedef struct
{
    char *buffer;
    int buffer_size;

    int index;

    int line;
    int column;

    token next_token;
    bool next_token_valid;
} lexer;


bool is_ascii_space(char c);
bool is_ascii_whitespace(char c);
bool is_ascii_alpha(char c);
bool is_ascii_digit(char c);
bool is_valid_identifier_head(char c);
bool is_valid_identifier_body(char c);

char lexer_get_char(lexer *);
char lexer_eat_char(lexer *);
void consume_while(lexer *, predicate *);
void consume_until(lexer *, predicate *);

token lexer_get_token(lexer *);
token lexer_eat_token(lexer *);


#endif // LEXER_H
