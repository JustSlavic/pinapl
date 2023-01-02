#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <base.h>
#include <memory.h>
#include <array.h>


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

#define ALLOCATE_BUFFER_(ALLOCATOR, SIZE) allocate_(ALLOCATOR, SIZE, 4);
#define ALLOCATE_BUFFER(ALLOCATOR, SIZE) allocate(ALLOCATOR, SIZE, 4);

#define ALLOCATE_ARRAY(ALLOCATOR, TYPE, COUNT) allocate_array(ALLOCATOR, sizeof(TYPE) * COUNT, alignof(TYPE))

#define ALLOCATE_BLOCK_(ALLOCATOR, SIZE) allocate_block_(ALLOCATOR, SIZE, 4);
#define ALLOCATE_BLOCK(ALLOCATOR, SIZE) allocate_block(ALLOCATOR, SIZE, 4);

void *allocate_(struct allocator *a, usize size, usize alignment);
void *allocate(struct allocator *a, usize size, usize alignment);
struct array allocate_array(struct allocator *a, usize size, usize alignment);
struct memory_block allocate_block_(struct allocator *a, usize size, usize alignment);
struct memory_block allocate_block(struct allocator *a, usize size, usize alignment);
void *reallocate(struct allocator *a, void *memory, usize size);
void deallocate(struct allocator *a, void *memory, usize size);

#endif // ALLOCATOR_H

