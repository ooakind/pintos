#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include <string.h>
#include "userprog/process.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
int write(int fd, const void *buffer, unsigned size);
int get_arg_cnt(int);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(void)
{
  shutdown_power_off ();
}

void exit(int status)
{
  thread_current()->exit_status = status;
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
  if (!(pointer < PHYS_BASE && pointer > (void *)0x08084000)) // >= ?
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
    /**
     * validate_user_pointer((void *)args[0]);
     * f->eax = create((const char *)args[0], (unsigned)args[1]);
     */
      break;
    case SYS_REMOVE:
    /**
     * validate_user_pointer((void *)args[0]); 
     * f->eax = remove((const char *)args[0]);
     */
      break;
    case SYS_OPEN:
      /**
       * validate_user_pointer((void *)args[0]); 
       * f->eax = open((const char *)args[0]);
       */
      break;
    case SYS_FILESIZE:
      //f->eax = filesize(args[0]);
      break;
    case SYS_READ:
      //f->eax = read(args[0], (void *)args[1], (unsigned)args[2]);
      break;
    case SYS_WRITE:
      f->eax = write(args[0], (const void *)args[1], (unsigned)args[2]);
      break;
    case SYS_SEEK:
      //seek(args[0], (unsigned)args[1]);
      break;
    case SYS_TELL:
      //f->eax = tell(args[0]);
      break;
    case SYS_CLOSE:
      //close(args[0]);
      break;
    default:
      break;
  }
  thread_exit ();
}

int write(int fd, const void *buffer, unsigned size)
{
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}