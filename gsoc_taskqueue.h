/* Implemented after Chapter 16 of "The Art of Multiprocessor Programming"
   This is lock-free work stealing deque. */
#ifndef _GSOC_TASKQUEUE_H_
#define _GSOC_TASKQUEUE_H_

#include "gsoc_task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define GSOC_TASKQUEUE_INIT_SIZE 1

// #define GSOC_TASKQUEUE_INIT_SIZE 13172

// Priorities: 0 = high, 1 = medium, 2 = low
#define PRIORITY_LEVELS 3

typedef struct _gsoc_taskqueue
{
  // gsoc_task_circular_array* _taskqueue;       // array of gsoc_task*
  gsoc_task _array[30];
  unsigned long long _size;
  volatile size_t _top;    // where stealing starts from
  volatile size_t _bottom; // where pushing occurs
} gsoc_taskqueue;

// A triple-priority deque per processor
typedef struct _gsoc_taskqueue_set
{
  gsoc_taskqueue queues[PRIORITY_LEVELS]; // 0: high, 1: medium, 2: low
} gsoc_taskqueue_set;

// Task operations on a single queue
void gsoc_taskqueue_push(gsoc_taskqueue *this, gsoc_task task);
gsoc_task gsoc_taskqueue_pop(gsoc_taskqueue *this);
gsoc_task gsoc_taskqueue_take(gsoc_taskqueue *this);

// Constructor/destructor for the 3-deque structure
gsoc_taskqueue_set *gsoc_taskqueue_set_new();
void gsoc_taskqueue_set_delete(gsoc_taskqueue_set *set);

// Push/pop by priority
void gsoc_taskqueue_set_push(gsoc_taskqueue_set *set, gsoc_task task);
gsoc_task gsoc_taskqueue_set_pop(gsoc_taskqueue_set *set, int priority);

// Stealing strategy to pick best task among priorities
gsoc_task gsoc_taskqueue_set_steal_best(gsoc_taskqueue_set *victim_set, int priority_level);

#endif /* _GSOC_TASKQUEUE_H_ */
