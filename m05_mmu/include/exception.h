#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <stdint.h>
#include <common.h>

#define EXCEPTION_STR_LEN           64

// ESR_EL1 Masks
#define ISS2_MASK                   BIT_MASK(55, 32)
#define EC_MASK                     BIT_MASK(31, 26)
#define IL_MASK                     BIT(25)
#define ISS_MASK                    BIT_MASK(24, 0)

/**
 * Exception string message for each exception code
 */
typedef struct exception_msg {
    uint8_t ec_code;
    char msg[EXCEPTION_STR_LEN];
} exception_msg_t;

// Exception Codes (6-bits)
#define EC_UNKNOWN              0b000000
#define EC_TRAPPED_WF           0b000001
#define EC_SIMD_ACCESS          0b000111
#define EC_TRAPPED_UNKNOWN      0b001010
#define EC_BRANCH_TARGET        0b001101
#define EC_ILLEGAL_EXEC         0b001110
#define EC_SVC_INSTR_A32        0b010001
#define EC_SVC_INSTR_A64        0b010101
#define EC_PAC_FAIL             0b011100
#define EC_ABORT_LOWER_EL       0b100000
#define EC_ABORT_SAME_EL        0b100001
#define EC_PC_ALIGNMENT_FAULT   0b100010
#define EC_D_ABORT_LOWER_EL     0b100100
#define EC_D_ABORT_SAME_EL      0b100101
#define EC_SP_ALIGNMENT_FAULT   0b100110
#define EC_MEM_OP_EXC           0b100111
#define EC_GCS_EXC              0b101101
#define EC_SERROR_EXC           0b101111
#define EC_PROFILING_EXC        0b111101

void exception_report(uint64_t type, uint64_t esr_reg, uint64_t elr, uint64_t spsr);

#endif /* __EXCEPTION_H__*/