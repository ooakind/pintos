#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm/page.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include <list.h>


struct frame {
    void* p_addr;
    struct page* p_entry;
    struct thread* thread;
    struct list_elem elem;
};

void ft_init(void);
void ft_push_back(struct frame* frame);
void ft_remove(struct frame* frame);
struct frame* ft_find_by_addr(void* p_addr);
struct list_elem* ft_select_next(void);
struct frame* allocate_frame(enum palloc_flags flag);
void free_frame(void* p_addr);
void evict_frame(enum palloc_flags flag);

#endif /* vm/frame.h */