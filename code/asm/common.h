#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#define true 1
#define false 0
typedef int32_t bool32;
typedef int8_t int8;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#endif // COMMON_H
