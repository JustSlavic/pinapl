#ifndef BASE_H
#define BASE_H

#define alignof(TYPE) _Alignof(TYPE)

#define NULL (void *)0
#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#if defined(__arm__)
__attribute__((always_inline))
__inline__ static void debug_break(void)
{
    __asm__ volatile(".inst 0xe7f001f0");
}
#endif

#define UNUSED(VAR) (void)(VAR)
#define ASSERT(EXP) if (EXP) {} else { debug_break(); } (void)0
#define ASSERT_MSG(EXP, MSG) ASSERT(EXP)
#define NOT_IMPLEMENTED ASSERT(false)

typedef unsigned int b32;

typedef unsigned char u8;
typedef   signed char s8;

typedef unsigned short u16;
typedef   signed short s16;

typedef unsigned int u32;
typedef   signed int s32;

typedef unsigned int usize;
typedef   signed int ssize;

#define true 1
#define false 0


#endif // BASE_H

