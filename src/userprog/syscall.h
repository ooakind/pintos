#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

void syscall_init (void);
void exit(int status);
pid_t exec (const char *cmd_line);
void halt(void);
void validate_user_pointer(void *pointer);
void get_syscall_arg(void *sp, int *arg, int arg_cnt);

#endif /* userprog/syscall.h */
