# Lock-Free Priority-Aware Work Stealing

This project implements a **lock-free, priority-aware work-stealing scheduler** for parallel task execution on multicore systems. Each processor maintains independent lock-free deques for each priority level, enabling efficient, decentralized scheduling of tasks while minimizing priority inversion and synchronization overhead.

## Features

- **Lock-Free Deques:** Each processor maintains a set of lock-free double-ended queues (deques), one per priority level, implemented using atomic compare-and-swap (CAS) operations and explicit memory barriers (`__sync_synchronize()`).
- **Priority-Aware Scheduling:** Tasks are classified into priority levels. Processors always prefer to execute or steal higher-priority tasks.
- **Decentralized Stealing:** Idle processors attempt to steal tasks from the same priority level in other processors’ deques, reducing contention and balancing load.

## How It Works

1. **Task Queues:**  
   Each processor/core maintains multiple lock-free deques, one for each priority level (e.g., high, medium, low).
2. **Task Execution:**  
   Processors work on the highest available priority, popping tasks from their local deque.
3. **Work Stealing:**  
   If a processor’s deque is empty at the current priority, it attempts to steal tasks from other processors at the same priority.
4. **Priority Progression:**  
   Only after failing to find work at a given priority (locally and via stealing) does a processor move to the next lower priority.
5. **Atomicity and Memory Ordering:**  
   All concurrent operations use atomic CAS and explicit memory barriers (`__sync_synchronize()`) to ensure correctness and visibility across cores.

## Directory Structure

```
├── taskqueue.c
├── taskqueue.h
├── time.h
├── Makefile
├── tests/
│   ├── test_fib1.c
│   └── ...
├── build/ # Created automatically
│   ├── test_fib1
│   └── ...
└── logs/ # Created automatically
    ├── test_taskqueue.log
    ├── test_taskqueue.debug.log
```

## How to run

The project uses a Makefile to automate building and testing:

- `make all`  
  Compiles and runs all test files in the `tests/` directory. The output of each test is saved to a corresponding log file in the `logs/` directory.

- `make clean`  
  Removes all build artifacts and logs (i.e., cleans the `build/` and `logs/` directories).

**Tip:**  
You can always inspect the logs in the `logs/` directory for detailed output or debugging information from your test runs.

## References

- Shams Imam, Vivek Sarkar. ["Load Balancing Prioritized Tasks via Work-Stealing." *Euro-Par 2015*](https://link.springer.com/chapter/10.1007/978-3-662-48096-0_38)
- Aleksandar Prokopec et al. ["Efficient lock-free work-stealing iterators for data-parallel collections." *PDP 2015*](https://infoscience.epfl.ch/bitstreams/19aec310-0e30-41e0-928b-40bc6e1aa243/download)

## Acknowledgments

This project was developed as part of coursework at Carnegie Mellon University. Special thanks to Prof. Todd C. Mowry and Prof. Brian P. Railing for their guidance and support.

