#include "allocator.h"


enum allocator_type
{
    ALLOCATOR_INVALID = 0,
    ALLOCATOR_ARENA,
    ALLOCATOR_POOL,
    ALLOCATOR_HEAP,
};


struct allocator_base
{
    enum allocator_type type;
};


struct arena_allocator
{
    struct allocator_base tag;

    void *memory;
    usize size;
    usize used;
    usize last_allocation_size;
};

static_assert(sizeof(struct arena_allocator) <= sizeof(struct allocator), "size of arena_allocator is too large!");


void initialize_memory_arena(allocator *a, void *memory, usize size)
{
    struct arena_allocator *arena = (struct arena_allocator *) a;

    arena->tag.type = ALLOCATOR_ARENA;
    arena->memory = memory;
    arena->size = size;
    arena->used = 0;
    arena->last_allocation_size = 0;
}

void initialize_memory_pool(allocator *a, void *memory, usize size);
void initialize_memory_heap(allocator *a, void *memory, usize size);

void *allocate_(allocator *a, usize size, usize alignment)
{
    void *result = NULL;

    struct allocator_base *base = (struct allocator_base *) a;
    switch (base->type)
    {
        case ALLOCATOR_INVALID:
        {
            ASSERT_MSG(false, "Invalid allocator!");
        }
        break;

        case ALLOCATOR_ARENA:
        {
            struct arena_allocator *arena = (struct arena_allocator *) a;
            
            void *pointer = arena->memory + arena->used;
            usize padding = get_padding(pointer, alignment);
            if ((arena->used + padding + size) <= arena->size)
            {
                result = arena->memory + arena->used + padding;
                arena->used += size + padding;
                arena->last_allocation_size = size;
            }
        }
        break;

        case ALLOCATOR_POOL:
        {
            ASSERT(false);
        }
        break;

        case ALLOCATOR_HEAP:
        {
            ASSERT(false);
        }
        break;
    }

    return result;
}

void *allocate(allocator *a, usize size, usize alignment)
{
    void *result = allocate_(a, size, alignment);
    if (result)
    {
        memset(result, 0, size);
    }
    return result;
}

void *reallocate(allocator *a, void *memory, usize size);
void deallocate(allocator *a, void *memory, usize size);

