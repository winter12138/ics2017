#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	if(NULL == free_) {
		return NULL;
	}
	WP *wp = free_;
	free_ = free_->next;
	wp->next = head;
	head = wp;
	return wp;
}

void free_wp(WP *wp) {
	WP *p;
	if(!wp) {
		return;
	}
	if(head == wp) {
		head = head->next;
	} else {
		p=head; 
		while(p && p->next!=wp) p=p->next;
		if(p) {
			p->next = wp->next;
		} else {
			printf("Error pointer\n");
			return;
		}
	}
	
	wp->next = free_;
	free_ = wp;
}

void check_wps(void) {
	WP *wp;
	bool success;
	uint32_t new_value;
	for(wp=head; wp; wp=wp->next) {
		new_value = expr(wp->str, &success);
		if(false == success) {
			printf("Bad expression\n");
		} else if(new_value != wp->value){
			nemu_state = NEMU_STOP;
			printf("Watchpoint %d: %s\n", wp->NO, wp->str);
			printf("Old value = %-#10x %-10d\n", wp->value, wp->value);
			printf("New value = %-#10x %-10d\n\n", new_value, new_value);
			wp->value = new_value;
		}
	}
}

int delete_wp(int no) {
	WP *wp;
	for(wp=head; wp; wp=wp->next) {
		if(no == wp->NO) {
			free_wp(wp);
			return true;
		}
	}
	return false;
}

void list_wps(void) {
	WP *wp;
	printf("%-10s %-10s\n", "Num", "What");
	for(wp=head; wp; wp=wp->next) {
		printf("%-10d %-10s\n", wp->NO, wp->str);
	}
}