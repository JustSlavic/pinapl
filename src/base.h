#ifndef BASE_H
#define BASE_H

#define GLOBAL static
#define PERSIST static
#define INTERNAL static

#define static_assert(VAR, MSG) _Static_assert((VAR), MSG)
#define alignof(TYPE) _Alignof(TYPE)

#ifndef NULL
#define NULL (void *)0
#endif
#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#define SET_ZERO(x) memset(&x, 0, sizeof(x))

#define ALWAYS_INLINE __attribute__((always_inline))
#define INLINE __inline__

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_arg   __builtin_va_arg
#define va_end   __builtin_va_end

#if defined(__arm__)
#define DEBUG_BREAK() { __asm__ volatile(".inst 0xe7f001f0"); } (void)0
#endif

#define UNUSED(VAR) (void)(VAR)
#define ASSERT(EXP) if (EXP) {} else { DEBUG_BREAK(); } (void)0
#define ASSERT_MSG(EXP, MSG) ASSERT(EXP)
#define ASSERT_FAIL(MSG) ASSERT_MSG(false, MSG)
#define NOT_IMPLEMENTED() ASSERT(false)

typedef unsigned int b32;
typedef unsigned int bool32;

#define true 1
#define false 0

typedef unsigned char uint8;
typedef   signed char  int8;

typedef uint8 u8;
typedef  int8 i8;

typedef unsigned short uint16;
typedef   signed short  int16;

typedef uint16 u16;
typedef  int16 i16;

typedef unsigned int uint32;
typedef   signed int  int32;

typedef uint32 u32;
typedef  int32 i32;

typedef unsigned int usize;
typedef   signed int isize;

typedef usize uintptr;
typedef isize  intptr;

#define INT8_MAX 0x7F
#define INT8_MIN 0x80

#define UINT8_MAX 0xFF
#define UINT8_MIN 0x0

#define INT16_MAX 0x7FFF
#define INT16_MIN 0x8000

#define UINT16_MAX 0xFFFF
#define UINT16_MIN 0x0

#define INT32_MAX 0x7FFFFFFF
#define INT32_MIN 0x80000000

#define UINT32_MAX 0xFFFFFFFF
#define UINT32_MIN 0x0

#define SIZE_MAX INT32_MAX
#define SIZE_MIN INT32_MiN

#define USIZE_MAX UINT32_MAX
#define USIZE_MIN UINT32_MIN

#define KILOBYTES(N) (1024 * N)
#define MEGABYTES(N) (1024 * KILOBYTES(N))
#define GIGABYTES(N) (1024 * MEGABYTES(N))

#define TOGGLE(VALUE) { VALUE = !(VALUE); } (void)0

#define SWAP(A, B) { A ^= B; B ^= A; A ^= B; } (void)0

#endif // BASE_H

