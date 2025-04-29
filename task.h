// This file defines the task structure and its associated priority levels.

typedef enum
{
  PRIORITY_HIGH = 0,
  PRIORITY_MEDIUM = 1,
  PRIORITY_LOW = 2
} task_priority_t;

// The task structure contains a priority level, an input value, and a test ID.
typedef struct task
{
  task_priority_t priority;
  int input;
  unsigned long long test_id;
} task;
