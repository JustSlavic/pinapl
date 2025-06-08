#include "string_view.h"


string_view make_string_view_from_cstring(char const *s)
{
    string_view result = { .data = s };
    while (*s++) result.size++;
    return result;
}

bool32 string_view_equal(string_view s1, string_view s2)
{
    if (s1.size != s2.size) return false;
    for (uint32 i = 0; i < s1.size; i++)
    {
        bool32 diff = (s1.data[i] - s2.data[i]);
        if (diff != 0) return false;
    }
    return true;
}
