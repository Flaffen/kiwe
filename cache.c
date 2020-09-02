#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "hashtable.h"
#include "cache.h"

/**
 * Allocate a cache entry
 */
struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)
{
	struct cache_entry *ce;
	char *patha, *content_typea;
	patha = malloc((strlen(path) + 1) * sizeof(char));
	content_typea = malloc((strlen(content_type) + 1) * sizeof(char));

	strcpy(patha, path);
	strcpy(content_typea, content_type);

	ce = malloc(sizeof(*ce));

	ce->path = patha;
	ce->content_type = content_typea;
	ce->content_length = content_length;
	ce->content = content;
	ce->created_at = time(NULL);

	ce->prev = ce->next = NULL;

	return ce;
}

/**
 * Deallocate a cache entry
 */
void free_entry(struct cache_entry *entry)
{
	free(entry->path);
	free(entry->content_type);
	free(entry->content);
	free(entry);
}

/**
 * Insert a cache entry at the head of the linked list
 */
void dllist_insert_head(struct cache *cache, struct cache_entry *ce)
{
	if (cache->head == NULL) {
		cache->head = cache->tail = ce;
	} else {
		cache->head->prev = ce;
		ce->next = cache->head;
		cache->head = ce;
		ce->prev = NULL;
	}
}

/**
 * Move a cache entry to the head of the list
 */
void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)
{
	if (ce != cache->head) {
		if (ce == cache->tail) {
			// We're the tail
			cache->tail = ce->prev;
			cache->tail->next = NULL;

		} else {
			// We're neither the head nor the tail
			ce->prev->next = ce->next;
			ce->next->prev = ce->prev;
		}

		ce->next = cache->head;
		cache->head->prev = ce;
		ce->prev = NULL;
		cache->head = ce;
	}
}

/**
 * Removes the tail from the list and returns it
 * 
 * NOTE: does not deallocate the tail
 */
struct cache_entry *dllist_remove_tail(struct cache *cache)
{
	struct cache_entry *oldtail = cache->tail;

	cache->tail = oldtail->prev;
	cache->tail->next = NULL;

	cache->cur_size--;

	return oldtail;
}

/**
 * Create a new cache
 * 
 * max_size: maximum number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache *cache_create(int max_size, int hashsize, pthread_mutex_t *cachemutex)
{
	struct cache *newcache = malloc(sizeof(*newcache));
	struct hashtable *newht = hashtable_create(hashsize, NULL);

	newcache->index = newht;
	newcache->cur_size = 0;
	newcache->max_size = max_size;
	newcache->cachemutex = cachemutex;

	return newcache;
}

void cache_free(struct cache *cache)
{
	struct cache_entry *cur_entry = cache->head;

	hashtable_destroy(cache->index);

	while (cur_entry != NULL) {
		struct cache_entry *next_entry = cur_entry->next;

		free_entry(cur_entry);

		cur_entry = next_entry;
	}

	free(cache);
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)
{
	struct cache_entry *ce = alloc_entry(path, content_type, content, content_length);

	hashtable_put(cache->index, path, ce);
	dllist_insert_head(cache, ce);
	cache->cur_size++;

	if (cache->cur_size > cache->max_size) {
		struct cache_entry *saved = dllist_remove_tail(cache);
		hashtable_delete(cache->index, saved->path);
		free_entry(saved);
	}
}

/**
 * Retrieve an entry from the cache
 */
struct cache_entry *cache_get(struct cache *cache, char *path)
{
	struct cache_entry *ce = hashtable_get(cache->index, path);

	if (ce == NULL) {
		return NULL;
	} else {
		dllist_move_to_head(cache, ce);
		return ce;
	}
}

void cache_delete(struct cache *cache, struct cache_entry *ce)
{
	struct cache_entry *cur = cache->head;

	if (cur == NULL) {
		return;
	}

	hashtable_delete(cache->index, ce->path);
	cache->cur_size--;

	if (cur->prev == NULL && cur->next == NULL) {
		free_entry(cur);
		cache->head = cache->tail = NULL;
		return;
	}

	if (cur->prev == NULL) {
		cur->next->prev = NULL;
		cache->head = cur->next;
		free_entry(cur);

		return;
	}

	if (cur->next == NULL) {
		cur->prev->next = NULL;
		cache->tail = cur->prev;
		free_entry(cur);

		return;
	}

	while (cur != ce)
		cur = cur->next;

	cur->prev->next = cur->next;
	cur->prev = cur->prev;
	free_entry(cur);
}

void cache_print(struct cache *cache)
{
	struct cache_entry *ce = cache->head;
	size_t totalsize = 0;

	printf("{\n");
	int i = 1;
	if (ce == NULL) {
		printf("\tNULL\n");
	} else {
		while (ce != NULL) {
			printf("\t%d: %s (%dK)\n", i, ce->path, ce->content_length / 1024);
			totalsize += ce->content_length;
			ce = ce->next;
			i++;
		}
	}
	printf("\tTOTAL: %ldK or %ldM\n", totalsize / 1024, totalsize / 1024 / 1024);
	printf("}\n");
}
