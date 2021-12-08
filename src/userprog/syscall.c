#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include <string.h>
#include "threads/synch.h"
/* Added for Project 2 */
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include <devices/input.h>
/* Added for Project 3 */
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);
int get_arg_cnt(int);

struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

void
syscall_init (void) 
{
  lock_init(&file_system_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(void)
{
  shutdown_power_off ();
}

void exit(int status)
{
  thread_current()->exit_status = status;
  int i;
  for (i = 2; i < FD_TABLE_SIZE; i++)
  {
    struct file * file = process_fd_file_ptr(i); 
    if(file != NULL) 
    {
      close(i);
    }
  }
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit();
}

pid_t exec (const char *cmd_line)
{
  pid_t pid = process_execute(cmd_line);
  struct thread *new_process = get_child(pid);
  if (new_process == NULL) return -1;

  sema_down(&new_process->exec_sema);

  if (new_process->load_status == 1) return pid;
  else if (new_process->load_status == -1) return -1;
  else return -1;   //load_status == 0(which means process is not loaded yet. should not happen)
}

int wait (pid_t pid)
{
  return process_wait(pid);
}

struct page* validate_user_pointer(void *pointer)
{
  if (pointer == NULL || !(pointer < PHYS_BASE && pointer > (void *)0x8048000)) // >= ?
  {
    exit(-1);
  }
  return spt_page_find(&thread_current()->spt, pg_round_down(pointer));
}

void validate_fd(int fd)
{
  if (!(fd >= 0 && fd < FD_TABLE_SIZE))
  {
    exit(-1);
  }
}

void validate_buffer(void* buffer, unsigned size, bool writable)
{
  void* page_begin_addr = pg_round_down(buffer);
  void* page_end_addr = pg_round_up((char *)buffer + size);
  struct page* p;
  
  while (page_begin_addr < page_end_addr)
  {
    p = validate_user_pointer(page_begin_addr);
    if (p == NULL)
      exit(-1);
    if (writable == true && p->write == false)
      exit(-1);

    page_begin_addr = (void *) ((uintptr_t) page_begin_addr + PGSIZE);
  }
}

void validate_string(void* str, bool writable)
{
  validate_buffer(str, strlen((char *)str), writable);
}

void get_syscall_arg(void *sp, int *arg, int arg_cnt)
{
  validate_user_pointer(sp);
  int i;
  for (i = 0; i < arg_cnt; i++)
  {
    sp = (uint32_t *)sp + 1; // Argument size is 4 bytes(32bits)
    validate_user_pointer(sp); 
    arg[i] = *(uint32_t *)sp;
  }
}

int get_arg_cnt(int syscall_num)
{
  switch(syscall_num)
  {
    case SYS_HALT:
      return 0;
    case SYS_EXIT:
      return 1;
    case SYS_EXEC:
      return 1;
    case SYS_WAIT:
      return 1;
    case SYS_CREATE:
      return 2;
    case SYS_REMOVE:
      return 1;
    case SYS_OPEN:
      return 1;
    case SYS_FILESIZE:
      return 1;
    case SYS_READ:
      return 3;
    case SYS_WRITE:
      return 3;
    case SYS_SEEK:
      return 2;
    case SYS_TELL:
      return 1;
    case SYS_CLOSE:
      return 1;
    case SYS_MMAP:
      return 2;
    case SYS_MUNMAP:
      return 1;
    default:
      printf("Syscall number error: %d\n", syscall_num);
      return 0;
  }
}

/*
 * Call each system call handler checking stack pointer addresses. 
 */
static void
syscall_handler (struct intr_frame *f) 
{
  int syscall_num = *(uint32_t *)(f->esp);
  int args[3];
  get_syscall_arg(f->esp, args, get_arg_cnt(syscall_num));

  switch(syscall_num)
  {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      exit(args[0]);
      break;
    case SYS_EXEC:
       validate_string((void *)args[0], false);
       f->eax = exec((const char *)args[0]);
      break;
    case SYS_WAIT:
      f->eax = wait((pid_t)args[0]);
      break;
    case SYS_CREATE:
      f->eax = create((const char *)args[0], (unsigned)args[1]);
      break;
    case SYS_REMOVE:
      f->eax = remove((const char *)args[0]);
      break;
    case SYS_OPEN:
      f->eax = open((const char *)args[0]);
      break;
    case SYS_FILESIZE:
      f->eax = filesize(args[0]);
      break;
    case SYS_READ:
      f->eax = read(args[0], (void *)args[1], (unsigned)args[2]);
      break;
    case SYS_WRITE:
      f->eax = write(args[0], (const void *)args[1], (unsigned)args[2]);
      break;
    case SYS_SEEK:
      seek(args[0], (unsigned)args[1]);
      break;
    case SYS_TELL:
      f->eax = tell(args[0]);
      break;
    case SYS_CLOSE:
      close(args[0]);
      break;
    case SYS_MMAP:
      f->eax = mmap(args[0], (void *)args[1]);
      break;
    case SYS_MUNMAP:
      munmap(args[0]);
      break;
    default:
      break;
  }
}

int write(int fd, const void *buffer, unsigned size)
{
  validate_fd(fd);
  validate_buffer(buffer, size, false);
  int written_size = -1;
  struct thread *t = thread_current ();

  if (fd == 1) 
  {
    putbuf(buffer, size);
    written_size = size;
  }
  else if (fd >= 2)
  {
    struct file * file = process_fd_file_ptr(fd);
    if(file == NULL) exit(-1);

    lock_acquire(&file_system_lock);
    if (strcmp(t->name, file) == 0) file_deny_write(file);
    written_size = file_write(file, buffer, size);
    lock_release(&file_system_lock);
  }
  return written_size;
}

int read (int fd, void *buffer, unsigned size)
{
  validate_fd(fd);
  validate_buffer(buffer, size, true);
  int read_size = -1;
  if (fd == 0)
  {
    unsigned i;
    for (i = 0; i < size; i++)
    {
      ((char *)buffer)[i] = input_getc();
      if (((char *)buffer)[i] == '\0') break; 
    }
    read_size = i;
  }else if(fd >= 2)
  {
    struct file * file = process_fd_file_ptr(fd);
    if(file == NULL) exit(-1);

    lock_acquire(&file_system_lock);
    read_size = file_read(file, buffer, size);
    lock_release(&file_system_lock); 
  }
  return read_size;
}

bool create(const char *file, unsigned initial_size)
{
  if (file == NULL) exit(-1); 
  return filesys_create(file, initial_size);
}

bool remove (const char *file)
{
  if (file == NULL) exit(-1);
  return filesys_remove(file);
}

int open (const char *file)
{
  if (file == NULL) exit(-1);
  int fd = -1;
  struct thread *t = thread_current ();
  
  lock_acquire(&file_system_lock); 
  struct file * opened_file = filesys_open(file);
  if (opened_file != NULL)
  { 
    fd = process_fd_open(opened_file);
    if (strcmp(t->name, file) == 0) file_deny_write(opened_file);
  }
  lock_release(&file_system_lock);

  return fd;
}

int filesize (int fd)
{
  struct file * file = process_fd_file_ptr(fd);
  int size = file_length(file);

  return size;
}

void seek (int fd, unsigned position)
{
  struct file * file = process_fd_file_ptr(fd);
  file_seek(file, position);
}

unsigned tell (int fd)
{
  struct file * file = process_fd_file_ptr(fd);
  unsigned pos;
  pos = file_tell(file);

  return pos;
}

void close (int fd)
{
  struct thread *t = thread_current ();
  validate_fd(fd);
  if (t->fd_table[fd] == NULL) exit(-1);

  file_close(t->fd_table[fd]);
  t->fd_table[fd] = NULL;
}

mapid_t mmap(int fd, void *addr)
{
  struct thread *t = thread_current ();

  if (fd == 0 || fd == 1) return -1; // validate fd
  if (addr == NULL || !is_user_vaddr(addr) || pg_round_down(addr) != addr) return -1; // check addr is user address
  if (spt_page_find(&t->spt, addr)) return -1; //check if a page with addr already exists

  // lock_acquire(&file_system_lock);

  struct file *file_copy = file_reopen(process_fd_file_ptr(fd));
  if (file_copy == NULL) return -1;
  
  // Create page objects
  int file_len = file_length(file_copy);
  if (file_len == 0) return -1;

  struct fmm_file *fmm_file = (struct fmm_file*)malloc(sizeof(struct fmm_file));
  fmm_file->mapid = t->fmm_last_mapid++; 
  fmm_file->file = file_copy;
  list_push_back(&t->fmm_list, &fmm_file->elem);
  list_init(&fmm_file->p_list);

  size_t offset = 0;
  for(;file_len > 0; file_len -= PGSIZE)
  {
    struct page *p = (struct page*)malloc(sizeof(struct page));
    p->type = PAGE_FILE;
    p->addr = addr;
    p->write = true;
    p->loaded = false;
    p->file = file_copy;
    p->offset = offset;
    p->read_bytes = PGSIZE > file_len ? file_len : PGSIZE;
    p->zero_bytes = PGSIZE - p->read_bytes;
    spt_page_insert(&t->spt, p);
    list_push_back(&fmm_file->p_list, &p->fmm_elem);

    addr = (void*)((uintptr_t)addr + PGSIZE);
    offset += PGSIZE;
  }
  // lock_release(&file_system_lock);

  return fmm_file->mapid;
}

void munmap(mapid_t mapping)
{
  // If mapping is 0, free all page objects in fmm_list.  
  if (mapping == 0)
  {
    struct thread *t = thread_current ();
    struct list_elem *e;
    for (e = list_begin (&t->fmm_list); e != list_end (&t->fmm_list);)
    {
      struct fmm_file *f = list_entry (e, struct fmm_file, elem);
      munmap_all_pages(f);
      e = list_remove(e);
      file_close(f->file);
      free(f);
    }
  }
  else
  {
    struct fmm_file *fmm_file = find_fmm_by_mapid(mapping);
    if (fmm_file == NULL) return; //exit(-1);

    munmap_all_pages(fmm_file);
    list_remove(&fmm_file->elem);
    file_close(fmm_file->file);
    free(fmm_file);
  }
}

void munmap_all_pages(struct fmm_file *fmm_file)
{
  struct thread *t = thread_current ();
  struct list_elem *e;
 
  for (e = list_begin (&fmm_file->p_list); e != list_end (&fmm_file->p_list);)
  {
    struct page *p = list_entry (e, struct page, fmm_elem);
    if (p->loaded && pagedir_is_dirty(t->pagedir, p->addr))
    {
      lock_acquire(&file_system_lock);
      file_write_at(p->file, p->addr, p->read_bytes, p->offset);
      lock_release(&file_system_lock);
    }
    e = list_remove (e);
    spt_page_delete(&t->spt, p);
  }
}

struct fmm_file* find_fmm_by_mapid(mapid_t mapping)
{
  struct thread *t = thread_current ();

  struct fmm_file *f;
  struct list_elem *e;
  for (e = list_begin (&t->fmm_list); e != list_end (&t->fmm_list); e = list_next (e))
  {
    f = list_entry (e, struct fmm_file, elem);
    if (f->mapid == mapping) return f;
  }

  return NULL;
}

