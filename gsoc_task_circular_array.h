/* Circular array implemented after Chapter 16 of "The Art of Multiprocessor Programming"
   Always use from push/pop/take of taskqueue. task_circular_array itself is not thread safe. */

#ifndef _GSOC_TASK_CIRCULAR_ARRAY_H_
#define _GSOC_TASK_CIRCULAR_ARRAY_H_

#include "gsoc_task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define STATIC_BUFFER_SIZE (1 << 28) // 1MB

typedef struct _gsoc_task_circular_array {
    gsoc_task _array[50];
    unsigned long long _size;
} gsoc_task_circular_array;

static inline gsoc_task
gsoc_task_circular_array_get(gsoc_task_circular_array* this, unsigned long long index)
{
    return this->_array[index % this->_size];
}

static inline void
gsoc_task_circular_array_set(gsoc_task_circular_array* this, unsigned long long index, gsoc_task task)
{
    this->_array[index % this->_size] = task;
}

static inline unsigned long long
gsoc_task_circular_array_size(gsoc_task_circular_array* this)
{
    return this->_size;
}

#endif /* _GSOC_TASK_CIRCULAR_ARRAY_H_ */

