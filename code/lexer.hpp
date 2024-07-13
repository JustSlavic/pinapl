#ifndef PINAPL__LEXER_HPP
#define PINAPL__LEXER_HPP

#include <base.h>
#include <lexer.hpp>
#include <string_view.hpp>


namespace pinapl
{


enum token_kind
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
    TOKEN_KW_INT,
    TOKEN_KW_FLOAT,

    TOKEN_EOF = 500,
};

char const *token_kind_to_cstr(token_kind tk);

struct token
{
    enum token_kind kind;

    int line;
    int column;

    union
    {
        int32 boolean_value;
        int64 integer_value;
        float floating_value;
    };

    string_view span;
};


struct lexer : lexer_base
{
    token  current_token;
    bool32 current_token_ok;

    static lexer from(void *data, usize size);

    token get_token();
    token eat_token();
};


} // namespace pinapl

// FORCE_INLINE token token__invalid()
// {
//     token result;
//     result.kind = TOKEN_INVALID;
//     return result;
// }

// struct token_stream
// {
//     token *tokens;
//     usize count;
//     usize capacity;
// };

// struct pinapl_lexer : lexer
// {
//     struct token_stream stream;
//     struct logger *logger;
// };


// void lexer__tokenize(struct lexer *lexer);
// int get_precedence(struct token operator);
// void debug_print_token_stream(struct lexer *lexer);


#endif // PINAPL__LEXER_HPP
