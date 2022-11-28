#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2

#define O_CREAT     0x040
#define O_APPEND    0x400

SYSCALL void exit(int return_code);
SYSCALL int write(int fd, char const *buffer, int count);
SYSCALL int read(int fd, char *buffer, int count);
SYSCALL int open(char const *pathname, int flags, int mode);
SYSCALL int close(int fd);

#endif // SYSCALL_H
