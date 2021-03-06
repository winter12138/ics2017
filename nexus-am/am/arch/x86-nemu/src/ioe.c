#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64

static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long now;
  now = inl(RTC_PORT);
  return now - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i,j;
  for(i = 0; i < w; ++i) {
  	for(j = 0; j < h; ++j) {
  		fb[ (y+j) * _screen.width + x + i] = *pixels;
  	}
  }
}

void _draw_sync() {
}

int _read_key() {
  uint8_t status;
  uint32_t data;
  status = inb(I8042_STATUS_PORT);
  if(status) {
  	data = inl(I8042_DATA_PORT);
  	return data;
  }
  return _KEY_NONE;
}
