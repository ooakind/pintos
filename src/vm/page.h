#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <list.h>
#include <hash.h>
#include <debug.h>
#include "filesys/file.h"

enum page_type
{
    PAGE_EXE,
    PAGE_FILE,
    PAGE_SWAP,
    PAGE_STACK,
    PAGE_OTHER
};

struct page
{
    enum page_type type;
    void* addr;
    bool write;
    bool loaded;
    //struct frame* frame;
    struct file* file;
    size_t offset;
    size_t read_bytes;
    size_t zero_bytes;
    struct hash_elem hash_elem;
    struct list_elem fmm_elem;
    size_t swap_elem;
};

void spt_init(struct hash* spt);
void spt_destroy(struct hash* spt);
unsigned page_hash_func(const struct hash_elem* element, void* aux UNUSED);
bool page_less_func(const struct hash_elem* a, const struct hash_elem* b, void* aux UNUSED);
void hash_destroy_func(struct hash_elem *element, void* aux UNUSED);
struct page* spt_page_find(struct hash* spt, void* addr);
bool spt_page_insert(struct hash* spt, struct page* p);
bool spt_page_delete(struct hash* spt, struct page* p);
bool load_file(struct page* page, void* frame_addr);

#endif /* vm/page.h */