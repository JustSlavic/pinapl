#include "memory.h"

#if CRT 

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS
#endif

#define mmap2 mmap

#else

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

#endif // CRT


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
    result.memory = mmap2(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    result.size = size;

    return result; 
}

struct memory_block allocate_pages_at(void *address, usize size)
{
    struct memory_block result;
    result.memory = mmap2(address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    result.size = size;

    return result;
}

