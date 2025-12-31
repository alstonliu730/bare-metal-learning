#include <common.h>
#include <irq.h>
#include <sched.h>

static task_struct_t init_task = INIT_TASK;
task_struct_t* current = &(init_task);
task_struct_t* task[NR_TASKS] = {&(init_task), };
int nr_tasks = 1;


void preempt_disable() {
    current->preempt_count++;
}

void preempt_enable() {
    current->preempt_count--;
}

static void _schedule() {
    preempt_disable();
    int next, count;
    task_struct_t* p;

    while (1) {
        count = -1;
        next = 0;
        
        // Find task with a RUNNING state and max counter
        for (int i = 0; i < NR_TASKS; i++) {
            p = task[i];
            if (p && p->state == TASK_RUNNING && p->counter > count) {
                count = p->counter;
                next = i;
            }
        }

        // Found task then break
        if (count < 0) {
            break;
        }

        // Tasks are waiting, reconfigure counters based on priority
        for (int i = 0; i < NR_TASKS; i++) {
            p = task[i];
            if (p) {
                p->counter = (p->counter >> 1) + p->priority;
            }
        }
    }
    switch_task(task[next]);
    preempt_enable();
}

void schedule() {
    current->counter = 0;
    _schedule();
}

void switch_task(task_struct_t* next) {
    if (current == next) return;
    task_struct_t* prev = current;
    current = next;

    cpu_switch_task(prev, next);
}

void cpu_switch_task(task_struct_t* prev, task_struct_t* next) {
	__asm__ volatile(
        "mov    x10, %[offset]\n\t"           // Load CPU_CONTEXT_OFFSET offset
        "add    x8, %[prev], x10\n\t"         // x8 = prev + offset
        "mov    x9, sp\n\t"                   // Save current stack pointer
        
        // Save callee-saved registers to 'prev' context
        "stp    x19, x20, [x8], #16\n\t"
        "stp    x21, x22, [x8], #16\n\t"
        "stp    x23, x24, [x8], #16\n\t"
        "stp    x25, x26, [x8], #16\n\t"
        "stp    x27, x28, [x8], #16\n\t"
        "stp    x29, x9, [x8], #16\n\t"       // Save FP and SP
        "str    x30, [x8]\n\t"                // Save link register
        
        // Now restore from 'next' context
        "add    x8, %[next], x10\n\t"         // x8 = next + offset
        "ldp    x19, x20, [x8], #16\n\t"
        "ldp    x21, x22, [x8], #16\n\t"
        "ldp    x23, x24, [x8], #16\n\t"
        "ldp    x25, x26, [x8], #16\n\t"
        "ldp    x27, x28, [x8], #16\n\t"
        "ldp    x29, x9, [x8], #16\n\t"       // Restore FP and new SP
        "ldr    x30, [x8]\n\t"                // Restore link register
        "mov    sp, x9\n\t"                   // Switch stack
        
        : /* no outputs */
        : [prev] "r" (prev),                  // Map prev to a register
          [next] "r" (next),                  // Map next to a register
          [offset] "i" (CPU_CONTEXT_OFFSET)   // Immediate constant
        : "x8", "x9", "x10", "memory"         // Clobbered registers
    );
}

void schedule_tail() {
    preempt_enable();
}

void timer_tick() {
    current->counter--;
    if (current->counter > 0 || current->preempt_count > 0) return;

    current->counter = 0;
    irq_enable();
    _schedule();
    irq_disable();
}