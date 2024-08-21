#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

static void *thread_pool_worker(void *arg);

thread_pool_t *thread_pool_create(int thread_count, int queue_size) {
    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (pool == NULL) {
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = 0;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->task_queue = (task_t *)malloc(sizeof(task_t) * queue_size);

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0 ||
        pool->threads == NULL || pool->task_queue == NULL) {
        thread_pool_destroy(pool);
        return NULL;
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, thread_pool_worker, (void*)pool) != 0) {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

int thread_pool_add(thread_pool_t *pool, void (*function)(void *), void *arg) {
    int next;

    pthread_mutex_lock(&(pool->lock));

    next = (pool->tail + 1) % pool->queue_size;

    if (pool->count == pool->queue_size) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    pool->task_queue[pool->tail].function = function;
    pool->task_queue[pool->tail].arg = arg;
    pool->tail = next;
    pool->count += 1;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool->threads);
    free(pool->task_queue);
    free(pool);
}

static void *thread_pool_worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->count == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        task_t task = pool->task_queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));

        (*(task.function))(task.arg);
    }

    pthread_exit(NULL);
    return NULL;
}
