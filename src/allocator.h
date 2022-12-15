#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <base.h>
#include <memory.h>


#define ALLOCATOR_BASE_STRUCT struct { void *memory; usize used; usize size; }

struct allocator
{
    ALLOCATOR_BASE_STRUCT;
    u8 data[8];
};


void initialize_memory_arena(struct allocator *a, void *memory, usize size);
void initialize_memory_pool(struct allocator *a, void *memory, usize size);
void initialize_memory_heap(struct allocator *a, void *memory, usize size);

#define ALLOCATE_(ALLOCATOR, TYPE) allocate_(ALLOCATOR, sizeof(TYPE), alignof(TYPE))
#define ALLOCATE(ALLOCATOR, TYPE) allocate(ALLOCATOR, sizeof(TYPE), alignof(TYPE))

#define ALLOCATE_BUFFER_(ALLOCATOR, SIZE) allocate_(ALLOCATOR, SIZE, 1);
#define ALLOCATE_BUFFER(ALLOCATOR, SIZE) allocate(ALLOCATOR, SIZE, 1);

void *allocate_(struct allocator *a, usize size, usize alignment);
void *allocate(struct allocator *a, usize size, usize alignment);
void *reallocate(struct allocator *a, void *memory, usize size);
void deallocate(struct allocator *a, void *memory, usize size);

#endif // ALLOCATOR_H

