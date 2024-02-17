#include "memory.h"
#include <stdlib.h>


usize get_padding(void *pointer, usize alignment)
{
    usize result = (alignment - ((usize)pointer & (alignment - 1))) & (alignment - 1);
    return result;
}


void *align_pointer(void *pointer, usize alignment)
{
    void *result = pointer + get_padding(pointer, alignment);
    return result;
}


struct memory_block allocate_pages(usize size)
{
    struct memory_block result;
    result.memory = malloc(size);
    result.size = size;

    return result; 
}

struct memory_block allocate_pages_at(void *address, usize size)
{
    struct memory_block result;
    result.memory = malloc(size);
    result.size = size;

    return result;
}

