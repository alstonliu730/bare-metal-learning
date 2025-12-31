#include <mmu.h>
#include <sched.h>
#include <entry.h>

int fork(uintptr_t func, uintptr_t args) {
    preempt_disable();
    task_struct_t* p;

    p = (task_struct_t *) palloc();

    if (!p) return 1;

    // Set Task Properties
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority;
    p->preempt_count = 1;

    p->cpu_frame.x19 = func;
    p->cpu_frame.x20 = args;
    p->cpu_frame.pc = (uintptr_t)ret_from_fork;
    p->cpu_frame.sp = (uintptr_t)p + THREAD_SIZE;

    int pid = nr_tasks++;
    task[pid] = p;
    preempt_enable();
    return 0;
}