#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

#include <stdbool.h>
#include "threads/thread.h"
#include "threads/synch.h"
#include "vm/page.h"

struct lock file_system_lock;

void syscall_init (void);
void halt(void);
void exit(int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
struct page* validate_user_pointer(void *pointer);
void validate_fd(int fd);
void validate_buffer(void* buffer, unsigned size, bool writable);
void validate_string(void* str, bool writable);
void get_syscall_arg(void *sp, int *arg, int arg_cnt);
int write(int fd, const void *buffer, unsigned size);
int read (int fd, void *buffer, unsigned size);
bool create(const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
void munmap (mapid_t mapping);
void munmap_all_pages (struct fmm_file *fmm_file);
struct fmm_file* find_fmm_by_mapid (mapid_t mapping);
mapid_t mmap(int fd, void *addr);

#endif /* userprog/syscall.h */
