#include "gsoc_taskqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Single queue management

gsoc_taskqueue* gsoc_taskqueue_new()
{
  gsoc_taskqueue* this;

  this = malloc(sizeof(gsoc_taskqueue));
  assert(this);

  this->_taskqueue = malloc(sizeof(gsoc_task_circular_array));
  assert(this->_taskqueue); 

  this->_taskqueue->_size = 50;
  this->_top = 0;
  this->_bottom = 1;

  return this;
}

void gsoc_taskqueue_push(gsoc_taskqueue* this, gsoc_task task)
{
  gsoc_task_circular_array_set(this->_taskqueue, this->_bottom, task);
  ++this->_bottom;
  
  __sync_synchronize();
}

gsoc_task gsoc_taskqueue_pop(gsoc_taskqueue* this)
{
  size_t old_top, new_top;
  size_t num_tasks;

  gsoc_task dummy_task;
  dummy_task.priority = -1; 

  --this->_bottom;
  __sync_synchronize();
  old_top = this->_top;
  new_top = old_top + 1;
  num_tasks = this->_bottom - old_top;

  if (__builtin_expect(num_tasks < 0, 0)) {
    this->_bottom = old_top;
    return dummy_task;
  } else if (__builtin_expect(num_tasks == 0, 0)) {
    gsoc_task ret = gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);
    __sync_synchronize();
    if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
      return dummy_task;
    else {
      this->_bottom = new_top;
      __sync_synchronize();
      return ret;
    }
  } else {
    return gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);
  }
}

gsoc_task gsoc_taskqueue_take(gsoc_taskqueue* this)
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

  if (__builtin_expect(num_tasks <= 0, 0))
    return dummy_task;

  __sync_synchronize();
  if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
    return dummy_task;
  else
    return gsoc_task_circular_array_get(this->_taskqueue, old_top);
}

// Taskqueue set: 3-priority deques per processor

gsoc_taskqueue_set* gsoc_taskqueue_set_new() {
  gsoc_taskqueue_set* set = malloc(sizeof(gsoc_taskqueue_set));
  assert(set);
  for (int i = 0; i < PRIORITY_LEVELS; i++) {
    set->queues[i] = gsoc_taskqueue_new();
  }
  return set;
}

void gsoc_taskqueue_set_push(gsoc_taskqueue_set* set, gsoc_task task, int priority) {
  assert(priority >= 0 && priority < PRIORITY_LEVELS);
  gsoc_taskqueue_push(set->queues[priority], task);
}

gsoc_task gsoc_taskqueue_set_pop(gsoc_taskqueue_set* set, int priority) {
  assert(priority >= 0 && priority < PRIORITY_LEVELS);
  return gsoc_taskqueue_pop(set->queues[priority]);
}

// Steal the best available task by trying from high to low priority

gsoc_task gsoc_taskqueue_set_steal_best(gsoc_taskqueue_set* victim_set, int priority_level) {
    gsoc_task task = gsoc_taskqueue_take(victim_set->queues[priority_level]);
    return task;
}

