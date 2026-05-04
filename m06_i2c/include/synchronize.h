#ifndef __SYNCHRONIZE_H__
#define __SYNCHRONIZE_H__

#include <common.h>
#include <stdint.h>

// Instruction Cache Operations
#define INSTR_CLEAN_ALL                     __asm__ volatile("ic iallu" ::: "memory");
#define INSTR_CLEAN_VA                      __asm__ volatile("ic ivau"  ::: "memory");

// Data cache Operations
#define DATA_CLEAN_INV_SET(set_way)         __asm__ volatile("dc cisw,  %0" :: "r"(set_way));
#define DATA_CLEAN_INV_VA(virt_addr)        __asm__ volatile("dc civac, %0" :: "r"(virt_addr));
#define DATA_CLEAN_SET(set_way)             __asm__ volatile("dc csw,   %0" :: "r" (set_way));

// Barrier Operations
#define DATA_SYNC_BARRIER()         
#endif /* __SYNCHRONIZE_H__ */

