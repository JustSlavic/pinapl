#include "lexer.hpp"
#include <util.hpp>


namespace pinapl
{

char const *token_kind_to_cstr(token_kind tk)
{
    switch (tk)
    {
    case TOKEN_INVALID: return "TOKEN_INVALID";

    case TOKEN_PAREN_OPEN: return "TOKEN_PAREN_OPEN";
    case TOKEN_PAREN_CLOSE: return "TOKEN_PAREN_CLOSE";

    case TOKEN_BRACKET_OPEN: return "TOKEN_BRACKET_OPEN";
    case TOKEN_BRACKET_CLOSE: return "TOKEN_BRACKET_CLOSE";

    case TOKEN_BRACE_OPEN: return "TOKEN_BRACE_OPEN";
    case TOKEN_BRACE_CLOSE: return "TOKEN_BRACE_CLOSE";

    case TOKEN_EQUALS: return "TOKEN_EQUALS";
    case TOKEN_COLON: return "TOKEN_COLON";
    case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
    case TOKEN_PLUS: return "TOKEN_PLUS";
    case TOKEN_MINUS: return "TOKEN_MINUS";
    case TOKEN_ASTERICS: return "TOKEN_ASTERICS";
    case TOKEN_SLASH: return "TOKEN_SLASH";
    case TOKEN_COMMA: return "TOKEN_COMMA";

    case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
    case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";

    case TOKEN_ARROW_RIGHT: return "TOKEN_ARROW_RIGHT";

    case TOKEN_KW_RETURN: return "TOKEN_KW_RETURN";

    case TOKEN_KW_BOOL: return "TOKEN_KW_BOOL";
    case TOKEN_KW_TRUE: return "TOKEN_KW_TRUE";
    case TOKEN_KW_FALSE: return "TOKEN_KW_FALSE";
    case TOKEN_KW_INT: return "TOKEN_KW_INT";
    case TOKEN_KW_FLOAT: return "TOKEN_KW_FLOAT";

    case TOKEN_EOF: return "TOKEN_EOF";
    }
    return "<error>";
}


struct keyword_table_entry
{
    char const *keyword;
    token_kind  keyword_kind;
}

GLOBAL keywords[] = {
    { "return", TOKEN_KW_RETURN },
    { "int", TOKEN_KW_INT },
};


lexer lexer::from(void *data, usize size)
{
    lexer result;
    *(lexer_base *) &result = lexer_base::from(data, size);
    result.current_token_ok = false;

    return result;
}

token lexer::get_token()
{
    consume_while(is_ascii_whitespace);

    token t = {};
    t.line = line;
    t.column = column;

    char c = get_char();
    if (c == 0)
    {
        t.kind = TOKEN_EOF;
    }
    else if (is_valid_identifier_head(c))
    {
        t.kind = TOKEN_IDENTIFIER;
        t.span.data = get_remaining_input();
        t.span.size = consume_while(is_valid_identifier_body);

        for (auto kw : keywords)
        {
            if (t.span == kw.keyword)
            {
                t.kind = kw.keyword_kind;
                break;
            }
        }
    }
    else
    {
        t.kind = (token_kind) c;
        eat_char();
    }

    current_token = t;
    current_token_ok = true;

    return t;
}

token lexer::eat_token()
{
    token result = current_token_ok ? current_token : get_token();
    current_token_ok = false;
    return result;
}



} // namespace pinapl

// void lexer__tokenize(struct lexer *lexer)
// {
//     while (lexer->stream.count < lexer->stream.capacity)
//     {
//         consume_while(lexer, is_ascii_whitespace);

//         token t = {
//             .line = lexer->line,
//             .column = lexer->column,
//         };


//         char c = get_char(lexer);
//         if (c == 0)
//         {
//             t.kind = TOKEN_EOF;
//             break;
//         }
//         else if (is_valid_identifier_head(c))
//         {
//             t.kind = TOKEN_IDENTIFIER;
//             t.span.data = lexer->buffer + lexer->cursor;
//             t.span.size = consume_while(lexer, is_valid_identifier_body);

//             for (int keyword_index = 0; keyword_index < ARRAY_COUNT(keywords); keyword_index++)
//             {
//                 char *keyword = keywords[keyword_index];
//                 if (cstring__compare_cn(keyword, t.span.data, t.span.size) == 0)
//                 {
//                     t.kind = keyword_types[keyword_index];
//                     break;
//                 }
//             }
//         }
//         else if (is_ascii_digit(c))
//         {
//             t.kind = TOKEN_LITERAL_INT,
//             t.span.data = lexer->buffer + lexer->cursor;

//             isize integer_value = 0;
//             while (is_ascii_digit(c))
//             {
//                 integer_value *= 10;
//                 integer_value += (c - '0');
//                 eat_char(lexer);
//                 c = get_char(lexer);
//                 t.span.size += 1;
//             }

//             t.integer_value = integer_value;
//         }
//         else
//         {
//             t.span.data = lexer->buffer + lexer->cursor;

//             eat_char(lexer);
//             char c1 = get_char(lexer);

//             if ((c == '-') && (c1 == '>'))
//             {
//                 t.kind = TOKEN_ARROW_RIGHT;
//                 t.span.size = 2;
//                 eat_char(lexer);
//             }
//             else
//             {
//                 t.kind = (enum token_type) c;
//                 t.span.size = 1;
//             }
//         }

//         lexer->stream.tokens[lexer->stream.count++] = t;
//     }
// }

// int get_precedence(struct token operator)
// {
//     switch (operator.kind)
//     {
//         case '+':
//         case '-':
//             return 1;
//         case '*':
//         case '/':
//             return 2;
//         default:
//             return 0;
//     }
//     return 0;
// }
