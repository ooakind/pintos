#include "vm/frame.h"
#include <list.h>
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/malloc.h"

struct list frame_table;
struct list_elem* frame_selector;

void ft_init()
{
    list_init(&frame_table);
    frame_selector = NULL;
}

void ft_push_back(struct frame* frame)
{
    list_push_back(&frame_table, &frame->elem);
}

void ft_remove(struct frame* frame)
{
    list_remove(&frame->elem);
}

struct frame* ft_find_by_addr(void* p_addr)
{
    struct frame* frame;
    struct list_elem* e;
    for (e = list_begin(&frame_table); e != list_end(&frame_table); e = list_next(e))
    {
        frame = list_entry(e, struct frame, elem);
        if (p_addr == frame->p_addr) return frame;
    }

    return NULL;
}

struct list_elem* ft_select_next()
{
    if (list_empty(&frame_table))
    {
        return frame_selector = NULL;
    }
    else if (frame_selector == NULL || frame_selector == list_end(&frame_table))
    {
        return frame_selector = list_begin(&frame_table);
    }
    
    frame_selector = list_next(frame_selector);
    if (frame_selector == list_end(&frame_table))
    {
        ft_select_next();
    }
    return frame_selector;
}

struct frame* allocate_frame(enum palloc_flags flag)
{
    uint8_t* frame_addr;
    struct frame* frame = (struct frame*) malloc(sizeof(struct frame));
    if (frame == NULL)
        return NULL;
    
    while ((frame_addr = palloc_get_page(flag)) == NULL)
    {
        free(frame);
        return NULL;
        evict_frame(flag);
    }
    
    frame->p_addr = frame_addr;
    frame->thread = thread_current();
    ft_push_back(frame);

    return frame;
}

void free_frame(void* p_addr)
{
    struct frame* frame = ft_find_by_addr(p_addr);
    if (frame == NULL)
        return;
    ft_remove(frame);
    palloc_free_page(frame->p_addr);
    free(frame);
}

void evict_frame(enum palloc_flags flag)
{

}