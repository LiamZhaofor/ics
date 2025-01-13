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

#include "watchpoint.h"


 WP wp_pool[NR_WP] = {};
 WP *head = NULL, *free_ = NULL;
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
WP *new_wp(char *str) {
    if (free_ == NULL) {
        printf("No enough watchpoints to use.\n");
        assert(0); // 如果没有可用观察点，终止程序
    }

    // 从空闲链表中取出一个节点
    WP *node = free_;
    free_ = free_->next;

    // 初始化节点
    node->expr = strdup(str); // 复制字符串，避免悬空指针
    if (node->expr == NULL) {
        printf("Failed to allocate memory for expression.\n");
        assert(0);
    }

    bool success = true;
    node->val = expr(node->expr, &success);
    if (!success) {
        printf("Failed to evaluate expression: %s\n", node->expr);
        free(node->expr); // 释放内存
        node->expr = NULL;
        return NULL;
    }

    node->next = NULL;

    // 将节点添加到使用中的链表
    if (head == NULL) {
        head = node;
    } else {
        WP *cur = head;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = node;
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
