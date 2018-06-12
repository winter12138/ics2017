#include "common.h"
#include "syscall.h"

uintptr_t sys_none()
{
  return 1;
}

ssize_t sys_write(int fd, const void *buf, size_t count)
{
  char ch;
  size_t i;
  if(1 == fd || 2 == fd){
    for (i = 0; i < count; ++i)
    {
      ch = *(char*)(buf + i);
      _putc(ch);
    }
  }
  Log("1");
  return count;
}

int sys_brk(void *addr)
{
  return 0;
}

void sys_exit(int code)
{
  _halt(code);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none: {
      SYSCALL_ARG1(r) = sys_none();
      break;
    }
    case SYS_write: {
      SYSCALL_ARG1(r) = sys_write(a[1], (void*)a[2], a[3]);
      break;
    }
    case SYS_brk: {
      SYSCALL_ARG1(r) = sys_brk((void*)a[1]);
      break;
    }
  	case SYS_exit: {
  		sys_exit(a[1]);
  		break;
  	}
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}