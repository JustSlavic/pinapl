#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <base.h>
#include <memory.h>


typedef struct allocator
{
    u8 data[64];
} allocator;


void initialize_memory_arena(allocator *a, void *memory, usize size);
void initialize_memory_pool(allocator *a, void *memory, usize size);
void initialize_memory_heap(allocator *a, void *memory, usize size);

#define ALLOCATE_(ALLOCATOR, TYPE) allocate_((ALLOCATOR), sizeof(TYPE), alignof(TYPE))
#define ALLOCATE(ALLOCATOR, TYPE) allocate((ALLOCATOR), sizeof(TYPE), alignof(TYPE))

void *allocate_(allocator *a, usize size, usize alignment);
void *allocate(allocator *a, usize size, usize alignment);
void *reallocate(allocator *a, void *memory, usize size);
void deallocate(allocator *a, void *memory, usize size);

#endif // ALLOCATOR_H
