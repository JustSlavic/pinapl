#ifndef PINAPL_BASE_H_
#define PINAPL_BASE_H_


typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef float              float32;
typedef double             float64;

typedef int8               bool;
typedef int32              bool32;

#define true 1
#define false 0


#define ARRAY_COUNT(ARR) (sizeof(ARR) / sizeof((ARR)[0]))


#endif /* PINAPL_BASE_H_ */
