#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <elf.h>


int load_entire_file(char const *filename, void **out_memory, size_t *out_size)
{
    int result = 0;

    struct stat st;
    stat(filename, &st);
    size_t size = st.st_size;

    void *memory = mmap(0, size + 1, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // malloc(size + 1);
    int fd = open(filename, O_RDONLY);
    if (fd > 0)
    {
        size_t bytes_read = read(fd, memory, size);
        if (bytes_read < size)
        {
            printf("Oops! Can't read the bytes!\n");
        }
        else
        {
            *((unsigned char *) memory + bytes_read) = 0; // Null terminator

            close(fd);

            *out_memory = memory;
            *out_size = size;

            result = 1;
        }
    }
    else
    {
        printf("Oops! Can't open the file!\n");
    }

    return result;
}


int main(int argc, char **argv)
{
    char const *filename = "bin/pinapl";
    if (argc > 1)
    {
        filename = argv[1];
    }

    void *memory = NULL;
    size_t size = 0;

    if (load_entire_file(filename, &memory, &size))
    {
        elf_load(memory, size);
    }

    return 0;
}

