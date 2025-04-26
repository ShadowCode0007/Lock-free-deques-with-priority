/* Circular array implemented after Chapter 16 of "The Art of Multiprocessor Programming"
   Always use from push/pop/take of taskqueue. task_circular_array itself is not thread safe. */

#ifndef _GSOC_TASK_CIRCULAR_ARRAY_H_
#define _GSOC_TASK_CIRCULAR_ARRAY_H_

#include "gsoc_task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define STATIC_BUFFER_SIZE (1 << 20) // 1MB
static char static_buffer[STATIC_BUFFER_SIZE];
static size_t static_buffer_offset = 0;

static inline void* static_alloc(size_t size) {
    if (static_buffer_offset + size > STATIC_BUFFER_SIZE) {
        fprintf(stderr, "Out of static memory\n");
        exit(1);
    }
    void* ptr = &static_buffer[static_buffer_offset];
    static_buffer_offset += size;
    return ptr;
}

typedef struct _gsoc_task_circular_array {
    gsoc_task** _array;
    unsigned long long _size;
} gsoc_task_circular_array;

static inline gsoc_task_circular_array*
gsoc_task_circular_array_new(unsigned long long capacity)
{
    gsoc_task_circular_array* this;

    this = static_alloc(sizeof(gsoc_task_circular_array));
    assert(this);

    this->_array = static_alloc(sizeof(gsoc_task*) * capacity);
    assert(this->_array);

    this->_size = capacity;

    return this;
}

static inline void
gsoc_task_circular_array_delete(gsoc_task_circular_array* this)
{
    (void)this; 
}

static inline gsoc_task*
gsoc_task_circular_array_get(gsoc_task_circular_array* this, unsigned long long index)
{
    return this->_array[index % this->_size];
}

static inline void
gsoc_task_circular_array_set(gsoc_task_circular_array* this, unsigned long long index, gsoc_task* task)
{
    this->_array[index % this->_size] = task;
}

static inline gsoc_task_circular_array*
gsoc_task_circular_array_get_double_sized_copy(gsoc_task_circular_array* old)
{
    gsoc_task_circular_array* new_arr;
    if (old->_size * 2 >= (1ULL << 31))
    {
        fprintf(stderr, "gsoc_task_circular_array cannot deal with more than 2^31 tasks.\n");
        exit(1);
    }
    new_arr = gsoc_task_circular_array_new(old->_size * 2);

    for (unsigned long long i = 0; i < old->_size; ++i) {
        new_arr->_array[i] = old->_array[i];
    }

    return new_arr;
}

static inline unsigned long long
gsoc_task_circular_array_size(gsoc_task_circular_array* this)
{
    return this->_size;
}

#endif /* _GSOC_TASK_CIRCULAR_ARRAY_H_ */

