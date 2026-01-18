#include <mmu.h>
#include <uart.h>
#include <common.h>
#include <mb.h>
#include <sysreg.h>
#include <timer.h>

/**
 * mmu.c - Memory Management Unit Configuration for Raspberry Pi 4
 * 
 * Memory Map (39-bit VA, 4KB granule, 2MB blocks):
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Virtual Address    │ L1 Index    │ Description                  │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ 0x00000000-0x3FFFFFFF │    0     │ GB0: RAM + VideoCore         │
 * │ 0x40000000-0x7FFFFFFF │    1     │ GB1: High RAM                │
 * │ 0x80000000-0xBFFFFFFF │    2     │ GB2: High RAM + Unmapped     │
 * │ 0xC0000000-0xFFFFFFFF │    3     │ GB3: RAM + Peripherals       │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 */

#define HIGH_MEM_GB1        0x40000000ULL
#define HIGH_MEM_GB2        0x80000000ULL
#define HIGH_MEM_GB2_MID    (HIGH_MEM_GB2 + 0x20000000ULL)
#define HIGH_MEM_GB3        0xC0000000ULL
#define PERIPHERAL_START    0xFC000000ULL

// Function declarations
static void zero_page_table(page_table_t *table);
// uint64_t make_table_descriptor(void *next_table_addr);
static inline uint64_t make_block_descriptor(uint64_t phys_addr, uint64_t attributes);
static inline uint64_t get_mair();
static void setup_tcr();
static void setup_page_tables();
static void setup_ttbr0();
static void invalidate_caches();
static void enable_mmu();

// set the page tables
// static page_table_t lvl0_table;
static page_table_t lvl1_table;
static page_table_t lvl2_table_gb0;
static page_table_t lvl2_table_gb1;
static page_table_t lvl2_table_gb2;
static page_table_t lvl2_table_gb3;

void verify_page_table_alignment(void) {
    // uart_printf("Level 0 table at: %lx\n", (uint64_t)&lvl0_table);
    uart_printf("Level 1 table at: %lx\n", (uint64_t)&lvl1_table);
    uart_printf("Level 2 GB0 at:   %lx\n", (uint64_t)&lvl2_table_gb0);
    uart_printf("Level 2 GB1 at:   %lx\n", (uint64_t)&lvl2_table_gb1);
    uart_printf("Level 2 GB2 at:   %lx\n", (uint64_t)&lvl2_table_gb2);
    uart_printf("Level 2 GB3 at:   %lx\n", (uint64_t)&lvl2_table_gb3);
    
    // Check alignment (last 12 bits should be 0)
    if ((uint64_t)&lvl1_table & 0xFFF) {
        uart_printf("ERROR: lvl0_table not 4KB aligned!\n");
    } else {
        uart_printf("All tables properly aligned!\n");
    }
}

// zero the given page table
static inline void zero_page_table(page_table_t *table) {
    for (int i = 0; i < PT_ENTRIES; i++) {
        (*table)[i] = 0;
    }
}

/**
 * Create a table descriptor (Level 0, 1, 2 -> next level)
 * 
 * @param next_table_addr: Physical address of next level table (must be 4KB aligned)
 * 
 * @return: 64-bit table descriptor
 */
uint64_t make_table_descriptor(void *next_table_addr) {
    uint64_t addr = (uint64_t)next_table_addr;
    
    // Verify alignment (optional, can remove in production)
    if (addr & 0xFFF) {
        // Table not 4KB aligned - this will cause issues!
        uart_printf("Table not 4KB aligned.\n");
        __asm__ volatile("b err_hang");
    }
    
    return (addr & TABLE_ADDR_MASK) | DESC_TABLE;
}

static inline uint64_t make_block_descriptor(uint64_t phys_addr, uint64_t attributes) {
    return (phys_addr & BLOCK_ADDR_MASK) | attributes | DESC_BLOCK;
}

// helper function to get the mair value
static inline uint64_t get_mair() {
    uint64_t mair = 0;
    
    __asm__ volatile("mrs %0, MAIR_EL1" : "=r"(mair));

    return mair;
}

/**
 * Setting up the MAIR attributes for the MMU
 * [0] - Normal Memory, Cacheable
 * [1] - Device Memory, Non-Cacheable
 */
static void setup_mair() {
    // set current mair value
    uint64_t mair_value = MAIR_VALUE;
    
    // Set the MAIR value into the register
    __asm__ volatile("msr MAIR_EL1, %0":: "r"(mair_value));
    uart_printf("MAIR_EL1 = %lx\n", mair_value);
}

// helper function to get the tcr value
static inline uint64_t get_tcr() {
    uint64_t tcr = 0;
    
    __asm__ volatile("mrs %0, TCR_EL1" : "=r"(tcr));

    return tcr;
}

/**
 * Setting up the TCR Value
 */
