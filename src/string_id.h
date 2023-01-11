#ifndef SRC_STRING_ID
#define SRC_STRING_ID

#include <base.h>
#include <string.h>


struct string_id
{
    usize id;
};

void initialize_string_id_storage(void *memory, usize size);
struct string_id make_string_id(char *string, usize size);
struct string_id make_string_id_from_cstring(char *string);
struct string get_string_by_id(struct string_id id);


#endif // SRC_STRING_ID

