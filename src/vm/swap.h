#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stddef.h>
#include <stdio.h>
#include "devices/block.h"

void swap_init(void);
bool swap_in(size_t idx, void* p_addr);
size_t swap_out(void* p_addr);
void swap_block_read(struct block* block, size_t idx, void* p_addr);
void swap_block_write(struct block* block, size_t idx, void* p_addr);

#endif /* vm/swap.h */