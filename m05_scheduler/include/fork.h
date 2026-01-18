#ifndef _FORK_H
#define _FORK_H

#include <common.h>
#include <stdint.h>

int fork(uintptr_t func, uintptr_t args); 

#endif /* _FORK_H*/