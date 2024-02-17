#ifndef BASE_HPP
#define BASE_HPP

#ifdef _MSC_VER
#define COMPILER_MSVC 1
#elif __GNUC__
#define COMPILER_GNU 1
#elif __clang__
#define COMPILER_CLANG 1
#elif __MINGW32__
#define COMPILER_MINGW 1
#endif

// Detect the 32-bit or 64-bit architecture
#if defined(__arm__) || defined(__i386__)
#define ARCHITECTURE_32BIT 1
#endif
#if defined(__aarch64__) || defined(__x86_64__)
#define ARCHITECTURE_64BIT 1
#endif

#if COMPILER_MSVC
#endif // COMPILER_MSVC

#if COMPILER_GNU

typedef   signed char        int8;
typedef   signed short       int16;
typedef   signed int         int32;
typedef unsigned char        uint8;
typedef unsigned short       uint16;
typedef unsigned int         uint32;

#define INT8_MIN             0x80
#define INT16_MIN            0x8000
#define INT32_MIN            0x80000000

#define INT8_MAX             0x7F
#define INT16_MAX            0x7FFF
#define INT32_MAX            0x7FFFFFFF

#define UINT8_MIN            0
#define UINT16_MIN           0
#define UINT32_MIN           0

#define UINT8_MAX            0xFF
#define UINT16_MAX           0xFFFF
#define UINT32_MAX           0xFFFFFFFF

#if ARCHITECTURE_32BIT

typedef int32                isize;
typedef uint32               usize;
typedef int32                intptr;
typedef uint32               uintptr;

#define SIZE_MAX             INT32_MAX
#define SIZE_MIN             INT32_MiN
#define USIZE_MAX            UINT32_MAX
#define USIZE_MIN            UINT32_MIN

#endif // ARCHITECTURE_32BIT

#if ARCHITECTURE_64BIT

typedef   signed long long   int64;
typedef unsigned long long   uint64;
typedef int64                isize;
typedef uint64               usize;
typedef int64                intptr;
typedef uint64               uintptr;

#define INT64_MAX            0x7FFFFFFFFFFFFFFF
#define INT64_MIN            0x8000000000000000
#define UINT64_MIN           0
#define UINT64_MAX           0xFFFFFFFFFFFFFFFF

#define SIZE_MAX             INT64_MAX
#define SIZE_MIN             INT64_MiN
#define USIZE_MAX            UINT64_MAX
#define USIZE_MIN            UINT64_MIN

#endif // ARCHITECTURE_64BIT

#if DEBUG
#define DEBUG_BREAK __builtin_trap
#else
#define DEBUG_BREAK
#endif // DEBUG

#endif // COMPILER_GNU

#if COMPILER_CLANG
#endif // COMPILER_CLANG

// #if defined(__arm__) && !defined(__thumb__) // ARM
// #define DEBUG_BREAK() { __asm__ volatile(".inst 0xe7f001f0"); } (void)0
// #elif defined(__aarch64__) && defined(__APPLE__) // Apple
// #define DEBUG_BREAK() { __builtin_debugtrap(); } (void)0
// #elif defined(__aarch64__)
// #define DEBUG_BREAK() { __asm__ volatile(".inst 0xd4200000"); } (void)0
// #endif

typedef uint8                byte;

#ifndef __cplusplus // C
#define static_assert(VAR, MSG) _Static_assert((VAR), MSG)
#define alignof(TYPE) _Alignof(TYPE)
typedef uint8                bool;
typedef uint32               bool32;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef va_start
typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_arg   __builtin_va_arg
#define va_end   __builtin_va_end
#endif
#endif // C

#ifndef NULL
#define NULL (void *)0
#endif

#define FUNCTION   static
#define PERSIST    static
#define GLOBAL     static
#define INLINE     inline

#ifdef DEBUG
#define ASSERT(COND)          if (COND) {} else { DEBUG_BREAK(); } (void)(0)
#define ASSERT_MSG(COND, ...) if (COND) {} else { DEBUG_BREAK(); } (void)(0)
#else
#define ASSERT(COND)          (void)(0)
#define ASSERT_MSG(COND, ...) (void)(0)
#endif // DEBUG
#define ASSERT_FAIL(...) ASSERT_MSG(false, __VA_ARGS__)
#define UNREACHABLE() ASSERT_FAIL("Unreachable code path!")
#define NOT_IMPLEMENTED() ASSERT_FAIL("This code path is not implemented yet!")

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))
#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
#define UNUSED(X) (void) (X)
#define MACRO_EXPAND(X) X

#define MACRO_OVERLOAD_1(_1, NAME, ...) NAME
#define MACRO_OVERLOAD_2(_1, _2, NAME, ...) NAME
#define MACRO_OVERLOAD_3(_1, _2, _3, NAME, ...) NAME
#define MACRO_OVERLOAD_4(_1, _2, _3, _4, NAME, ...) NAME

#define KILOBYTES(N) (1024 * N)
#define MEGABYTES(N) (1024 * KILOBYTES(N))
#define GIGABYTES(N) (1024 * MEGABYTES(N))

#endif // BASE_HPP
