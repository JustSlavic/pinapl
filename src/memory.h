#ifndef MEMORY_H
#define MEMORY_H

#include "base.h"

#ifdef CRT
#include <string.h>
#else
void *memset(void *destination, int value, usize count);
void *memcpy(void *destination, void const *source, usize count);
#endif

usize get_padding(void *pointer, usize alignment);
void *align_pointer(void *pointer, usize alignment);

struct memory_block
{
    void *memory;
    usize size;
};


struct memory_block allocate_pages(usize size);
struct memory_block allocate_pages_at(void *address, usize size);


#endif // MEMORY_H
