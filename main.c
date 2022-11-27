#include "syscall.h"


int main(int argc, char **argv, char **env)
{   
    for (int i = 0; i < argc; i++)
    {
        int count = 0;
        for (char *c = argv[i]; *c != 0; c++)
        {
            count += 1;
        }

        write(1, argv[i], count);
        write(1, "\n", 1);
    }

    for (int env_index = 0;; env_index++)
    {
        char *e = env[env_index];
        if (e == 0) break;

        int count = 0;
        for (char *c = e; *c != 0; c++)
        {
            count += 1;
        }
        write(1, e, count);
        write(1, "\n", 1);
    }

    int fd = open("./bui.sh", 0, O_RDONLY);
    if (fd == -1)
    {
        write(1, "Error 1\n", 8);
    } else if (fd == 0)
    {
        write(1, "Error 0\n", 8);
    }

    char buffer[32] = {0};
    int buffer_index = 0;
    int n = fd;
    while (n)
    {
        buffer[buffer_index++] = (char) ('0' + (n % 10));
        n /= 10;
    }
    write(1, buffer, 32);
    write(1, "\n", 1);
    close(fd);

    // write(1, "\nHELLO, WORLD!\n", 15);
    return 0;
}

