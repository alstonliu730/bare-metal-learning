#ifndef _MMU_H
#define _MMU_H

#include <common.h>
#include <stdint.h>

// Page macros
#define TABLE_SHIFT         9
#define PAGE_SHIFT          12
#define BLOCK_SHIFT         (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE           4096
#define BLOCK_SIZE          LSHIFT(1, BLOCK_SHIFT)
#define PT_ENTRIES          512

// page table with 4KB alignment
typedef uint64_t page_table_t[PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

// ============================================================
// ADDRESS MASKS
// ============================================================
// For table descriptors: address at bits [47:12] (4KB aligned)
#define TABLE_ADDR_MASK             (BIT_MASK(47,12))

// For 2MB block descriptors: address at bits [47:21] (2MB aligned)
#define BLOCK_ADDR_MASK             (BIT_MASK(47,21))

// ============================================================
// DESCRIPTOR TYPE BITS [1:0]
// ============================================================
#define DESC_INVALID                0b00
#define DESC_TABLE                  0b11
#define DESC_BLOCK                  0b01

// ============================================================
// ACCESS PERMISSIONS [7:6] - AP
// ============================================================
#define AP_RW_EL1                   LSHIFT(0b00, 6)
#define AP_RW_ALL                   LSHIFT(0b01, 6)
#define AP_RO_EL1                   LSHIFT(0b10, 6)
#define AP_RO_ALL                   LSHIFT(0b11, 6)

// ============================================================
// SHAREABILITY [9:8]
// ============================================================
#define SH_NON_SHAREABLE            LSHIFT(0b00, 8)
#define SH_OUTER_SHAREABLE          LSHIFT(0b10, 8)
#define SH_INNER_SHAREABLE          LSHIFT(0b11, 8)

// ============================================================
// ACCESS FLAG BIT [10]
// ============================================================
#define AF_ACCESSED         BIT(10)

// ============================================================
// MEMORY ATTRIBUTE INDEX [4:2] = 3-bit index into MAIR_EL1 (0-7)
// ============================================================
#define ATTR_IDX(n)     ((n) << 2)  // Generic: index n (0-7)

// ============================================================
// NOT GLOBAL [11] - nG
// ============================================================
// 0 = Global (applies to all ASID), 1 = Process-specific
#define NG_GLOBAL       (0ULL << 11)
#define NG_NOT_GLOBAL   (1ULL << 11)

// ============================================================
// EXECUTE-NEVER [54:53] - UXN, PXN
// ============================================================
#define PXN                 BIT(53)
#define UXN                 BIT(54)
#define XN                  (PXN | UXN)

// ============================================================
//                  MAIR Attribute Encodings
// ============================================================
//   Bits [7:4] = Outer policy
//   Bits [3:0] = Inner policy

// Device Memory: nGnRnE (most restrictive)
#define MAIR_DEVICE_nGnRnE              0
#define MAIR_DEVICE_nGnRnE_VAL          LSHIFT(0x00, MAIR_DEVICE_nGnRnE * 8)

#define MAIR_DEVICE_nGnRE               1
#define MAIR_DEVICE_nGnRE_VAL           LSHIFT(0x04, MAIR_DEVICE_nGnRE * 8)

#define MAIR_DEVICE_GRE                 2
#define MAIR_DEVICE_GRE_VAL             LSHIFT(0x0C, MAIR_DEVICE_GRE * 8)

// Normal Memory: Inner/Outer Write-Back, Read-Allocate, Write-Allocate
#define MAIR_NORMAL_NC                  3
#define MAIR_NORMAL_NC_VAL              LSHIFT(0x44, MAIR_NORMAL_NC * 8)

#define MAIR_NORMAL_MEMORY              4
#define MAIR_NORMAL_MEMORY_VAL          LSHIFT(0xFF, MAIR_NORMAL_MEMORY * 8)

// set MAIR VALUE 
#define MAIR_VALUE \
    (MAIR_DEVICE_nGnRnE_VAL | MAIR_DEVICE_nGnRE_VAL | MAIR_DEVICE_GRE_VAL | MAIR_NORMAL_NC_VAL | MAIR_NORMAL_MEMORY_VAL)
// ============================================================
// Translation Control Register (TCR_ELx)
// ============================================================

// ----- TCR Masks -----
#define T0SZ_MASK           BIT_MASK(5,0)
#define IRGN0_MASK          BIT_MASK(9,8)
#define ORGN0_MASK          BIT_MASK(11,10)
#define SH0_MASK            BIT_MASK(13,12)
#define TG0_MASK            BIT_MASK(15,14)
#define T1SZ_MASK           BIT_MASK(21,16)
#define TG1_MASK            BIT_MASK(31,30)
#define IPS_MASK            BIT_MASK(34,32)

// ----- TCR Values -----

/**
 * T0SZ_VAL [5:0] - Size offset of the memory region for TTBR0_EL1
 * To calculate: T0SZ_VAL = (64 - [bits in VA]) & T0SZ_MASK
 * Represents the region size to be 2 ^ T0SZ_VAL
 * 
 * @warning This is hardcoded for my 39-bit VA
 */
#define T0SZ_VAL            (25ULL << 0)

/**
 * EPD0 [7] - controls whether a translation table walk is performed on a TLB Miss
 */
#define EPD0_WALK_ENABLE        LSHIFT(0b0, 7)
#define EPD0_WALK_DISABLE       LSHIFT(0b1, 7)

/**
 * IRGN0 [9:8] - Inner cacheability attr. for memory associated with transaltion table walks.]
 */
#define IRGN0_NC                LSHIFT(0b00, 8)
#define IRGN0_WBWA              LSHIFT(0b01, 8)
#define IRGN0_WT                LSHIFT(0b10, 8)
#define IRGN0_WBnWA             LSHIFT(0b11, 8)

/**
 * ORGN0 [11:10] - Outer cacheability attr. for memory associated with transaltion table walks.
 */
#define ORGN0_NC                LSHIFT(0b00, 10)
#define ORGN0_WBWA              LSHIFT(0b01, 10)
#define ORGN0_WT                LSHIFT(0b10, 10)
#define ORGN0_WBnWA             LSHIFT(0b11, 10)

/**
 * SH0 [13:12] - SHAREABILITY attr for memory TCR
 */
#define SH0_NON_SHAREABLE        LSHIFT(0b00, 12)
#define SH0_OUTER_SHAREABLE      LSHIFT(0b10, 12)
#define SH0_INNER_SHAREABLE      LSHIFT(0b11, 12)

/**
 * TG0 [15:14] - Granule Size
 */
#define TG0_4KB                 LSHIFT(0b00, 14)
#define TG0_64KB                LSHIFT(0b01, 14)
#define TG0_16KB                LSHIFT(0b10, 14)

/**
 * T1SZ_VAL [5:0] - Size offset of the memory region used for TTBR1_EL1
 * To calculate: T1SZ_VAL = (64 - [bits in VA]) & T1SZ_MASK
 * Represents the region size to be 2 ^ T1SZ_VAL
 * 
 * @warning This is hardcoded for my 39-bit VA
 */
#define T1SZ_VALUE              LSHIFT(25, 16)

/**
 * A1 [22] - Selects TTBR0 or TTBR1 as ASID (Address Space Identifier)
 */
#define A1_TTBR0                LSHIFT(0b0, 22)
#define A1_TTBR1                LSHIFT(0b1, 22)

/**
 * EPD1 [23] - controls whether a translation table walk is performed on a TLB Miss (TTBR1)
 */
#define EPD1_WALK_ENABLE        LSHIFT(0b0, 23)
#define EPD1_WALK_DISABLE       LSHIFT(0b1, 23)

/**
 * IRGN1 [25:24] - Inner cacheability attr. for memory associated with transaltion table walks.] (TTBR1)
 */
#define IRGN1_NC                LSHIFT(0b00, 24)
#define IRGN1_WBWA              LSHIFT(0b01, 24)
#define IRGN1_WT                LSHIFT(0b10, 24)
#define IRGN1_WBnWA             LSHIFT(0b11, 24)

/**
 * ORGN1 [27:26] - Outer cacheability attr. for memory associated with transaltion table walks. (TTBR1)
 */
#define ORGN1_NC                LSHIFT(0b00, 26)
#define ORGN1_WBWA              LSHIFT(0b01, 26)
#define ORGN1_WT                LSHIFT(0b10, 26)
#define ORGN1_WBnWA             LSHIFT(0b11, 26)

/**
 * SH1 [29:28] - SHAREABILITY attr for memory (TTBR1)
 */
#define SH1_NON_SHAREABLE        LSHIFT(0b00, 28)
#define SH1_OUTER_SHAREABLE      LSHIFT(0b10, 28)
#define SH1_INNER_SHAREABLE      LSHIFT(0b11, 28)

/**
 * TG1 [31:30] - Granule Size for TTBR1
 */
#define TG1_4KB                 LSHIFT(0b10, 30)
#define TG1_64KB                LSHIFT(0b11, 30)
#define TG1_16KB                LSHIFT(0b01, 30)

/**
 * IPS [34:32] - Intermediate Physical Address Size
 */ 
#define IPS_32                  LSHIFT(0b000, 32)
#define IPS_36                  LSHIFT(0b001, 32)
#define IPS_40                  LSHIFT(0b010, 32)
#define IPS_42                  LSHIFT(0b011, 32)
#define IPS_44                  LSHIFT(0b100, 32)
#define IPS_48                  LSHIFT(0b101, 32)
#define IPS_52                  LSHIFT(0b110, 32)
#define IPS_56                  LSHIFT(0b111, 32)

/**
 * AS [36] - ASID Size
 */
#define AS_8                    LSHIFT(0b0, 36)
#define AS_16                   LSHIFT(0b1, 36)

/**
 * TBI0 [37] - Top Byte ignored (if top byte is used for address match for TTBR0)
 */
#define TBI0_USED               LSHIFT(0b0, 37)
#define TBI0_IGN                LSHIFT(0b1, 37)

/**
 * TBI1 [38] - Top Byte ignored (if top byte is used for address match for TTBR1)
 */
#define TBI1_USED               LSHIFT(0b0, 38)
#define TBI1_IGN                LSHIFT(0b1, 38)

/**
 * Common Memory Attributes to Block Descriptors
 */
#define BLOCK_ATTR_NORMAL_MEMORY \
    (AF_ACCESSED | SH_INNER_SHAREABLE | AP_RW_EL1 | ATTR_IDX(MAIR_NORMAL_MEMORY))

#define BLOCK_ATTR_NORMAL_NC \
    (AF_ACCESSED | SH_OUTER_SHAREABLE | AP_RW_EL1 | ATTR_IDX(MAIR_NORMAL_NC))
    
#define BLOCK_ATTR_DEVICE_MEMORY \
    (AF_ACCESSED | SH_NON_SHAREABLE | AP_RW_EL1 | ATTR_IDX(MAIR_DEVICE_nGnRE) | XN)

// Memory Map Addresses
#define HIGH_MEM_GB1        0x40000000ULL
#define HIGH_MEM_GB2        0x80000000ULL
#define HIGH_MEM_GB2_MID    (HIGH_MEM_GB2 + 0x20000000ULL)
#define HIGH_MEM_GB3        0xC0000000ULL
#define PERIPHERAL_START    0xFA000000ULL

// Initialization function
void verify_page_table_alignment();
uint64_t make_table_descriptor(void *next_table_addr);
void mmu_init();
void inv_cache(uintptr_t start, uintptr_t end);
void clean_cache(uintptr_t start, uintptr_t end);

#endif /*_MMU_H*/