static void setup_tcr() {
    // set the current tcr_value
    uint64_t tcr_value = 0;

    // Set the size offset of the memory region
    tcr_value |= T0SZ_VAL; // number of 0 in MSB to ignore

    // Set the Inner Cacheability
    tcr_value |= IRGN0_WBWA;

    // Set the Outer Cacheability
    tcr_value |= ORGN0_WBWA;

    // Set the Shareability
    tcr_value |= SH0_INNER_SHAREABLE;

    // Set the Granule Size
    tcr_value |= TG0_4KB;

    // ----- SETTING TTBR1 SETTINGS -----
    // Set the size offset for TTBR1
    tcr_value |= T1SZ_VALUE;

    // Set the Granule Size for TTBR1
    tcr_value |= TG1_4KB;

    // Setting Physical Address Size
    tcr_value |= IPS_32;

    // Set the TCR Value into the register
    __asm__ volatile("msr TCR_EL1, %0" :: "r"(tcr_value));
    uart_printf("TCR_EL1 = %lx (39-bit VA, 32-bit PA)\n", tcr_value);
}

static uint32_t get_vc_memory() {
    // Setting mbox array
    mbox[0] = 8 * 4; // Size in Bytes
    mbox[1] = MBOX_REQUEST; // REQUEST TAG

    mbox[2] = MBOX_TAG_VC_MEM; // Tag to get the arm memory address
    mbox[3] = 8;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = 0;            // Base Address in Bytes
    mbox[6] = 0;            // size in Bytes
    mbox[7] = MBOX_TAG_LAST;
    if(mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        return mbox[5];
    } else {
        return 0;
    }
} 

/**
 * Sets up the page tables for the MMU using only L1/L2 tables.
 * Use 2MB blocks descriptors to describe the memory format.
 */
static void setup_page_tables() {
    uart_printf("Building page tables...\n");

    // set the data to zero
    uart_printf("   Zeroing page tables...\n");
    zero_page_table(&lvl1_table);
    zero_page_table(&lvl2_table_gb0);
    zero_page_table(&lvl2_table_gb1);
    zero_page_table(&lvl2_table_gb2);
    zero_page_table(&lvl2_table_gb3);

    // ------ Level 1: Table Descriptors ------
    uart_printf("   Setting Level 1 Table Descriptors...\n");
    uint64_t desc0 = make_table_descriptor(&lvl2_table_gb0);
    uart_printf("DEBUG: desc0 = %lx\n", desc0);
    lvl1_table[0] = desc0;
    
    uint64_t desc1 = make_table_descriptor(&lvl2_table_gb1);
    uart_printf("DEBUG: desc1 = %lx\n", desc1);
    lvl1_table[1] = desc1;
    
    uint64_t desc2 = make_table_descriptor(&lvl2_table_gb2);
    uart_printf("DEBUG: desc2 = %lx\n", desc2);
    lvl1_table[2] = desc2;
    
    uint64_t desc3 = make_table_descriptor(&lvl2_table_gb3);
    uart_printf("DEBUG: desc3 = %lx\n", desc3);
    lvl1_table[3] = desc3;

    // ------ Level 2 GB 0: Normal Memory (Low RAM + VC) ------
    
    // get the current videocore memory start
    uint64_t vc_start = (uint64_t) get_vc_memory(); 
    if (vc_start == 0) {
        uart_printf("Failed to get VC Base Address.\n");
        __asm__ volatile("b err_hang");
    }
    
    uart_printf("   Level 2 GB 0 (Normal Memory)...\n");
    for (int i = 0; i < PT_ENTRIES; i++) {
        uint64_t phys_addr = (uint64_t)i << BLOCK_SHIFT; // i * 2MB

        if (phys_addr >= vc_start) { // set videocore memory as non-cacheable
            lvl2_table_gb0[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_DEVICE_MEMORY);
        } else {
            lvl2_table_gb0[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_NORMAL_MEMORY);
        }
    }

    // ------ Level 2 GB 1: Normal Memory(High RAM) ------
    uart_printf("   Level 2 GB 1 (Normal Memory)...\n");
    for(int i = 0; i < PT_ENTRIES; i++) {
        uint64_t phys_addr = HIGH_MEM_GB1 + ((uint64_t)i << BLOCK_SHIFT);
        lvl2_table_gb1[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_NORMAL_MEMORY);
    }

    // ------ Level 2 GB 2: Normal Memory + Invalid Region ------
    uart_printf("   Level 2 GB 2 (Normal Memory)...\n");
    uint64_t pivot = PT_ENTRIES >> 1;
    for(int i = 0; i < (int) pivot; i++) {
        uint64_t phys_addr = HIGH_MEM_GB2 + ((uint64_t)i << BLOCK_SHIFT);
        lvl2_table_gb2[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_NORMAL_MEMORY);
    }

    uart_printf("   Level 2 GB 2 (Invalid Region)...\n");
    for(int i = (int) pivot; i < PT_ENTRIES; i++) {
        lvl2_table_gb2[i] = 0;
    }

    // ------ Level 2 GB 3: Normal Memory + Device Memory (Peripherals) ------
    uart_printf("   Level 2 GB 3 (Normal Memory)...\n");
    uint64_t region_size = PERIPHERAL_START - HIGH_MEM_GB3;
    pivot = region_size >> BLOCK_SHIFT; // dividing by 2^21 (2MB)

    for(uint64_t i = 0; i < pivot; i++) {
        uint64_t phys_addr = HIGH_MEM_GB3 + ((uint64_t)i << BLOCK_SHIFT);
        lvl2_table_gb3[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_NORMAL_MEMORY);
    }

    uart_printf("   Level 2 GB 3 (Device Memory)...\n");
    for(uint64_t i = pivot; i < PT_ENTRIES; i++) {
        uint64_t phys_addr = PERIPHERAL_START + ((uint64_t)(i - pivot) << BLOCK_SHIFT);
        lvl2_table_gb3[i] = make_block_descriptor(phys_addr, BLOCK_ATTR_DEVICE_MEMORY);
    }
    
    uart_printf("MMU Page Tables built successfully.\n");
    timer_wait(1000);
}

