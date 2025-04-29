
// Scenario 4:
// Every CPU has unequal number of tasks

#include "peer.h"

#define num_of_tasks 150

void execute_task(task task)
{
  printf("(CPU%d) Executing task %lld (priority %d)\n", sched_getcpu(), task.test_id, task.priority);
  usleep(task.input * 1000);
}

void *process_task(void *s)
{
  peer *data = (peer *)s;
  pthread_setaffinity_np(pthread_self(), sizeof(data->cpu), &data->cpu);

  int current_priority = 0;
  double start_time = gettimeofday_sec();

  int steal_count_per_priority = 0;
  int steal_count_total = 0;

  while (1)
  {
    task task;

    if (current_priority > 2)
    {
      fprintf(stderr, "\nTasks stolen by CPU%d in total is %d \n", sched_getcpu(), steal_count_total);
      break;
    }

    task = set_pop(data->taskqs, current_priority);

    if (task.priority == current_priority)
    {
      execute_task(task);
      continue;
    }

    int found = 0;

    if (task.priority == -1)
    {
      for (int i = 0; i < data->num_peers; i++)
      {
        task = set_steal_best(data->peers[i].taskqs, current_priority, i);

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
  peer peers[num_cpu];

  fprintf(stderr, "==========\nWith %d CPUs\n===========\n", (int)num_cpu);

  double t1 = gettimeofday_sec();
  for (i = 0; i < num_cpu; ++i)
  {
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    peers[i].cpu = cpuset;
    peers[i].id = i * 100;
    peers[i].num_peers = num_cpu;
    peers[i].taskqs = set_new();
  }

  int num_tasks = 0;

  for (int i = 0; i < num_cpu; ++i)
  {
    if (i == 0)
      num_tasks = 50;
    if (i == 1)
      num_tasks = 75;
    if (i == 2)
      num_tasks = 100;
    if (i == 3)
      num_tasks = 150;
    if (i > 3)
      num_tasks = 200;

    for (int j = 0; j < num_tasks; ++j)
    {
      int priority = (j + 1) % 3;

      if (i == 0 && j < 20)
        priority = 0;

      if (i == 0 && j >= 20 && j < 30)
        priority = 1;

      if (i == 0 && j >= 30)
        priority = 2;

      if (i == 1 && j < 50)
        priority = 0;

      if (i == 1 && j >= 50 && j < 60)
        priority = 1;

      if (i == 1 && j >= 60)
        priority = 2;

      if (i == 2 && j < 50)
        priority = 0;

      if (i == 2 && j >= 50 && j < 70)
        priority = 1;

      if (i == 2 && j >= 70)
        priority = 2;

      if (i == 3 && j < 120)
        priority = 0;

      if (i == 3 && j >= 120 && j < 130)
        priority = 1;

      if (i == 3 && j >= 130)
        priority = 2;

      if (i > 3)
      {
        if (j <= 100)
        {
          priority = 0;
        }
        if (j > 100 && j <= 150)
        {
          priority = 1;
        }
        if (j > 150)
        {
          priority = 2;
        }
      }

      task task;
      task.priority = priority;

      if (i == 0)
        task.input = 90;
      if (i == 1)
        task.input = 10;
      if (i == 2)
        task.input = 60;
      if (i == 3)
        task.input = 20;
      if (i > 3)
        task.input = 20 + (rand() % (40 - 20 + 1));

      task.test_id = peers[i].id + j + 1;

      printf("(CPU%d) Creating task %lld with priority %u\n", sched_getcpu(), task.test_id, task.priority);
      set_push(peers[i].taskqs, task);
    }
  }

  for (int i = 0; i < num_cpu; ++i)
  {
    peers[i].peers = peers;
    pthread_create(&tids[i], NULL, process_task, &peers[i]);
  }

  for (i = 0; i < num_cpu; ++i)
  {
    pthread_join(tids[i], NULL);
  }

  double t2 = gettimeofday_sec();
  fprintf(stderr, "\n%f sec elapsed for all the tasks in total()\n", t2 - t1);

  return 0;
}
