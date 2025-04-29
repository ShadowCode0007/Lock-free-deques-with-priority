#define _GNU_SOURCE
#include "../taskqueue.h"
#include "../time.h"
#include <pthread.h>
#include <utmpx.h>
#include <unistd.h>
#include <math.h>

typedef struct peer
{
  int id;
  cpu_set_t cpu;
  long num_peers;
  taskqueue_set *taskqs;
  struct peer *peers;
} peer;