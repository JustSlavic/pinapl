#ifndef ARRAY_H
#define ARRAY_H

#include <base.h>


struct array
{
    void *data;
    usize size;
    usize capacity;
};


#define array_push(ARRAY, VALUE) array_push_(&ARRAY, &VALUE, sizeof(VALUE))

void array_push_impl(struct array *array, void *memory, usize size);


#endif // ARRAY_H
