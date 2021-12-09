#include "vm/swap.h"
#include <bitmap.h>
#include <stdio.h>
#include "devices/block.h"

struct bitmap* swap_table;

void swap_init()
{
    swap_table = bitmap_create(1024); //4MB swap disk
}

bool swap_in(size_t idx, void* p_addr)
{
    struct block* block = block_get_role(BLOCK_SWAP);
    if (block == NULL) return false;
    if (!bitmap_test(swap_table, idx)) return false;

    swap_block_read(block, idx, p_addr);
    bitmap_set(swap_table, idx, false);
    return true;
}

size_t swap_out(void* p_addr)
{
    struct block* block = block_get_role(BLOCK_SWAP);
    if (block == NULL) return -1;

    size_t idx = bitmap_scan(swap_table, 0, 1, false);
    if (idx == BITMAP_ERROR) return -1;

    swap_block_write(block, idx, p_addr);
    bitmap_set(swap_table, idx, true);

    return idx;
}

void swap_block_read(struct block* block, size_t idx, void* p_addr)
{
    //1 Swap slot(per 4 KB) = 8 Block sector(per 512 B)
    block_sector_t start_sector = 8 * idx;
    int i;
    for (i = 0; i < 8; i++)
    {
        block_read(block, start_sector + i, p_addr + BLOCK_SECTOR_SIZE * i);
    }
}

void swap_block_write(struct block* block, size_t idx, void* p_addr)
{
    block_sector_t start_sector = 8 * idx;
    int i;
    for (i = 0; i < 8; i++)
    {
        block_write(block, start_sector + i, p_addr + BLOCK_SECTOR_SIZE * i);
    }
}