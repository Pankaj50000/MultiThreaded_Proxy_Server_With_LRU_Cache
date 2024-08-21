
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

typedef struct {
    void (*function)(void *arg);
    void *arg;
} task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    task_t *task_queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
} thread_pool_t;

thread_pool_t *thread_pool_create(int thread_count, int queue_size);
int thread_pool_add(thread_pool_t *pool, void (*function)(void *), void *arg);
void thread_pool_destroy(thread_pool_t *pool);

#endif /* THREAD_POOL_H */
