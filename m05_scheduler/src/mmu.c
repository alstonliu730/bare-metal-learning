#include <mmu.h>

static uint16_t mem_map[PAGING_PAGES] = {0, };

/**
 * Naive implementation of page allocation
 */
uintptr_t palloc() {
    for (uint32_t i = 0; i < PAGING_PAGES; i++) {
        // get the next free page
        if (mem_map[i] == 0) {
            mem_map[i] = 1;
            return LOW_MEMORY + i*PAGE_SIZE;
        }
    }
    // no free pages
    return 0;
}

/**
 * Sets the page to free in the memory map
 */
void free_page(uintptr_t ptr) {
    mem_map[(ptr - LOW_MEMORY) / PAGE_SIZE] = 0;
}

//((0xFE000000 - (2 * (1<<(12 + 9)))) / (1<<12))