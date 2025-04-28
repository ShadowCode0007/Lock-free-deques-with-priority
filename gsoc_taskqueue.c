#include "gsoc_taskqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM_TASKS 150

void gsoc_taskqueue_push(gsoc_taskqueue *this, gsoc_task task)
{
  size_t b = this->_bottom;
  this->_array[b % NUM_TASKS] = task;
  this->_bottom = b + 1;
  __sync_synchronize();
}

// Pop a task from the queue (used by owner thread)
gsoc_task gsoc_taskqueue_pop(gsoc_taskqueue* this) {
  gsoc_task dummy_task;
  dummy_task.priority = -1;

  size_t b = this->_bottom;
  if (b == 0) {
    // Queue is empty
    return dummy_task;
  }

  b = b - 1;
  this->_bottom = b;
  __sync_synchronize(); // Memory barrier after updating bottom

  size_t t = this->_top;

  if (t <= b) {
    // There is at least one item
    gsoc_task ret = this->_array[b % NUM_TASKS];

    if (t == b) {
      // Last item, need to use CAS
      if (!__sync_bool_compare_and_swap(&this->_top, t, t + 1)) {
        // Lost race with a thief
        ret = dummy_task;
      }
      this->_bottom = t + 1; // Reset bottom
    }
    return ret;
  } else {
    // Queue became empty
    this->_bottom = t;
    return dummy_task;
  }
}

gsoc_task gsoc_taskqueue_take(gsoc_taskqueue *this)
{
  size_t old_top, new_top;
  size_t old_bottom;
  size_t num_tasks;
  __sync_synchronize();

  old_top = this->_top;
  old_bottom = this->_bottom;
  new_top = old_top + 1;
  num_tasks = old_bottom - old_top;

  gsoc_task dummy_task;
  dummy_task.priority = -1;

  if (__builtin_expect(num_tasks <= 0, 0)) {
    printf("CPU %d Empty num_tasks is %ld \n", sched_getcpu(), num_tasks);
    return dummy_task;
  }

  __sync_synchronize();

  if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top)) {
    printf("CPU %d Compare and swap contention \n", sched_getcpu());
    return dummy_task;
  }
  else
  {
    return this->_array[old_top % NUM_TASKS];
  }
}

// Taskqueue set: 3-priority deques per processor

gsoc_taskqueue_set *gsoc_taskqueue_set_new()
{
  gsoc_taskqueue_set *set = malloc(sizeof(gsoc_taskqueue_set));
  assert(set);
 
  for (int i = 0; i < PRIORITY_LEVELS; i++) {
    set->queues[i]._top = 0;
    set->queues[i]._bottom = 0;
  }

  return set;
}

void gsoc_taskqueue_set_push(gsoc_taskqueue_set *set, gsoc_task task)
{
  assert(task.priority >= 0);
  assert(task.priority < PRIORITY_LEVELS);
  gsoc_taskqueue_push(&set->queues[task.priority], task);
}

gsoc_task gsoc_taskqueue_set_pop(gsoc_taskqueue_set *set, int priority)
{
  assert(priority >= 0 && priority < PRIORITY_LEVELS);
  return gsoc_taskqueue_pop(&set->queues[priority]);
}

// Steal the best available task by trying from high to low priority

gsoc_task gsoc_taskqueue_set_steal_best(gsoc_taskqueue_set *victim_set, int priority_level, int cpu)
{
  printf("CPU of queue is %d\n", cpu); 
  gsoc_task task = gsoc_taskqueue_take(&victim_set->queues[priority_level]);
  return task;
}
