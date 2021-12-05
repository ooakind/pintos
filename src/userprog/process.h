#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "vm/page.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
struct thread *get_child(tid_t pid);
void remove_child(struct thread *child);

/* Project 2 */
int process_fd_open(struct file *file);
struct file * process_fd_file_ptr(int fd);
void process_fd_close(int fd);

/* Project 3 */
bool page_fault_handler(struct page* page);

#endif /* userprog/process.h */
