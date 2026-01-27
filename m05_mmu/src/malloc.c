#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <common.h>
#include <uart.h>

const MemPoolSize mem_sizes[NUM_FIXED_SIZE] = {
    POOL_4KB, POOL_512, POOL_256, POOL_128, POOL_64, POOL_32, POOL_16
};

// metadata array
static metadata_t meta16[MAX_BLOCKS_PER_POOL]; // 
static metadata_t meta32[MAX_BLOCKS_PER_POOL];
static metadata_t meta64[MAX_BLOCKS_PER_POOL];
static metadata_t meta128[MAX_BLOCKS_PER_POOL];
static metadata_t meta256[MAX_BLOCKS_PER_POOL];
static metadata_t meta512[MAX_BLOCKS_PER_POOL];
static metadata_t pages[MAX_PAGES];

// memory pool for each fixed size
static pool_t memory_pool[NUM_FIXED_SIZE];

// helper function initialize the metadata array
static void init_metadata() {
    // Initializing metadata array
    for(size_t i = 0; i < MAX_BLOCKS_PER_POOL; i++) {
        // Initialize the pages
        if (i < MAX_PAGES) {
            pages[i] = (metadata_t) {
                .free = 1,
                .next = (i == MAX_PAGES - 1) ? NULL : &pages[i + 1]
            };
        }

        // Initialize the other blocks
        meta16[i] = (metadata_t) {
            .free = 1, 
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta16[i + 1]
        };
        meta32[i] = (metadata_t) {
            .free = 1,
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta32[i + 1]
        };
        meta64[i] = (metadata_t) {
            .free = 1,
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta64[i + 1]
        };
        meta128[i] = (metadata_t) {
            .free = 1,
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta128[i + 1],
        };
        meta256[i] = (metadata_t) {
            .free = 1, 
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta256[i + 1]
        };
        meta512[i] = (metadata_t) {
            .free = 1,
            .next = (i == MAX_BLOCKS_PER_POOL - 1) ? NULL : &meta512[i + 1]
        };
    }

    uart_printf("Metadata Array initialized.\n");
}

// helper function to initialize the memory pool data structure for each fixed size
static void init_pool() {
    // 4KB Page
    memory_pool[0] = (pool_t) {
        .block_size = POOL_4KB,
        .start = PAGES_START,
        .end = PAGES_END,
        .free_list = &pages[0],
        .metadata = &pages[0]
    };

    // 512 Byte Fixed Size
    memory_pool[1] = (pool_t) {
        .block_size = POOL_512,
        .start = POOL512_START,
        .end = POOL512_END,
        .free_list = &meta512[0],
        .metadata = &meta512[0]
    };

    // 256 Byte Fixed Size
    memory_pool[2] = (pool_t) {
        .block_size = POOL_256,
        .start = POOL256_START,
        .end = POOL256_END,
        .free_list = &meta256[0],
        .metadata = &meta256[0]
    };

    // 128 Byte Fixed Size
    memory_pool[3] = (pool_t) {
        .block_size = POOL_128,
        .start = POOL128_START,
        .end = POOL128_END,
        .free_list = &meta128[0],
        .metadata = &meta128[0]
    };

    // 64 Byte Fixed Size
    memory_pool[4] = (pool_t) {
        .block_size = POOL_64,
        .start = POOL64_START,
        .end = POOL64_END,
        .free_list = &meta64[0],
        .metadata = &meta64[0]
    };

    // 32 Byte Fixed Size
    memory_pool[5] = (pool_t) {
        .block_size = POOL_32,
        .start = POOL32_START,
        .end = POOL32_END,
        .free_list = &meta32[0],
        .metadata = &meta32[0]
    };

    // 16 Byte Fixed Size
    memory_pool[6] = (pool_t) {
        .block_size = POOL_16,
        .start = POOL16_START,
        .end = POOL16_END,
        .free_list = &meta16[0],
        .metadata = &meta16[0]
    };
    uart_printf("Memory Pool Initialized.\n");
}

