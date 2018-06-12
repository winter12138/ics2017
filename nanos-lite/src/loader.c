#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
    Log("%s\n", filename);
    int fd;
    size_t len;
    fd = fs_open(filename, 0, 0);
    len = fs_filesz(fd);
    Log("%s, %d\n", filename, len);
    
    fs_read(fd, DEFAULT_ENTRY, len);
    fs_close(fd);

    return (uintptr_t)DEFAULT_ENTRY;
}
