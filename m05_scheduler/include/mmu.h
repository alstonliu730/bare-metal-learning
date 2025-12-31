#ifndef _MMU_H
#define _MMU_H

#include <common.h>

#define PAGE_SHIFT                  12
#define TABLE_SHIFT                 9
#define SECTION_SHIFT               (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE                   (1 << PAGE_SHIFT)
#define SECTION_SIZE                (1 << SECTION_SHIFT)

#define LOW_MEMORY                  (2 * SECTION_SIZE)
#define HIGH_MEMORY                 PERIPHERAL_BASE

#define PAGING_MEMORY               (HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES                (PAGING_MEMORY / PAGE_SIZE)

uintptr_t palloc(); // primitive memory allocator
void free_page(uintptr_t ptr);

#endif /*_MMU_H*/