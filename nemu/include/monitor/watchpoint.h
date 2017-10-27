#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char str[256];
  uint32_t value;

} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);
void check_wps(void);
int delete_wp(int no);
void list_wps(void);

#endif
