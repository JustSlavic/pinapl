#include "syscall.h"


int main(int argc, char **argv, char **env)
{   
    if (argc < 2)
    {
        return 0;
    }

    int fd = open(argv[1], 0, O_RDONLY);
    if (fd < 0)
    {
        write(1, "Error < 0\n", 10);
    }

    char buffer[1024] = {0};
    int  buffer_size = read(fd, buffer, 1024);
    write(1, buffer, buffer_size);
    write(1, "\n", 1);
    close(fd);

    return 0;
}

