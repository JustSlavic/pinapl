#include <array.h>
#include <memory.h>


void array_push_impl(struct array *array, void *memory, usize size)
{
    if (array->capacity < (array->size + 1))
    {
        void *slot = array->data + array->size * size;
        memcpy(slot, memory, size);
    }
}

