#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stddef.h>

void swap_init(void);
void swap_in(size_t idx, void* p_addr);
size_t swap_out(void* p_addr);

#endif /* vm/swap.h */