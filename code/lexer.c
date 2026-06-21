#include "lexer.h"
#include "ascii.h"


static bool is_valid_identifier_head(char c) { return (c == '_') || ascii_is_alpha(c); }
static bool is_valid_identifier_body(char c) { return (c == '_') || ascii_is_alpha(c) || ascii_is_digit(c); }

char const *token_tag_to_cstring(int tag)
{
    switch (tag)
    {
        case TOKEN_INVALID: return "TOKEN_INVALID";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_PAREN_OPEN: return "TOKEN_PAREN_OPEN";
        case TOKEN_PAREN_CLOSE: return "TOKEN_PAREN_CLOSE";
        case TOKEN_BRACKET_OPEN: return "TOKEN_BRACKET_OPEN";
        case TOKEN_BRACKET_CLOSE: return "TOKEN_BRACKET_CLOSE";
        case TOKEN_BRACE_OPEN: return "TOKEN_BRACE_OPEN";
        case TOKEN_BRACE_CLOSE: return "TOKEN_BRACE_CLOSE";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_ASTERICS: return "TOKEN_ASTERICS";
        case TOKEN_SLASH: return "TOKEN_SLASH";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_PERIOD: return "TOKEN_PERIOD";
        case TOKEN_EQUALS: return "TOKEN_EQUALS";
        case TOKEN_MORE: return "TOKEN_MORE";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_LITERAL_INTEGER: return "TOKEN_LITERAL_INTEGER";
        case TOKEN_LITERAL_FLOAT: return "TOKEN_LITERAL_FLOAT";
        case TOKEN_LITERAL_STRING: return "TOKEN_LITERAL_STRING";
        default: return "TOKEN_UNLISTED";
    }
    return "TOKEN_UNLISTED";
}

char lexer_get_char(lexer *l)
{
    char c = 0;
    if (l->cursor < l->size)
        c = l->data[l->cursor];
    return c;
}

char lexer_eat_char(lexer *l)
{
    char c = lexer_get_char(l);
    if (c != 0)
    {
        l->cursor += 1;
        l->column += 1;
    }

    if ((c == '\r') || (c == '\n'))
    {
        l->column = 1;
        if (c == '\n')
            l->line += 1;
    }
    return c;
}

int32 lexer_consume_while(lexer *l, lexer_predicate_t *p)
{
    int count = 0;
    char c = lexer_get_char(l);
    while ((c > 0) && p(c))
    {
        lexer_eat_char(l);
        count += 1;
        c = lexer_get_char(l);
    }
    return count;
}

int32 lexer_consume_until(lexer *l, lexer_predicate_t *p)
{
    int count = 0;
    char c = lexer_get_char(l);
    while ((c > 0) && !p(c))
    {
        lexer_eat_char(l);
        count += 1;
        c = lexer_get_char(l);
    }
    return count;
}

int32 lexer_eat_string(lexer *l, char const *s, uint32 n)
{
    uint32 count = 0;
    char const *s1 = (char const *) (l->data + l->cursor);
    while ((n != 0) && (*s && *s1 && (*s == *s1)))
    {
        if (n > 0) n -= 1;
        s += 1;
        s1 += 1;
        count += 1;
    }
    if ((n == 0) || (*s == 0))
    {
        l->cursor += count;
        return count;
    }
    return 0;
}

int32 lexer_find_keyword(lexer *l, string_view span)
{
    uint32 keyword_index;
    for (keyword_index = 0;
         keyword_index < l->keyword_count;
         keyword_index++)
    {
        if (string_view_is_equal(l->keywords[keyword_index], span))
        {
            return l->keyword_tags[keyword_index];
        }
    }
    return TOKEN_INVALID;
}

token lexer_get_token(lexer *l)
{
    if (l->current_token_ok)
        return l->current_token;

    lexer_consume_while(l, ascii_is_whitespace);

    token t = {
        .tag = TOKEN_INVALID,
        .line = l->line,
        .column = l->column,
    };

    char c = lexer_get_char(l);
    if (c == 0)
    {
        t.tag = TOKEN_EOF;
    }
    else if (is_valid_identifier_head(c))
    {
        t.tag = TOKEN_IDENTIFIER;
        t.span.data = (char const *) (l->data + l->cursor);
        lexer_eat_char(l); /* Consume identifier head now, becuse consume_while
                              might have predicate that does not include head character. */
        t.span.size = lexer_consume_while(l, is_valid_identifier_body) + 1;

        int32 keyword_tag = lexer_find_keyword(l, t.span);
        if (keyword_tag > TOKEN_INVALID)
            t.tag = keyword_tag;
    }
    else if (ascii_is_digit(c))
    {
        t.tag = TOKEN_LITERAL_INTEGER;
        char const *s = (char const *) (l->data + l->cursor);
        uint32 parsed_characters = parse_integer(s, l->size - l->cursor, &t.integer_value);
        t.span.data = s;
        t.span.size = parsed_characters;
        l->cursor += parsed_characters;
    }
    else
    {
        t.tag = c;
        t.span.data = (char const *) (l->data + l->cursor);
        t.span.size = 1;

        lexer_eat_char(l);
    }

    l->current_token = t;
    l->current_token_ok = true;
    return t;
}

token lexer_eat_token(lexer *l)
{
    token result = lexer_get_token(l);
    l->current_token_ok = false;
    return result;
}
