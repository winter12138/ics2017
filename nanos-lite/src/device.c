#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};


#define KEYDOWN_MASK 0x8000
size_t events_read(void *buf, size_t len) {
	int key;
	key = _read_key();
  if(key == _KEY_NONE) {
  	sprintf(buf, "t %uld\n", _uptime());
  }else if((key & KEYDOWN_MASK) != 0){
    key &= ~KEYDOWN_MASK;
    sprintf(buf, "kd %s\n", keyname[key]);
  } else {
    sprintf(buf, "ku %s\n", keyname[key]);
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
	uint32_t *pixel = (uint32_t*) buf;
	int i, x, y;
	for (i = 0; i < len/4; ++i)
	{
		x = (offset/4 + i) % _screen.width;
		y = (offset/4 - 1 +i) / _screen.width;
		_draw_rect(pixel + i, x, y, 1, 1);
	}
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
