#include "lru_cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

typedef struct lru_node_t {
    char *key;
    char *value;
    struct lru_node_t *prev;
    struct lru_node_t *next;
} lru_node_t;

struct lru_cache_t {
    int capacity;
    int size;
    lru_node_t *head;
    lru_node_t *tail;
    lru_node_t **table;
};

// Hash function to map keys to indices
static int hash(const char *key, int capacity) {
    int hash = 0;
    while (*key) {
        hash = (hash << 5) - hash + *key++;
    }
    return hash % capacity;
}

// Helper function to remove a node
static void remove_node(lru_cache_t *cache, lru_node_t *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        cache->tail = node->prev;
    }
}

// Helper function to add a node to the head
static void add_node_to_head(lru_cache_t *cache, lru_node_t *node) {
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (!cache->tail) {
        cache->tail = node;
    }
}

lru_cache_t *lru_cache_create(int capacity) {
    lru_cache_t *cache = (lru_cache_t *)malloc(sizeof(lru_cache_t));
    if (!cache) return NULL;

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    cache->table = (lru_node_t **)calloc(capacity, sizeof(lru_node_t *));
    if (!cache->table) {
        free(cache);
        return NULL;
    }
    return cache;
}

void lru_cache_put(lru_cache_t *cache, const char *key, const char *value) {
    int index = hash(key, cache->capacity);
    lru_node_t *node = cache->table[index];

    // If the key already exists, update the value and move the node to the head
    while (node) {
        if (strcmp(node->key, key) == 0) {
            free(node->value);
            node->value = strdup(value);
            remove_node(cache, node);
            add_node_to_head(cache, node);
            return;
        }
        node = node->next;
    }

    // If the cache is full, remove the LRU item
    if (cache->size == cache->capacity) {
        lru_node_t *lru = cache->tail;
        remove_node(cache, lru);
        index = hash(lru->key, cache->capacity);
        lru_node_t **bucket = &cache->table[index];

        // Remove the node from the hash table
        if (*bucket == lru) {
            *bucket = lru->next;
        } else {
            lru_node_t *prev = *bucket;
            while (prev->next != lru) {
                prev = prev->next;
            }
            prev->next = lru->next;
        }

        free(lru->key);
        free(lru->value);
        free(lru);
        cache->size--;
    }

    // Create a new node and add it to the cache
    node = (lru_node_t *)malloc(sizeof(lru_node_t));
    node->key = strdup(key);
    node->value = strdup(value);
    node->next = cache->table[index];
    cache->table[index] = node;

    add_node_to_head(cache, node);
    cache->size++;
}
char *lru_cache_get(lru_cache_t *cache, const char *key) {
    int index = hash(key, cache->capacity);
    lru_node_t *node = cache->table[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            remove_node(cache, node);
            add_node_to_head(cache, node);
            printf("Cache hit for key: %s\n", key);
            return node->value;
        }
        node = node->next;
    }

    printf("Cache miss for key: %s\n", key);
    return NULL;  // Cache miss
}


void lru_cache_destroy(lru_cache_t *cache) {
    for (int i = 0; i < cache->capacity; i++) {
        lru_node_t *node = cache->table[i];
        while (node) {
            lru_node_t *next = node->next;
            free(node->key);
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(cache->table);
    free(cache);
}
