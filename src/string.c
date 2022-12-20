#include "string.h"


int cstring_size_no0(char const *s)
{
    int count = 0;
    while (*s++) count++;
    return count;
}


int cstring_size_with0(char const *s)
{
    int count = cstring_size_no0(s);
    return count + 1;
}

struct string make_string_from_cstring(char const *s)
{
    usize size = cstring_size_no0(s);

    struct string result;
    result.data = (char *) s;
    result.size = size;
    return result;
}

