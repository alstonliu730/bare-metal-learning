#ifndef _GIC_H
#define _GIC_H

#include <common.h>

// ----------------------- GIC Base Addresses -----------------------
#define GIC_BASE            0xFF840000
#define GICD_BASE           (GIC_BASE + 0x1000)
#define GICC_BASE           (GIC_BASE + 0x2000) 

// ----------------------- GIC Distributor -----------------------
#define GICD_CTLR           (GICD_BASE + 0x000)
#define GICD_IIDR           (GICD_BASE + 0x008)
#define GICD_SET_ENABLER    (GICD_BASE + 0x100)
#define GICD_CLR_ENABLER    (GICD_BASE + 0x180)
#define GICD_SET_PENDING    (GICD_BASE + 0x200)
#define GICD_CLR_PENDING    (GICD_BASE + 0x280)
#define GICD_SET_ACTIVE     (GICD_BASE + 0x300)
#define GICD_CLR_ACTIVE     (GICD_BASE + 0x380)
#define GICD_PRIORITY       (GICD_BASE + 0x400)
#define GICD_TARGET         (GICD_BASE + 0x800)
#define GICD_ICFGR          (GICD_BASE + 0xC08)

typedef enum {
    gicdctlr_EnableGrp0 = (1 << 0),
    gicdctlr_EnableGrp1NS = (1 << 1),
    gicdctlr_EnableGrp1A = (1 << 1),
    gicdctlr_EnableGrp1S = (1 << 2),
    gicdctlr_EnableAll = (1 << 2) | (1 << 1) | (1 << 0),
    gicdctlr_ARE_S = (1 << 4),	/* Enable Secure state affinity routing */
    gicdctlr_ARE_NS = (1 << 5),	/* Enable Non-Secure state affinity routing */
    gicdctlr_DS = (1 << 6),	/* Disable Security support */
    gicdctlr_E1NWF = (1 << 7)	/* Enable "1-of-N" wakeup model */
} gicd_ctlr_flags_t;

typedef enum {
    edge_triggered = 0x2,
    level_sensitive = 0x0
} gicd_cfg_flags_t;

// ----------------------- GIC CPU Interface -----------------------
#define GICC_CTLR           (GICC_BASE + 0x0000)
#define GICC_PMR            (GICC_BASE + 0x0004)
#define GICC_BPR            (GICC_BASE + 0x0008)
#define GICC_IAR            (GICC_BASE + 0x000C)
#define GICC_EOIR           (GICC_BASE + 0x0010)

#define GICC_IIDR           (GICC_BASE + 0x00FC)
#define GICC_DIR            (GICC_BASE + 0x1000)

// ----------------------- GIC Functions -----------------------
void gic_dist_init();
void gic_dist_clr();
void gic_cpu_init();
void enable_interrupt(uint32_t irq);
void set_irq_priority(uint32_t irq, uint32_t priority);
void assign_target(uint32_t irq);
void clear_interrupt(uint32_t irq);
void setup_interrupt(uint32_t irq, uint32_t priority, gicd_cfg_flags_t flag);

void gic_init();

#endif /* _GIC_H */