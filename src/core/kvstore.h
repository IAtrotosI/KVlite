#ifndef KVSTORE_H
#define KVSTORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 8

typedef struct 
{
    char* key;
    char* value;
} kv_pair_t;

typedef struct 
{
    kv_pair_t* pairs;
    size_t count;
    size_t capacity;
} kv_store_t;

kv_store_t* kv_store_create(void);
void kv_store_destroy(kv_store_t* store);

int kv_store_set(kv_store_t* store, const char* key, const char* value);
char* kv_store_get(kv_store_t* store, const char* key);
int kv_store_exists(kv_store_t* store, const char* key);
int kv_store_delete(kv_store_t* store, const char* key);
int kv_store_save_to_file(kv_store_t* store, const char* filename);
int kv_store_load_from_file(kv_store_t* store, const char* filename);
int kv_store_keys(kv_store_t* store, char* buffer, size_t buffer_size);
void kv_store_flushdb(kv_store_t* store);
int kv_store_info(kv_store_t* store, char* buffer, size_t buffer_size, time_t start_time);
int kv_store_incr(kv_store_t* store, const char* key, long* result);
int kv_store_decr(kv_store_t* store, const char* key, long* result);
int kv_store_append(kv_store_t* store, const char* key, const char* value);
int kv_store_rename(kv_store_t* store, const char* old_key, const char* new_key);
char* kv_store_getset(kv_store_t* store, const char* key, const char* new_value);
const char* kv_store_type(kv_store_t* store, const char* key);

#endif