#include "string_view.h"


string_view string_view_make_from_cstring(char const *s)
{
    string_view result = { .data = s };
    while (*s++) result.size++;
    return result;
}

bool string_view_is_equal(string_view s1, string_view s2)
{
    size_t i;
    if (s1.size != s2.size) return false;
    for (i = 0; i < s1.size; i++)
    {
        if (s1.data[i] != s2.data[i]) return false;
    }
    return true;
}