/**
 * Initializes the metadata array and the memory pool
 */
void allocator_init() {
    uart_printf("\n======= Memory Pool Init =======\n");

    uart_printf("   Initializing metadata arrays...\n");
    init_metadata();

    uart_printf("   Initializing memory pool...\n");
    init_pool();

    uart_printf("=================================\n");
}

/**
 * Get a ptr from the heap through the memory pool.
 * 
 * @param mem_pool the address to the memory pool
 * @param nBytes the number of bytes to allocate
 * 
 * @return the address of the heap-allocated memory 
 */
void* malloc(size_t nBytes) {
    size_t pool_size;
    // Determine the pool size we will use
    if (nBytes == 0) {
        return NULL;
    } else if (nBytes < POOL_16) {
        pool_size = POOL_16;
    } else if (nBytes > POOL_512) {
        pool_size = POOL_4KB;
    } else {
        pool_size = 1 << (32 - __builtin_clz(nBytes - 1));
    }
    uart_printf("Determined pool size: %d\n", pool_size);
    
    // Loop through to find the memory pool
    int pool_index = -1;
    for (int i = 0; i < NUM_FIXED_SIZE; i++) {
        if (memory_pool[i].block_size >= pool_size) {
            pool_index = i;
        }
    }
    uart_printf("Determined pool index: %d\n", pool_index);

    pool_t *pool = &memory_pool[pool_index];

    // check if the memory pool is full
    if (pool->free_list == NULL) {
        uart_printf("%dB Memory Pool is full.\n", pool->block_size);
        return NULL;
    }

    // pop the next free entry
    metadata_t* entry = pool->free_list;
    pool->free_list = entry->next;

    // change the next pointer of the entry
    entry->next = NULL;
    
    // change the free status of the entry
    entry->free = 0;

    // Get the index of the entry using pointer arithmetic
    size_t index = entry - pool->metadata;
    void * block = (void *)(pool->start + (index * pool->block_size));

    // clear the memory in the block
    memset(block, 0, pool->block_size);

    // return the address to the block of memory
    return block;
}

/**
 * Free the given ptr from the memory pool.
 * 
 * @param ptr address to the block of memory
 */
void free(void* ptr) {
    // Check if it's a valid pointer
    uintptr_t p = (uintptr_t) ptr;
    if (p < HEAP_START || p >= HEAP_END) {
        uart_printf("Invalid pointer given to free: %p\n", p);
        return;
    }

    // Find which memory pool this pointer is from
    pool_t* pool = NULL;
    for(int i = 0; i < NUM_FIXED_SIZE; i++) {
        if (p >= memory_pool[i].start && p < memory_pool[i].end) {
            pool = &memory_pool[i];
            break;
        }
    }

    // Couldn't find a pool
    if (pool == NULL) { 
        uart_printf("Failed to find memory pool with given pointer: %p\n", p);
        return;
    }

    // get metadata from the ptr
    size_t index = (p - pool->start) / pool->block_size;
    metadata_t* entry = &pool->metadata[index];

    // check if the entry is already freed 
    if (entry->free) {
        uart_printf("Double free detected: %p\n", p);
        return;
    }

    // set the free status of the entry
    entry->free = 1;

    // set it back to the free list
    entry->next = pool->free_list;
    pool->free_list = entry;
}

// print out the heap boundaries for the memory allocator
void print_pool_boundaries() {
    uart_printf("====== Memory Pool Boundaries ======\n");

    for(int i = 0; i < NUM_FIXED_SIZE; i++) {
        uart_printf("Fixed Size: %d\n", memory_pool[i].block_size);
        uart_printf("   START:  %p\n", memory_pool[i].start);
        uart_printf("   END:    %p\n", memory_pool[i].end);
        uart_printf("   SIZE:   %x\n", (memory_pool[i].end - memory_pool[i].start));
    }
    uart_printf("=====================================\n");
}