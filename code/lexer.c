#include "lexer.h"
#include "ascii.h"


static bool is_valid_identifier_head(char c) { return (c == '_') || ascii_is_alpha(c); }
static bool is_valid_identifier_body(char c) { return (c == '_') || ascii_is_alpha(c) || ascii_is_digit(c); }

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

int lexer_eat_string(lexer *l, char const *s, uint32 n)
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
        t.span.data = (char const *) (l->data + l->cursor);
        t.span.size = lexer_consume_while(l, ascii_is_digit);

        int64 integer_value = 0;
        uint32 i;
        for (i = 0; i < t.span.size; i++)
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
