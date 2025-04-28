#define _GNU_SOURCE

#include "task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
// Priorities: 0 = high, 1 = medium, 2 = low
#define PRIORITY_LEVELS 3

typedef struct taskqueue
{
  task _array[3000];
  unsigned long long _size;
  volatile size_t _top;    // where stealing starts from
  volatile size_t _bottom; // where pushing occurs
} taskqueue;

// A triple-priority deque per processor
typedef struct taskqueue_set
{
  taskqueue queues[PRIORITY_LEVELS]; // 0: high, 1: medium, 2: low
} taskqueue_set;

// Task operations on a single queue
void taskqueue_push(taskqueue *this, task task);
task taskqueue_pop(taskqueue *this);
task taskqueue_take(taskqueue *this);

// Constructor/destructor for the 3-deque structure
taskqueue_set *taskqueue_set_new();
void taskqueue_set_delete(taskqueue_set *set);

// Push/pop by priority
void taskqueue_set_push(taskqueue_set *set, task task);
task taskqueue_set_pop(taskqueue_set *set, int priority);

// Stealing strategy to pick best task among priorities
task taskqueue_set_steal_best(taskqueue_set *victim_set, int priority_level, int cpu);
