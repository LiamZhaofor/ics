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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <stdlib.h>

#include <limits.h>

#include <common.h>

enum
{
  TK_NOTYPE = 256,
  NUM = 1,
  RESGISTER = 2,
  HEX = 3,
  EQ = 4,
  NOTEQ = 5,
  OR = 6,
  AND = 7,
  ZUO = 8,
  YOU = 9,
  LEQ = 10,
  YINYONG = 11,
  NEG,
  DEREF = 12

  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus
    {"\\-", '-'},      // sub
    {"\\*", '*'},      // mul
    {"\\/", '/'},      // div

    {"\\(", '('},
    {"\\)", ')'},

    {"\\<\\=", LEQ}, //
    {"\\=\\=", EQ},  // equal
    {"\\!\\=", NOTEQ},

    {"\\|\\|", OR}, // Opetor
    {"\\&\\&", AND},
    {"\\!", '!'},

    {"^(\\$0|ra|sp|gp|tp|t[0-6]|s[0-9]|s10|s11|a[0-7])$", RESGISTER},
    {"0[xX][0-9a-fA-F]+", HEX},
    {"^[0-9]+", NUM},

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* Record the token in the array */
        switch (rules[i].token_type)
        {
        case NUM:
          tokens[nr_token].type = NUM;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        case RESGISTER:
          tokens[nr_token].type = RESGISTER;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0'; // Ensure null-termination
          bool success = false;
          isa_reg_str2val(tokens[nr_token].str, &success);
          if (!success)
          {
            printf("Invalid register: %s\n", tokens[nr_token].str);
            return false;
          }
          nr_token++;
          break;

        case HEX:
          tokens[nr_token].type = HEX;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0'; // Ensure null-termination
          word_t decvalue = strtol(tokens[nr_token].str, NULL, 16);
          sprintf(tokens[nr_token].str, "%d", decvalue);
          nr_token++;
          break;

        case EQ:
          tokens[nr_token].type = EQ;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        case NOTEQ:
          tokens[nr_token].type = NOTEQ;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        case OR:
          tokens[nr_token].type = OR;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;
        case AND:
          tokens[nr_token].type = AND;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        case LEQ:
          tokens[nr_token].type = LEQ;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        case NEG:
          tokens[nr_token].type = NEG;
          tokens[nr_token].str[0] = '-';
          tokens[nr_token].str[1] = '\0';
          nr_token++;
          break;

        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case '!':
          tokens[nr_token].type = rules[i].token_type;
          tokens[nr_token].str[0] = substr_start[0];
          tokens[nr_token].str[1] = '\0'; // Ensure null-termination
          nr_token++;
          break;
        case TK_NOTYPE:
          break;
        default:
          printf("i = %d and no rule.\n", i);
          break;
        }
        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q)
{
  if (tokens[p].type != '(' || tokens[q].type != ')')
  {
    return false;
  }

  int balance = 0;
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
      balance++;
    if (tokens[i].type == ')')
      balance--;
    if (balance < 0)
      return false; // 检测不匹配
  }

  return balance == 0; // 确保括号最终匹配
}

word_t findMainop(int p, int q)
{

  word_t main_op_pos = -1;       // 记录主运算符的位置
  word_t min_priority = INT_MAX; // 当前最小优先级
  word_t par_level = 0;          // 当前括号嵌套深度

  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
    {
      par_level++; // 遇到左括号增加嵌套深度
    }
    else if (tokens[i].type == ')')
    {
      par_level--; // 遇到右括号减少嵌套深度
    }
    else if (par_level == 0)
    {
      // 只有在不在括号中的情况下才判断运算符
      int priority;
      if (tokens[i].type == '+' || tokens[i].type == '-')
      {
        priority = 10;
      }
      else if (tokens[i].type == '*' || tokens[i].type == '/')
      {
        priority = 20;
      }
      else if (tokens[i].type == DEREF)
      {
        priority = 30;
      }
      else if (tokens[i].type == AND)
      {
        priority = 5;
      }
      else if (tokens[i].type == OR)
      {
        priority = 4;
      }
      else
      {
        continue; // 跳过非运算符
      }

      // 根据优先级和结合性判断是否更新主运算符
      if (priority < min_priority || (priority == min_priority && i > main_op_pos))
      {
        min_priority = priority;
        main_op_pos = i;
      }
    }
  }

  printf("Main operator found at position: %d (%s)\n", main_op_pos, tokens[main_op_pos].str);

  return main_op_pos;
}

/* TODO: Insert codes to evaluate the expression. */
// the subexpr index
word_t eval(int p, int q)
{

  if (p > q)
  {
    /* Bad expression */
    assert(0);
    return -1;
  }
  else if (p == q)
  {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    switch (tokens[p].type)
    {
    case NUM:
      return atoi(tokens[p].str);
    case HEX:
      return strtol(tokens[p].str, NULL, 16);
    case RESGISTER:
      // 获取寄存器值
      bool success = false;
      word_t val = isa_reg_str2val(tokens[p].str, &success); // 跳过$前缀
      if (!success)
      {
        printf("Invalid register: %s\n", tokens[p].str);
        assert(0);
      }
      return val;
    default:
      assert(0);
    }
  }
  else if (check_parentheses(p, q) == true)
  {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */

    return eval(p + 1, q - 1);
  }
  else
  {
    /* We should do more things here. */
    word_t op = findMainop(p, q);
    if (op == -1)
    {
      printf("find failed\n");
      return -1;
    }
    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);
    printf("Evaluating: p = %d, q = %d\n", p, q);

    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '/':
      if (val2 == 0)
      {
        printf("div zero\n");
        assert(0);
      }
      return val1 / val2;
    case '*':
      return val1 * val2;
    case EQ:
      return val1 == val2;
    case NOTEQ:
      return val1 != val2;
    case DEREF:
      // assert((uintptr_t)val2 != 0);
      uintptr_t tmp = val2;
      return *(word_t *)tmp;
    case OR:
      return val1 || val2;
    case AND:
      return val1 && val2;
    default:
      assert(0);
    }
  }
}

word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    printf("token unsuccess\n");
    return 0;
  }

  for (int i = 0; i < nr_token; i++)
  { // use for debug

    printf("%s \n", tokens[i].str);
    printf("\n");
  }

  for (int i = 0; i < nr_token; i++)
  {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-' || tokens[i - 1].type == '*' || tokens[i - 1].type == '/' || tokens[i - 1].type == '('))
    {
      tokens[i].type = DEREF;
    }
  }
  return eval(0, nr_token - 1);
}