/**
 * Sets up the Translation Table Base Register
 */
static void setup_ttbr0() {
    // Set the TTBR0 value to the level 1 table
    uint64_t ttbr0_value = (uint64_t)&lvl1_table;

    // Verify Alignment
    if (ttbr0_value & 0xFFF) {
        uart_printf("ERROR: Level 1 tabel not 4KB aligned!\n");
        __asm__ volatile("b err_hang");
    }

    uart_printf("TTBR0_EL1 = %lx\n", ttbr0_value);
    __asm__ volatile("msr TTBR0_EL1, %0":: "r"(ttbr0_value));
}


/**
 * Enable the MMU by setting bits in SCTLR_EL1
 * 
 * CRITICAL: This must be called AFTER all other MMU setup:
 * - Page tables built
 * - MAIR_EL1 configured
 * - TCR_EL1 configured
 * - TTBR0_EL1 loaded
 */
static void enable_mmu(void) {
    uart_printf("Enabling MMU...\n");
    
    // ensure all previous register writes are visible
    __asm__ volatile("isb" ::: "memory");

    // read current SCTLR_EL1 value
    uint64_t sctlr_value;
    __asm__ volatile("mrs %0, SCTLR_EL1" : "=r"(sctlr_value));
    
    // 3. Set the bits we need:
    //    M bit [0]  = Enable MMU
    //    C bit [2]  = Enable data cache
    //    I bit [12] = Enable instruction cache
    sctlr_value |= SCTLR_MMU_ENABLED;   // M - MMU enable
    // sctlr_value |= SCTLR_D_CACHE_ENABLED;   // C - Data cache enable
    // sctlr_value |= SCTLR_I_CACHE_ENABLED;  // I - Instruction cache enable
    
    // Write back to SCTLR_EL1
    __asm__ volatile("msr SCTLR_EL1, %0" :: "r"(sctlr_value));

    // ISB to ensure MMU enable is seen by next instruction
    //    ISB forces these changes to be seen by the next instruction
    __asm__ volatile("isb" ::: "memory");
}

/**
 * Complete MMU initialization sequence
 * 
 */
void mmu_init() {
    uart_printf("\n=== MMU Initialization (39-bit VA) ===\n");
    
    // Debug: Show page table locations
    verify_page_table_alignment();
    timer_wait(1000);
    
    setup_page_tables();
    
    uart_printf("Writing to MAIR, TCR, TTBR0...\n");
    timer_wait(1000);
    setup_mair();
    setup_tcr();
    timer_wait(1000);
    setup_ttbr0();
    enable_mmu();
    
    uart_printf("\n=== MMU ENABLED SUCCESSFULLY ===\n");
    uart_printf("Virtual addressing now active!\n");
    uart_printf("Caches enabled!\n\n");
    
    // Verify it worked by reading back registers
    uint64_t ttbr0, sctlr;
    __asm__ volatile("mrs %0, TTBR0_EL1" : "=r"(ttbr0));
    __asm__ volatile("mrs %0, SCTLR_EL1" : "=r"(sctlr));

    uart_printf("Verification:\n");
    uart_printf("  MAIR_EL1  = %lx\n", get_mair());
    uart_printf("  TCR_EL1   = %lx\n", get_tcr());
    uart_printf("  TTBR0_EL1 = %lx\n", ttbr0);
    uart_printf("  SCTLR_EL1 = %lx\n", sctlr);
    uart_printf("    MMU (M):    %s\n", (sctlr & (1 << 0)) ? "ON" : "OFF");
    uart_printf("    DCache (C): %s\n", (sctlr & (1 << 2)) ? "ON" : "OFF");
    uart_printf("    ICache (I): %s\n", (sctlr & (1 << 12)) ? "ON" : "OFF");
}