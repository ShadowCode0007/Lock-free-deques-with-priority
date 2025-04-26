#define TEST_USE_TASK_ID

#define _GNU_SOURCE
#include "gsoc_taskqueue.h"
#include "gsoc_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <utmpx.h>
#include <unistd.h>

#ifndef TESTVAL_EXTENDS  /* It may be defined by Makefile */
#define TESTVAL_EXTENDS 1
#endif

typedef struct _worker {
  int id;
  cpu_set_t cpu;
  gsoc_taskqueue* taskqs[3]; // Priority 0 (High), 1 (Medium), 2 (Low)
  gsoc_task* tasks;
  struct _worker* workers;
  long num_workers;
  int logged_worker;
} worker;

void* parallel_push_pop_take(void* s)
{
  worker* data = (worker*)s;
  pthread_setaffinity_np(pthread_self(), sizeof(data->cpu), &data->cpu);
  pthread_t tid = pthread_self();
  printf("Creating %d tasks in pthread ID: %lu\n", (GSOC_TASKQUEUE_INIT_SIZE * TESTVAL_EXTENDS), (unsigned long)tid);

  for (int i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * TESTVAL_EXTENDS / 2; ++i) {
    int priority = i % 3;
    gsoc_taskqueue_push(data->taskqs[priority], &data->tasks[2 * i]);
    gsoc_taskqueue_push(data->taskqs[priority], &data->tasks[2 * i + 1]);

    for (int p = 0; p < 3; ++p) {
      gsoc_task* task = gsoc_taskqueue_pop(data->taskqs[p]);
      if (task) {//&& data->id == data->logged_worker)
        printf("%lld is popped by thread %lu CPU%d (priority %d)\n", task->test_id, (unsigned long)tid, sched_getcpu(), p);
        break;
      }
    }
  }

  printf("Actual test starts now ------------------------------------\n");
  printf("Current pthread ID: %lu\n", (unsigned long)tid);
  
  while (1) {
    gsoc_task* task = NULL;

    for (int p = 0; p < 3; ++p) {
      task = gsoc_taskqueue_pop(data->taskqs[p]);
      if (task) {// data->id == data->logged_worker)
        printf("%lld is popped by CPU%d (priority %d)\n", task->test_id, sched_getcpu(), p);
        break;
      }
    }

    if (!task) {
      size_t victim;
      do {
        victim = random() % data->num_workers;
      } while (victim == data->id);

      for (int p = 0; p < 3; ++p) {
        task = gsoc_taskqueue_take(data->workers[victim].taskqs[p]);
        if (task) {
          //if (data->id == data->logged_worker)
            printf("%lld is taken by CPU%d from CPU%ld (priority %d)\n",
                   task->test_id, sched_getcpu(), victim, p);
          break;
        }
      }

      if (!task) return NULL;
    }
  }
}

int main()
{
  int i;
  long num_tasks = GSOC_TASKQUEUE_INIT_SIZE * 100;
  gsoc_task *tasks = malloc(sizeof(gsoc_task) * num_tasks);

  for (i = 0; i < num_tasks; ++i)
    tasks[i].test_id = i;

  // === SINGLE-THREADED PRIORITY TEST ===
  gsoc_taskqueue* qs[3];
  for (int p = 0; p < 3; ++p)
    qs[p] = gsoc_taskqueue_new();

  // Push based on priority
  for (i = 0; i < num_tasks; ++i) {
    int priority = i % 3;
    gsoc_taskqueue_push(qs[priority], &tasks[i]);
  }

  printf("After all pushes:\n");
  for (int p = 0; p < 3; ++p) {
    printf("  priority %d: q->_top = %zu, q->_bottom = %zu, q->_taskqueue->_size = %llu\n",
           p, qs[p]->_top, qs[p]->_bottom, qs[p]->_taskqueue->_size);
  }

  // Pop in reverse order based on priority
  for (i = num_tasks - 1; i >= 0; --i) {
    int priority = i % 3;
    gsoc_task* task = gsoc_taskqueue_pop(qs[priority]);
    assert(task && task->test_id == i);
  }

  for (int p = 0; p < 3; ++p)
    assert(gsoc_taskqueue_pop(qs[p]) == NULL);

  // Push again for take
  for (i = 0; i < num_tasks; ++i) {
    int priority = i % 3;
    gsoc_taskqueue_push(qs[priority], &tasks[i]);
  }

  for (i = 0; i < num_tasks; ++i) {
    int priority = i % 3;
    gsoc_task* task = gsoc_taskqueue_take(qs[priority]);
    assert(task && task->test_id == i);
  }

  for (int p = 0; p < 3; ++p)
    assert(gsoc_taskqueue_take(qs[p]) == NULL);

  for (int p = 0; p < 3; ++p)
    gsoc_taskqueue_delete(qs[p]);

  // === MULTITHREADED PARALLEL TEST ===
  long num_cpu = sysconf(_SC_NPROCESSORS_CONF);
  cpu_set_t cpuset;
  pthread_t tids[num_cpu];
  worker workers[num_cpu];
  int logged_worker = random() % num_cpu;

  fprintf(stderr, "==========\nWith %d CPUs\n===========\n", (int)num_cpu);

  double t1 = gettimeofday_sec();
  for (i = 0; i < num_cpu; ++i) {
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    workers[i].cpu = cpuset;
    workers[i].id = i;
    workers[i].tasks = tasks;
    workers[i].workers = workers;
    workers[i].num_workers = num_cpu;
    workers[i].logged_worker = logged_worker;

    for (int p = 0; p < 3; ++p)
      workers[i].taskqs[p] = gsoc_taskqueue_new();

    pthread_create(&tids[i], NULL, parallel_push_pop_take, &workers[i]);
  }

  for (i = 0; i < num_cpu; ++i)
    pthread_join(tids[i], NULL);

  double t2 = gettimeofday_sec();
  fprintf(stderr, "%f sec elapsed for parallel_push_pop_take()\n", t2 - t1);

  for (i = 0; i < num_cpu; ++i)
    for (int p = 0; p < 3; ++p)
      gsoc_taskqueue_delete(workers[i].taskqs[p]);

  free(tasks);
  return 0;
}
