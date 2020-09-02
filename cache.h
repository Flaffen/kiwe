#ifndef _WEBCACHE_H_
#define _WEBCACHE_H_
#include <time.h>

// Individual hash table entry
struct cache_entry {
    char *path;   // Endpoint path--key to the cache
    char *content_type;
    int content_length;
    void *content;
    time_t created_at;

    struct cache_entry *prev, *next; // Doubly-linked list
};

// A cache
struct cache {
    struct hashtable *index;
    struct cache_entry *head, *tail; // Doubly-linked list
    int max_size; // Maxiumum number of entries
    int cur_size; // Current number of entries
    pthread_mutex_t *cachemutex;
};

extern struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length);
extern void free_entry(struct cache_entry *entry);
extern struct cache *cache_create(int max_size, int hashsize, pthread_mutex_t *cachemutex);
extern void cache_free(struct cache *cache);
extern void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length);
extern struct cache_entry *cache_get(struct cache *cache, char *path);
extern void cache_delete(struct cache *cache, struct cache_entry *ce);
extern void cache_print(struct cache *cache);

#endif
