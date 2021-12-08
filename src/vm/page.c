#include "vm/page.h"
#include <debug.h>
#include <string.h>
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"

void spt_init(struct hash* spt)
{
    hash_init(spt, page_hash_func, page_less_func, NULL);
}

void spt_destroy(struct hash* spt)
{
    hash_destroy(spt, hash_destroy_func);
    return;
}

unsigned page_hash_func(const struct hash_elem* element, void* aux UNUSED)
{
    struct page* p = hash_entry(element, struct page, hash_elem);
    return hash_int((int)(p->addr));
}

bool page_less_func(const struct hash_elem* a, const struct hash_elem* b, void* aux)
{
    struct page* pa = hash_entry(a, struct page, hash_elem);
    struct page* pb = hash_entry(b, struct page, hash_elem);
    return pg_no(pa->addr) < pg_no(pb->addr);
}

void hash_destroy_func(struct hash_elem *element, void* aux UNUSED)
{
    struct page* p = hash_entry(element, struct page, hash_elem);
    struct thread* t = thread_current();
    if (p->loaded) {
        //palloc_free_page(p->frame->p_addr);     //Will not work before frame is implemented.
        palloc_free_page(pagedir_get_page(t->pagedir, p->addr));
        pagedir_clear_page(t->pagedir, p->addr);
    }
    free(p);
}

struct page* spt_page_find(struct hash* spt, void* addr)
{
    struct page p;
    p.addr = pg_round_down(addr);
    struct hash_elem* elem = hash_find(spt, &(p.hash_elem));
    if (elem == NULL) return NULL;
    else return hash_entry(elem, struct page, hash_elem);
}

//Return true if not existed, else return false.
bool spt_page_insert(struct hash* spt, struct page* p)
{
    struct hash_elem* e = hash_insert(spt, &(p->hash_elem));
    if (e == NULL) return true;
    else return false;
}

//Return true if existed, else return false.
bool spt_page_delete(struct hash* spt, struct page* p)
{
    struct hash_elem* e = hash_delete(spt, &(p->hash_elem));
    if (e != NULL) {
        struct thread* t = thread_current();
        if (p->loaded) {
            //palloc_free_page(p->frame->p_addr);     //Will not work before frame is implemented.
            palloc_free_page(pagedir_get_page(t->pagedir, p->addr));
            pagedir_clear_page(t->pagedir, p->addr);
        }
        free(p);
        return true;
    }
    else return false;
}

bool load_file(struct page* page, void* frame_addr)
{
    file_seek (page->file, page->offset);
    //Load this page.
    //if (file_read (page->file, frame_addr, page->read_bytes) != (int) page->read_bytes)
    int read_bytes = file_read (page->file, frame_addr, page->read_bytes);// file_read_at(page->file, frame_addr, page->read_bytes, page->offset);
    if (read_bytes != (int) page->read_bytes)
    {
        return false; 
    }
    memset (frame_addr + page->read_bytes, 0, page->zero_bytes);
    return true;
}