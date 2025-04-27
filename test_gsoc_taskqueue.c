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
  gsoc_taskqueue_set* taskqs; // Priority 0 (High), 1 (Medium), 2 (Low)
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

  for (int i = 0; i < 30; ++i) {
    int priority = i % 3;
    gsoc_task task;
    task.priority = priority;

    gsoc_taskqueue_set_push(data->taskqs,task, priority);
  }

  printf("Actual test starts now ------------------------------------\n");
  printf("Current pthread ID: %lu\n", (unsigned long)tid);
  
  int current_priority = 0;

  while (1) {
    gsoc_task task;
    
    if (current_priority > 2) break;

    task = gsoc_taskqueue_set_pop(data->taskqs, current_priority);
    
    if (task.priority == current_priority) {// data->id == data->logged_worker)
       printf("%lld is popped by CPU%d (priority %d)\n", task.test_id, sched_getcpu(), current_priority);
       break;
    }
   
    if (task.priority == -1) { 
      size_t victim;

      task.priority = -1;

      for (int i = 0; i < data->num_workers; i++) {
	  task = gsoc_taskqueue_set_steal_best(data->workers[victim].taskqs, current_priority);
          if (task.priority != -1) {
            //if (data->id == data->logged_worker)
              printf("%lld is taken by CPU%d from CPU%ld (priority %d)\n",
                   task.test_id, sched_getcpu(), victim, current_priority);
              break;
          }
      }

      if (task.priority == -1) {
      	 current_priority++;
      }
    }
  }
}

int main()
{
  int i;
  long num_tasks = GSOC_TASKQUEUE_INIT_SIZE * 150;
  gsoc_task tasks[num_tasks + 1];

  for (i = 0; i < num_tasks; ++i)
    tasks[i].test_id = i;

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
    workers[i].num_workers = num_cpu;
    workers[i].logged_worker = logged_worker;

    gsoc_taskqueue_set_new();

  }

  for(int i = 0; i < num_cpu; ++i) {
    workers[i].workers = workers;
    pthread_create(&tids[i], NULL, parallel_push_pop_take, &workers[i]);
  }

  for (i = 0; i < num_cpu; ++i)
    pthread_join(tids[i], NULL);

  double t2 = gettimeofday_sec();
  fprintf(stderr, "%f sec elapsed for parallel_push_pop_take()\n", t2 - t1);

  return 0;
}

