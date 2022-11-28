#ifndef STRING_H
#define STRING_H


typedef struct string
{
    char *data;
    int   size;
} string;


int cstring_size_no0(char const *s);
int cstring_size_with0(char const *s);


#endif // STRING_H
