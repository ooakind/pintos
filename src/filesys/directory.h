#ifndef FILESYS_DIRECTORY_H
#define FILESYS_DIRECTORY_H

#include <stdbool.h>
#include <stddef.h>
#include "devices/disk.h"

/* Maximum length of a filename.
   This is the traditional UNIX maximum.
   (This macro name comes from POSIX.1.) */
#define NAME_MAX 14

struct file;
struct dir *dir_create (size_t entry_cnt);
size_t dir_size (size_t entry_cnt);
void dir_destroy (struct dir *);
void dir_read (struct dir *, struct file *);
void dir_write (struct dir *, struct file *);
bool dir_lookup (const struct dir *, const char *name, disk_sector_t *);
bool dir_add (struct dir *, const char *name, disk_sector_t);
bool dir_remove (struct dir *, const char *name);
void dir_list (const struct dir *);
void dir_dump (const struct dir *);

#endif /* filesys/directory.h */
