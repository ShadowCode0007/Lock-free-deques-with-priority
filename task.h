// Task priorities
typedef enum
{
  PRIORITY_HIGH = 0,
  PRIORITY_MEDIUM = 1,
  PRIORITY_LOW = 2
} task_priority_t;

typedef struct task
{
  task_priority_t priority;
  int input;
  unsigned long long test_id;
} task;
