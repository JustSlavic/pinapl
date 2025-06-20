#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include "common.h"


typedef struct
{
    char const *data;
    uint32      size;
} string_view;


string_view make_string_view_from_cstring(char const *);
bool32 string_view_equal(string_view s1, string_view s2);

#define STRING_VIEW_FMT "\"%.*s\""
#define STRING_VIEW_FMT_ "%.*s"
#define STRING_VIEW_PRINT(SV) (int) ((SV).size), ((SV).data)


#endif // STRING_VIEW_H
