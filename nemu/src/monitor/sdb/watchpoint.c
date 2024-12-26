/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expr;
  int var_Value;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp(char *expr)
{
  if (free_ == NULL)
  {
    printf("NO enough watch point to use\n");
    assert(0);
  }
  WP *node = NULL;
  if (head == NULL)
  {
    node = free_;
    free_ = free_->next;
    node->next = NULL;
    head = node;
  }
  else
  {
    WP *cur = head;
    while (cur->next)
    {
      cur = cur->next;
    }
    node = free_;
    free_ = free_->next;
    cur->next = node;
    node->next = NULL;
    node->expr = expr;
  }
  return node;
}
void free_wp(WP *wp)
{
  WP *dummyHead = (WP *)malloc(sizeof(WP));
  dummyHead->next = head;
  WP *cur = dummyHead;
  while (cur->next != NULL)
  {
    if (cur->next == wp)
    {
      WP *tmp = cur->next;
      cur->next = cur->next->next;
      free(tmp);
    }
    else
    {
      cur = cur->next;
    }
  }
  head = dummyHead->next;
  free(dummyHead);
}
