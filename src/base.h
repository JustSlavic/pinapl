#ifndef BASE_H
#define BASE_H

#define static_assert(VAR, MSG) _Static_assert((VAR), MSG)
#define alignof(TYPE) _Alignof(TYPE)

#define NULL (void *)0
#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#define ALWAYS_INLINE __attribute__((always_inline))
#define INLINE __inline__

#if defined(__arm__)
#define DEBUG_BREAK() { __asm__ volatile(".inst 0xe7f001f0"); } (void)0
#endif

#define UNUSED(VAR) (void)(VAR)
#define ASSERT(EXP) if (EXP) {} else { DEBUG_BREAK(); } (void)0
#define ASSERT_MSG(EXP, MSG) ASSERT(EXP)
#define ASSERT_FAIL(MSG) ASSERT_MSG(false, MSG)
#define NOT_IMPLEMENTED ASSERT(false)

#define KILOBYTES(N) (1024 * N)
#define MEGABYTES(N) (1024 * KILOBYTES(N))
#define GIGABYTES(N) (1024 * MEGABYTES(N))


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

