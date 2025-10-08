#ifndef COMMON_H
#define COMMON_H

// stdint typedef
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

typedef unsigned long      size_t;
typedef unsigned long      uintptr_t;  // Pointer-sized integer

#define NULL ((void*)0)
#define PERIPHERAL_BASE         0xFE000000

#endif /* COMMON_H */