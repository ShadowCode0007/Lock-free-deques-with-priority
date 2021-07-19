# Lockless Work stealing Deque written in C.

- There are a number of tasks
- Tasks are the same number of task queues as CPUs
- A `worker' has a task queue
- A worker pops/pushes tasks from its own task queue
- A worker takes tasks from other workers
- `Pop'/`push' accesses the bottom of task queues
- `Take ' accesses the top of task queues

# Features

- Task queue is implemented as `unbound circular array',
    which doesn't have start and end points and expands when the size
    is less than the number of tasks.
    
- There are already some implementation of Lock-free Work-stealing Deque.
    But one written in C is not so common.
    Unlike some languages like Java, the support of `volatile' variable is
    limited in C. This implementation covers it by using memory barriers.
    
- This uses compare-and-swap (CAS) instruction instead of mutex lock.

# Compile

Compile the deque source.

```bash
make
```

# Test

To test the unit test of deque, just type

```bash
make TEST_gsoc_taskqueue
```

You can see the number of CPUs used in test and calculation time by

```bash
./test_gsoc_taskqueue > /dev/null
```

If you also want to test circular array, type

```bash
make TEST_gsoc_task_circular_array    
```
   
Note that it will fail if you use 32-bit CPUs.
Or if you want to test all of them,

```bash
make test
```

you can clean the directory.

```bash
make clean
```

# Reference

1. [The Art of Multiprocessor Programming.  Maurice Herlihy, Nir Shavit / Morgan Kaufmann](https://www.elsevier.com/books/the-art-of-multiprocessor-programming/herlihy/978-0-12-415950-1)
2. [Dynamic Circular Work-Stealing Deque. David Chase, Yossi Lev / Sun Microsystems](https://dl.acm.org/doi/10.1145/1073970.1073974)
3. [laysakura's blog](http://d.hatena.ne.jp/laysakura)
4. [laysakura's project in Google Summer of Code 2011](http://www.google-melange.com/gsoc/proposal/review/google/gsoc2011/laysakura/1)
5. [laysakura's twitter](http://twitter.com/laysakura)
