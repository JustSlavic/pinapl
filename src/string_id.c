#include <string_id.h>
#include <primes.h>
#include <allocator.h>
#include <memory.h>


struct string_id_storage_hash_entry
{
    u32 hash;
    usize offset_in_arena;
};

struct string_id_storage
{
    struct string_id_storage_hash_entry hash_table[1024];
    struct allocator arena;
};

GLOBAL struct string_id_storage global_storage;


void
initialize_string_id_storage(void *memory, usize size)
{
    initialize_memory_arena(&global_storage.arena, memory, size);
}

struct string_id_storage_hash_entry *
string_id_get_hash_slot(u32 hash)
{
    struct string_id_storage_hash_entry *result = NULL;
    for (int offset = 0; offset < ARRAY_COUNT(global_storage.hash_table); offset++)
    {
        struct string_id_storage_hash_entry *slot = global_storage.hash_table + (hash + offset) % ARRAY_COUNT(global_storage.hash_table);
        if (slot->hash == hash || slot->hash == 0)
        {
            result = slot;
            break;
        }
    }

    return result;
}

struct string_id
make_string_id(char *string, usize size)
{
    u32 hash = 0;
    for (usize index = 0; index < size; index++)
    {
        hash += string[index] * primes[index % ARRAY_COUNT(primes)];
    }
    
    struct string_id result;

    struct string_id_storage_hash_entry *entry = string_id_get_hash_slot(hash);
    if (entry->hash == 0)
    {
        void *buffer = allocate_(&global_storage.arena, size + sizeof(usize), alignof(usize));

        usize *size_in_buffer = buffer;
        char *string_in_buffer = buffer + sizeof(usize);
        
        usize id = (iptr) string_in_buffer - (iptr) global_storage.arena.memory;
        
        entry->hash = hash;
        entry->offset_in_arena = id;
        result.id = id;

        *size_in_buffer = size;
        memcpy(string_in_buffer, string, size);
    }
    else
    {
        result.id = entry->offset_in_arena;
    }

    return result;
}


struct string
get_string_by_id(struct string_id id)
{
    struct string result;
    result.data = (char *) global_storage.arena.memory + id.id;
    result.size = *((usize *)(((char *) global_storage.arena.memory + id.id) - sizeof(usize)));

    return result;
}

