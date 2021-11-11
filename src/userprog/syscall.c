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

static void syscall_handler (struct intr_frame *);
int get_arg_cnt(int);

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
  for (i = 2; i < 128; i++)
  {
    struct file * file = process_fd_file_ptr(i); 
    if(file != NULL) 
    {
      file_allow_write(file);
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

void validate_user_pointer(void *pointer)
{
  if (pointer == NULL && !(pointer < PHYS_BASE && pointer > (void *)0x08084000)) // >= ?
  {
    exit(-1);
  }
}

void validate_fd(int fd)
{
  if (!(fd >= 0 && fd <= 127))
  {
    exit(-1);
  }
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
  // printf ("system call!\n");
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
       validate_user_pointer((void *)args[0]);
       f->eax = exec((const char *)args[0]);
      break;
    case SYS_WAIT:
      f->eax = wait((pid_t)args[0]);
      break;
    case SYS_CREATE:
      validate_user_pointer((void *)args[0]);
      f->eax = create((const char *)args[0], (unsigned)args[1]);
      break;
    case SYS_REMOVE:
      validate_user_pointer((void *)args[0]); 
      f->eax = remove((const char *)args[0]);
      break;
    case SYS_OPEN:
      validate_user_pointer((void *)args[0]); 
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
    default:
      break;
  }
}

int write(int fd, const void *buffer, unsigned size)
{
  validate_fd(fd);
  int written_size = -1;
  if (fd == 1) 
  {
    putbuf(buffer, size);
    written_size = size;
  }else if (fd >= 2)
  {
    struct file * file = process_fd_file_ptr(fd);
    validate_user_pointer((void *)file);

    lock_acquire(&file_system_lock);
    written_size = file_write(file, buffer, size);
    lock_release(&file_system_lock);
  }
  return written_size;
}

int read (int fd, void *buffer, unsigned size)
{
  validate_fd(fd);
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
    validate_user_pointer((void *)file);

    lock_acquire(&file_system_lock);
    read_size = file_read(file, buffer, size);
    lock_release(&file_system_lock); 
  }
  return read_size;
}

bool create(const char *file, unsigned initial_size)
{
  validate_user_pointer((void *)file);
  return filesys_create(file, initial_size);
}

bool remove (const char *file)
{
  validate_user_pointer((void *)file);
  return filesys_remove(file);
}

int open (const char *file)
{
  validate_user_pointer((void *)file);
  int fd = -1;
  
  lock_acquire(&file_system_lock); 
  struct file * opened_file = filesys_open(file);
  if (opened_file != NULL)
  { 
    fd = process_fd_open(opened_file);
  }
  lock_release(&file_system_lock);

  return fd;
}

int filesize (int fd)
{
  struct file * file = process_fd_file_ptr(fd);
  // if (file == NULL) return -1;// Not necessary? ASSERT error in file_length or just return -1. Which one is better?

  lock_acquire(&file_system_lock); 
  int size = file_length(file);
  lock_release(&file_system_lock);

  return size;
}

void seek (int fd, unsigned position)
{
  struct file * file = process_fd_file_ptr(fd);
  
  lock_acquire(&file_system_lock); 
  file_seek(file, position);
  lock_release(&file_system_lock); 
}

unsigned tell (int fd)
{
  struct file * file = process_fd_file_ptr(fd);
  unsigned pos;
  lock_acquire(&file_system_lock); 
  pos = file_tell(file);
  lock_release(&file_system_lock); 

  return pos;
}

void close (int fd)
{
  struct thread *t = thread_current ();
  validate_fd(fd);
  validate_user_pointer((void *)t->fd_table[fd]);

  lock_acquire(&file_system_lock); 
  file_close(t->fd_table[fd]);
  t->fd_table[fd] = NULL;
  lock_release(&file_system_lock);
}
  
