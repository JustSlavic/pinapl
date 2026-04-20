#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef   signed char        int8;
typedef   signed short       int16;
typedef   signed int         int32;
typedef   signed long long   int64;
typedef unsigned char        uint8;
typedef unsigned short       uint16;
typedef unsigned int         uint32;
typedef unsigned long long   uint64;
typedef float                float32;
typedef double               float64;

typedef uint32               uint;
typedef  int64               isize;
typedef uint64               usize;
typedef uint8                byte;
typedef uint8                bool;
typedef uint32               bool32;
typedef  int64               intptr;
typedef uint64               uintptr;
typedef int16                sound_sample_t;
#define true                 1
#define false                0

#include <sys/mman.h> // mmap
#include <dlfcn.h> // dlopen
#include <sys/stat.h> // stat
#include <fcntl.h> // open, close
#include <unistd.h> // read

usize platform_get_file_size(char const *filename)
{
    usize result = 0;
    struct stat st = {};
    if (stat(filename, &st) == 0)
    {
        result = st.st_size;
    }
    return result;
}

uint32 platform_read_file_into_memory(char const *filename, void *memory, usize size)
{
    int fd = open(filename, O_NOFOLLOW | O_RDONLY);
    if (fd != -1)
    {
        uint32 bytes_read = read(fd, memory, size);
        close(fd);
        return bytes_read;
    }
    return 0;
}

uint32 platform_write_memory_buffer_into_file(char const *filename, void *memory, usize size)
{
    int fd = open(filename, O_NOFOLLOW | O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1)
    {
        uint32 bytes_written = write(fd, memory, size);
        close(fd);
        return bytes_written;
    }
    return 0;
}

int main()
{
    usize input_capacity = platform_get_file_size("input2.txt");
    if (input_capacity == 0)
        goto exit_failure;

    char *input_buffer = malloc(input_capacity);
    uint32 input_size = platform_read_file_into_memory("input.txt", input_buffer, input_capacity);

    int r = 0;
    int w = 0;
    int comment = false;
    while (r < input_size)
    {
        char a = input_buffer[r++];

        if (a == '\n' && comment)
        {
            comment = false;
            continue;
        }
        if (comment)
            continue;
        if (a == '#')
        {
            comment = true;
            continue;
        }

        bool a_is_digit = ('0' <= a) && (a <= '9');
        bool a_is_alpha = ('a' <= a) && (a <= 'z');
        bool a_is_ALPHA = ('A' <= a) && (a <= 'Z');
        if (!a_is_digit && !a_is_alpha && !a_is_ALPHA)
            continue;

        char b = input_buffer[r++];
        bool b_is_digit = ('0' <= b) && (b <= '9');
        bool b_is_alpha = ('a' <= b) && (b <= 'z');
        bool b_is_ALPHA = ('A' <= b) && (b <= 'Z');
        if (!b_is_digit && !b_is_alpha && !b_is_ALPHA)
            continue;

        int c = 0;
        if (a_is_digit) c = (a - '0');
        if (a_is_alpha) c = (a - 'a' + 10);
        if (a_is_ALPHA) c = (a - 'A' + 10);

        c = (c << 4);
        if (b_is_digit) c = c | (b - '0');
        if (b_is_alpha) c = c | (b - 'a' + 10);
        if (b_is_ALPHA) c = c | (b - 'A' + 10);

        input_buffer[w++] = (char) c;
    }

    uint32 bytes_written = platform_write_memory_buffer_into_file("output.bin", input_buffer, w);
    printf("Done. %u bytes written.\n", bytes_written);

exit_success:
    return 0;
exit_failure:
    return 1;
}
