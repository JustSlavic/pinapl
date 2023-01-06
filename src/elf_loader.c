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


int main()
{
    char const *filename = "write_hello_world";

    void *memory = NULL;
    size_t size = 0;

    if (load_entire_file(filename, &memory, &size))
    {
        struct elf_header header = elf_load(memory, size);

        printf("ELF_MAGIC_0 %s\n", header.e_ident[ELF_IDENT_MAGIC0] == 0x7f ? "OK" : "FAILURE");
        printf("ELF_MAGIC_1 %s\n", header.e_ident[ELF_IDENT_MAGIC1] == 'E' ? "OK" : "FAILURE");
        printf("ELF_MAGIC_2 %s\n", header.e_ident[ELF_IDENT_MAGIC2] == 'L' ? "OK" : "FAILURE");
        printf("ELF_MAGIC_3 %s\n", header.e_ident[ELF_IDENT_MAGIC3] == 'F' ? "OK" : "FAILURE");

        uint32 cls = header.e_ident[ELF_IDENT_CLASS];
        printf("ELF_CLASS: %s\n", cls == 1 ? "32-bit object" : cls == 2 ? "64-bit object" : "Invalid object class");

        printf("ELF_DATA: %s\n", header.e_ident[ELF_IDENT_DATA] == ELF_DATA_2LSB ? "Little-endian" : "Big-endian");
        printf("ELF_VERSION: %d\n", header.e_ident[ELF_IDENT_VERSION]);

        switch (header.e_type)
        {
            case ET_REL:  printf("Elf type: Relocatable file\n"); break;
            case ET_EXEC: printf("Elf type: Executable\n"); break;
            case ET_DYN:  printf("Elf type: Shared object\n"); break;
            case ET_CORE: printf("Elf type: coredump\n"); break;
            case ET_NONE:
            default:
                printf("Unknown elf type file\n");
        };

        switch (header.e_machine)
        {
            case EM_M32: printf("Elf machine: M32\n"); break;
            case EM_SPARC: printf("Elf machine: SPARC\n"); break;
            case EM_386:  printf("Elf machine: 386\n"); break;
            case EM_68K:  printf("Elf machine: 68K\n"); break;
            case EM_88K:  printf("Elf machine: 88K\n"); break;
            case EM_860:  printf("Elf machine: 860\n"); break;
            case EM_MIPS: printf("Elf machine: MIPS\n"); break;

            case EM_NONE:
            default:
                printf("Elf machine: %d\n", header.e_machine);
        }
        
        printf("Elf version: %d%s\n", header.e_version, header.e_version == EV_CURRENT ? " (EV_CURRENT)" : "");
        printf("Entry point: 0x%x\n", header.e_entry);
    }

    return 0;
}

