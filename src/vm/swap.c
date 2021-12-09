#include "vm/swap.h"
#include <bitmap.h>

struct bitmap* swap_table;

void swap_init()
{
    swap_table = bitmap_create(1024); //4MB swap disk
}

void swap_in(size_t idx, void* p_addr)
{
    
}

size_t swap_out(void* p_addr);