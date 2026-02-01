#include <exception.h>
#include <uart.h>
#include <entry.h>
#include <irq.h>

#define EXC_STR_ENTRY(code, string) [code] = {code, string}

static const uint8_t exception_codes[] = {EC_UNKNOWN,
                                          EC_TRAPPED_WF,
                                          EC_SIMD_ACCESS,
                                          EC_TRAPPED_UNKNOWN,
                                          EC_BRANCH_TARGET,
                                          EC_ILLEGAL_EXEC,
                                          EC_SVC_INSTR_A32,
                                          EC_SVC_INSTR_A64,
                                          EC_PAC_FAIL,
                                          EC_ABORT_LOWER_EL,
                                          EC_ABORT_SAME_EL,
                                          EC_PC_ALIGNMENT_FAULT,
                                          EC_D_ABORT_LOWER_EL,
                                          EC_D_ABORT_SAME_EL,
                                          EC_SP_ALIGNMENT_FAULT,
                                          EC_MEM_OP_EXC,
                                          EC_GCS_EXC,
                                          EC_SERROR_EXC,
                                          EC_PROFILING_EXC};
                                          
static const exception_msg_t exception_msgs[] = {
    EXC_STR_ENTRY(EC_UNKNOWN, "Unknown Reason"),
    EXC_STR_ENTRY(EC_TRAPPED_WF, "Trapped WF instruction"),
    EXC_STR_ENTRY(EC_SIMD_ACCESS, "SIMD/FP Access Trapped"),
    EXC_STR_ENTRY(EC_BRANCH_TARGET, "Branch Target Exception"),
    EXC_STR_ENTRY(EC_ILLEGAL_EXEC, "Illegal Execution State"),
    EXC_STR_ENTRY(EC_SVC_INSTR_A32, "SVC instruction execution in AArch32 State"),
    EXC_STR_ENTRY(EC_SVC_INSTR_A64, "SVC instruction execution in AArch64 State"),
    EXC_STR_ENTRY(EC_PAC_FAIL, "Exception from PAC Fail"),
    EXC_STR_ENTRY(EC_ABORT_LOWER_EL, "Instruction Abort from Lower Exception Level"),
    EXC_STR_ENTRY(EC_ABORT_SAME_EL, "Instruction Abort from Same Exception Level"),
    EXC_STR_ENTRY(EC_PC_ALIGNMENT_FAULT, "PC Alignment Fault Exception"),
    EXC_STR_ENTRY(EC_D_ABORT_LOWER_EL, "Data Abort from Lower Exception Level"),
    EXC_STR_ENTRY(EC_D_ABORT_SAME_EL, "Data Abort from Same Exception Level"),
    EXC_STR_ENTRY(EC_SP_ALIGNMENT_FAULT, "SP Alignment Fault Exception"),
    EXC_STR_ENTRY(EC_MEM_OP_EXC, "Memory Operation Exception"),
    EXC_STR_ENTRY(EC_GCS_EXC, "GCS Exception"),
    EXC_STR_ENTRY(EC_SERROR_EXC, "SError Exception"),
    EXC_STR_ENTRY(EC_PROFILING_EXC, "Profiling Exception")};

static uint8_t check_ec(uint8_t ec) {
    int size = sizeof(exception_codes) / sizeof(exception_codes[0]);
    for(int i = 0; i < size; i++) {
        if (ec == exception_codes[i]) {
            return 1;
        }
    }
    return 0;
}
/**
 * Reports which interrupt is set and the exception information on the invalid entry.
 */
void exception_report(uint64_t type, uint64_t esr, uint64_t elr, uint64_t spsr)
{
    irq_disable();
    // printing Exception label
    switch (type)
    {
        case SYNC_INVALID_EL1h:
            uart_printf("Invalid Synchronous Exception EL1h Detected!\n");
            break;
        case IRQ_INVALID_EL1h:
            uart_printf("Invalid Interrupt Exception EL1h Detected!\n");
            break;
        case FIQ_INVALID_EL1h:
            uart_printf("Invalid FIQ Exception EL1h Detected!\n");
            break;
        case ERROR_INVALID_EL1h:
            uart_printf("Invalid Error Exception EL1h Detected!\n");
            break;
        default:
            uart_printf("Type: %lx\n", type);
            break;
    }

    // Exception Status Register Information
    uint8_t ec = esr & EC_MASK;
    exception_msg_t ec_info;
    if (check_ec(ec)) {
        ec_info = exception_msgs[esr & EC_MASK];
    } else {
        ec_info = exception_msgs[0];
    }
    uint64_t iss2_code = esr & ISS2_MASK;
    uint64_t il_code = esr & IL_MASK;
    uint64_t iss_code = esr & ISS_MASK;

    uart_printf("=== %s ===\n", ec_info.msg);
    uart_printf("Exception Code: %d\n", ec_info.ec_code);
    uart_printf("ISS2 Code: %x\n", iss2_code);
    uart_printf("IL Code: %d\n", il_code);
    uart_printf("ISS Code: %x\n", iss_code);
    uart_printf("===================\n");

    uart_printf("ELR REG: %lx\n", elr);

    uart_printf("SPSR REG: %lx\n", spsr);
    irq_enable();
    return;
}
