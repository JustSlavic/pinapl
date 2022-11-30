#include "memory.h"
#include <syscall.h>


void *memset(void *destination, int value, usize count)
{
    char *d = (char *) destination;
    while (count--)
    {
        *d++ = (char) value;
    }
    return destination;
}


void *memcpy(void *destination, void const *source, usize count)
{
    char *d = (char *) destination;
    char *s = (char *) source;
    while (count--)
    {
        *d++ = *s++;
    }
    return destination;
}


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


memory_block allocate_pages(usize size)
{
    memory_block result;
    result.memory = mmap2(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    result.size = size;

    return result; 
}

memory_block allocate_pages_at(void *address, usize size)
{
    memory_block result;
    result.memory = mmap2(address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    result.size = size;

    return result;
}

