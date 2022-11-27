#include "memory.h"


void *memset(void *destination, int value, size_t count)
{
    char *d = (char *) destination;
    while (count--)
    {
        *d++ = (char) value;
    }
    return destination;
}


void *memcpy(void *destination, void const *source, size_t count)
{
    char *d = (char *) destination;
    char *s = (char *) source;
    while (count--)
    {
        *d++ = *s++;
    }
    return destination;
}

