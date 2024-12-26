#define NR_WP 32;

typedef struct Watchpoint {
  int NO;
  struct Watchpoint *next;
  /* TODO: Add more members if necessary */
    char *expr;
    word_t val;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool();
void new_wp(char *str);
void free_wp();

