#include "lexer.h"
#include "ascii.h"

static bool32 is_valid_identifier_head(char c) { return (c == '_') || is_ascii_alpha(c); }
static bool32 is_valid_identifier_body(char c) { return (c == '_') || is_ascii_alpha(c) || is_ascii_digit(c); }

char get_char(lexer *l)
{
    char c = 0;
    if (l->cursor < l->size)
        c = l->data[l->cursor];
    return c;
}

char eat_char(lexer *l)
{
    char c = get_char(l);
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

int32 consume_while(lexer *l, predicate_t *p)
{
    int count = 0;
    char c = get_char(l);
    while ((c > 0) && p(c))
    {
        eat_char(l);
        count += 1;
        c = get_char(l);
    }
    return count;
}

int32 consume_until(lexer *l, predicate_t *p)
{
    int count = 0;
    char c = get_char(l);
    while ((c > 0) && !p(c))
    {
        eat_char(l);
        count += 1;
        c = get_char(l);
    }
    return count;
}

int eat_string(lexer *l, char const *s, uint32 n)
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

int32 find_keyword(lexer *l, string_view span)
{
    for (uint32 keyword_index = 0; keyword_index < l->keyword_count; keyword_index++)
    {
        if (string_view_equal(l->keywords[keyword_index], span))
        {
            return l->keyword_tags[keyword_index];
        }
    }
    return Token_Invalid;
}

token get_token(lexer *l)
{
    if (l->current_token_ok)
        return l->current_token;

    consume_while(l, is_ascii_whitespace);

    token t = {
        .tag = Token_Invalid,
        .line = l->line,
        .column = l->column,
    };

    char c = get_char(l);
    if (c == 0)
    {
        t.tag = Token_Eof;
    }
    else if (is_valid_identifier_head(c))
    {
        t.tag = Token_Identifier;
        t.span.data = (char const *) (l->data + l->cursor);
        t.span.size = consume_while(l, is_valid_identifier_body);

        int32 keyword_tag = find_keyword(l, t.span);
        if (keyword_tag > Token_Invalid)
            t.tag = keyword_tag;
    }
    else if (is_ascii_digit(c))
    {
        t.tag = Token_LiteralInteger;
        t.span.data = (char const *) (l->data + l->cursor);
        t.span.size = consume_while(l, is_ascii_digit);

        int64 integer_value = 0;
        for (uint32 i = 0; i < t.span.size; i++)
        {
            integer_value *= 10;
            integer_value += (t.span.data[i] - '0');
        }
        t.integer_value = integer_value;
    }
    else
    {
        t.tag = c;
        t.span.data = (char const *) (l->data + l->cursor);
        t.span.size = 1;

        eat_char(l);
    }

    l->current_token = t;
    l->current_token_ok = true;
    return t;
}

token eat_token(lexer *l)
{
    token result = get_token(l);
    l->current_token_ok = false;
    return result;
}
