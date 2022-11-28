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

