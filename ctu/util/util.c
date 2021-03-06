#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void *ctu_malloc(size_t size) {
    return malloc(size);
}

void *ctu_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void ctu_free(void *ptr) {
    free(ptr);
}

static uint32_t hash_string(const char *str) {
    uint32_t hash = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        hash = (hash << 5) - hash + str[i];
    }
    return hash;
}

static void *entry_get(entry_t *it, const char *key) {
    if (it->id && strcmp(it->id, key) == 0) {
        return it->data;
    }

    if (it->next) {
        return entry_get(it->next, key);
    }

    return NULL;
}

map_t *new_map(size_t size) {
    map_t *map = ctu_malloc(sizeof(map_t));
    map->size = size;
    map->data = ctu_malloc(sizeof(entry_t) * size);
    
    for (size_t i = 0; i < size; i++) {
        map->data[i].id = NULL;
        map->data[i].next = NULL;
    }

    return map;
}

void *map_get(map_t *map, const char *id) {
    uint32_t hash = hash_string(id);
    return entry_get(&map->data[hash % map->size], id);
}

void map_put(map_t *map, const char *id, void *data) {
    uint32_t hash = hash_string(id);
    entry_t *entry = &map->data[hash % map->size];
    while (entry) {
        if (entry->id == NULL) {
            entry->id = id;
            entry->data = data;
            break;
        } else if (strcmp(entry->id, id) == 0) {
            break;
        } else {
            entry->next = ctu_malloc(sizeof(entry_t));
            entry->next->id = NULL;
            entry->next->next = NULL;
            entry = entry->next;
        }
    }
}
