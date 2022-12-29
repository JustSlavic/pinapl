#include "allocator.h"


enum allocator_type
{
    ALLOCATOR_INVALID = 0,
    ALLOCATOR_ARENA,
    ALLOCATOR_POOL,
    ALLOCATOR_HEAP,
};


struct arena_allocator
{
    ALLOCATOR_BASE_STRUCT;
    enum allocator_type type;
};

static_assert(sizeof(struct arena_allocator) >= sizeof(ALLOCATOR_BASE_STRUCT), "size of arena_allocator is too small!");
static_assert(sizeof(struct arena_allocator) <= sizeof(struct allocator), "size of arena_allocator is too large!");


enum allocator_type allocator_get_type(struct allocator *allocator)
{
    enum allocator_type result = *(enum allocator_type *) ((u8 *) allocator + sizeof(ALLOCATOR_BASE_STRUCT));
    return result;
}

void initialize_memory_arena(struct allocator *a, void *memory, usize size)
{
    struct arena_allocator *arena = (struct arena_allocator *) a;

    arena->type = ALLOCATOR_ARENA;
    arena->memory = memory;
    arena->size = size;
    arena->used = 0;
}

void initialize_memory_pool(struct allocator *a, void *memory, usize size);
void initialize_memory_heap(struct allocator *a, void *memory, usize size);

void *allocate_(struct allocator *a, usize size, usize alignment)
{
    void *result = NULL;

    enum allocator_type type = allocator_get_type(a);
    switch (type)
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

void *allocate(struct allocator *a, usize size, usize alignment)
{
    void *result = allocate_(a, size, alignment);
    if (result)
    {
        memset(result, 0, size);
    }
    return result;
}

struct memory_block allocate_block_(struct allocator *a, usize size, usize alignment)
{
    struct memory_block result;
    result.memory = allocate_(a, size, alignment);
    result.size   = size;
    return result;
}

struct memory_block allocate_block(struct allocator *a, usize size, usize alignment)
{
    struct memory_block result;
    result.memory = allocate(a, size, alignment);
    result.size   = size;
    return result;
}

void *reallocate(struct allocator *a, void *memory, usize size)
{
    NOT_IMPLEMENTED();
    return NULL;
}

void deallocate(struct allocator *a, void *memory, usize size)
{
    // Nothing to do yet
}

