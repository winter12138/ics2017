#include "fs.h"
#include "common.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0},
  {"stdout", 0, 0},
  {"stderr", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int strcmp(const char *s, const char *t);

int str_cmp(const char *s, const char *t)
{
  while(*s && *t && *s++ == *t++);
  return *s - *t;
}

int fs_open(const char *pathname, int flags, int mode)
{
  int i;
  Finfo *fp;

  for (i = 0; i < NR_FILES; ++i)
  {
    fp = &file_table[i];
    if(0 == strcmp(pathname, fp->name)){
      fp->open_offset = 0;
      return i;
    }
  }
  assert(i < NR_FILES);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len)
{
  Finfo *fp = &file_table[fd];
  assert(fp->open_offset + len <= fp->size);
  ramdisk_read(buf, fp->disk_offset + fp->open_offset, len);
  fp->open_offset += len;
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len)
{
  char ch;
  size_t i;
  Finfo *fp = &file_table[fd];

  if(1 == fd || 2 == fd){
    for (i = 0; i < len; ++i)
    {
      ch = *(char*)(buf + i);
      _putc(ch);
    }
  } else {
    assert(fp->open_offset + len <= fp->size);
    ramdisk_write(buf, fp->disk_offset + fp->open_offset, len);
    fp->open_offset += len;
  }

  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence)
{
  Finfo *fp = &file_table[fd];
  switch(whence){
    case SEEK_SET: {
      assert(offset <= fp->size);
      fp->open_offset = offset;
      break;
    }
    case SEEK_CUR: {
      assert(fp->open_offset + offset <= fp->size);
      fp->open_offset += offset;
      break;
    }
    case SEEK_END: {
      assert(offset <= fp->size);
      fp->open_offset = fp->size - offset;
      break;
    }
  }
  return fp->open_offset;
}

int fs_close(int fd)
{
  return 0;
}

size_t fs_filesz(int fd)
{
  Finfo *fp = &file_table[fd];
  return fp->size;
}