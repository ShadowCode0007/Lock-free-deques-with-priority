#ifndef _GSOC_TASK_H_
#define _GSOC_TASK_H_

// Task priorities
typedef enum {
  PRIORITY_HIGH = 0,
  PRIORITY_MEDIUM = 1,
  PRIORITY_LOW = 2
} task_priority_t;

typedef struct gsoc_task {
  task_priority_t priority;  
  int task_durartion;
#ifdef TEST_USE_TASK_ID
  unsigned long long test_id;
#endif
} gsoc_task;

#endif /* _GSOC_TASK_H_ */

