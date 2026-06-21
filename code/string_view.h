#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include <stdint.h>


typedef struct
{
    char const *data;
    size_t      size;
} string_view;


string_view string_view_make_from_cstring(char const *);
bool string_view_is_equal(string_view s1, string_view s2);

#define STRING_VIEW_FMT "\"%.*s\""
#define STRING_VIEW_FMT_UNQUOTED "%.*s"
#define STRING_VIEW_ARG(SV) (int) ((SV).size), ((SV).data)


#endif /* STRING_VIEW_H_ */
