#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#include "common.h"

#define NR_WP 32

typedef struct Watchpoint {
  int NO;
  struct Watchpoint *next;
  /* TODO: Add more members if necessary */
    char *expr;
    word_t val;
} WP;

extern WP wp_pool[NR_WP];
extern WP *head;
extern WP * free_;

void init_wp_pool();
WP* new_wp(char *str);
void free_wp(WP* wp);

#endif