#ifndef _SCHED_H
#define _SCHED_H

#include <common.h>

#define CPU_CONTEXT_OFFSET          0       // offset of cpu_context

#define THREAD_SIZE                 4096
#define NR_TASKS                    3

#define INIT_TASK  /*cpu_context*/	{ {0,0,0,0,0,0,0,0,0,0,0,0,0},\
                   /* state etc */	0,0,1,0 }

#define TASK_RUNNING                0

typedef struct cpu_context {
    uintptr_t x19;
    uintptr_t x20;
    uintptr_t x21;
    uintptr_t x22;
    uintptr_t x23;
    uintptr_t x24;
    uintptr_t x25;
    uintptr_t x26;
    uintptr_t x27;
    uintptr_t x28;
    uintptr_t fp;
    uintptr_t sp;
    uintptr_t pc;
} cpu_context_t;

typedef struct task_struct {
    cpu_context_t cpu_frame;
    long state;
    long counter;
    long priority;
    long preempt_count;
} task_struct_t;

extern task_struct_t* current;
extern task_struct_t* task[NR_TASKS];
extern int nr_tasks;

void preempt_disable();
void preempt_enable();

void schedule();
void schedule_tail();

void switch_task(task_struct_t* next);

void timer_tick();

void cpu_switch_task(task_struct_t* prev, task_struct_t* next);
#endif /* _SCHED_H */