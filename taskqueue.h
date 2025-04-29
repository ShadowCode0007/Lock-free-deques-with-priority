#define _GNU_SOURCE

#include "task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

#define PRIORITY_LEVELS 3
#define TASKQUEUE_SIZE 3000

typedef struct taskqueue
{
  task _array[TASKQUEUE_SIZE];
  volatile size_t _top;    // stealing occurs
  volatile size_t _bottom; // pushing/popping occurs
} taskqueue;

// A triple-priority deque per processor
typedef struct taskqueue_set
{
  taskqueue queues[PRIORITY_LEVELS];
} taskqueue_set;

// Task operations on a single queue
void push(taskqueue *this, task task);
task pop(taskqueue *this);
task take(taskqueue *this);

// Constructor/destructor for the 3-deque structure
taskqueue_set *set_new();
void set_delete(taskqueue_set *set);

// Push/pop by priority
void set_push(taskqueue_set *set, task task);
task set_pop(taskqueue_set *set, int priority);

// Stealing strategy to pick best task among priorities
task set_steal_best(taskqueue_set *victim_set, int priority_level, int cpu);
