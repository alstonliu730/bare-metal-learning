#ifndef __MALLOC_T__
#define __MALLOC_T__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// The fixed-size options for the memory allocator
typedef enum {
    POOL_16 = 16,
    POOL_32 = 32,
    POOL_64 = 64,
    POOL_128 = 128,
    POOL_256 = 256,
    POOL_512 = 512,
    POOL_4KB = 4096
} MemPoolSize;

// metadata for each block of memory
typedef struct metadata{
    uint8_t free;
    struct metadata* next;
    struct metadata* prev;
} metadata_t;

// memory pool allocator structure
typedef struct {
    MemPoolSize block_size;
    uintptr_t start;
    uintptr_t end;
    metadata_t* free_list; // use to pop
    metadata_t* metadata; // use to store the head of the data
} pool_t;

#define HEAP_START                  0x40000000
#define HEAP_END                    0xA0000000
#define HEAP_SIZE                   (HEAP_START - HEAP_END)

#define MAX_BLOCKS_PER_POOL         4096
#define MAX_PAGES                   512
#define NUM_FIXED_SIZE              7

#define PAGES_START                 (HEAP_START)
#define PAGES_SIZE                  (MAX_PAGES * POOL_4KB)
#define PAGES_END                   (PAGES_START + PAGES_SIZE)

#define POOL512_START               (PAGES_END)
#define POOL512_SIZE                (MAX_BLOCKS_PER_POOL * POOL_512)
#define POOL512_END                 (POOL512_START + POOL512_SIZE)

#define POOL256_START               (POOL512_END)
#define POOL256_SIZE                (MAX_BLOCKS_PER_POOL * POOL_256)
#define POOL256_END                 (POOL256_START + POOL256_SIZE)

#define POOL128_START               (POOL256_END)
#define POOL128_SIZE                (MAX_BLOCKS_PER_POOL * POOL_128)
#define POOL128_END                 (POOL128_START + POOL128_SIZE)

#define POOL64_START                (POOL128_END)
#define POOL64_SIZE                 (MAX_BLOCKS_PER_POOL * POOL_64)
#define POOL64_END                  (POOL64_START + POOL64_SIZE)

#define POOL32_START                (POOL64_END)
#define POOL32_SIZE                 (MAX_BLOCKS_PER_POOL * POOL_32)
#define POOL32_END                  (POOL32_START + POOL32_SIZE)

#define POOL16_START                (POOL32_END)
#define POOL16_SIZE                 (MAX_BLOCKS_PER_POOL * POOL_16)
#define POOL16_END                  (POOL16_START + POOL16_SIZE)

/**
 * Initializes the metadata array and the memory pool
 */
void allocator_init();

/**
 * Free the given ptr from the memory pool.
 * 
 * @param ptr address to the block of memory
 */
void free(void* ptr);

/**
 * Get a ptr from the heap through the memory pool.
 * 
 * @param mem_pool the address to the memory pool
 * @param nBytes the number of bytes to allocate
 * 
 * @return the address of the heap-allocated memory 
 */
void* malloc(size_t nBytes);

/**
 * Print out the Fixed Size Boundaries
 */
void print_pool_boundaries();

#endif /* __MALLOC_T__ */