#ifndef LRU_CACHE_H
#define LRU_CACHE_H

typedef struct lru_cache_t lru_cache_t;

lru_cache_t *lru_cache_create(int capacity);
void lru_cache_put(lru_cache_t *cache, const char *key, const char *value);
char *lru_cache_get(lru_cache_t *cache, const char *key);
void lru_cache_destroy(lru_cache_t *cache);

#endif /* LRU_CACHE_H */
