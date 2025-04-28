#define _GNU_SOURCE
#include "../taskqueue.h"
#include "../time.h"
#include <pthread.h>
#include <utmpx.h>
#include <unistd.h>
#include <math.h>

#define num_of_tasks 2880

typedef struct _worker
{
  int id;
  cpu_set_t cpu;
  taskqueue_set *taskqs; // Priority 0 (High), 1 (Medium), 2 (Low)
  struct _worker *workers;
  long num_workers;
} worker;

int nthFibonacci(int n){
    if (n <= 1){
        return n;
    }
    return nthFibonacci(n - 1) + nthFibonacci(n - 2);
}

void execute_task(task task)
{
  printf("(CPU%d) Executing task %lld (priority %d)\n", sched_getcpu(), task.test_id, task.priority);
  nthFibonacci(task.input);
}

void *parallel_push_pop_take(void *s)
{
  worker *data = (worker *)s;
  pthread_setaffinity_np(pthread_self(), sizeof(data->cpu), &data->cpu);

  int current_priority = 0;
  double start_time = gettimeofday_sec();

  int steal_count_per_priority = 0;
  int steal_count_total = 0;

  while (1)
  {
    task task;

    if (current_priority > 2) {
	 fprintf(stderr, "\nTasks stolen by CPU%d in total is %d \n", sched_getcpu(), steal_count_total);
         break;
    }

    task = taskqueue_set_pop(data->taskqs, current_priority);

    if (task.priority == current_priority)
    {
      execute_task(task);
      continue;
    }

    int found = 0;

    if (task.priority == -1)
    {
       int check_cpus = (int)sqrt((double)data->num_workers);

       for (int i = 0; i < check_cpus; i++) {
         task = taskqueue_set_steal_best(data->workers[i].taskqs, current_priority, i);

         if (task.priority != -1)
         {
           printf("%lld is taken by CPU%d from CPU%d (priority %d)\n",
                 task.test_id, sched_getcpu(), i, current_priority);
           execute_task(task);
	   steal_count_total++;
	   steal_count_per_priority++;
	   found = 1;
           break;
         }

       }

      if (found == 0)
      {
         current_priority++;
	 double end_time = gettimeofday_sec();
  	 fprintf(stderr, "%f sec taken for priority %d tasks in CPU%d\n", end_time - start_time, current_priority - 1, sched_getcpu());
	 start_time = end_time;

	 fprintf(stderr, "\nTasks stolen by CPU %d of priority %d is %d \n", sched_getcpu(), current_priority - 1, steal_count_per_priority);
	 steal_count_per_priority = 0;
         printf("(CPU%d) current_priority has been incremented to %d\n", sched_getcpu(), current_priority);
      }
    }

  }

  printf("~~~~~ CPU%d EXITED HERE ~~~~~\n", sched_getcpu());
  return NULL;
}

int main()
{
  int i;

  long num_cpu = sysconf(_SC_NPROCESSORS_CONF);
  cpu_set_t cpuset;
  pthread_t tids[num_cpu];
  worker workers[num_cpu];

  fprintf(stderr, "==========\nWith %d CPUs\n===========\n", (int)num_cpu);

  double t1 = gettimeofday_sec();
  for (i = 0; i < num_cpu; ++i)
  {
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    workers[i].cpu = cpuset;
    workers[i].id = i * 100;
    workers[i].num_workers = num_cpu;
    workers[i].taskqs = taskqueue_set_new();
  }

  int tasks_per_processor = num_of_tasks / num_cpu;

  for (int i = 0; i < num_cpu; ++i)
  {
    for (int j = 0; j < tasks_per_processor; ++j)
    {
      int priority = (j + 1) % 3;
 
      task task;
      task.priority = priority;
     
      if (priority == 0)
         task.input = 20 + (rand() % (40 - 20 + 1)); // random between 20 and 40 inclusive
      else if (priority == 1)
         task.input = 10 + (rand() % (20 - 10 + 1)); // random between 10 and 20 inclusive
      else if (priority == 2)
          task.input = 0 + (rand() % (10 - 0 + 1));


      task.test_id = workers[i].id + j + 1;

      printf("(CPU%d) Creating task %lld with priority %u\n", sched_getcpu(), task.test_id, task.priority);
      taskqueue_set_push(workers[i].taskqs, task);
    }
  }

  for (int i = 0; i < num_cpu; ++i)
  {
    workers[i].workers = workers;

    pthread_create(&tids[i], NULL, parallel_push_pop_take, &workers[i]);
  }

  for (i = 0; i < num_cpu; ++i)
    pthread_join(tids[i], NULL);

  double t2 = gettimeofday_sec();
  fprintf(stderr, "\n%f sec elapsed for all the tasks in total()\n", t2 - t1);

  return 0;
}
