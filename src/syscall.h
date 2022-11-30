#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL

#define O_RDONLY         0x0
#define O_WRONLY         0x1
#define O_RDWR           0x2

#define O_CREAT          0x040
#define O_APPEND         0x400

#define PROT_READ        0x1
#define PROT_WRITE       0x2
#define PROT_EXEC        0x4

#define PROT_NONE        0x0
#define MAP_FILE         0x0
#define MAP_SHARED       0x1
#define MAP_PRIVATE      0x2
#define MAP_FIXED        0x10
#define MAP_ANONYMOUS    0x20
#define MAP_ANON         0x20
#define MAP_GROWSDOWN    0x100
#define MAP_DENYWRITE    0x800
#define MAP_EXECUTABLE   0x1000
#define MAP_NORESERVE    0x4000
#define MAP_LOCKED       0x2000

SYSCALL void exit(int return_code);
SYSCALL int write(int fd, char const *buffer, int count);
SYSCALL int read(int fd, char *buffer, int count);
SYSCALL int open(char const *pathname, int flags, int mode);
SYSCALL int close(int fd);
SYSCALL void *mmap2(void *address, int length, int protection, int flags, int fd, int page_offset);
SYSCALL void munmap(void *start, int length);


#endif // SYSCALL_H
