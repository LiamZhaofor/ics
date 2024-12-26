#define NR_WP 32;

struct {
  int NO;
  struct Watchpoint *next;
  /* TODO: Add more members if necessary */
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

