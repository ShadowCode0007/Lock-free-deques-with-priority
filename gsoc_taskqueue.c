#include "gsoc_taskqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM_TASKS 30

void gsoc_taskqueue_push(gsoc_taskqueue *this, gsoc_task task)
{
  size_t b = this->_bottom;
  this->_array[b % NUM_TASKS] = task;
  // this->_array[b] = task;
  this->_bottom = b + 1;
  __sync_synchronize();
}

// gsoc_task gsoc_taskqueue_pop(gsoc_taskqueue* this)
// {
//   size_t old_top, new_top;
//   size_t num_tasks;

//   gsoc_task dummy_task;
//   dummy_task.priority = -1;

//   --this->_bottom;
//   __sync_synchronize();
//   old_top = this->_top;
//   new_top = old_top + 1;
//   num_tasks = this->_bottom - old_top;

//   if (__builtin_expect(num_tasks < 0, 0)) {
//     this->_bottom = old_top;
//     return dummy_task;
//   } else if (__builtin_expect(num_tasks == 0, 0)) {
//     // gsoc_task ret = gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);
//     gsoc_task ret = this->_array[this->_bottom];
//     __sync_synchronize();
//     if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
//       return dummy_task;
//     else {
//       this->_bottom = new_top;
//       __sync_synchronize();
//       return ret;
//     }
//   } else {
//     // return gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);
//     return this->_array[this->_bottom];
//   }
// }

gsoc_task gsoc_taskqueue_pop(gsoc_taskqueue *this)
{
  gsoc_task dummy_task;
  dummy_task.priority = -1;

  size_t b = this->_bottom;
  if (b == 0)
  {
    // Queue is empty
    return dummy_task;
  }

  b = b - 1;
  this->_bottom = b;
  __sync_synchronize();

  size_t t = this->_top;
  gsoc_task ret;

  if (t <= b)
  {
    // There is at least one item
    ret = this->_array[b % NUM_TASKS];
    // ret = this->_array[b];
    // No need for CAS, only owner can pop here
    return ret;
  }
  else
  {
    // Possibly last item
    this->_bottom = 0;
    if (t != b)
    {
      // Queue is empty
      return dummy_task;
    }
    // Try to claim the last item
    if (!__sync_bool_compare_and_swap(&this->_top, t, t + 1))
    {
      // Lost race with a thief
      return dummy_task;
    }
    // Successfully claimed last item
    ret = this->_array[b % NUM_TASKS];
    // ret = this->_array[b];
    return ret;
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

  if (__builtin_expect(num_tasks <= 0, 0))
  {
    printf("gsoc_taskqueue_take: no tasks available\n");
    return dummy_task;
  }

  __sync_synchronize();

  if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
  {
    printf("gsoc_taskqueue_take: CAS failed\n");
    return dummy_task;
  }
  else
  {
    return this->_array[old_top % NUM_TASKS];
    // return this->_array[old_top];
  }
}

// Taskqueue set: 3-priority deques per processor

gsoc_taskqueue_set *gsoc_taskqueue_set_new()
{
  gsoc_taskqueue_set *set = malloc(sizeof(gsoc_taskqueue_set));
  assert(set);
  // for (int i = 0; i < PRIORITY_LEVELS; i++) {
  //   set->queues[i] = gsoc_taskqueue_new();
  // }
  return set;
}

void gsoc_taskqueue_set_push(gsoc_taskqueue_set *set, gsoc_task task)
{
  assert(task.priority >= 0);
  // printf("in gsoc_taskqueue_set_push, priority = %d\n", task.priority);
  assert(task.priority < PRIORITY_LEVELS);
  gsoc_taskqueue_push(&set->queues[task.priority], task);
}

gsoc_task gsoc_taskqueue_set_pop(gsoc_taskqueue_set *set, int priority)
{
  assert(priority >= 0 && priority < PRIORITY_LEVELS);
  return gsoc_taskqueue_pop(&set->queues[priority]);
}

// Steal the best available task by trying from high to low priority

gsoc_task gsoc_taskqueue_set_steal_best(gsoc_taskqueue_set *victim_set, int priority_level)
{
  gsoc_task task = gsoc_taskqueue_take(&victim_set->queues[priority_level]);
  return task;
}
