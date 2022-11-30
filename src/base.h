#ifndef BASE_H
#define BASE_H

#define NULL (void *)0
#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#if defined(__arm__)
__attribute__((always_inline))
__inline__ static void debug_break(void)
{
    __asm__ volatile(".inst 0xe7f001f0");
}
#endif

typedef unsigned int uint32_t;
typedef   signed int  int32_t;

typedef unsigned int  size_t;
typedef   signed int ssize_t;

typedef uint32_t bool;
typedef uint32_t b32;

#define true 1
#define false 0


#endif // BASE_H

